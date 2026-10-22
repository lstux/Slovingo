#!/usr/bin/env python3
"""Mini moteur de rendu SVG (sous-ensemble) base sur matplotlib, avec poses.

Couvre ce qu'utilisent les personnages Slovingo / MathPulse (fox-full.svg, rabbit-full.svg) :
<g>, <path>, <ellipse>, <circle>, <rect rx>, <use href>, <defs>, <clipPath>,
attributs fill, stroke, stroke-width, stroke-linecap, opacity, transform, clip-path.
Commandes de chemin : M L H V C S Q T A Z (absolues et relatives).

Pose : un dictionnaire {id de groupe: transformation SVG supplementaire}, appliquee
avant la transformation propre du groupe, par ex. {"foxf-arm-l": "rotate(30 182 356)"}.
Un id peut aussi etre masque : {"foxf-shadow": None}.
"""
import math
import re
import xml.etree.ElementTree as ET

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
import numpy as np  # noqa: E402
from matplotlib.patches import PathPatch  # noqa: E402
from matplotlib.path import Path  # noqa: E402
from matplotlib.transforms import Affine2D  # noqa: E402

SVGNS = "{http://www.w3.org/2000/svg}"
XLINK = "{http://www.w3.org/1999/xlink}href"


def _tag(el):
    return el.tag.replace(SVGNS, "")


# ---------------------------------------------------------------- transformations
def parse_transform(s):
    m = np.identity(3)
    if not s:
        return m
    for name, args in re.findall(r"(\w+)\s*\(([^)]*)\)", s):
        a = [float(x) for x in re.split(r"[\s,]+", args.strip()) if x]
        t = np.identity(3)
        if name == "translate":
            t[0, 2] = a[0]
            t[1, 2] = a[1] if len(a) > 1 else 0
        elif name == "scale":
            t[0, 0] = a[0]
            t[1, 1] = a[1] if len(a) > 1 else a[0]
        elif name == "rotate":
            r = math.radians(a[0])
            c, s_ = math.cos(r), math.sin(r)
            rot = np.array([[c, -s_, 0], [s_, c, 0], [0, 0, 1]])
            if len(a) == 3:
                cx, cy = a[1], a[2]
                t = (np.array([[1, 0, cx], [0, 1, cy], [0, 0, 1]]) @ rot
                     @ np.array([[1, 0, -cx], [0, 1, -cy], [0, 0, 1]]))
            else:
                t = rot
        elif name == "matrix":
            t = np.array([[a[0], a[2], a[4]], [a[1], a[3], a[5]], [0, 0, 1]])
        m = m @ t
    return m


# ---------------------------------------------------------------- chemins
def _arc_to_bezier(x1, y1, rx, ry, phi, fa, fs, x2, y2):
    """Arc elliptique SVG -> liste de segments cubiques [(c1, c2, p)]."""
    if rx == 0 or ry == 0:
        return [((x1, y1), (x2, y2), (x2, y2))]
    phi = math.radians(phi)
    cp, sp = math.cos(phi), math.sin(phi)
    dx, dy = (x1 - x2) / 2, (y1 - y2) / 2
    x1p, y1p = cp * dx + sp * dy, -sp * dx + cp * dy
    rx, ry = abs(rx), abs(ry)
    lam = x1p ** 2 / rx ** 2 + y1p ** 2 / ry ** 2
    if lam > 1:
        rx *= math.sqrt(lam)
        ry *= math.sqrt(lam)
    num = rx * rx * ry * ry - rx * rx * y1p * y1p - ry * ry * x1p * x1p
    den = rx * rx * y1p * y1p + ry * ry * x1p * x1p
    co = math.sqrt(max(0, num / den)) if den else 0
    if fa == fs:
        co = -co
    cxp, cyp = co * rx * y1p / ry, -co * ry * x1p / rx
    cx = cp * cxp - sp * cyp + (x1 + x2) / 2
    cy = sp * cxp + cp * cyp + (y1 + y2) / 2

    def ang(ux, uy, vx, vy):
        a = math.atan2(ux * vy - uy * vx, ux * vx + uy * vy)
        return a

    t1 = ang(1, 0, (x1p - cxp) / rx, (y1p - cyp) / ry)
    dt = ang((x1p - cxp) / rx, (y1p - cyp) / ry, (-x1p - cxp) / rx, (-y1p - cyp) / ry)
    if not fs and dt > 0:
        dt -= 2 * math.pi
    elif fs and dt < 0:
        dt += 2 * math.pi
    n = max(1, int(math.ceil(abs(dt) / (math.pi / 2))))
    d = dt / n
    k = 4 / 3 * math.tan(d / 4)
    out = []
    for i in range(n):
        a1, a2 = t1 + i * d, t1 + (i + 1) * d

        def pt(a, dd=0.0):
            ex = math.cos(a) - dd * math.sin(a)
            ey = math.sin(a) + dd * math.cos(a)
            x, y = rx * ex, ry * ey
            return (cp * x - sp * y + cx, sp * x + cp * y + cy)
        c1 = pt(a1, k)
        e = pt(a2)
        # second point de controle : depuis la fin, en arriere
        ex, ey = math.cos(a2) + k * math.sin(a2), math.sin(a2) - k * math.cos(a2)
        x, y = rx * ex, ry * ey
        c2 = (cp * x - sp * y + cx, sp * x + cp * y + cy)
        out.append((c1, c2, e))
    return out


