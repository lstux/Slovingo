#!/usr/bin/env python3
"""tower.py -- Slovingo "control tower": one console UI for the whole
local workflow.

    python3 src/tower.py

Screens (single-key navigation, the available keys are always shown at
the bottom):

    home          state of the engine, the deployment, the dependencies
                  and every course (branch, clean/dirty, ahead/behind,
                  age of the last build)
    1  Courses    clone / update the course repositories (what
                  src/langs.py does), choose a branch per course
    2  Deployment view, edit, validate and test (ssh) deploy.json
    3  Dependencies  git, Pillow, rsync, ssh, ImageMagick, rich... what
                  each one is needed for, and how to install it here
    4  Build & publish  pick courses, course branches, Slovingo branch
                  and options (force exercises / icons, pull, --delete,
                  dry run vs real upload), see the exact commands, then
                  run them

This tool adds no logic of its own to the pipeline: it assembles and
runs the existing scripts (langs.py's sync(), build.py, publish.py) and
always shows the equivalent command lines, so anything it does can be
replayed by hand. deploy.json is the only file it edits. Its own
preferences (selected courses, branches, options) live in .tower.json
(git-ignored).

`rich` (pip install rich) is optional: with it the UI gets colours,
boxes and spinners; without it the same screens render as plain text.

Non-interactive use:
    python3 src/tower.py --status     # home screen once, then exit
    python3 src/tower.py --deps       # dependency report, then exit
        (exit status 1 if a required dependency is missing)

Options:
    --plain              never use rich, even if installed
    --deploy-config P    deploy.json location (default: ./deploy.json)
    --langs-dir D        where the courses live (default: langs/)
    --base URL           course repository base (see langs.py)
"""

from __future__ import annotations

import argparse
import contextlib
import importlib.util
import json
import os
import platform
import shlex
import shutil
import subprocess
import sys
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Callable, Iterator, Optional

import langs

ROOT = Path(__file__).resolve().parent.parent
STATE_FILE = ROOT / ".tower.json"

# ---------------------------------------------------------------------------
# UI layer: the same calls render with rich (if available) or as plain text.
# A "cell" is either a string or a (string, style) pair, style being one of
# ok / warn / bad / dim / accent / key (or None).
# ---------------------------------------------------------------------------

try:  # optional dependency
    from rich import box
    from rich.console import Console
    from rich.panel import Panel
    from rich.table import Table
    from rich.text import Text

    HAVE_RICH = True
except ImportError:  # pragma: no cover - depends on the environment
    HAVE_RICH = False

RICH_STYLES = {
    "ok": "green",
    "warn": "yellow",
    "bad": "bold red",
    "dim": "dim",
    "accent": "bold #cfa445",
    "key": "bold #e0705f",
}

LETTERS = {
    "S": [" ___ ", "/ __|", "\\__ \\", "|___/"],
    "L": [" _    ", "| |   ", "| |__ ", "|____|"],
    "O": ["  ___  ", " / _ \\ ", "| (_) |", " \\___/ "],
    "V": ["__   __", "\\ \\ / /", " \\ V / ", "  \\_/  "],
    "I": [" ___ ", "|_ _|", " | | ", "|___|"],
    "N": [" _  _ ", "| \\| |", "| .` |", "|_|\\_|"],
    "G": ["  ___ ", " / __|", "| (_ |", " \\___|"],
}


def banner_lines(word: str = "SLOVINGO") -> list[str]:
    """ASCII-art title, one string per row."""
    return ["".join(LETTERS[ch][row] for ch in word) for row in range(4)]


def cell_parts(cell: Any) -> tuple[str, Optional[str]]:
    if isinstance(cell, tuple):
        return str(cell[0]), cell[1]
    return str(cell), None


def unicode_ok() -> bool:
    enc = (getattr(sys.stdout, "encoding", None) or "").lower()
    return "utf" in enc


class PlainUI:
    """Dependency-free rendering (also used for --status and in tests)."""

    rich = False

    def __init__(self) -> None:
        uni = unicode_ok()
        self.sym = {
            "ok": "✔" if uni else "OK",
            "bad": "✘" if uni else "KO",
            "warn": "▲" if uni else "!!",
            "dim": "·" if uni else "-",
        }

    # -- output -------------------------------------------------------
    def clear(self) -> None:
        if sys.stdout.isatty():
            sys.stdout.write("\033[2J\033[H")
        else:
            print()

    def banner(self, subtitle: str) -> None:
        print()
        for line in banner_lines():
            print(line)
        print(subtitle)
        print()

    def rule(self, title: str) -> None:
        print(f"\n== {title} " + "=" * max(3, 60 - len(title)))

    def say(self, text: str, style: Optional[str] = None) -> None:
        prefix = self.sym.get(style or "", "")
        print(f"{prefix} {text}" if prefix and style in ("ok", "bad", "warn") else text)

    def table(self, title: str, headers: list[str], rows: list[list[Any]]) -> None:
        parts = [[cell_parts(c)[0] for c in row] for row in rows]
        widths = [len(h) for h in headers]
        for row in parts:
            for i, text in enumerate(row):
                widths[i] = max(widths[i], len(text))
        if title:
            print(title)
        print("  ".join(h.ljust(widths[i]) for i, h in enumerate(headers)))
        print("  ".join("-" * w for w in widths))
        for row in parts:
            print("  ".join(text.ljust(widths[i]) for i, text in enumerate(row)))

    def panel(self, title: str, lines: list[Any]) -> None:
        print(f"-- {title} --")
        for cell in lines:
            print("  " + cell_parts(cell)[0])

    def footer(self, hints: list[tuple[str, str]]) -> None:
        print()
        print("   ".join(f"[{k}] {label}" for k, label in hints))

    @contextlib.contextmanager
    def spinner(self, message: str) -> Iterator[None]:
        if sys.stdout.isatty():
            print(f"{message}…", flush=True)
        yield

    # -- input --------------------------------------------------------
    def key(self) -> str:
        return read_key()

    def ask(self, prompt: str, default: Optional[str] = None) -> str:
        suffix = f" [{default}]" if default not in (None, "") else ""
        try:
            answer = input(f"{prompt}{suffix} : ").strip()
        except EOFError:
            return default or ""
        return answer or (default or "")

    def confirm(self, prompt: str, default: bool = False) -> bool:
        hint = "O/n" if default else "o/N"
        try:
            answer = input(f"{prompt} ({hint}) ").strip().lower()
        except EOFError:
            return False
        if not answer:
            return default
        return answer in ("o", "oui", "y", "yes")

    def pause(self) -> None:
        try:
            input("\nEntrée pour continuer… ")
        except EOFError:
            pass

    def choose(self, title: str, options: list[str], current: Optional[str] = None) -> Optional[str]:
        """Numbered list; accepts a number or a free value. None = cancel."""
        print(title)
        for i, opt in enumerate(options, 1):
            print(f"  {i}. {opt}{'   (actuel)' if opt == current else ''}")
        answer = self.ask("Numéro ou nom (vide = annuler)")
        if not answer:
            return None
        if answer.isdigit() and 1 <= int(answer) <= len(options):
            return options[int(answer) - 1]
        return answer


