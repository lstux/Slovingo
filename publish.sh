#!/bin/sh
BACKUP="${BACKUP:-false}"
GENERATE="${GENERATE:-false}"
DRY_RUN="${DRY_RUN:-false}"
LANG_DIR="${LANG_DIR:-langs/sk-fr}"

usage() {
  exec >&2
  printf "Usage : %s [options]\n" "$(basename "$0")"
  printf "Génère et publie un site de mémos de langue (moteur générique,\n"
  printf "voir langs/<code>/lang.json pour la config d'un cours donné).\n"
  printf "Options :\n"
  printf "  -l DIR : dossier de langue à publier (défaut : %s)\n" "${LANG_DIR}"
  printf "  -b : do backup\n"
  printf "  -g : regenerate html/json\n"
  printf "  -d : dry-run (skip rsync)\n"
  printf "  -h : display this help message\n"
  exit 1
}

while getopts bgdl:h opt; do case "${opt}" in
  b) BACKUP=true;;
  g) GENERATE=true;;
  d) DRY_RUN=true;;
  l) LANG_DIR="${OPTARG}";;
  *) usage;;
esac; done

if [ ! -f "${LANG_DIR}/lang.json" ]; then
  printf "Introuvable : %s/lang.json — dossier de langue invalide ?\n" "${LANG_DIR}" >&2
  exit 1
fi

# Toutes les valeurs qui dépendent de la langue apprise (lang.json)
# viennent de ce seul dossier — voir src/langconfig.py.
export LANG_CONFIG_PATH="${LANG_DIR}/lang.json"

MD_DIR="${LANG_DIR}/md"
HTML_DIR="${LANG_DIR}/html"
PARCOURS="${LANG_DIR}/parcours.txt"
BACKUPS_DIR="${LANG_DIR}/backups"

mkdir -p "${HTML_DIR}"

# ---------------------------------------------------------------------------
# Config langue (${LANG_DIR}/lang.json — voir src/langconfig.py)
#
# publish.sh ne connaît aucune langue en dur : tout ce qui dépend de la
# langue apprise (titre du site, drapeau, hôte/chemin de déploiement,
# footer...) est lu ici une bonne fois pour toutes.
# ---------------------------------------------------------------------------

lang_field() {
  python3 src/lang_field.py "$1" || exit 5
}

SITE_TITLE="$(lang_field site.title)"
TARGET_FLAG="$(lang_field target_lang.flag)"
DEPLOY_HOST="$(lang_field site.deploy_host)"
DEPLOY_PATH="$(lang_field site.deploy_path)"
THEME_COLOR="$(lang_field site.theme_color)"
INDEX_FOOTER="$(lang_field --index-footer)"

# Remplace "_" et "-" par des espaces, puis met en
# majuscule la première lettre du titre obtenu.
format_title() {
  sed -e "s/[_-]/ /g" | awk '{ print toupper(substr($0,1,1)) substr($0,2) }'
}

# Émoji indiquant la nature de la fiche, pour garder l'information
# "type" alors que le classement, lui, suit désormais l'ordre
# pédagogique. Seul "Introduction_*" dépend de la langue apprise
# (le drapeau de la langue cible) ; le reste est une convention de
# nommage de contenu, indépendante de la langue.
card_emoji() {
  case "$1" in
    Introduction_*) printf "%s" "${TARGET_FLAG}" ;;
    Vocabulaire_*)  printf "📖" ;;
    Grammaire_*)    printf "🧩" ;;
    Conjugaison_*)  printf "🔀" ;;
    Dialogues_*)    printf "💬" ;;
    Situations_*)   printf "🎭" ;;
    Revisions_*)    printf "📝" ;;
    Serie_*)        printf "🧵" ;;
    Adulte_*)       printf "🌶️" ;;
    Endy_*)         printf "💛" ;;
    *)              printf "🗂️" ;;
  esac
}

# Thème visuel d'une fiche à partir de son nom de base, en miroir de
# detect_theme() dans smd2html.py : une fiche de série (Serie_08_Tatry_...)
# suit le thème de son unité, tout le reste retombe sur "uvod".
theme_key() {
  case "$1" in
    Serie_*)
      printf "%s" "$1" | sed -E "s/^Serie_[0-9]+_([A-Za-z]+)_.*/\1/" | tr "[:upper:]" "[:lower:]"
      ;;
    *)
      printf "uvod"
      ;;
  esac
}

# Titre lisible d'une fiche à partir de son nom de base.
card_title() {
  printf "%s" "$1" | sed -e "s/^[A-Za-z0-9]\+_[0-9]*_//"| format_title
}

# ---------------------------------------------------------------------------
# Génération de l'index
#
#   $1 : dossier html à indexer
#   $2 : fichier de parcours (sommaire ordonné)
#   $3 : titre de la page
#   $4 : lien "retour" optionnel (href) — vide si aucun
#
# L'ordre des fiches est celui du parcours, pas celui du nom de fichier :
# on arrive sur la page, on prend la première non faite, on avance.
# ---------------------------------------------------------------------------

