#!/usr/bin/env python3
"""publish.py

Build one or more languages (see build.py -- same lang-dir convention:
langs/<code>/{md,json,dist,img,lang.json}) and deploy each dist/ to
the configured remote server with rsync.

Deployment target = deploy.json's remote_root + lang.json's
site.url_path, e.g. /var/www/lslinux.org/htdocs + /slovingo/sk-fr/.

Safety: by default this is a DRY RUN (rsync --dry-run) that only
prints what would change. Pass --execute to actually deploy. This
mirrors the exercises/icons persistence guards elsewhere in the
toolchain: nothing gets overwritten -- here, nothing gets sent --
without an explicit, deliberate flag.

rsync is a Linux/macOS tool (WSL on Windows); this is a deliberate
exception to the "pure Python, cross-platform" rule that governs the
rest of the toolchain -- deployment is a one-off, personal step run
from Eric's own machine, not something contributors need portable.

Usage (run from the repo root -- --static-dir and --deploy-config
default to ./static and ./deploy.json):
    # One language:
    python3 src/publish.py --lang-dir langs/sk-fr

    # Every language dir under langs/:
    python3 src/publish.py --langs-root langs

    # For real (default is a dry run):
    python3 src/publish.py ... --execute
"""

from __future__ import annotations

import argparse
import json
import shutil
import subprocess
import sys
from pathlib import Path

from build import discover_lang_dirs, run_build
from smd2data import SheetNameError


def build_rsync_command(
    dist_dir: Path, deploy_cfg: dict, url_path: str, execute: bool, delete: bool
) -> list[str]:
    """Build the rsync command line for one language's deployment.

    Args:
        dist_dir: The local, already-built site to upload.
        deploy_cfg: Loaded deploy.json (host, user, ssh_port,
            remote_root, identity_file).
        url_path: This language's site.url_path from lang.json (e.g.
            "/slovingo/sk-fr/"), appended to remote_root to get the
            actual remote target directory.
        execute: If False, add --dry-run (nothing is actually sent).
        delete: If True, add --delete (remove remote files no longer
            present locally -- stale exercise files, old icons...).

    Returns:
        The full command as a list, ready for subprocess.run().
    """
    remote_root = deploy_cfg["remote_root"].rstrip("/")
    remote_path = f"{remote_root}{url_path}"
    target = f"{deploy_cfg['user']}@{deploy_cfg['host']}:{remote_path}"

    ssh_cmd = f"ssh -p {deploy_cfg.get('ssh_port', 22)}"
    if deploy_cfg.get("identity_file"):
        ssh_cmd += f" -i {deploy_cfg['identity_file']}"

    cmd = ["rsync", "-avz", "-e", ssh_cmd]
    if delete:
        cmd.append("--delete")
    if not execute:
        cmd.append("--dry-run")
    cmd.append(f"{dist_dir}/")
    cmd.append(target)
    return cmd


def publish_one(lang_dir: Path, deploy_cfg: dict, execute: bool, delete: bool) -> int:
    """Deploy one already-built language dir. Returns rsync's exit code."""
    with (lang_dir / "lang.json").open(encoding="utf-8") as f:
        lang_cfg = json.load(f)

    cmd = build_rsync_command(lang_dir / "dist", deploy_cfg, lang_cfg["site"]["url_path"], execute, delete)
    print(f"$ {' '.join(cmd)}")
    result = subprocess.run(cmd)
    if result.returncode != 0:
        print(f"Error: rsync exited with status {result.returncode} for {lang_dir.name}", file=sys.stderr)
    return result.returncode


def main() -> None:
    parser = argparse.ArgumentParser(description="Build and deploy one or more languages with rsync.")
    target = parser.add_mutually_exclusive_group(required=True)
    target.add_argument("--lang-dir", type=Path, help="One language directory (e.g. langs/sk-fr)")
    target.add_argument("--langs-root", type=Path, help="Build+deploy every language dir under this directory (e.g. langs)")
    parser.add_argument(
        "--static-dir", type=Path, default=Path("static"),
        help="Directory of static front-end files to copy into dist/ (default: ./static)",
    )
    parser.add_argument(
        "--deploy-config", type=Path, default=Path("deploy.json"),
        help="Path to deploy.json, see deploy.example.json (default: ./deploy.json)",
    )
    parser.add_argument(
        "--execute", action="store_true",
        help="Actually upload (default: dry run, prints what would change without sending anything)",
    )
    parser.add_argument(
        "--delete", action="store_true",
        help="Also remove remote files no longer present locally (stale exercises, old icons...)",
    )
    parser.add_argument("--force-exercises", action="store_true", help="See build.py")
    parser.add_argument("--force-icons", action="store_true", help="See build.py")
    parser.add_argument(
        "--skip-build", action="store_true",
        help="Deploy dist/ as it already is, without rebuilding first",
    )
    args = parser.parse_args()

    if shutil.which("rsync") is None:
        print(
            "Error: rsync not found on this machine. On Windows, run this "
            "from WSL -- see publish.py's module docstring.",
            file=sys.stderr,
        )
        sys.exit(1)

    lang_dirs = discover_lang_dirs(args.langs_root) if args.langs_root else [args.lang_dir]
    if not lang_dirs:
        print(f"Error: no language directory (with a lang.json) found under {args.langs_root}", file=sys.stderr)
        sys.exit(1)

    with args.deploy_config.open(encoding="utf-8") as f:
        deploy_cfg = json.load(f)

    if not args.skip_build:
        try:
            for lang_dir in lang_dirs:
                run_build(
                    lang_dir, args.static_dir,
                    force_exercises=args.force_exercises, force_icons=args.force_icons,
                )
                print()
        except (SheetNameError, ValueError) as exc:
            print(f"Error: {exc}", file=sys.stderr)
            sys.exit(1)

    if not args.execute:
        print("Dry run -- nothing will be sent. Re-run with --execute to actually deploy.")

    worst_exit_code = 0
    for lang_dir in lang_dirs:
        exit_code = publish_one(lang_dir, deploy_cfg, args.execute, args.delete)
        worst_exit_code = worst_exit_code or exit_code

    if worst_exit_code:
        sys.exit(worst_exit_code)


if __name__ == "__main__":
    main()