class RichUI(PlainUI):
    """Same calls, rendered with rich. Uses only Console, Table, Panel and
    Text on purpose: the smallest, most stable part of the API."""

    rich = True

    def __init__(self) -> None:
        super().__init__()
        self.console = Console(highlight=False)
        self.sym = {"ok": "✔", "bad": "✘", "warn": "▲", "dim": "·"}

    def _text(self, cell: Any) -> "Text":
        text, style = cell_parts(cell)
        return Text(text, style=RICH_STYLES.get(style or "", ""))

    def clear(self) -> None:
        self.console.clear()

    def banner(self, subtitle: str) -> None:
        lines = banner_lines()
        art = Text()
        width = max(len(line) for line in lines)
        start, end = (168, 54, 43), (207, 164, 69)  # project palette: brick -> gold
        for line in lines:
            for col, ch in enumerate(line):
                t = col / max(1, width - 1)
                r, g, b = (int(start[i] + (end[i] - start[i]) * t) for i in range(3))
                art.append(ch, style=f"bold rgb({r},{g},{b})")
            art.append("\n")
        art.append(subtitle, style="dim")
        self.console.print(Panel(art, box=box.ROUNDED, border_style="#a8362b", padding=(0, 2)))

    def rule(self, title: str) -> None:
        self.console.rule(Text(title, style=RICH_STYLES["accent"]))

    def say(self, text: str, style: Optional[str] = None) -> None:
        sym = self.sym.get(style or "", "")
        line = Text()
        if sym and style in ("ok", "bad", "warn"):
            line.append(sym + " ", style=RICH_STYLES[style])
        line.append(text, style=RICH_STYLES[style] if style in ("dim", "accent", "bad", "warn") else "")
        self.console.print(line)

    def table(self, title: str, headers: list[str], rows: list[list[Any]]) -> None:
        table = Table(title=title or None, box=box.ROUNDED, header_style="bold",
                      title_style=RICH_STYLES["accent"], border_style="dim")
        for header in headers:
            table.add_column(header)
        for row in rows:
            table.add_row(*[self._text(cell) for cell in row])
        self.console.print(table)

    def panel(self, title: str, lines: list[Any]) -> None:
        body = Text()
        for i, cell in enumerate(lines):
            if i:
                body.append("\n")
            body.append_text(self._text(cell))
        self.console.print(Panel(body, title=title, box=box.ROUNDED, border_style="dim"))

    def footer(self, hints: list[tuple[str, str]]) -> None:
        line = Text()
        for i, (k, label) in enumerate(hints):
            if i:
                line.append("  ")
            line.append(f"[{k}]", style=RICH_STYLES["key"])
            line.append(f" {label}")
        self.console.print()
        self.console.print(line)

    @contextlib.contextmanager
    def spinner(self, message: str) -> Iterator[None]:
        with self.console.status(message):
            yield


def make_ui(plain: bool) -> PlainUI:
    return PlainUI() if plain or not HAVE_RICH else RichUI()


def read_key() -> str:
    """Read one key press (no Enter needed on a terminal). Returns a
    lowercase character, "\\n" for Enter, "esc" for Escape/arrows."""
    if not sys.stdin.isatty():
        line = sys.stdin.readline()
        if line == "":
            return "q"
        return line.strip()[:1].lower() or "\n"
    if os.name == "nt":  # pragma: no cover - Windows
        import msvcrt

        ch = msvcrt.getwch()
        if ch in ("\x00", "\xe0"):
            msvcrt.getwch()
            return "esc"
        return "\n" if ch == "\r" else ch.lower()
    import termios
    import tty

    fd = sys.stdin.fileno()
    old = termios.tcgetattr(fd)
    try:
        tty.setcbreak(fd)
        ch = os.read(fd, 1).decode("utf-8", "ignore")
        if ch == "\x1b":
            # swallow the rest of an escape sequence (arrows etc.)
            import select

            while select.select([fd], [], [], 0.02)[0]:
                os.read(fd, 1)
            return "esc"
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old)
    return "\n" if ch in ("\r", "\n") else ch.lower()


# ---------------------------------------------------------------------------
# Small helpers
# ---------------------------------------------------------------------------

def shell_repr(argv: list[str]) -> str:
    return " ".join(shlex.quote(a) for a in argv)


def rel(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(ROOT))
    except ValueError:
        return str(path)


def age(timestamp: Optional[float]) -> str:
    if not timestamp:
        return "—"
    seconds = max(0, time.time() - timestamp)
    for limit, unit, size in ((90, "s", 1), (5400, "min", 60), (172800, "h", 3600)):
        if seconds < limit:
            return f"il y a {int(seconds / size)} {unit}"
    return f"il y a {int(seconds / 86400)} j"


def run_quiet(argv: list[str], timeout: int = 10) -> tuple[int, str]:
    """Run a command, return (returncode, combined output). 127 if missing."""
    try:
        proc = subprocess.run(argv, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                              universal_newlines=True, timeout=timeout)
    except FileNotFoundError:
        return 127, ""
    except (OSError, subprocess.TimeoutExpired) as exc:
        return 1, str(exc)
    return proc.returncode, proc.stdout.strip()


# ---------------------------------------------------------------------------
# Persistent preferences (.tower.json)
# ---------------------------------------------------------------------------

DEFAULT_STATE: dict[str, Any] = {
    "ssh": False,                # clone courses over SSH instead of HTTPS
    "selection": [],             # selected course codes
    "branches": {},              # code -> wanted branch
    "options": {
        "force_exercises": False,
        "force_icons": False,
        "pull": True,            # git pull --ff-only the courses before building
        "delete": False,         # rsync --delete
        "execute": False,        # real upload (default: dry run)
    },
    "action": "build+publish",   # build | build+publish | publish
}


def load_state(path: Path = STATE_FILE) -> dict[str, Any]:
    state = json.loads(json.dumps(DEFAULT_STATE))
    try:
        saved = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, ValueError):
        return state
    if isinstance(saved, dict):
        for key, value in saved.items():
            if key == "options" and isinstance(value, dict):
                state["options"].update({k: v for k, v in value.items() if k in state["options"]})
            elif key in state:
                state[key] = value
    return state


