# Apres le build : fabrique ZajkaRun.fw (firmware d'origine de l'ODROID-GO) a cote de firmware.bin.
# Tourne avec le Python de PlatformIO : tools/mkfw.py n'a pas besoin de Pillow avec l'icone .raw.
import os
import subprocess
import sys

Import("env")


def make_fw(source, target, env):
    build = env.subst("$BUILD_DIR")
    proj = env.subst("$PROJECT_DIR")
    out = os.path.join(build, "ZajkaRun.fw")
    subprocess.check_call([sys.executable, os.path.join(proj, "tools", "mkfw.py"),
                           os.path.join(build, "firmware.bin"), "-o", out,
                           "-t", os.path.join(proj, "art", "fw_tile.raw")])


env.AddPostAction("$BUILD_DIR/firmware.bin", make_fw)
