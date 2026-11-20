#!/usr/bin/env python3
"""slovingo_push.py -- sends Slovingo's push notifications (run every minute).

Server side of the optional notifications (docs/PocketBase.md). Reads, with a
PocketBase superuser account, the `kv` records written by notify.js
("notify.<course>.<device>", app = "slovingo") and the learner's progress
("backup.<course>"), then sends through Web Push:

  - training   at each training time (on the chosen weekdays): a 5-question review;
  - reminder   at the chosen time when no sheet was opened for N days
               (then at most once every N days);
  - streak     at the chosen time when the streak is running (a sheet
               yesterday) but nothing was done today;
  - test       when the learner pressed "Send a test".

A notification missed by up to 10 minutes (server busy, timer late) is still
sent; what was sent is kept in a small JSON state file so nothing goes twice.
Subscriptions the push service rejects (404/410) are cleared on the server.

Usage:
    slovingo_push.py genkeys --out /etc/slovingo-push/vapid.pem   # once: prints the public key
    slovingo_push.py run [--dry-run]                              # every minute (systemd timer)

Configuration (environment, e.g. an EnvironmentFile for systemd):
    PB_URL               https://store.example.org
    PB_ADMIN_EMAIL       superuser email
    PB_ADMIN_PASSWORD    superuser password
    VAPID_PRIVATE_KEY    path of the PEM written by genkeys
    VAPID_SUBJECT        mailto:you@example.org (contact for the push services)
    STATE_FILE           default /var/lib/slovingo-push/state.json

Requires: pip install pywebpush (pulls py-vapid, cryptography, requests).
"""

from __future__ import annotations

import argparse
import base64
import json
import os
import sys
import urllib.parse
import urllib.request
from datetime import date, datetime, timedelta, timezone
from pathlib import Path
from zoneinfo import ZoneInfo

APP = "slovingo"
WINDOW = timedelta(minutes=10)   # late by up to this much: still sent

TEXTS = {
    "fr": {
        "training": "C'est l'heure de l'entraînement : 5 questions ?",
        "reminder": "Ça fait {days} jours… Une petite fiche aujourd'hui ?",
        "streak": "Ta série de {streak} jour(s) t'attend : une fiche avant ce soir ?",
        "test": "Les notifications fonctionnent 🎉",
    },
    "en": {
        "training": "Training time: 5 questions?",
        "reminder": "It's been {days} days… A quick sheet today?",
        "streak": "Your {streak}-day streak is waiting: one sheet before tonight?",
        "test": "Notifications work 🎉",
    },
    "sk": {
        "training": "Čas na tréning: 5 otázok?",
        "reminder": "Už {days} dní… Jedna karta dnes?",
        "streak": "Tvoja séria {streak} dní čaká: jedna karta ešte dnes?",
        "test": "Upozornenia fungujú 🎉",
    },
    "de": {
        "training": "Trainingszeit: 5 Fragen?",
        "reminder": "Schon {days} Tage… Heute eine kleine Karte?",
        "streak": "Deine Serie von {streak} Tagen wartet: eine Karte bis heute Abend?",
        "test": "Benachrichtigungen funktionieren 🎉",
    },
}


# ----------------------------------------------------------------------------
# Decisions (pure, unit-tested)
# ----------------------------------------------------------------------------

def _at(now_local: datetime, hhmm: str) -> datetime | None:
    try:
        h, m = (int(x) for x in str(hhmm).split(":")[:2])
        return now_local.replace(hour=h, minute=m, second=0, microsecond=0)
    except (ValueError, TypeError):
        return None


def _in_window(now_local: datetime, hhmm: str) -> bool:
    at = _at(now_local, hhmm)
    return at is not None and at <= now_local < at + WINDOW


def streak_length(visits: set[str], today_utc: date) -> int:
    """Consecutive visit days ending yesterday (UTC dates, as the app stores them)."""
    n, day = 0, today_utc - timedelta(days=1)
    while day.isoformat() in visits:
        n += 1
        day -= timedelta(days=1)
    return n


