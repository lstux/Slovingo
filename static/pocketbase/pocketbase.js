/**
 * pocketbase.js -- optional sync of settings > progress with a PocketBase server.
 *
 * NOT part of the default build: src/build.py copies this file (and pbstore.js)
 * into dist/ only with --pocketbase, and adds its <script> tags to index.html.
 * Without it the app is untouched (no account, no server, no network call).
 *
 * What is synced, per course (one record per course in the server's `kv`
 * collection: app = "slovingo", key = "backup.<storage_prefix>"):
 *   - exercise progress, visited sheets, visit days (the streak);
 *   - the learner's name, only to fill an empty one (never overwrites).
 * Not synced: settings (voices differ from device to device), missed questions
 * to review (a merge would bring back the ones already cleared), the last
 * opened sheet.
 *
 * Merge, not "last write wins": a sync pulls the server's copy, merges it into
 * the local progress with backup.js (best scores kept, nothing is lowered), then
 * pushes the result. Two devices can therefore never erase each other's progress.
 *
 * Needs, at call time: backup.js (buildBackup, normalizeBackup, applyBackup,
 * backupKeys, backupStoragePrefix), exercises.js (el), settings.js
 * (renderSettingsScreen), pbstore.js (PBStore), and lang.json's optional
 * `pocketbase: { url }` written by the build.
 * Same-origin courses share one login (PBStore sharedAuth).
 */

"use strict";

const PB_APP = "slovingo";
const PB_DEFAULT_URL = "https://store.lslinux.org";
const PB_MIN_AUTO_MS = 20000;   // the server limits authentication requests: no sync loop

const PB_STRINGS = {
    en: {
        title: "Sync (PocketBase)", intro: "Optional: keep your progress on all your devices.",
        server: "Server", email: "Email", password: "Password", connect: "Connect", connecting: "Connecting…",
        connected: "Connected: {email}", sync_now: "Sync now", syncing: "Syncing…", disconnect: "Disconnect",
        last: "Last sync: {time}", never: "Not synced yet",
        note: "Progress and your name are synced to the server (best scores are kept), the data is stored unencrypted. Settings stay on each device.",
        updated: "Progress updated from another device.", reload: "Reload",
        bad: "Wrong email or password.", fail: "Sync failed", many: "Too many attempts, wait a few seconds.",
    },
    fr: {
        title: "Synchronisation (PocketBase)", intro: "Facultatif : retrouve ta progression sur tous tes appareils.",
        server: "Serveur", email: "Email", password: "Mot de passe", connect: "Se connecter", connecting: "Connexion…",
        connected: "Connecté : {email}", sync_now: "Synchroniser maintenant", syncing: "Synchronisation…", disconnect: "Se déconnecter",
        last: "Dernière synchronisation : {time}", never: "Pas encore synchronisé",
        note: "La progression et ton nom sont synchronisés sur le serveur (les meilleurs scores sont gardés), les données sont stockées sans chiffrement. Les réglages restent propres à chaque appareil.",
        updated: "Progression mise à jour depuis un autre appareil.", reload: "Recharger",
        bad: "Email ou mot de passe incorrect.", fail: "Échec de la synchronisation", many: "Trop d'essais, attends quelques secondes.",
    },
    sk: {
        title: "Synchronizácia (PocketBase)", intro: "Nepovinné: tvoj postup na všetkých zariadeniach.",
        server: "Server", email: "E-mail", password: "Heslo", connect: "Pripojiť", connecting: "Pripájam…",
        connected: "Pripojený: {email}", sync_now: "Synchronizovať teraz", syncing: "Synchronizujem…", disconnect: "Odpojiť",
        last: "Posledná synchronizácia: {time}", never: "Zatiaľ nesynchronizované",
        note: "Postup a tvoje meno sa synchronizujú na server (zachovajú sa najlepšie skóre), údaje sú uložené bez šifrovania. Nastavenia ostávajú na každom zariadení.",
        updated: "Postup bol aktualizovaný z iného zariadenia.", reload: "Načítať znova",
        bad: "Nesprávny e-mail alebo heslo.", fail: "Synchronizácia zlyhala", many: "Príliš veľa pokusov, počkaj chvíľu.",
    },
    de: {
        title: "Synchronisierung (PocketBase)", intro: "Optional: dein Fortschritt auf all deinen Geräten.",
        server: "Server", email: "E-Mail", password: "Passwort", connect: "Verbinden", connecting: "Verbinde…",
        connected: "Verbunden: {email}", sync_now: "Jetzt synchronisieren", syncing: "Synchronisiere…", disconnect: "Trennen",
        last: "Letzte Synchronisierung: {time}", never: "Noch nicht synchronisiert",
        note: "Fortschritt und dein Name werden mit dem Server synchronisiert (die besten Ergebnisse bleiben erhalten), die Daten werden unverschlüsselt gespeichert. Einstellungen bleiben auf jedem Gerät.",
        updated: "Fortschritt von einem anderen Gerät aktualisiert.", reload: "Neu laden",
        bad: "E-Mail oder Passwort falsch.", fail: "Synchronisierung fehlgeschlagen", many: "Zu viele Versuche, warte einen Moment.",
    },
};

