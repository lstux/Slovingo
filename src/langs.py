#!/usr/bin/env python3
"""langs.py

Fetch the course repositories into langs/<code>/, where the engine expects
them (build.py --lang-dir langs/<code>, or --langs-root langs).

Each course lives in its own repository, lstux/Slovingo-<code>. This script
clones the ones you ask for, or fast-forwards them when they are already
there. It never touches a course directory that has local changes, and it
never rewrites history: `git pull --ff-only` is the only update it does.

Pure standard library + the `git` command, so it runs wherever Python 3.8+
and git do (Linux, macOS, Windows).

Usage:
    python3 src/langs.py                      # interactive menu (terminal)
    python3 src/langs.py de-fr-kids sk-fr     # these courses
    python3 src/langs.py de-fr-kids@animals   # one course, on a given branch
    python3 src/langs.py --all                # every known course
    python3 src/langs.py --list               # status of every known course

Options:
    --ssh          clone over SSH (git@github.com:lstux/...) instead of HTTPS
    --base URL     other repository base (default: https://github.com/lstux,
                   or the SLOVINGO_REPO_BASE environment variable); the
                   repository is <base>/Slovingo-<code>[.git]
    --langs-dir D  where courses go (default: langs/ at the repo root)

A code that is not in the known list is accepted too (forks, private
courses): it is simply cloned from <base>/Slovingo-<code>.
"""

from __future__ import annotations

import argparse
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

DEFAULT_BASE = "https://github.com/lstux"
SSH_BASE = "git@github.com:lstux"

# Known courses: code -> short description (shown in the menu).
COURSES = {
    "sk-fr": "Slovaque pour francophones",
    "sk-fr-kids": "Slovaque pour enfants (Zajka)",
    "sk-fr-friends": "Slovaque, version amis",
    "fr-sk": "Français pour slovaques",
    "bzh-fr": "Breton pour francophones",
    "de-fr-kids": "Allemand pour enfants (Fuchsbau)",
}

CODE_RE = re.compile(r"^[a-z0-9][a-z0-9-]*$")
BRANCH_RE = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._/-]*$")


class SyncError(Exception):
    """One course could not be cloned or updated (message is user-facing)."""


def git(*args: str, cwd: Path | None = None) -> str:
    """Run a git command and return its stdout; raise SyncError on failure."""
    try:
        proc = subprocess.run(
            ["git", *args],
            cwd=str(cwd) if cwd else None,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True,
        )
    except OSError as exc:
        raise SyncError(f"cannot run git: {exc}")
    if proc.returncode != 0:
        detail = (proc.stderr or proc.stdout).strip().splitlines()
        raise SyncError(f"git {args[0]} failed: {detail[-1] if detail else 'unknown error'}")
    return proc.stdout.strip()


def parse_spec(spec: str) -> tuple[str, str | None]:
    """Split "code" or "code@branch" and validate both parts."""
    code, _, branch = spec.partition("@")
    if not CODE_RE.match(code):
        raise SystemExit(f"Invalid course code: {code!r} (lowercase letters, digits and '-')")
    if branch and not BRANCH_RE.match(branch):
        raise SystemExit(f"Invalid branch name: {branch!r}")
    return code, branch or None


def repo_url(code: str, base: str) -> str:
    return f"{base.rstrip('/')}/Slovingo-{code}.git"


def status(lang_dir: Path) -> str:
    """One-line state of langs/<code>: absent / not a clone / branch + dirty."""
    if not lang_dir.exists():
        return "absent"
    if not (lang_dir / ".git").exists():
        return "present (not a git clone)"
    try:
        branch = git("rev-parse", "--abbrev-ref", "HEAD", cwd=lang_dir)
        dirty = git("status", "--porcelain", cwd=lang_dir)
    except SyncError:
        return "present (git error)"
    return f"installed ({branch}{', local changes' if dirty else ''})"