def parse_path(d):
    toks = re.findall(r"[A-Za-z]|[-+]?(?:\d*\.\d+|\d+\.?)(?:[eE][-+]?\d+)?", d)
    verts, codes = [], []
    i = 0
    cmd = None
    cur = (0.0, 0.0)
    start = (0.0, 0.0)
    last_c = None  # dernier point de controle (pour S/T)
    last_q = None

    def num():
        nonlocal i
        v = float(toks[i])
        i += 1
        return v

    while i < len(toks):
        if re.match(r"[A-Za-z]", toks[i]):
            cmd = toks[i]
            i += 1
            if cmd in "Zz":
                codes.append(Path.CLOSEPOLY)
                verts.append(start)
                cur = start
                last_c = last_q = None
                continue
        rel = cmd.islower()
        C = cmd.upper()
        ox, oy = cur if rel else (0, 0)
        if C == "M":
            p = (num() + ox, num() + oy)
            codes.append(Path.MOVETO)
            verts.append(p)
            cur = start = p
            cmd = "l" if rel else "L"
            last_c = last_q = None
        elif C == "L":
            p = (num() + ox, num() + oy)
            codes.append(Path.LINETO)
            verts.append(p)
            cur = p
            last_c = last_q = None
        elif C == "H":
            x = num() + (cur[0] if rel else 0)
            cur = (x, cur[1])
            codes.append(Path.LINETO)
            verts.append(cur)
            last_c = last_q = None
        elif C == "V":
            y = num() + (cur[1] if rel else 0)
            cur = (cur[0], y)
            codes.append(Path.LINETO)
            verts.append(cur)
            last_c = last_q = None
        elif C in "CS":
            if C == "C":
                c1 = (num() + ox, num() + oy)
            else:
                c1 = (2 * cur[0] - last_c[0], 2 * cur[1] - last_c[1]) if last_c else cur
            c2 = (num() + ox, num() + oy)
            p = (num() + ox, num() + oy)
            codes += [Path.CURVE4] * 3
            verts += [c1, c2, p]
            last_c = c2
            last_q = None
            cur = p
        elif C in "QT":
            if C == "Q":
                q = (num() + ox, num() + oy)
            else:
                q = (2 * cur[0] - last_q[0], 2 * cur[1] - last_q[1]) if last_q else cur
            p = (num() + ox, num() + oy)
            codes += [Path.CURVE3] * 2
            verts += [q, p]
            last_q = q
            last_c = None
            cur = p
        elif C == "A":
            rx, ry, phi, fa, fs = num(), num(), num(), num(), num()
            p = (num() + ox, num() + oy)
            for c1, c2, e in _arc_to_bezier(cur[0], cur[1], rx, ry, phi, int(fa), int(fs), p[0], p[1]):
                codes += [Path.CURVE4] * 3
                verts += [c1, c2, e]
            cur = p
            last_c = last_q = None
        else:
            raise ValueError("commande de chemin non geree : " + cmd)
    return Path(verts, codes)


def ellipse_path(cx, cy, rx, ry):
    return Path.unit_circle().transformed(Affine2D().scale(rx, ry).translate(cx, cy))


def rect_path(x, y, w, h, rx=0.0, ry=None):
    ry = rx if ry is None else ry
    rx, ry = min(rx, w / 2), min(ry, h / 2)
    if rx <= 0:
        return Path([(x, y), (x + w, y), (x + w, y + h), (x, y + h), (x, y)],
                    [Path.MOVETO, Path.LINETO, Path.LINETO, Path.LINETO, Path.CLOSEPOLY])
    k = 0.5523
    d = (f"M{x+rx} {y} H{x+w-rx} C{x+w-rx+k*rx} {y} {x+w} {y+ry-k*ry} {x+w} {y+ry} "
         f"V{y+h-ry} C{x+w} {y+h-ry+k*ry} {x+w-rx+k*rx} {y+h} {x+w-rx} {y+h} "
         f"H{x+rx} C{x+rx-k*rx} {y+h} {x} {y+h-ry+k*ry} {x} {y+h-ry} "
         f"V{y+ry} C{x} {y+ry-k*ry} {x+rx-k*rx} {y} {x+rx} {y}Z")
    return parse_path(d)


# ---------------------------------------------------------------- rendu
INHERIT = ("fill", "stroke", "stroke-width", "stroke-linecap", "stroke-linejoin")