/** Interface text in the course's native language (fr / sk / de), English otherwise. */
function pbText(key, vars) {
    const code = (typeof LANG === "object" && LANG && LANG.native_lang && LANG.native_lang.code) || "en";
    let text = (PB_STRINGS[code] || PB_STRINGS.en)[key] || PB_STRINGS.en[key] || key;
    Object.entries(vars || {}).forEach(([k, v]) => { text = text.replace(`{${k}}`, v); });
    return text;
}

function pbServerUrl() {
    return (typeof LANG === "object" && LANG && LANG.pocketbase && LANG.pocketbase.url) || PB_DEFAULT_URL;
}

/** PBStore plus the two remote calls this module needs (uses PBStore's internal _refresh/_find/_api). */
class PBSlovingo extends PBStore {
    constructor() { super({ app: PB_APP, sharedAuth: true }); }

    /** The server's record for `key` (or null), after refreshing the token. */
    async remoteGet(key) {
        if (this._shared) this._auth = this._readJSON(this._authKey);   // login/logout done in another course
        if (!this.connected) { const e = new Error("not connected"); e.status = 401; throw e; }
        await this._refresh();
        return this._find(key);
    }

    async remotePut(key, value, rec) {
        if (rec) return this._api("PATCH", `/api/collections/kv/records/${rec.id}`, { value });
        try {
            return await this._api("POST", "/api/collections/kv/records", { owner: this._auth.id, app: PB_APP, key, value });
        } catch (e) {   // created meanwhile by another device: update that one
            if (e.status !== 400) throw e;
            const again = await this._find(key);
            if (!again) throw e;
            return this._api("PATCH", `/api/collections/kv/records/${again.id}`, { value });
        }
    }
}

// ----------------------------------------------------------------------------
// Pure helpers (also unit-testable in Node)
// ----------------------------------------------------------------------------

/** What is synced of a progress object: the rest is blanked so merging leaves it alone. */
function pbSlice(progress) {
    return {
        exercises: (progress && progress.exercises) || {},
        visitedSheets: (progress && progress.visitedSheets) || {},
        reviewMistakes: {},
        visitDays: (progress && progress.visitDays) || [],
        lastSheetId: null,
        lastSeriesSheetId: null,
    };
}

/** JSON with sorted keys: equal content gives an equal string, whatever the key order. */
function pbCanon(value) {
    if (Array.isArray(value)) return `[${value.map(pbCanon).join(",")}]`;
    if (value && typeof value === "object") {
        return `{${Object.keys(value).sort().map((k) => `${JSON.stringify(k)}:${pbCanon(value[k])}`).join(",")}}`;
    }
    return JSON.stringify(value);
}

