#!/bin/bash
# Script to add backup export/import keys and remove obsolete keys from all course lang.json files
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

for repo in "${REPOS[@]}"; do
  owner=$(echo $repo | cut -d/ -f1)
  name=$(echo $repo | cut -d/ -f2)
  tmpdir="/tmp/$name"
  
  echo "Processing $name..."
  git clone "https://github.com/$repo" "$tmpdir" 2>/dev/null || continue
  cd "$tmpdir"
  
  git checkout -b feat/backup-keys
  
  # Update lang.json: add backup keys and remove obsolete ones
  python3 - <<'PYTHON'
import json

BACKUP_KEYS = {
    "backup_export": "📥 Export settings and progress (JSON)",
    "backup_import": "📤 Import a backup (JSON)",
}

OBSOLETE_KEYS = [
    "exercise_field_l1", "exercise_field_l2", "exercise_field_distractors_l1",
    "exercise_field_distractors_l2", "exercise_field_sentence_l1",
    "exercise_field_sentence_l2", "exercise_field_sentence_l2_heard",
    "exercise_field_sentence_l2_to_reorder", "exercise_field_translation_l1",
    "exercise_field_items_l1", "exercise_field_items_l2",
    "exercise_fillblank_blank_display",
    "font_serif", "font_mono", "font_grotesk",
    "of_normal_speed"
]

with open('lang.json') as f:
    config = json.load(f)

if 'ui' not in config:
    config['ui'] = {}

# Add backup keys
for key, value in BACKUP_KEYS.items():
    config['ui'][key] = value

# Remove obsolete keys
for key in OBSOLETE_KEYS:
    config['ui'].pop(key, None)

with open('lang.json', 'w') as f:
    json.dump(config, f, ensure_ascii=False, indent=2)
PYTHON
    
    git add lang.json
    git commit -m "Add backup keys and clean up obsolete UI keys

Changes:
- Add: backup_export, backup_import (new backup feature)
- Remove: 15 obsolete keys now hard-coded in front-end
  * exercise_field_* (11 editor labels)
  * font_serif, font_mono, font_grotesk (proper nouns)
  * exercise_fillblank_blank_display (blank marker)
  * of_normal_speed (playback speed label)

See https://github.com/lstux/Slovingo#56"
    
    git push -u origin feat/backup-keys
    result=$(gh pr create --title "Add backup keys and clean up obsolete UI keys" \
      --body "Adds backup_export and backup_import UI keys required by backup feature.

Also removes 15 obsolete keys that are now hard-coded in the front-end, reducing lang.json size.

See https://github.com/lstux/Slovingo#56 for context." 2>&1)
    
    if echo "$result" | grep -q "github.com"; then
      echo "  ✓ PR created"
    else
      echo "  ⚠️ PR: $result"
    fi
    
  cd -
  rm -rf "$tmpdir"
done

echo "✅ Done! All course repos updated."