index_generator() {
  htmldir="$1"
  parcours="$2"
  pagetitle="$3"
  backlink="$4"

  tmpdir="$(mktemp -d)"
  : > "${tmpdir}/present"
  : > "${tmpdir}/used"
  : > "${tmpdir}/body"
  : > "${tmpdir}/rest"

  total=0
  printf "Building index (%s)...\n" "${htmldir}" >&2
  for f in "${htmldir}"/*.html; do
    [ -e "${f}" ] || continue
    fname="$(basename "${f}")"
    case "${fname}" in
      index.html|exercises.html) continue ;;
    esac
    printf "%s\n" "${fname%.html}" >> "${tmpdir}/present"
    total=$((total + 1))
  done

  # --- Sections du parcours ---
  # Une unité n'est écrite que si au moins une de ses fiches existe :
  # on accumule d'abord ses cartes, on la vide ensuite si elle est vide.
  # Chaque section reçoit le data-theme de sa première fiche : les
  # variables CSS du thème ne s'appliquent qu'à ce sous-arbre (accent,
  # bordures des cartes...), sans changer le fond général de la page.
  num=0
  label=""
  unit_theme=""
  : > "${tmpdir}/unit"

  flush_unit() {
    if [ -n "${label}" ] && [ -s "${tmpdir}/unit" ]; then
      printf '  <details class="index-section" data-theme="%s">\n    <summary><h2>%s</h2></summary>\n    <div class="index-grid">\n' \
        "${unit_theme:-uvod}" "${label}" >> "${tmpdir}/body"
      cat "${tmpdir}/unit" >> "${tmpdir}/body"
      printf '    </div>\n  </details>\n\n' >> "${tmpdir}/body"
    fi
    : > "${tmpdir}/unit"
    unit_theme=""
  }

  if [ -f "${parcours}" ]; then
    while IFS= read -r line || [ -n "${line}" ]; do
      line="$(printf "%s" "${line}" | sed -e "s/[[:space:]]*$//")"
      case "${line}" in
        ""|"#"*)
          continue
          ;;
        "="*)
          flush_unit
          label="$(printf "%s" "${line#=}" | sed -e "s/^[[:space:]]*//")"
          ;;
        *)
          base="${line%.md}"
          if ! grep -qxF "${base}" "${tmpdir}/present"; then
            printf "  ⚠ au sommaire mais pas de HTML : %s\n" "${base}" >&2
            continue
          fi
          num=$((num + 1))
          printf "%s\n" "${base}" >> "${tmpdir}/used"
          [ -z "${unit_theme}" ] && unit_theme="$(theme_key "${base}")"
          printf '      <a class="index-card" href="%s.html"><span class="index-num">%02d</span>%s %s</a>\n' \
            "${base}" "${num}" "$(card_emoji "${base}")" "$(card_title "${base}")" >> "${tmpdir}/unit"
          ;;
      esac
    done < "${parcours}"
    flush_unit
  else
    printf "  ⚠ pas de sommaire : %s\n" "${parcours}" >&2
  fi

  # --- Fiches présentes mais absentes du sommaire ---
  orphans=0
  while IFS= read -r base; do
    grep -qxF "${base}" "${tmpdir}/used" && continue
    orphans=$((orphans + 1))
    printf '      <a class="index-card" href="%s.html">%s %s</a>\n' \
      "${base}" "$(card_emoji "${base}")" "$(card_title "${base}")" >> "${tmpdir}/rest"
  done < "${tmpdir}/present"

  printf "  %d fiche(s), %d au sommaire, %d à classer\n" "${total}" "${num}" "${orphans}" >&2

  # --- Page ---
  # data-theme="uvod" en toile de fond neutre : chaque section d'unité
  # écrase localement ses propres variables (voir flush_unit ci-dessus).
  cat <<EOF
<!DOCTYPE html>
<html lang="fr">

<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=yes">
  <title>${pagetitle}</title>
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&family=Fraunces:opsz,wght@9..144,500;9..144,700&family=Space+Grotesk:wght@500;700&family=JetBrains+Mono:wght@500;700&display=swap" rel="stylesheet">
  <link rel="stylesheet" href="style.css">
  <link rel="manifest" href="manifest.json">
  <link rel="icon" href="icons/favicon.png">
  <link rel="apple-touch-icon" href="icons/apple-touch-icon.png">
  <meta name="theme-color" content="${THEME_COLOR}">
</head>

<body data-theme="uvod">

<header class="band">
  <div class="band__photo"></div>
  <div class="band__tint"></div>
  <div class="band__pattern"></div>
  <p class="band__credit"></p>
  <div class="wrap">
    <h1>📚 ${pagetitle}</h1>
  </div>
</header>

<div class="wrap">

EOF

  if [ -n "${backlink}" ]; then
    cat <<EOF
  <div class="toolbar">
    <button onclick="window.location.href='${backlink}'">🏠 Accueil</button>
  </div>
