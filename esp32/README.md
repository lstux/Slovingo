# Slovingo sur ESP32

Portage de Slovingo sur des appareils ESP32 à écran.

- `firmware/` : firmware Slovingo pour la LilyGo T-HMI (ESP32-S3, écran 2,8" tactile résistif, carte SD). Compilé en `.bin` et lancé depuis le [Launcher](https://github.com/bmorcelli/Launcher) (installation depuis la carte SD).
- `twatch-app/` : app Slovingo pour la LilyGo T-Watch 2020 v2 (fork `lstux/T-Watch-2020`).
- `zajka-run/` : Zajka Run, jeu de plateforme pour l'ODROID-GO dans l'univers de Zajka Friends (`.fw` pour le firmware d'origine).

Le firmware et l'app récupèrent en JSON les cours publiés via GitHub Pages ; Zajka Run est autonome.