/** The comparable part of a backup-shaped object: synced progress + name. */
function pbComparable(backup) {
    const p = pbSlice(backup.progress);
    return pbCanon({ e: p.exercises, v: p.visitedSheets, d: [...p.visitDays].sort(), n: (backup.settings && backup.settings.userName) || "" });
}

// ----------------------------------------------------------------------------
// Sync
// ----------------------------------------------------------------------------

let pbBusy = false;
let pbLastRun = 0;

const pbLastKey = () => `${backupStoragePrefix()}-pb-last`;

/**
 * Pull, merge, push. Returns { changed, pushed } (changed: local progress was
 * updated by the server's copy), or null when not connected or already running.
 * Errors propagate (e.status 401/403: the login is no longer valid).
 */
async function pbSyncNow() {
    if (pbBusy) return null;
    const pb = new PBSlovingo();   // fresh: reads the shared login
    if (!pb.connected) return null;
    pbBusy = true; pbLastRun = Date.now();
    try {
        const key = `backup.${backupStoragePrefix()}`;
        const rec = await pb.remoteGet(key);
        const before = pbComparable(buildBackup());

        let remote = null;
        if (rec && rec.value && typeof rec.value === "object") {
            try { remote = normalizeBackup(rec.value); } catch (e) { remote = null; }
        }
        if (remote) {
            const localName = localStorage.getItem(backupKeys().userName) || "";
            const fillName = localName ? "" : (remote.settings.userName || "");
            applyBackup({ settings: { userName: fillName, themeMode: null, app: null }, progress: pbSlice(remote.progress), course: null, version: 1, skipped: 0 });
        }

        const merged = buildBackup();
        const changed = pbComparable(merged) !== before;
        let pushed = false;
        if (!remote || pbComparable(remote) !== pbComparable(merged)) {
            const p = pbSlice(merged.progress);
            await pb.remotePut(key, {
                format: merged.format, version: merged.version, exportedAt: merged.exportedAt, course: merged.course,
                settings: { userName: merged.settings.userName },
                progress: { exercises: p.exercises, visitedSheets: p.visitedSheets, visitDays: p.visitDays },
            }, rec);
            pushed = true;
        }
        try { localStorage.setItem(pbLastKey(), String(Date.now())); } catch (e) { /* storage unavailable */ }
        if (changed) pbNotifyUpdated();
        return { changed, pushed };
    } finally {
        pbBusy = false;
    }
}

/** A small banner: the page shows the old progress until reloaded. */
function pbNotifyUpdated() {
    if (document.getElementById("pb-updated")) return;
    const bar = document.createElement("div");
    bar.id = "pb-updated";
    bar.setAttribute("role", "status");
    bar.style.cssText = "position:fixed;left:50%;transform:translateX(-50%);bottom:16px;z-index:1000;max-width:calc(100vw - 24px);"
        + "display:flex;gap:12px;align-items:center;padding:10px 14px;border-radius:12px;background:var(--text,#222);color:var(--bg,#fff);"
        + "box-shadow:0 6px 24px #0005;font-size:14px";
    const msg = document.createElement("span"); msg.textContent = pbText("updated");
    const btn = document.createElement("button");
    btn.type = "button"; btn.textContent = pbText("reload");
    btn.style.cssText = "border:0;background:none;color:inherit;font:inherit;font-weight:700;text-decoration:underline;cursor:pointer";
    btn.addEventListener("click", () => window.location.reload());
    bar.append(msg, btn);
    document.body.appendChild(bar);
}

/** Automatic sync (start, back online, tab visible or hidden), throttled, errors only logged. */
function pbAutoSync() {
    if (Date.now() - pbLastRun < PB_MIN_AUTO_MS) return;
    pbSyncNow().catch((e) => console.warn("pocketbase sync:", e));
}