EOF
  fi

  printf '<main id="content">\n\n'

  cat "${tmpdir}/body"

  if [ -s "${tmpdir}/rest" ]; then
    printf '  <details class="index-section index-unsorted">\n    <summary><h2>À classer</h2></summary>\n    <div class="index-grid">\n'
    cat "${tmpdir}/rest"
    printf '    </div>\n  </details>\n\n'
  fi

  cat <<EOF
  <section class="index-section">
    <h2>S'entraîner</h2>
    <div class="index-grid">
      <a class="index-card" href="exercises.html">🎯 Exercices</a>
    </div>
  </section>

EOF

  cat <<EOF
</main>

</div>

<footer>
${INDEX_FOOTER}
</footer>

<script src="lang-config.js"></script>
<script src="progress.js"></script>
<script src="index.js"></script>

<script>
if ('serviceWorker' in navigator) {
  navigator.serviceWorker.register('service-worker.js');
}
</script>

</body>

</html>
EOF

  rm -rf "${tmpdir}"
}

generate_derived_assets() {
  python3 src/gen_lang_config.py --out src/lang-config.js || exit 5
  python3 src/gen_manifest.py --out src/manifest.json || exit 5
  python3 src/gen_service_worker.py --out src/service-worker.js || exit 5
  python3 src/gen_exercises_html.py --out src/exercises.html || exit 5
}

install_assets() {
  dir="$1"
  for f in src/style.css src/app.js src/lang-config.js src/progress.js src/index.js \
           src/exercises.html src/exercises.css src/exercises.js \
           src/manifest.json src/service-worker.js; do
    install -m 644 "${f}" "${dir}/$(basename "${f}")" || exit 4
  done
  # Photos de thème (facultatives) : tant qu'elles ne sont pas fournies
  # dans ${LANG_DIR}/img/, les fiches retombent sur motif + dégradé,
  # sans erreur. Les photos sont propres à un cours (contenu), pas au
  # moteur : elles vivent avec la langue, pas dans src/.
  if [ -d "${LANG_DIR}/img" ]; then
    mkdir -p "${dir}/img"
    cp -f "${LANG_DIR}/img/"*.* "${dir}/img/" 2>/dev/null
  fi
  # Icônes PWA (favicon, home screen, maskable...).
  if [ -d "${LANG_DIR}/icons" ]; then
    mkdir -p "${dir}/icons"
    cp -f "${LANG_DIR}/icons/"*.* "${dir}/icons/" 2>/dev/null
  fi
}


# ---------------------------------------------------------------------------
# Conversion md -> html
# ---------------------------------------------------------------------------

if ${GENERATE}; then
  for f in "${MD_DIR}"/*.md; do
    [ -e "${f}" ] || continue
    ./src/smd2html.py "${f}" || exit 2
  done
fi

if ${BACKUP}; then
  mkdir -p "${BACKUPS_DIR}"
  # Sous-shell : ne change pas le répertoire courant du script, donc
  # ça reste sûr quelle que soit la profondeur de LANG_DIR (langs/sk,
  # langs/br, ou tout autre chemin imbriqué).
  backup_file="$(cd "${BACKUPS_DIR}" && pwd)/html-$(date "+%Y%m%d-%H%M").tgz"
  ( cd "${HTML_DIR}" && tar czf "${backup_file}" ./* ) || exit 3
fi

if ${GENERATE}; then
  rm -f "${HTML_DIR}/"*.html

  # Dossier vide (première utilisation, avant d'y avoir mis du contenu) :
  # on ne casse pas, on prévient juste qu'il n'y a rien à faire.
  if ls "${MD_DIR}/"*.html >/dev/null 2>&1; then
    mv "${MD_DIR}/"*.html "${HTML_DIR}/" || exit 3
  fi
  if ls "${MD_DIR}/"*.md >/dev/null 2>&1; then
    ./src/smd2exercises.py --out-dir "${HTML_DIR}/" --index --parcours "${PARCOURS}" \
      "${MD_DIR}/"*.md || exit 3
  else
    printf "  (aucune fiche .md dans %s — rien à indexer)\n" "${MD_DIR}" >&2
  fi
fi

# ---------------------------------------------------------------------------
# Index + assets
# ---------------------------------------------------------------------------

generate_derived_assets

index_generator "${HTML_DIR}" "${PARCOURS}" "${SITE_TITLE}" "" \
  > "${HTML_DIR}/index.html" || exit 4
install_assets "${HTML_DIR}"

# ---------------------------------------------------------------------------
# Sync avec le serveur
# ---------------------------------------------------------------------------

if ${DRY_RUN}; then
  printf "DRY_RUN : rsync vers %s:%s ignoré.\n" "${DEPLOY_HOST}" "${DEPLOY_PATH}" >&2
else
  rsync -4av --delete "${HTML_DIR}/" "${DEPLOY_HOST}:${DEPLOY_PATH}"
fi
