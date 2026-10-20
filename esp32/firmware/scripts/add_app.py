# Ajoute au build le code de l'app Slovingo partage avec la montre (../twatch-app/slovingo).
#
# slovingo_app.cpp (l'enregistrement dans la grille d'apps du firmware de la montre) n'est pas
# compile ici : src/main.cpp appelle directement slovingo_app_main_setup().
from os.path import join, isdir

Import("env")

app_dir = join(env.subst("$PROJECT_DIR"), "..", "twatch-app", "slovingo")
if not isdir(app_dir):
    raise SystemExit("Dossier de l'app introuvable : " + app_dir)

# les en-tetes de l'app (slovingo_*.h, polices, images)
env.Append(CPPPATH=[app_dir])

env.BuildSources(
    join("$BUILD_DIR", "slovingo_app"),
    app_dir,
    src_filter="-<*> +<*.cpp> +<font/*.c> +<images/*.c> -<slovingo_app.cpp>",
)