def due(cfg: dict, visits: set[str], now_utc: datetime, state: dict) -> list[tuple[str, dict, dict]]:
    """Notifications to send now for one record: [(kind, format vars, state updates)].

    cfg: the record value written by notify.js; visits: the "YYYY-MM-DD" visit
    days (UTC) of that course; state: what was already sent for this record.
    """
    try:
        tz = ZoneInfo(cfg.get("tz") or "UTC")
    except Exception:
        tz = timezone.utc
    now = now_utc.astimezone(tz)
    today = now.date().isoformat()
    today_utc = now_utc.date()
    out: list[tuple[str, dict, dict]] = []

    test_at = cfg.get("testAt")
    if isinstance(test_at, (int, float)) and test_at > state.get("test", 0):
        out.append(("test", {}, {"test": test_at}))

    for slot in cfg.get("training") or []:
        if not isinstance(slot, dict) or now.weekday() not in (slot.get("days") or []):
            continue
        key = f"train:{slot.get('time')}"
        if _in_window(now, slot.get("time")) and state.get(key) != today:
            out.append(("training", {}, {key: today}))

    last = max(visits) if visits else None
    did_today = today_utc.isoformat() in visits

    rem = cfg.get("reminder") or {}
    if rem.get("on") and last and _in_window(now, rem.get("time")):
        days = max(1, int(rem.get("days") or 3))
        idle = (today_utc - date.fromisoformat(last)).days
        sent = state.get("reminder")
        if idle >= days and (not sent or (now.date() - date.fromisoformat(sent)).days >= days):
            out.append(("reminder", {"days": idle}, {"reminder": today}))

    st = cfg.get("streak") or {}
    if st.get("on") and not did_today and _in_window(now, st.get("time")) and state.get("streak") != today:
        n = streak_length(visits, today_utc)
        if n >= 1:
            out.append(("streak", {"streak": n}, {"streak": today}))
    return out


def payload(kind: str, cfg: dict, fmt: dict) -> dict:
    texts = TEXTS.get(cfg.get("lang"), TEXTS["en"])
    course = cfg.get("course") or {}
    url = course.get("url") or "/"
    if kind == "training":
        url = url.split("#")[0] + "#/exercises/review?n=5"
    return {"title": course.get("title") or "Slovingo", "body": texts[kind].format(**fmt), "url": url, "tag": f"slovingo-{kind}"}


# ----------------------------------------------------------------------------
# PocketBase
# ----------------------------------------------------------------------------

class PocketBase:
    def __init__(self, url: str, email: str, password: str):
        self.url = url.rstrip("/")
        self.token = self._call("POST", "/api/collections/_superusers/auth-with-password",
                                {"identity": email, "password": password})["token"]

    def _call(self, method: str, path: str, body: dict | None = None) -> dict:
        req = urllib.request.Request(self.url + path, method=method,
                                     data=json.dumps(body).encode() if body is not None else None)
        req.add_header("Content-Type", "application/json")
        if getattr(self, "token", None):
            req.add_header("Authorization", self.token)
        with urllib.request.urlopen(req, timeout=20) as res:
            return json.loads(res.read() or b"{}")

    def records(self, flt: str) -> list[dict]:
        items, page = [], 1
        while True:
            q = urllib.parse.urlencode({"filter": flt, "perPage": 200, "page": page})
            r = self._call("GET", f"/api/collections/kv/records?{q}")
            items += r.get("items", [])
            if page >= r.get("totalPages", 1):
                return items
            page += 1

    def clear(self, rec_id: str) -> None:
        self._call("PATCH", f"/api/collections/kv/records/{rec_id}", {"value": None})


def _quote(s: str) -> str:
    return s.replace("\\", "\\\\").replace("'", "\\'")


def visits_of(backup: dict | None) -> set[str]:
    days = ((backup or {}).get("progress") or {}).get("visitDays") or []
    return {d for d in days if isinstance(d, str)}


# ----------------------------------------------------------------------------
# Commands
# ----------------------------------------------------------------------------