def save_state(state: dict[str, Any], path: Path = STATE_FILE) -> None:
    try:
        path.write_text(json.dumps(state, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    except OSError:
        pass  # preferences are a convenience, never fatal


# ---------------------------------------------------------------------------
# Courses
# ---------------------------------------------------------------------------

@dataclass
class CourseInfo:
    code: str
    desc: str
    path: Path
    installed: bool = False
    is_git: bool = False
    branch: str = ""
    dirty: bool = False
    ahead: Optional[int] = None
    behind: Optional[int] = None
    built_at: Optional[float] = None
    buildable: bool = False      # has a lang.json
    error: str = ""


def known_codes(langs_dir: Path) -> list[tuple[str, str]]:
    """Known courses plus any other course directory already in langs/."""
    out = list(langs.COURSES.items())
    seen = {code for code, _ in out}
    if langs_dir.is_dir():
        for child in sorted(langs_dir.iterdir()):
            if child.is_dir() and child.name not in seen and langs.CODE_RE.match(child.name):
                out.append((child.name, "(cours local)"))
    return out


def course_info(code: str, desc: str, langs_dir: Path) -> CourseInfo:
    path = langs_dir / code
    info = CourseInfo(code=code, desc=desc, path=path, installed=path.is_dir())
    if not info.installed:
        return info
    info.buildable = (path / "lang.json").is_file()
    index = path / "dist" / "index.html"
    if index.is_file():
        info.built_at = index.stat().st_mtime
    info.is_git = (path / ".git").exists()
    if not info.is_git:
        return info
    try:
        info.branch = langs.git("rev-parse", "--abbrev-ref", "HEAD", cwd=path)
        info.dirty = bool(langs.git("status", "--porcelain", cwd=path))
    except langs.SyncError as exc:
        info.error = str(exc)
        return info
    try:
        counts = langs.git("rev-list", "--left-right", "--count", "HEAD...@{u}", cwd=path).split()
        info.ahead, info.behind = int(counts[0]), int(counts[1])
    except (langs.SyncError, ValueError, IndexError):
        pass  # no upstream / detached HEAD
    return info


def course_state_cell(info: CourseInfo) -> tuple[str, str]:
    if not info.installed:
        return "absent", "dim"
    if not info.is_git:
        return "pas un clone git", "warn"
    if info.error:
        return "erreur git", "bad"
    parts = ["modifié" if info.dirty else "propre"]
    if info.ahead:
        parts.append(f"↑{info.ahead}")
    if info.behind:
        parts.append(f"↓{info.behind}")
    style = "warn" if info.dirty or info.behind else "ok"
    return " ".join(parts), style


# ---------------------------------------------------------------------------
# Engine (this repository)
# ---------------------------------------------------------------------------

@dataclass
class EngineInfo:
    branch: str = "?"
    sha: str = ""
    dirty: bool = False
    ahead: Optional[int] = None
    behind: Optional[int] = None


def engine_info() -> EngineInfo:
    info = EngineInfo()
    try:
        info.branch = langs.git("rev-parse", "--abbrev-ref", "HEAD", cwd=ROOT)
        info.sha = langs.git("rev-parse", "--short", "HEAD", cwd=ROOT)
        info.dirty = bool(langs.git("status", "--porcelain", "--untracked-files=no", cwd=ROOT))
    except langs.SyncError:
        return info
    try:
        counts = langs.git("rev-list", "--left-right", "--count", "HEAD...@{u}", cwd=ROOT).split()
        info.ahead, info.behind = int(counts[0]), int(counts[1])
    except (langs.SyncError, ValueError, IndexError):
        pass
    return info


def engine_branches() -> list[str]:
    names: list[str] = []
    for argv in (["branch", "--format=%(refname:short)"], ["branch", "-r", "--format=%(refname:short)"]):
        try:
            out = langs.git(*argv, cwd=ROOT)
        except langs.SyncError:
            continue
        for name in out.splitlines():
            name = name.strip()
            if name.startswith("origin/"):
                name = name[len("origin/"):]
            if name and name != "HEAD" and not name.endswith("/HEAD") and name not in names:
                names.append(name)
    return names


# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

@dataclass
class Dep:
    key: str
    name: str
    needed_for: str
    level: str            # "required" | "remote" (needed for remote publish) | "optional"
    found: bool = False
    detail: str = ""
    hint: str = ""


def detect_platform() -> str:
    """apt / dnf / pacman / zypper / brew / winget / unknown."""
    system = platform.system()
    if system == "Darwin":
        return "brew"
    if system == "Windows":
        return "winget"
    for manager in ("apt-get", "dnf", "pacman", "zypper", "apk"):
        if shutil.which(manager):
            return {"apt-get": "apt"}.get(manager, manager)
    return "unknown"


INSTALL = {
    "git": {"apt": "sudo apt install git", "dnf": "sudo dnf install git", "pacman": "sudo pacman -S git",
            "zypper": "sudo zypper install git", "apk": "apk add git", "brew": "brew install git",
            "winget": "winget install Git.Git"},
    "rsync": {"apt": "sudo apt install rsync", "dnf": "sudo dnf install rsync", "pacman": "sudo pacman -S rsync",
              "zypper": "sudo zypper install rsync", "apk": "apk add rsync", "brew": "brew install rsync",
              "winget": "utiliser WSL (rsync n'existe pas nativement sous Windows)"},
    "ssh": {"apt": "sudo apt install openssh-client", "dnf": "sudo dnf install openssh-clients",
            "pacman": "sudo pacman -S openssh", "zypper": "sudo zypper install openssh-clients",
            "apk": "apk add openssh-client", "brew": "(fourni avec macOS)",
            "winget": "Paramètres > Fonctionnalités facultatives > Client OpenSSH"},
    "convert": {"apt": "sudo apt install imagemagick", "dnf": "sudo dnf install ImageMagick",
                "pacman": "sudo pacman -S imagemagick", "zypper": "sudo zypper install ImageMagick",
                "apk": "apk add imagemagick", "brew": "brew install imagemagick",
                "winget": "winget install ImageMagick.ImageMagick"},
}
PIP_HINTS = {"pillow": "pip install Pillow", "rich": "pip install rich"}


def install_hint(key: str, manager: str) -> str:
    if key in PIP_HINTS:
        return PIP_HINTS[key]
    return INSTALL.get(key, {}).get(manager, "")


def check_deps(remote_publish: bool, manager: Optional[str] = None) -> list[Dep]:
    manager = manager or detect_platform()
    deps: list[Dep] = []

    py = sys.version_info
    deps.append(Dep("python", "Python", "tout le pipeline (3.8 minimum)", "required",
                    found=py >= (3, 8), detail=f"{py.major}.{py.minor}.{py.micro}"))

    code, out = run_quiet(["git", "--version"])
    deps.append(Dep("git", "git", "cloner/mettre à jour les cours, changer de branche", "required",
                    found=code == 0, detail=out.replace("git version ", "") if code == 0 else "",
                    hint=install_hint("git", manager)))

    pillow = importlib.util.find_spec("PIL") is not None
    detail = ""
    if pillow:
        try:
            from importlib.metadata import version

            detail = version("Pillow")
        except Exception:  # noqa: BLE001 - version is only cosmetic
            detail = "installé"
    deps.append(Dep("pillow", "Pillow", "génération des icônes de l'appli (build.py)", "required",
                    found=pillow, detail=detail, hint=install_hint("pillow", manager)))

    for key, binary, why in (
        ("rsync", "rsync", "publication sur un serveur distant (publish.py)"),
        ("ssh", "ssh", "publication distante et test de connexion"),
    ):
        path = shutil.which(binary)
        code, out = run_quiet([binary, "--version"] if binary == "rsync" else [binary, "-V"]) if path else (127, "")
        first = out.splitlines()[0] if out else ""
        deps.append(Dep(key, binary, why, "remote" if remote_publish else "optional",
                        found=bool(path), detail=first.replace("rsync  version ", "")[:60] if path else "",
                        hint=install_hint(key, manager)))

    convert = shutil.which("convert")
    magick = shutil.which("magick")
    detail = ""
    if convert:
        _, out = run_quiet(["convert", "-version"])
        first = out.splitlines()[0] if out else convert
        detail = first.replace("Version: ", "").split(" http")[0][:60]
    elif magick:
        detail = "seulement `magick` (IM7) : fetch_images.py appelle `convert`"
    deps.append(Dep("convert", "ImageMagick", "redimensionner les images (fetch_images.py)", "optional",
                    found=bool(convert), detail=detail, hint=install_hint("convert", manager)))

    rich_found = importlib.util.find_spec("rich") is not None
    deps.append(Dep("rich", "rich", "cette interface en couleurs (sinon : texte simple)", "optional",
                    found=rich_found, detail="actif" if HAVE_RICH else "", hint=install_hint("rich", manager)))
    return deps


def dep_row(dep: Dep) -> list[Any]:
    if dep.found:
        status: tuple[str, str] = ("✔ OK", "ok")
    elif dep.level == "required":
        status = ("✘ manquant", "bad")
    elif dep.level == "remote":
        status = ("✘ manquant", "bad")
    else:
        status = ("▲ absent", "warn")
    need = {"required": "requis", "remote": "requis (publication distante)", "optional": "facultatif"}[dep.level]
    return [dep.name, status, (need, "dim"), dep.needed_for, (dep.detail or dep.hint, "dim")]


def missing_required(deps: list[Dep]) -> list[Dep]:
    return [d for d in deps if not d.found and d.level in ("required", "remote")]


# ---------------------------------------------------------------------------
# deploy.json
# ---------------------------------------------------------------------------

DEPLOY_DEFAULTS: dict[str, Any] = {
    "host": "", "user": "", "ssh_port": 22, "remote_root": "", "identity_file": None,
}


def load_deploy(path: Path) -> Optional[dict[str, Any]]:
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, ValueError):
        return None
    return data if isinstance(data, dict) else None


def deploy_mode(cfg: Optional[dict[str, Any]]) -> str:
    if not cfg or not cfg.get("host"):
        return "unset"
    return {"localhost": "local", "lan": "lan"}.get(cfg["host"], "remote")


def validate_deploy(cfg: Optional[dict[str, Any]]) -> list[str]:
    """Human-readable problems (empty list = publish.py can use it)."""
    if cfg is None:
        return ["deploy.json absent ou illisible"]
    mode = deploy_mode(cfg)
    problems: list[str] = []
    if mode == "unset":
        return ["host non défini"]
    if mode in ("local", "lan"):
        port = cfg.get("port", 8000)
        if not isinstance(port, int) or not 1 <= port <= 65535:
            problems.append("port invalide (1-65535)")
        return problems
    if not cfg.get("user"):
        problems.append("user SSH manquant")
    root = cfg.get("remote_root") or ""
    if not root.startswith("/"):
        problems.append("remote_root doit être un chemin absolu (commence par /)")
    port = cfg.get("ssh_port", 22)
    if not isinstance(port, int) or not 1 <= port <= 65535:
        problems.append("ssh_port invalide (1-65535)")
    ident = cfg.get("identity_file")
    if ident:
        if not Path(os.path.expanduser(ident)).is_file():
            problems.append(f"clé SSH introuvable : {ident}")
        if " " in ident:
            problems.append("le chemin de la clé contient un espace (rsync -e le coupera)")
        if ident.startswith("~"):
            problems.append("publish.py n'expanse pas « ~ » : utiliser un chemin absolu")
    return problems


def save_deploy(path: Path, cfg: dict[str, Any]) -> None:
    tmp = path.with_name(path.name + ".tmp")
    tmp.write_text(json.dumps(cfg, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    os.replace(tmp, path)


def ssh_base(cfg: dict[str, Any]) -> list[str]:
    argv = ["ssh", "-p", str(cfg.get("ssh_port", 22)), "-o", "BatchMode=yes", "-o", "ConnectTimeout=8"]
    if cfg.get("identity_file"):
        argv += ["-i", os.path.expanduser(cfg["identity_file"])]
    return argv + [f"{cfg['user']}@{cfg['host']}"]


def ssh_test(cfg: dict[str, Any]) -> tuple[str, str]:
    """Returns (level, message), level in ok / warn / bad."""
    if shutil.which("ssh") is None:
        return "bad", "ssh n'est pas installé"
    remote = (f"test -d {shlex.quote(cfg['remote_root'])} && echo DIR_OK; "
              "command -v rsync >/dev/null 2>&1 && echo RSYNC_OK; true")
    code, out = run_quiet(ssh_base(cfg) + [remote], timeout=20)
    if code != 0:
        last = out.splitlines()[-1] if out else f"code {code}"
        extra = ""
        if "host key" in out.lower():
            extra = " — connecte-toi une fois à la main (ssh user@host) pour accepter la clé du serveur"
        return "bad", f"connexion impossible : {last}{extra}"
    problems = []
    if "DIR_OK" not in out:
        problems.append(f"le dossier {cfg['remote_root']} n'existe pas encore sur le serveur")
    if "RSYNC_OK" not in out:
        problems.append("rsync absent sur le serveur (nécessaire des deux côtés)")
    if problems:
        return "warn", "connexion OK, mais " + " ; ".join(problems)
    return "ok", "connexion OK, dossier distant présent, rsync présent"


# ---------------------------------------------------------------------------
# Build & publish plan
# ---------------------------------------------------------------------------

ACTIONS = {
    "build": "Build seul",
    "build+publish": "Build + publication",
    "publish": "Publier sans rebuild (dist/ tel quel)",
}


@dataclass
class Step:
    kind: str                         # "git" | "sync" | "run"
    label: str
    argv: list[str] = field(default_factory=list)
    code: str = ""
    branch: Optional[str] = None
    serves: bool = False              # long-running local server: Ctrl+C is the normal way out
    clone: bool = False               # "sync" step that starts with a git clone


@dataclass
class Plan:
    steps: list[Step] = field(default_factory=list)
    problems: list[str] = field(default_factory=list)


def build_plan(
    state: dict[str, Any],
    infos: dict[str, CourseInfo],
    engine: EngineInfo,
    engine_target: Optional[str],
    deploy_path: Path,
    deploy_cfg: Optional[dict[str, Any]],
    deps: list[Dep],
    langs_dir: Path,
    py: str = "python3",
) -> Plan:
    """Turn the chosen options into an ordered list of steps (pure: nothing
    is executed here)."""
    plan = Plan()
    opts = state["options"]
    action = state["action"]
    selection = [c for c in state["selection"] if c in infos]
    if not selection:
        plan.problems.append("aucun cours sélectionné (écran 1)")
        return plan

    if engine_target and engine_target != engine.branch:
        if engine.dirty:
            plan.problems.append(f"changer de branche Slovingo impossible : modifications locales sur {engine.branch}")
        else:
            plan.steps.append(Step("git", f"branche Slovingo : {engine.branch} → {engine_target}",
                                   ["git", "switch", engine_target]))

    publishing = action in ("build+publish", "publish")
    mode = deploy_mode(deploy_cfg)
    if publishing:
        problems = validate_deploy(deploy_cfg)
        if problems:
            plan.problems.append("déploiement invalide : " + " ; ".join(problems))
        if mode in ("local", "lan") and len(selection) > 1:
            plan.problems.append("serveur local/LAN : un seul cours à la fois")
        if mode == "remote":
            for dep in deps:
                if dep.key in ("rsync", "ssh") and not dep.found:
                    plan.problems.append(f"{dep.name} manquant (publication distante)")
    for dep in deps:
        if dep.key in ("git", "pillow") and not dep.found:
            plan.problems.append(f"{dep.name} manquant")

    do_pull = bool(opts["pull"])
    for code in selection:
        if action == "publish":
            break  # dist/ is deployed as it is: no clone, no branch change, no pull
        info = infos[code]
        wanted = state["branches"].get(code) or None
        if info.installed and not info.is_git:
            if wanted:
                plan.problems.append(f"{code} : pas un clone git, impossible de passer sur la branche {wanted}")
            continue
        needs_clone = not info.installed
        switching = bool(wanted) and info.installed and wanted != info.branch
        if needs_clone or switching or do_pull:
            if info.installed and info.dirty and (switching or do_pull):
                plan.problems.append(f"{code} : modifications locales, impossible de changer de branche / tirer")
                continue
            what = "cloner" if needs_clone else ("changer de branche et tirer" if switching else "tirer (ff-only)")
            label = f"{code} : {what}" + (f" [{wanted}]" if wanted else "")
            plan.steps.append(Step("sync", label, code=code, branch=wanted, clone=needs_clone))

    for code in selection:
        lang_dir = rel(langs_dir / code)
        flags: list[str] = []
        if action != "publish":
            if opts["force_exercises"]:
                flags.append("--force-exercises")
            if opts["force_icons"]:
                flags.append("--force-icons")
        if action == "build":
            argv = [py, "src/build.py", "--lang-dir", lang_dir, "--static-dir", "static"] + flags
            label = f"{code} : build"
        else:
            argv = [py, "src/publish.py", "--lang-dir", lang_dir] + flags
            if deploy_path.resolve() != (ROOT / "deploy.json").resolve():
                argv += ["--deploy-config", str(deploy_path)]
            if action == "publish":
                argv.append("--skip-build")
            if mode == "remote":
                if opts["execute"]:
                    argv.append("--execute")
                if opts["delete"]:
                    argv.append("--delete")
            label = f"{code} : " + ("publier" if action == "publish" else "build + publication")
            if mode == "remote":
                label += " (envoi réel)" if opts["execute"] else " (essai à blanc)"
            elif mode in ("local", "lan"):
                label += " (serveur local, Ctrl+C pour arrêter)"
        plan.steps.append(Step("run", label, argv, code=code,
                               serves=action != "build" and mode in ("local", "lan")))
    return plan


def step_command(step: Step, langs_dir: Path, base: str = langs.DEFAULT_BASE) -> str:
    """Shell equivalent shown to the user."""
    if step.kind == "sync":
        target = rel(langs_dir / step.code)
        if step.clone:
            branch = f"--branch {shlex.quote(step.branch)} " if step.branch else ""
            return f"git clone {branch}{langs.repo_url(step.code, base)} {target}"
        if step.branch:
            return f"git -C {target} checkout {shlex.quote(step.branch)} && git -C {target} pull --ff-only"
        return f"git -C {target} pull --ff-only"
    return shell_repr(step.argv)


# ---------------------------------------------------------------------------
# The application
# ---------------------------------------------------------------------------

class App:
    def __init__(self, ui: PlainUI, deploy_path: Path, langs_dir: Path, base: Optional[str]) -> None:
        self.ui = ui
        self.deploy_path = deploy_path
        self.langs_dir = langs_dir
        self.base_override = base
        self.state = load_state()
        self.engine_target: Optional[str] = None   # wanted Slovingo branch (None = stay)
        self.infos: dict[str, CourseInfo] = {}
        self.engine = EngineInfo()
        self.deploy: Optional[dict[str, Any]] = None
        self.deps: list[Dep] = []
        self.refresh()

    # -- data ---------------------------------------------------------
    @property
    def base(self) -> str:
        if self.base_override:
            return self.base_override
        if self.state["ssh"]:
            return langs.SSH_BASE
        return os.environ.get("SLOVINGO_REPO_BASE", langs.DEFAULT_BASE)

    def refresh(self) -> None:
        with self.ui.spinner("Lecture de l'état des dépôts"):
            self.engine = engine_info()
            self.infos = {c: course_info(c, d, self.langs_dir) for c, d in known_codes(self.langs_dir)}
            self.deploy = load_deploy(self.deploy_path)
            self.deps = check_deps(deploy_mode(self.deploy) == "remote")
        self.state["selection"] = [c for c in self.state["selection"] if c in self.infos]

    def persist(self) -> None:
        save_state(self.state)

    # -- home ---------------------------------------------------------
    def health_lines(self) -> list[Any]:
        e = self.engine
        engine_state = "modifs locales" if e.dirty else "propre"
        sync = ""
        if e.behind:
            sync += f" ↓{e.behind}"
        if e.ahead:
            sync += f" ↑{e.ahead}"
        lines: list[Any] = [(f"Moteur       {e.branch} @ {e.sha}  ({engine_state}{sync})",
                             "warn" if e.dirty or e.behind else "ok")]

        mode = deploy_mode(self.deploy)
        problems = validate_deploy(self.deploy)
        if mode == "unset":
            lines.append(("Déploiement  non configuré (écran 2)", "warn"))
        else:
            cfg = self.deploy or {}
            target = ({"local": "serveur local (127.0.0.1)", "lan": "serveur LAN (0.0.0.0)"}.get(mode)
                      or f"{cfg.get('user', '?')}@{cfg.get('host', '?')}:{cfg.get('remote_root', '?')}")
            suffix = "" if not problems else "  — " + "; ".join(problems)
            lines.append((f"Déploiement  {target}{suffix}", "ok" if not problems else "bad"))

        missing = missing_required(self.deps)
        optional = [d for d in self.deps if not d.found and d.level == "optional"]
        if missing:
            lines.append(("Dépendances  manquantes : " + ", ".join(d.name for d in missing), "bad"))
        elif optional:
            lines.append(("Dépendances  OK (facultatives absentes : " + ", ".join(d.name for d in optional) + ")", "warn"))
        else:
            lines.append(("Dépendances  tout est là", "ok"))

        installed = sum(1 for i in self.infos.values() if i.installed)
        built = sum(1 for i in self.infos.values() if i.built_at)
        sel = ", ".join(self.state["selection"]) or "aucun"
        lines.append((f"Cours        {installed}/{len(self.infos)} installés, {built} buildés — sélection : {sel}", "accent"))
        return lines

    def courses_rows(self, numbered: bool = False) -> list[list[Any]]:
        rows: list[list[Any]] = []
        for n, info in enumerate(self.infos.values(), 1):
            wanted = self.state["branches"].get(info.code)
            branch_cell: Any = info.branch or ("—", "dim")
            if wanted and wanted != info.branch:
                branch_cell = (f"{info.branch or '—'} → {wanted}", "accent")
            mark = ("●" if info.code in self.state["selection"] else "○", "accent")
            row: list[Any] = [mark] if numbered else []
            if numbered:
                row.insert(0, str(n))
            row += [(info.code, "accent" if info.installed else "dim"), (info.desc, "dim"), branch_cell,
                    course_state_cell(info), age(info.built_at)]
            rows.append(row)
        return rows

    COURSE_HEADERS = ["Cours", "Description", "Branche", "État git", "Dernier build"]

    def draw_home(self) -> None:
        ui = self.ui
        ui.clear()
        ui.banner("control tower — cours, déploiement, build")
        ui.panel("État", self.health_lines())
        ui.table("Cours", self.COURSE_HEADERS, self.courses_rows())
        if not ui.rich:
            ui.say("(pip install rich pour une interface en couleurs)", "dim")

    def run(self) -> int:
        screens: dict[str, Callable[[], None]] = {
            "1": self.screen_courses, "2": self.screen_deploy,
            "3": self.screen_deps, "4": self.screen_build,
        }
        while True:
            self.draw_home()
            self.ui.footer([("1", "Cours"), ("2", "Déploiement"), ("3", "Dépendances"),
                            ("4", "Build & publication"), ("r", "Rafraîchir"), ("q", "Quitter")])
            key = self.ui.key()
            if key in ("q", "esc"):
                self.persist()
                return 0
            if key == "r":
                self.refresh()
            elif key in screens:
                screens[key]()
                self.persist()

    # -- screen 1: courses --------------------------------------------
    def pick_numbers(self, prompt: str) -> list[str]:
        codes = list(self.infos)
        picked: list[str] = []
        for token in self.ui.ask(prompt).replace(",", " ").split():
            if token.isdigit() and 1 <= int(token) <= len(codes):
                picked.append(codes[int(token) - 1])
            elif token in self.infos:
                picked.append(token)
            else:
                self.ui.say(f"ignoré : {token!r}", "warn")
        return picked

    def screen_courses(self) -> None:
        ui = self.ui
        while True:
            ui.clear()
            ui.rule("1 · Cours")
            ui.table("", ["#", "Sél."] + self.COURSE_HEADERS, self.courses_rows(numbered=True))
            ui.say(f"Source : {self.base}/Slovingo-<code>.git  ({'SSH' if self.state['ssh'] else 'HTTPS'})", "dim")
            ui.footer([("t", "cocher/décocher"), ("a", "tout"), ("n", "rien"), ("b", "branche d'un cours"),
                       ("g", "cloner / mettre à jour la sélection"), ("f", "fetch (réseau)"),
                       ("s", "SSH/HTTPS"), ("q", "retour")])
            key = ui.key()
            if key in ("q", "esc"):
                return
            if key == "t":
                for code in self.pick_numbers("Numéros à cocher/décocher (ex. 1 3)"):
                    sel = self.state["selection"]
                    sel.remove(code) if code in sel else sel.append(code)
            elif key == "a":
                self.state["selection"] = list(self.infos)
            elif key == "n":
                self.state["selection"] = []
            elif key == "s":
                self.state["ssh"] = not self.state["ssh"]
            elif key == "b":
                self.choose_course_branch()
            elif key == "f":
                self.fetch_all()
            elif key == "g":
                self.sync_selection()
            self.persist()

    def choose_course_branch(self) -> None:
        picked = self.pick_numbers("Cours (numéro)")
        if not picked:
            return
        code = picked[0]
        info = self.infos[code]
        options: list[str] = []
        if info.installed and info.is_git:
            rc, out = run_quiet(["git", "-C", str(info.path), "branch", "-a", "--format=%(refname:short)"])
            if rc == 0:
                for name in out.splitlines():
                    name = name.strip()
                    name = name[len("origin/"):] if name.startswith("origin/") else name
                    if name and name != "HEAD" and not name.endswith("/HEAD") and name not in options:
                        options.append(name)
        current = self.state["branches"].get(code) or info.branch or None
        if options:
            choice = self.ui.choose(f"Branche pour {code} (vide = annuler, « - » = laisser telle quelle)", options, current)
        else:
            choice = self.ui.ask(f"Branche pour {code} (« - » = laisser telle quelle)") or None
        if choice is None:
            return
        if choice == "-":
            self.state["branches"].pop(code, None)
        elif langs.BRANCH_RE.match(choice):
            self.state["branches"][code] = choice
        else:
            self.ui.say(f"nom de branche invalide : {choice!r}", "bad")
            self.ui.pause()

    def fetch_all(self) -> None:
        with self.ui.spinner("git fetch sur les cours installés"):
            for info in self.infos.values():
                if info.installed and info.is_git:
                    run_quiet(["git", "-C", str(info.path), "fetch", "--quiet", "origin"], timeout=60)
            run_quiet(["git", "-C", str(ROOT), "fetch", "--quiet", "origin"], timeout=60)
        self.refresh()

    def sync_selection(self) -> None:
        ui = self.ui
        if not self.state["selection"]:
            ui.say("Aucun cours sélectionné.", "warn")
            ui.pause()
            return
        ui.rule("Cloner / mettre à jour")
        results: list[list[Any]] = []
        for code in self.state["selection"]:
            branch = self.state["branches"].get(code) or None
            try:
                with ui.spinner(f"{code}"):
                    message = langs.sync(code, branch, self.langs_dir, self.base)
                results.append([code, ("✔ " + message, "ok")])
            except langs.SyncError as exc:
                results.append([code, ("✘ " + str(exc), "bad")])
        ui.table("", ["Cours", "Résultat"], results)
        self.refresh()
        ui.pause()

    # -- screen 2: deployment -----------------------------------------
    def deploy_table(self, cfg: dict[str, Any], dirty: bool) -> None:
        mode = deploy_mode(cfg)
        label = {"local": "local (127.0.0.1 seulement)", "lan": "LAN (visible par le téléphone)",
                 "remote": "serveur distant (rsync via SSH)", "unset": "non défini"}[mode]
        rows: list[list[Any]] = [["Mode", (label, "accent")], ["host", cfg.get("host") or ("—", "dim")]]
        if mode in ("local", "lan"):
            rows.append(["port", str(cfg.get("port", 8000))])
        else:
            rows += [["user", cfg.get("user") or ("—", "dim")], ["ssh_port", str(cfg.get("ssh_port", 22))],
                     ["remote_root", cfg.get("remote_root") or ("—", "dim")],
                     ["identity_file", cfg.get("identity_file") or ("(clé par défaut / ssh-agent)", "dim")]]
        self.ui.table(f"{rel(self.deploy_path)}{'  — modifié, non enregistré' if dirty else ''}", ["Clé", "Valeur"], rows)
        problems = validate_deploy(cfg)
        if problems:
            for p in problems:
                self.ui.say(p, "bad")
        else:
            self.ui.say("Configuration valide pour publish.py", "ok")

    def screen_deploy(self) -> None:
        ui = self.ui
        saved = load_deploy(self.deploy_path)
        draft: dict[str, Any] = dict(DEPLOY_DEFAULTS)
        if saved:
            draft.update(saved)
        note: Optional[tuple[str, str]] = None
        while True:
            ui.clear()
            ui.rule("2 · Déploiement")
            self.deploy_table(draft, draft != (saved or dict(DEPLOY_DEFAULTS)))
            if note:
                ui.say(note[1], note[0])
                note = None
            mode = deploy_mode(draft)
            hints = [("m", "mode")]
            if mode in ("local", "lan"):
                hints += [("p", "port")]
            else:
                hints += [("h", "host"), ("u", "user"), ("p", "port ssh"), ("r", "remote_root"), ("i", "clé SSH")]
            if mode == "remote" and not validate_deploy(draft):
                hints.append(("t", "tester la connexion"))
            hints += [("s", "enregistrer"), ("q", "retour")]
            ui.footer(hints)
            key = ui.key()
            if key in ("q", "esc"):
                changed = draft != (saved or dict(DEPLOY_DEFAULTS))
                if changed and ui.confirm("Modifications non enregistrées. Enregistrer ?", True):
                    key = "s"
                else:
                    break
            if key == "m":
                choice = ui.choose("Mode de publication",
                                   ["local — serveur sur cette machine uniquement",
                                    "lan — serveur sur le réseau local",
                                    "remote — serveur distant via rsync/ssh"])
                if choice:
                    word = choice.split()[0].lower()
                    if word == "local":
                        draft["host"] = "localhost"
                    elif word == "lan":
                        draft["host"] = "lan"
                    elif word == "remote":
                        draft["host"] = "" if deploy_mode(draft) in ("local", "lan", "unset") else draft["host"]
                        value = ui.ask("host du serveur", draft["host"] or None)
                        if value in ("localhost", "lan"):
                            note = ("warn", "« localhost » et « lan » sont les modes locaux, pas un serveur distant")
                        else:
                            draft["host"] = value
            elif key == "h" and mode != "local" and mode != "lan":
                value = ui.ask("host", draft.get("host") or None)
                if value in ("localhost", "lan"):
                    note = ("warn", "« localhost » et « lan » sont les modes locaux : utilise la touche m")
                else:
                    draft["host"] = value
            elif key == "u" and mode == "remote":
                draft["user"] = ui.ask("user SSH", draft.get("user") or None)
            elif key == "p":
                field_name = "port" if mode in ("local", "lan") else "ssh_port"
                default = draft.get(field_name, 8000 if field_name == "port" else 22)
                value = ui.ask(field_name, str(default))
                if value.isdigit():
                    draft[field_name] = int(value)
                else:
                    note = ("bad", "le port doit être un nombre")
            elif key == "r" and mode == "remote":
                draft["remote_root"] = ui.ask("remote_root (chemin absolu sur le serveur)", draft.get("remote_root") or None)
            elif key == "i" and mode == "remote":
                value = ui.ask("clé SSH (chemin, « - » = aucune)", draft.get("identity_file") or None)
                draft["identity_file"] = None if value in ("", "-") else os.path.expanduser(value)
            elif key == "t" and mode == "remote":
                with ui.spinner(f"Connexion à {draft['user']}@{draft['host']}"):
                    level, message = ssh_test(draft)
                note = (level, message)
            elif key == "s":
                problems = validate_deploy(draft)
                if problems and not ui.confirm("La configuration a des problèmes (" + "; ".join(problems) + "). Enregistrer quand même ?", False):
                    note = ("warn", "non enregistré")
                    continue
                try:
                    save_deploy(self.deploy_path, draft)
                    saved = dict(draft)
                    note = ("ok", f"{rel(self.deploy_path)} enregistré")
                except OSError as exc:
                    note = ("bad", f"écriture impossible : {exc}")
        self.refresh()

    # -- screen 3: dependencies ---------------------------------------
    def screen_deps(self) -> None:
        ui = self.ui
        ui.clear()
        ui.rule("3 · Dépendances")
        with ui.spinner("Vérification"):
            self.deps = check_deps(deploy_mode(self.deploy) == "remote")
        ui.table("", ["Outil", "État", "Niveau", "Sert à", "Version / installation"], [dep_row(d) for d in self.deps])
        missing = [d for d in self.deps if not d.found and d.hint]
        if missing:
            manager = detect_platform()
            ui.rule(f"Pour installer ({manager})")
            for dep in missing:
                ui.say(f"{dep.name:<12} {dep.hint}", "warn" if dep.level == "optional" else "bad")
        ui.footer([("q", "retour")])
        ui.key()

    # -- screen 4: build & publish ------------------------------------
    def current_plan(self) -> Plan:
        return build_plan(self.state, self.infos, self.engine, self.engine_target, self.deploy_path,
                          self.deploy, self.deps, self.langs_dir)

    def screen_build(self) -> None:
        ui = self.ui
        while True:
            opts = self.state["options"]
            plan = self.current_plan()
            mode = deploy_mode(self.deploy)
            ui.clear()
            ui.rule("4 · Build & publication")

            def box(flag: bool) -> str:
                return "[x]" if flag else "[ ]"

            publishing = self.state["action"] != "build"
            remote = mode == "remote" and publishing
            lines: list[Any] = [
                (f"Action        {ACTIONS[self.state['action']]}   (a)", "accent"),
                (f"Slovingo      {self.engine.branch}" + (f" → {self.engine_target}" if self.engine_target and self.engine_target != self.engine.branch else "") + "   (e)", None),
                (f"Cours         {', '.join(self.state['selection']) or 'aucun'}   (écran 1)", None),
                (f"{box(opts['pull'])} tirer les cours (ff-only) avant le build   (p)"
                 + ("   — sans effet : pas de rebuild" if self.state["action"] == "publish" else ""), None),
                (f"{box(opts['force_exercises'])} régénérer les exercices   (x)", None),
                (f"{box(opts['force_icons'])} régénérer les icônes   (i)", None),
            ]
            if remote:
                lines += [(f"{box(opts['delete'])} supprimer côté serveur ce qui n'existe plus (--delete)   (d)", None),
                          (f"{box(opts['execute'])} ENVOI RÉEL (sinon : essai à blanc)   (v)", "bad" if opts["execute"] else "dim")]
            elif publishing:
                lines.append((f"Publication : {'serveur LAN' if mode == 'lan' else 'serveur local' if mode == 'local' else 'non configurée'}", "dim"))
            ui.panel("Options", lines)

            if plan.problems:
                ui.panel("Problèmes", [(p, "bad") for p in plan.problems])
            if plan.steps:
                lines_plan: list[Any] = []
                for i, step in enumerate(plan.steps, 1):
                    lines_plan.append((f"{i}. {step.label}", "accent"))
                    lines_plan.append((f"   $ {step_command(step, self.langs_dir, self.base)}", "dim"))
                ui.panel("Commandes prévues", lines_plan)
            hints = [("a", "action"), ("e", "branche Slovingo"), ("p", "pull"), ("x", "exercices"), ("i", "icônes")]
            if remote:
                hints += [("d", "--delete"), ("v", "envoi réel")]
            hints += [("g", "lancer"), ("q", "retour")]
            ui.footer(hints)
            key = ui.key()
            if key in ("q", "esc"):
                return
            if key == "a":
                order = list(ACTIONS)
                self.state["action"] = order[(order.index(self.state["action"]) + 1) % len(order)]
            elif key == "e":
                self.choose_engine_branch()
            elif key == "p":
                opts["pull"] = not opts["pull"]
            elif key == "x":
                opts["force_exercises"] = not opts["force_exercises"]
            elif key == "i":
                opts["force_icons"] = not opts["force_icons"]
            elif key == "d" and remote:
                opts["delete"] = not opts["delete"]
            elif key == "v" and remote:
                opts["execute"] = not opts["execute"]
            elif key == "g":
                if plan.problems:
                    ui.say("Corrige d'abord les problèmes listés.", "bad")
                    ui.pause()
                else:
                    self.execute_plan(plan, remote)
            self.persist()

    def choose_engine_branch(self) -> None:
        branches = engine_branches()
        choice = self.ui.choose("Branche de Slovingo (le moteur)", branches, self.engine.branch)
        if choice is None:
            return
        if choice == self.engine.branch:
            self.engine_target = None
        elif langs.BRANCH_RE.match(choice):
            self.engine_target = choice
        else:
            self.ui.say(f"nom de branche invalide : {choice!r}", "bad")
            self.ui.pause()

    def execute_plan(self, plan: Plan, remote: bool) -> None:
        ui = self.ui
        opts = self.state["options"]
        if remote and opts["execute"]:
            cfg = self.deploy or {}
            if not ui.confirm(f"Envoyer pour de vrai vers {cfg.get('user')}@{cfg.get('host')}:{cfg.get('remote_root')} ?", False):
                ui.say("Annulé.", "warn")
                ui.pause()
                return
        elif not ui.confirm(f"Lancer {len(plan.steps)} étape(s) ?", True):
            return

        results: list[list[Any]] = []
        failed = False
        for i, step in enumerate(plan.steps, 1):
            ui.rule(f"{i}/{len(plan.steps)} · {step.label}")
            started = time.time()
            ok, message = self.run_step(step)
            took = f"{time.time() - started:.1f}s"
            results.append([str(i), step.label, ("✔ " + message if ok else "✘ " + message, "ok" if ok else "bad"), took])
            if not ok:
                failed = True
                break
        if self.engine_target and not failed:
            self.engine_target = None
        ui.rule("Bilan")
        ui.table("", ["#", "Étape", "Résultat", "Durée"], results)
        if failed:
            ui.say("Arrêt à la première erreur : les étapes suivantes n'ont pas été lancées.", "bad")
        self.refresh()
        ui.pause()

    def run_step(self, step: Step) -> tuple[bool, str]:
        if step.kind == "sync":
            try:
                return True, langs.sync(step.code, step.branch, self.langs_dir, self.base)
            except langs.SyncError as exc:
                return False, str(exc)
        argv = list(step.argv)
        if argv and argv[0] == "python3":
            argv[0] = sys.executable
        print(f"$ {shell_repr(step.argv)}", flush=True)
        try:
            code = subprocess.call(argv, cwd=str(ROOT))
        except KeyboardInterrupt:
            return step.serves, "serveur arrêté" if step.serves else "interrompu (Ctrl+C)"
        except OSError as exc:
            return False, str(exc)
        return code == 0, "terminé" if code == 0 else f"code de sortie {code}"


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main(argv: Optional[list[str]] = None) -> int:
    parser = argparse.ArgumentParser(
        description="Slovingo control tower: courses, deployment, dependencies, build & publish.",
    )
    parser.add_argument("--status", action="store_true", help="print the home screen once and exit")
    parser.add_argument("--deps", action="store_true", help="print the dependency report and exit (1 if a required one is missing)")
    parser.add_argument("--plain", action="store_true", help="never use rich")
    parser.add_argument("--deploy-config", type=Path, default=ROOT / "deploy.json", help="deploy.json location")
    parser.add_argument("--langs-dir", type=Path, default=ROOT / "langs", help="courses directory (default: langs/)")
    parser.add_argument("--base", default=None, help="course repository base URL (see langs.py)")
    args = parser.parse_args(argv)

    ui = make_ui(args.plain)
    app = App(ui, args.deploy_config.resolve(), args.langs_dir.resolve(), args.base)

    if args.deps:
        ui.table("", ["Outil", "État", "Niveau", "Sert à", "Version / installation"], [dep_row(d) for d in app.deps])
        return 1 if missing_required(app.deps) else 0
    if args.status:
        app.draw_home()
        return 0
    if not sys.stdin.isatty():
        print("Erreur : l'interface interactive demande un terminal (essaie --status ou --deps).", file=sys.stderr)
        return 2
    try:
        return app.run()
    except KeyboardInterrupt:
        app.persist()
        print()
        return 130


if __name__ == "__main__":
    sys.exit(main())