function pbStart() {
    let tries = 0;
    const timer = setInterval(() => {   // wait for app.js to load lang.json
        tries += 1;
        if (typeof LANG === "object" && LANG) { clearInterval(timer); pbAutoSync(); }
        else if (tries > 50) clearInterval(timer);
    }, 300);
    window.addEventListener("online", pbAutoSync);
    document.addEventListener("visibilitychange", pbAutoSync);   // arriving on and leaving the tab
}

// ----------------------------------------------------------------------------
// Settings screen glue
// ----------------------------------------------------------------------------

/** The "Sync" block of Settings > Data (called from settings.js when this file is loaded). */
function renderSyncControls() {
    const pb = new PBSlovingo();
    const box = el("div", { className: "settings-row" });
    const status = (text, err) => {
        let s = box.querySelector(".pb-status");
        if (!s) { s = el("p", { className: "pb-status" }); box.appendChild(s); }
        s.textContent = text; s.style.color = err ? "var(--danger, #c0392b)" : "";
    };
    box.appendChild(el("h3", { text: pbText("title") }));

    if (pb.connected) {
        const last = Number(localStorage.getItem(pbLastKey()));
        box.appendChild(el("p", { text: pbText("connected", { email: pb.email }) }));
        box.appendChild(el("p", { className: "pb-status", text: last ? pbText("last", { time: new Date(last).toLocaleString() }) : pbText("never") }));
        const nowBtn = el("button", { className: "settings-backup-btn", text: pbText("sync_now"), attrs: { type: "button" } });
        nowBtn.addEventListener("click", async () => {
            nowBtn.disabled = true; status(pbText("syncing"));
            try {
                await pbSyncNow();
                status(pbText("last", { time: new Date().toLocaleString() }));
            } catch (e) {
                status(`${pbText("fail")}: ${e.message || e}`, true);
            }
            nowBtn.disabled = false;
        });
        const outBtn = el("button", { className: "settings-backup-btn", text: pbText("disconnect"), attrs: { type: "button" } });
        outBtn.addEventListener("click", () => { pb.logout(); renderSettingsScreen(); });
        box.append(nowBtn, outBtn);
    } else {
        box.appendChild(el("p", { text: pbText("intro") }));
        const url = el("input", { className: "settings-input", attrs: { type: "text", value: pbServerUrl(), "aria-label": pbText("server"), placeholder: pbText("server"), autocapitalize: "off", spellcheck: "false" } });
        const mail = el("input", { className: "settings-input", attrs: { type: "email", "aria-label": pbText("email"), placeholder: pbText("email"), autocomplete: "username", autocapitalize: "off" } });
        const pw = el("input", { className: "settings-input", attrs: { type: "password", "aria-label": pbText("password"), placeholder: pbText("password"), autocomplete: "current-password" } });
        [url, mail, pw].forEach((i) => { i.style.cssText = "display:block;width:100%;box-sizing:border-box;margin-top:8px"; box.appendChild(i); });
        const go = el("button", { className: "settings-backup-btn", text: pbText("connect"), attrs: { type: "button" } });
        go.addEventListener("click", async () => {
            if (!mail.value.trim() || !pw.value) return;
            go.disabled = true; status(pbText("connecting"));
            try {
                await pb.login(url.value.trim(), mail.value.trim(), pw.value);
                await pbSyncNow();
                renderSettingsScreen();
            } catch (e) {
                status(e.status === 429 ? pbText("many") : e.status === 400 ? pbText("bad") : `${pbText("fail")}: ${e.message || e}`, true);
                go.disabled = false;
            }
        });
        box.appendChild(go);
    }
    box.appendChild(el("p", { className: "pb-note", text: pbText("note") }));
    return box;
}

if (typeof window !== "undefined") pbStart();

// Node (unit tests) can require() the pure parts; the browser ignores this.
if (typeof module !== "undefined" && module.exports) {
    module.exports = { pbSlice, pbCanon, pbComparable };
}
