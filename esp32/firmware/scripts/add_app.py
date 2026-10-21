# Ajoute au build le code de l'app Slovingo partage avec la montre (../twatch-app/slovingo).
#
# slovingo_app.cpp (l'enregistrement dans la grille d'apps du firmware de la montre) n'est pas
# compile ici : src/main.cpp appelle directement slovingo_app_main_setup().
#
# Script PRE (extra_scripts = pre:...) : BuildSources() est refuse une fois le programme
# construit. Mais en PRE, les includes des libs (lvgl, ArduinoJson, WiFi...) ne sont pas encore
# connus : on compile donc l'app au moment ou PlatformIO compile src/, avec le meme env.
from os.path import join, isdir

Import("env")

app_dir = join(env.subst("$PROJECT_DIR"), "..", "twatch-app", "slovingo")
if not isdir(app_dir):
    raise SystemExit("Dossier de l'app introuvable : " + app_dir)

# les en-tetes de l'app (slovingo_*.h, polices, images)
env.Append(CPPPATH=[app_dir])

build_sources = env.BuildSources.method


def build_sources_with_app(e, variant_dir, src_dir, src_filter=None):
    build_sources(e, variant_dir, src_dir, src_filter)
    if src_dir == "$PROJECT_SRC_DIR":
        build_sources(
            e,
            join("$BUILD_DIR", "slovingo_app"),
            app_dir,
            "-<*> +<*.cpp> +<font/*.c> +<images/*.c> -<slovingo_app.cpp>",
        )


env.AddMethod(build_sources_with_app, "BuildSources")
