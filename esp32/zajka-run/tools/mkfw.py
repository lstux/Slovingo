#!/usr/bin/env python3
"""Fabrique un .fw pour le firmware d'origine de l'ODROID-GO (portage de tools/mkfw de
OtherCrashOverride/odroid-go-firmware).

    python3 tools/mkfw.py firmware.bin -o ZajkaRun.fw

Format : en-tete « ODROIDGO_FIRMWARE_V00_01 », description (40 octets), icone 86x48 en
RGB565 petit-boutiste, puis pour chaque partition {type, sous-type, 2 octets reserves,
etiquette[16], drapeaux, taille} + longueur des donnees + donnees ; enfin un CRC32 (zlib)
de tout le fichier. Les tailles de partition doivent etre des multiples de 64 Ko.
"""
import argparse
import os
import struct
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
HEADER = b"ODROIDGO_FIRMWARE_V00_01"
TILE_W, TILE_H = 86, 48


def tile_bytes(path):
    """Icone : .raw deja en RGB565 (86x48x2 octets), ou image quelconque (Pillow requis)."""
    if path.endswith(".raw"):
        data = open(path, "rb").read()
        assert len(data) == TILE_W * TILE_H * 2, "icone .raw : 86x48 pixels RGB565 attendus"
        return data
    from PIL import Image
    img = Image.open(path).convert("RGB")
    if img.size != (TILE_W, TILE_H):
        img = img.resize((TILE_W, TILE_H), Image.LANCZOS)
    out = bytearray()
    for y in range(TILE_H):
        for x in range(TILE_W):
            r, g, b = img.getpixel((x, y))
            out += struct.pack("<H", ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))
    return bytes(out)


def partition(ptype, subtype, size, label, data):
    assert size % 0x10000 == 0, "taille de partition : multiple de 64 Ko"
    assert len(data) <= size, f"{label} : {len(data)} octets > partition de {size}"
    lab = label.encode()[:16].ljust(16, b"\0")
    return struct.pack("<BBBB16sII", ptype, subtype, 0, 0, lab, 0, size) + struct.pack("<I", len(data)) + data


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("app", help="binaire de l'application (sortie d'esptool elf2image / PlatformIO)")
    ap.add_argument("-o", "--output", default="ZajkaRun.fw")
    ap.add_argument("-d", "--description", default="Zajka Run")
    ap.add_argument("-t", "--tile", default=os.path.join(ROOT, "art", "fw_tile.raw"))
    ap.add_argument("--app-size", type=lambda s: int(s, 0), default=0x300000)
    a = ap.parse_args()
    app = open(a.app, "rb").read()
    desc = a.description.encode()[:39].ljust(40, b"\0")
    body = HEADER + desc + tile_bytes(a.tile)
    body += partition(0, 16, a.app_size, "app", app)        # app, ota_0
    body += partition(1, 2, 0x10000, "nvs", b"")            # data, nvs (efface par l'appli au besoin)
    crc = zlib.crc32(body) & 0xFFFFFFFF
    with open(a.output, "wb") as f:
        f.write(body + struct.pack("<I", crc))
    print(f"{a.output} : {len(body) + 4} octets, appli {len(app)} octets, crc {crc:#010x}")


if __name__ == "__main__":
    main()
