#!/bin/bash
# Script to add backup export/import keys to all course lang.json files
# Usage: bash update_courses_backup_keys.sh

REPOS=(
  "lstux/Slovingo-sk-fr"
  "lstux/Slovingo-sk-fr-kids"
  "lstux/Slovingo-sk-fr-friends"
  "lstux/Slovingo-fr-sk"
  "lstux/Slovingo-de-fr"
  "lstux/Slovingo-de-fr-kids"
  "lstux/Slovingo-bzh-fr"
)

BACKUP_KEYS='
    "backup_export": "📥 Export settings and progress (JSON)",
    "backup_import": "📤 Import a backup (JSON)",'

for repo in "${REPOS[@]}"; do
  owner=$(echo $repo | cut -d/ -f1)
  name=$(echo $repo | cut -d/ -f2)
  tmpdir="/tmp/$name"
  
  echo "Processing $name..."
  git clone "https://github.com/$repo" "$tmpdir" 2>/dev/null || continue
  cd "$tmpdir"
  
  git checkout -b feat/backup-keys
  
  # Add backup keys to lang.json if missing
  if ! grep -q '"backup_export"' lang.json; then
    # Find the "ui" object and add keys after the opening brace
    python3 - <<'PYTHON'
import json
with open('lang.json') as f:
    config = json.load(f)
if 'ui' not in config:
    config['ui'] = {}
config['ui']['backup_export'] = '📥 Export settings and progress (JSON)'
config['ui']['backup_import'] = '📤 Import a backup (JSON)'
with open('lang.json', 'w') as f:
    json.dump(config, f, ensure_ascii=False, indent=2)
PYTHON
    
    git add lang.json
    git commit -m "Add backup export/import UI keys

Add missing lang.json keys for new Settings > Data backup feature:
- backup_export: button to export settings and progress as JSON
- backup_import: button to import a backup file

Optional: 15 keys are now hard-coded in the front-end and can be removed:
exercise_field_* (11 keys), font_serif, font_mono, font_grotesk,
exercise_fillblank_blank_display, of_normal_speed"
    
    git push -u origin feat/backup-keys
    gh pr create --title "Add backup export/import UI keys" \
      --body "Adds backup_export and backup_import UI keys required by backup feature.

See https://github.com/lstux/Slovingo#56 for context.

Optional: 15 keys are now hard-coded and can optionally be removed."
    
    echo "  ✓ PR created"
  else
    echo "  ✓ Already has backup keys"
  fi
  
  cd -
  rm -rf "$tmpdir"
done
