#!/usr/bin/env python3
"""Local editor for the adventure-map step positions of a course.

Usage:
    python3 tools/map_editor.py path/to/course-dir [--port 8765] [--no-open]

course-dir is a course folder (lang.json, md/, img/), the same one given
to build.py. The editor lists the steps with the engine's own code
(map_generator.build_map_json), shows the course's img/map_background.*
and the steps' icons, and lets you drag each step on the map.

"Save" writes map.positions (and map.direction) in the course's lang.json,
changing nothing else in the file. Only the steps you pinned are written:
the others keep following the automatic zigzag.

The server listens on 127.0.0.1 only and needs no dependency.
"""

from __future__ import annotations

import argparse
import json
import mimetypes
import secrets
import sys
import threading
import webbrowser
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from typing import Any

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "src"))

from map_generator import (  # noqa: E402
    MAP_DIRECTIONS,
    build_map_json,
    find_map_background,
    generate_positions,
)

PAGE = Path(__file__).resolve().parent / "map-editor.html"
_DECODER = json.JSONDecoder()


# ----------------------------------------------------------------------
# lang.json: surgical edit of map.positions / map.direction
# ----------------------------------------------------------------------

def _skip_ws(text: str, pos: int) -> int:
    while pos < len(text) and text[pos] in " \t\r\n":
        pos += 1
    return pos


def _object_members(text: str, start: int) -> tuple[list[dict[str, Any]], int]:
    """Members of the JSON object whose '{' is at text[start].

    Returns:
        ([{"key", "value_start", "value_end"}...], index of the closing '}').
    """
    assert text[start] == "{"
    members: list[dict[str, Any]] = []
    pos = _skip_ws(text, start + 1)
    while text[pos] != "}":
        key, pos = _DECODER.raw_decode(text, pos)
        pos = _skip_ws(text, pos)
        assert text[pos] == ":"
        pos = _skip_ws(text, pos + 1)
        value_start = pos
        _, pos = _DECODER.raw_decode(text, pos)
        members.append({"key": key, "value_start": value_start, "value_end": pos})
        pos = _skip_ws(text, pos)
        if text[pos] == ",":
            pos = _skip_ws(text, pos + 1)
    return members, pos


def _indent_of(text: str, index: int) -> str:
    """Leading whitespace of the line containing text[index]."""
    line_start = text.rfind("\n", 0, index) + 1
    end = line_start
    while end < len(text) and text[end] in " \t":
        end += 1
    return text[line_start:end]


def _number(value: float) -> str:
    rounded = round(float(value), 1)
    return str(int(rounded)) if rounded == int(rounded) else repr(rounded)


def _format_positions(positions: dict[str, list[float]], indent: str) -> str:
    """The positions object, one step per line, at the given indentation."""
    if not positions:
        return "{}"
    inner = indent + "  "
    lines = [
        f"{inner}{json.dumps(step_id, ensure_ascii=False)}: [{_number(x)}, {_number(y)}]"
        for step_id, (x, y) in positions.items()
    ]
    return "{\n" + ",\n".join(lines) + "\n" + indent + "}"


def _set_member(text: str, obj_start: int, key: str, value_text: str) -> str:
    """Replace or insert `key` in the object at text[obj_start] == '{'."""
    members, close = _object_members(text, obj_start)
    for member in members:
        if member["key"] == key:
            return text[: member["value_start"]] + value_text + text[member["value_end"]:]
    if members:
        indent = _indent_of(text, members[0]["value_start"])
        # Insert before the first member, keeping the file's own style.
        first_key = text.index('"', obj_start)
        return text[:first_key] + f'{json.dumps(key)}: {value_text},\n{indent}' + text[first_key:]
    outer = _indent_of(text, obj_start)
    return text[: obj_start + 1] + f"\n{outer}  {json.dumps(key)}: {value_text}\n{outer}" + text[close:]