def sync(code: str, branch: str | None, langs_dir: Path, base: str) -> str:
    """Clone or update one course. Returns a short result line."""
    target = langs_dir / code
    if not target.exists():
        langs_dir.mkdir(parents=True, exist_ok=True)
        cmd = ["clone", "--quiet"]
        if branch:
            cmd += ["--branch", branch]
        git(*cmd, repo_url(code, base), str(target))
        return f"cloned{f' ({branch})' if branch else ''}"

    if not (target / ".git").exists():
        raise SyncError(f"{target} exists but is not a git clone; left untouched")
    if git("status", "--porcelain", cwd=target):
        raise SyncError(f"{target} has local changes; commit or stash them first")

    git("fetch", "--quiet", "origin", cwd=target)
    if branch:
        current = git("rev-parse", "--abbrev-ref", "HEAD", cwd=target)
        if current != branch:
            git("checkout", "--quiet", branch, cwd=target)
    current = git("rev-parse", "--abbrev-ref", "HEAD", cwd=target)
    if current == "HEAD":
        return "up to date (detached HEAD, fetched only)"
    before = git("rev-parse", "HEAD", cwd=target)
    git("pull", "--quiet", "--ff-only", cwd=target)
    after = git("rev-parse", "HEAD", cwd=target)
    return f"updated ({current})" if before != after else f"already up to date ({current})"


def interactive_menu(langs_dir: Path) -> list[str]:
    """Numbered menu; the user toggles courses and validates. Returns specs."""
    codes = list(COURSES)
    selected: set[str] = set()
    while True:
        print("\nCourses (lstux/Slovingo-<code>):")
        for i, code in enumerate(codes, 1):
            mark = "x" if code in selected else " "
            print(f"  [{mark}] {i}. {code:<14} {COURSES[code]}  — {status(langs_dir / code)}")
        answer = input("\nNumbers to toggle (e.g. 1 3), 'a' = all, Enter = validate, 'q' = quit: ").strip().lower()
        if answer == "q":
            return []
        if answer == "":
            return [c for c in codes if c in selected]
        if answer == "a":
            selected = set(codes)
            continue
        for token in answer.replace(",", " ").split():
            if token.isdigit() and 1 <= int(token) <= len(codes):
                selected ^= {codes[int(token) - 1]}
            else:
                print(f"  ignored: {token!r}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Clone or update course repositories into langs/<code>/.",
        epilog="Examples: langs.py de-fr-kids sk-fr  |  langs.py de-fr-kids@animals  |  langs.py --all",
    )
    parser.add_argument("courses", nargs="*", metavar="CODE[@BRANCH]", help="courses to fetch")
    parser.add_argument("--all", action="store_true", help="every known course")
    parser.add_argument("--list", action="store_true", help="show known courses and their local status")
    parser.add_argument("--ssh", action="store_true", help="clone over SSH instead of HTTPS")
    parser.add_argument("--base", default=None, help="repository base URL (see module docstring)")
    parser.add_argument("--langs-dir", type=Path, default=ROOT / "langs", help="destination (default: langs/)")
    args = parser.parse_args()

    base = args.base or (SSH_BASE if args.ssh else os.environ.get("SLOVINGO_REPO_BASE", DEFAULT_BASE))

    if args.list:
        for code, desc in COURSES.items():
            print(f"{code:<14} {desc:<36} {status(args.langs_dir / code)}")
        return 0

    if shutil.which("git") is None:
        print("Error: git is required but was not found in PATH.", file=sys.stderr)
        return 2

    specs = list(args.courses)
    if args.all:
        specs += [c for c in COURSES if c not in {parse_spec(s)[0] for s in specs}]
    if not specs:
        if not sys.stdin.isatty():
            parser.print_usage(sys.stderr)
            print("Error: no course given and no terminal for the menu (try --list or --all).", file=sys.stderr)
            return 2
        specs = interactive_menu(args.langs_dir)
        if not specs:
            print("Nothing selected.")
            return 0

    failures = 0
    synced = []
    for spec in specs:
        code, branch = parse_spec(spec)
        try:
            print(f"{code:<14} {sync(code, branch, args.langs_dir, base)}")
            synced.append(code)
        except SyncError as exc:
            failures += 1
            print(f"{code:<14} SKIPPED: {exc}", file=sys.stderr)

    if synced:
        try:
            shown = args.langs_dir.resolve().relative_to(ROOT)
        except ValueError:
            shown = args.langs_dir
        print(f"\nBuild one:  python3 src/publish.py --lang-dir {shown}/{synced[0]}")
        print(f"Build all:  python3 src/build.py --langs-root {shown} --static-dir static")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