def cmd_run(args: argparse.Namespace) -> int:
    env = os.environ
    state_path = Path(env.get("STATE_FILE", "/var/lib/slovingo-push/state.json"))
    try:
        state = json.loads(state_path.read_text())
    except (OSError, ValueError):
        state = {}

    pb = PocketBase(env["PB_URL"], env["PB_ADMIN_EMAIL"], env["PB_ADMIN_PASSWORD"])
    now = datetime.now(timezone.utc)
    seen, backups = set(), {}
    sent = 0
    for rec in pb.records(f"app='{APP}' && key~'notify.'"):
        cfg = rec.get("value")
        if not isinstance(cfg, dict) or not cfg.get("subscription"):
            continue
        seen.add(rec["id"])
        prefix = (cfg.get("course") or {}).get("prefix", "")
        bkey = (rec["owner"], prefix)
        if bkey not in backups:
            found = pb.records(f"app='{APP}' && owner='{_quote(rec['owner'])}' && key='backup.{_quote(prefix)}'")
            backups[bkey] = found[0].get("value") if found else None
        rstate = state.setdefault(rec["id"], {})
        for kind, fmt, update in due(cfg, visits_of(backups[bkey]), now, rstate):
            data = payload(kind, cfg, fmt)
            if args.dry_run:
                print(f"[dry-run] {rec['key']}: {kind}: {data['body']}")
                continue
            try:
                send(cfg["subscription"], data)
                sent += 1
                rstate.update(update)
            except Gone:
                print(f"{rec['key']}: subscription expired, cleared")
                pb.clear(rec["id"])
                break
            except Exception as exc:   # one bad record must not stop the others
                print(f"{rec['key']}: {kind}: {exc}", file=sys.stderr)

    if args.dry_run:   # nothing was sent: keep the state as it was
        return 0
    for rec_id in list(state):   # forget removed records
        if rec_id not in seen:
            del state[rec_id]
    state_path.parent.mkdir(parents=True, exist_ok=True)
    tmp = state_path.with_suffix(".tmp")
    tmp.write_text(json.dumps(state, indent=1))
    tmp.replace(state_path)
    if sent:
        print(f"{sent} notification(s) sent")
    return 0


class Gone(Exception):
    """The push service no longer knows this subscription."""


def send(subscription: dict, data: dict) -> None:
    from pywebpush import WebPushException, webpush
    try:
        webpush(subscription_info=subscription, data=json.dumps(data),
                vapid_private_key=os.environ["VAPID_PRIVATE_KEY"],
                vapid_claims={"sub": os.environ.get("VAPID_SUBJECT", "mailto:admin@example.org")},
                ttl=3600)
    except WebPushException as exc:
        if exc.response is not None and exc.response.status_code in (404, 410):
            raise Gone() from exc
        raise


def cmd_genkeys(args: argparse.Namespace) -> int:
    from cryptography.hazmat.primitives import serialization
    from cryptography.hazmat.primitives.asymmetric import ec
    out = Path(args.out)
    if out.exists():
        print(f"{out} exists: not overwritten (the browsers' subscriptions depend on it)", file=sys.stderr)
        return 1
    key = ec.generate_private_key(ec.SECP256R1())
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_bytes(key.private_bytes(serialization.Encoding.PEM, serialization.PrivateFormat.PKCS8,
                                      serialization.NoEncryption()))
    out.chmod(0o600)
    raw = key.public_key().public_bytes(serialization.Encoding.X962, serialization.PublicFormat.UncompressedPoint)
    print(base64.urlsafe_b64encode(raw).rstrip(b"=").decode())
    print(f"Private key: {out} -- public key above: build with --vapid-public-key <it>", file=sys.stderr)
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="cmd", required=True)
    g = sub.add_parser("genkeys", help="Create the VAPID key pair (once)")
    g.add_argument("--out", required=True, help="Private key file (PEM)")
    g.set_defaults(func=cmd_genkeys)
    r = sub.add_parser("run", help="Send what is due now (every minute)")
    r.add_argument("--dry-run", action="store_true", help="Print what is due instead of sending (state untouched)")
    r.set_defaults(func=cmd_run)
    args = parser.parse_args()
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