def write_map_settings(lang_path: Path, positions: dict[str, list[float]], direction: str) -> None:
    """Write map.positions and map.direction into lang.json, nothing else.

    The change is verified (the parsed result must equal the old file plus
    the new map settings) before the file is replaced.
    """
    old_text = lang_path.read_text(encoding="utf-8")
    old = json.loads(old_text)
    root_start = _skip_ws(old_text, 0)
    members, _ = _object_members(old_text, root_start)
    map_member = next((m for m in members if m["key"] == "map"), None)

    text = old_text
    if map_member is None:
        if not positions and direction == "down":
            return  # nothing to write: the defaults apply
        indent = _indent_of(text, members[0]["value_start"]) if members else "  "
        entries = []
        if direction != "down":
            entries.append(f'{indent}  "direction": {json.dumps(direction)}')
        if positions:
            entries.append(f'{indent}  "positions": {_format_positions(positions, indent + "  ")}')
        block = f'{json.dumps("map")}: {{\n' + ",\n".join(entries) + f'\n{indent}}},\n{indent}'
        first_key = text.index('"', root_start)
        text = text[:first_key] + block + text[first_key:]
    else:
        map_start = map_member["value_start"]
        map_indent = _indent_of(text, map_start)
        # direction first, then positions: each call re-reads the new text.
        current_direction = old.get("map", {}).get("direction", "down")
        if direction != current_direction:
            text = _set_member(text, map_start, "direction", json.dumps(direction))
        if positions or "positions" in old.get("map", {}):
            member_indent = map_indent + "  "
            text = _set_member(text, map_start, "positions", _format_positions(positions, member_indent))

    expected = json.loads(old_text)
    expected_map = expected.setdefault("map", {})
    if direction != expected_map.get("direction", "down"):
        expected_map["direction"] = direction
    if positions or "positions" in expected_map:
        expected_map["positions"] = {k: [round(float(v), 1) for v in xy] for k, xy in positions.items()}
    new = json.loads(text)  # raises if the edit broke the JSON
    if new != expected:
        raise ValueError("refusing to write lang.json: the edit would change other keys")

    tmp = lang_path.with_suffix(".json.tmp")
    tmp.write_text(text, encoding="utf-8")
    tmp.replace(lang_path)


# ----------------------------------------------------------------------
# Course data
# ----------------------------------------------------------------------

class Course:
    def __init__(self, course_dir: Path) -> None:
        self.dir = course_dir.resolve()
        self.lang_path = self.dir / "lang.json"
        self.md_dir = self.dir / "md"
        self.img_dir = self.dir / "img"
        for needed in (self.lang_path, self.md_dir):
            if not needed.exists():
                raise SystemExit(f"Not a course folder (missing {needed.name}): {self.dir}")

    def describe(self) -> dict[str, Any]:
        """Steps as the site shows them, plus what the editor needs."""
        lang_cfg = json.loads(self.lang_path.read_text(encoding="utf-8"))
        map_cfg = lang_cfg.get("map", {})
        direction = map_cfg.get("direction", "down")
        pinned_ids = set(map_cfg.get("positions", {}))
        data = build_map_json(self.md_dir, lang_cfg)
        transform = MAP_DIRECTIONS[direction]
        autos = generate_positions(len(data["steps"]))
        steps = []
        for step, auto in zip(data["steps"], autos):
            ax, ay = transform(*auto)
            steps.append(
                {
                    "id": step["id"],
                    "title": step["title"],
                    "icon": step.get("icon", ""),
                    "type": step["type"],
                    "x": step["x"],
                    "y": step["y"],
                    "auto": [round(ax, 1), round(ay, 1)],
                    "pinned": step["id"] in pinned_ids,
                }
            )
        background = find_map_background(self.img_dir)
        return {
            "name": self.dir.name,
            "direction": direction,
            "background": f"/{background}" if background else None,
            "steps": steps,
        }