class Renderer:
    def __init__(self, svg_file):
        self.tree = ET.parse(svg_file)
        self.root = self.tree.getroot()
        vb = [float(v) for v in self.root.get("viewBox").split()]
        self.vb = vb
        self.ids = {}
        for el in self.root.iter():
            i = el.get("id")
            if i and i not in self.ids:  # premier gagne (ids dupliques dans fox-full)
                self.ids[i] = el

    def element_path(self, el):
        t = _tag(el)
        g = lambda k, d=0.0: float(el.get(k, d))  # noqa: E731
        if t == "path":
            return parse_path(el.get("d"))
        if t == "ellipse":
            return ellipse_path(g("cx"), g("cy"), g("rx"), g("ry"))
        if t == "circle":
            return ellipse_path(g("cx"), g("cy"), g("r"), g("r"))
        if t == "rect":
            rx = el.get("rx")
            ry = el.get("ry")
            rxv = float(rx) if rx else (float(ry) if ry else 0)
            ryv = float(ry) if ry else rxv
            return rect_path(g("x"), g("y"), g("width"), g("height"), rxv, ryv)
        return None

    def clip_path(self, ref, m):
        cid = re.match(r"url\(#([^)]+)\)", ref).group(1)
        cp = self.ids[cid]
        paths = []

        def walk(el, mm):
            mm = mm @ parse_transform(el.get("transform"))
            t = _tag(el)
            if t == "use":
                href = (el.get("href") or el.get(XLINK))[1:]
                walk(self.ids[href], mm)
                return
            p = self.element_path(el)
            if p is not None:
                paths.append(p.transformed(Affine2D(mm)))
            for c in el:
                walk(c, mm)
        for c in cp:
            walk(c, m)
        return Path.make_compound_path(*paths)

    def render(self, height_px, pose=None, width_px=None, margin=0, crop=None):
        """Rendu RGBA (numpy uint8). crop = (x, y, w, h) en unites SVG (sinon viewBox)."""
        pose = pose or {}
        x0, y0, w, h = crop or self.vb
        scale = (height_px - 2 * margin) / h
        if width_px is None:
            width_px = int(round(w * scale)) + 2 * margin
        dpi = 100
        fig = plt.figure(figsize=(width_px / dpi, height_px / dpi), dpi=dpi)
        ax = fig.add_axes([0, 0, 1, 1])
        ax.set_xlim(x0 - margin / scale, x0 - margin / scale + width_px / scale)
        ax.set_ylim(y0 - margin / scale + height_px / scale, y0 - margin / scale)
        ax.axis("off")
        fig.patch.set_alpha(0)
        ax.patch.set_alpha(0)
        self._ax = ax
        self._px_per_unit = scale
        self._z = 0
        root_style = {"fill": "#000", "stroke": "none", "stroke-width": "1"}
        for c in self.root:
            self._draw(c, np.identity(3), root_style, 1.0, None, pose)
        fig.canvas.draw()
        img = np.asarray(fig.canvas.buffer_rgba()).copy()
        plt.close(fig)
        return img

    def _draw(self, el, m, style, opacity, clip, pose):
        t = _tag(el)
        if t in ("defs", "clipPath", "title", "desc"):
            return
        eid = el.get("id")
        if eid in pose and self.ids.get(eid) is el:
            if pose[eid] is None:
                return
            m = m @ parse_transform(pose[eid])
        m = m @ parse_transform(el.get("transform"))
        style = dict(style)
        for k in INHERIT:
            if el.get(k) is not None:
                style[k] = el.get(k)
        if el.get("opacity"):
            opacity *= float(el.get("opacity"))
        if el.get("clip-path"):
            clip = self.clip_path(el.get("clip-path"), m)
        if t == "use":
            href = (el.get("href") or el.get(XLINK))[1:]
            self._draw(self.ids[href], m, style, opacity, clip, pose)
            return
        p = self.element_path(el)
        if p is not None:
            self._paint(p, m, style, opacity, clip)
        for c in el:
            self._draw(c, m, style, opacity, clip, pose)

    def _paint(self, p, m, style, opacity, clip):
        tp = p.transformed(Affine2D(m))
        det = abs(np.linalg.det(m[:2, :2])) ** 0.5
        ax = self._ax
        fill = style.get("fill", "#000")
        if fill and fill != "none":
            patch = PathPatch(tp, facecolor=fill, edgecolor="none", alpha=opacity,
                              zorder=self._z, antialiased=True)
            self._z += 1
            ax.add_patch(patch)
            if clip is not None:
                patch.set_clip_path(clip, ax.transData)
        stroke = style.get("stroke", "none")
        if stroke and stroke != "none":
            sw = float(style.get("stroke-width", 1)) * det * self._px_per_unit * 72 / 100
            cap = {"round": "round", "square": "projecting"}.get(style.get("stroke-linecap"), "butt")
            join = {"round": "round", "bevel": "bevel"}.get(style.get("stroke-linejoin"), "miter")
            patch = PathPatch(tp, facecolor="none", edgecolor=stroke, linewidth=sw, alpha=opacity,
                              capstyle=cap, joinstyle=join, zorder=self._z, antialiased=True)
            self._z += 1
            ax.add_patch(patch)
            if clip is not None:
                patch.set_clip_path(clip, ax.transData)


if __name__ == "__main__":
    import sys
    from PIL import Image
    r = Renderer(sys.argv[1])
    Image.fromarray(r.render(int(sys.argv[3]) if len(sys.argv) > 3 else 400)).save(sys.argv[2])