# ----------------------------------------------------------------------
# HTTP
# ----------------------------------------------------------------------

def make_handler(course: Course, token: str) -> type[BaseHTTPRequestHandler]:
    class Handler(BaseHTTPRequestHandler):
        def log_message(self, fmt: str, *args: Any) -> None:  # quiet
            pass

        def _send(self, status: int, body: bytes, content_type: str) -> None:
            self.send_response(status)
            self.send_header("Content-Type", content_type)
            self.send_header("Content-Length", str(len(body)))
            self.send_header("Cache-Control", "no-store")
            self.end_headers()
            self.wfile.write(body)

        def _json(self, status: int, payload: Any) -> None:
            self._send(status, json.dumps(payload, ensure_ascii=False).encode("utf-8"), "application/json; charset=utf-8")

        def do_GET(self) -> None:  # noqa: N802
            path = self.path.split("?", 1)[0]
            if path == "/":
                self._send(200, PAGE.read_bytes(), "text/html; charset=utf-8")
            elif path == "/api/course":
                try:
                    payload = course.describe()
                except Exception as error:  # surfaced in the page
                    self._json(500, {"error": str(error)})
                    return
                payload["token"] = token
                self._json(200, payload)
            elif path.startswith("/img/"):
                name = path[len("/img/"):]
                target = (course.img_dir / name).resolve()
                if target.parent != course.img_dir.resolve() or not target.is_file():
                    self._send(404, b"not found", "text/plain")
                    return
                kind = mimetypes.guess_type(target.name)[0] or "application/octet-stream"
                self._send(200, target.read_bytes(), kind)
            else:
                self._send(404, b"not found", "text/plain")

        def do_POST(self) -> None:  # noqa: N802
            if self.path != "/api/save":
                self._send(404, b"not found", "text/plain")
                return
            if self.headers.get("X-Editor-Token") != token:
                self._json(403, {"error": "bad token"})
                return
            try:
                length = int(self.headers.get("Content-Length", "0"))
                body = json.loads(self.rfile.read(length))
                direction = body["direction"]
                if direction not in MAP_DIRECTIONS:
                    raise ValueError(f"unknown direction {direction!r}")
                known = {step["id"] for step in course.describe()["steps"]}
                positions: dict[str, list[float]] = {}
                for step_id, xy in body["positions"].items():
                    if step_id not in known:
                        raise ValueError(f"unknown step {step_id!r}")
                    x, y = float(xy[0]), float(xy[1])
                    if not (0 <= x <= 100 and 0 <= y <= 100):
                        raise ValueError(f"position out of range for {step_id!r}")
                    positions[step_id] = [x, y]
                write_map_settings(course.lang_path, positions, direction)
            except Exception as error:
                self._json(400, {"error": str(error)})
                return
            self._json(200, {"ok": True, "pinned": len(positions)})

    return Handler


def main() -> None:
    parser = argparse.ArgumentParser(description="Edit the adventure-map step positions of a course.")
    parser.add_argument("course_dir", type=Path, help="Course folder (lang.json, md/, img/)")
    parser.add_argument("--port", type=int, default=8765, help="First port to try (default 8765)")
    parser.add_argument("--no-open", action="store_true", help="Do not open the browser")
    args = parser.parse_args()

    course = Course(args.course_dir)
    token = secrets.token_urlsafe(16)

    server = None
    for port in range(args.port, args.port + 20):
        try:
            server = ThreadingHTTPServer(("127.0.0.1", port), make_handler(course, token))
            break
        except OSError:
            continue
    if server is None:
        raise SystemExit(f"No free port in {args.port}-{args.port + 19}")

    url = f"http://127.0.0.1:{server.server_address[1]}/"
    print(f"Map editor for {course.dir.name}: {url}  (Ctrl+C to stop)")
    if not args.no_open:
        threading.Timer(0.5, lambda: webbrowser.open(url)).start()
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print()


if __name__ == "__main__":
    main()
