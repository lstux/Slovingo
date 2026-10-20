# Slovingo app

Reads a language course published by the [Slovingo](https://github.com/lstux/Slovingo)
engine and lets you read its sheets and practise on the watch.

## Data

The app needs the compact export of a course (`src/export_api.py`, branch `feat/t-watch`
of Slovingo), served over http(s) from a folder:

```
<url>/index.json            course title and list of sheets
<url>/sheets/<id>.json      one sheet (text, tables, dialogue cards)
<url>/exercises/<id>.json   the exercises of that sheet
```

Every file is downloaded on demand (one at a time, by a worker task) and cached in the SPIFFS,
so a course already opened keeps working without wifi. Refresh (bottom middle) forces a download.

## Configuration

`/slovingo.json` on the SPIFFS, see `conf/slovingo.json.example`. It can also be changed from
the app (setup button, bottom right), which opens the course list.

The first start shows "Choisis ton cours". The list comes from
`https://lstux.github.io/Slovingo/courses.json` (each course has an `api` link to its export), downloaded
once per start and kept in the cache, so it also works offline once seen. If it cannot be
reached and is not cached, the list is empty and only the free address is left. "Autre adresse..." at the end of the list keeps the free `url` field. Each course has its own
cache (the cache file is named after the url).

| key         | default                                                  | meaning                      |
|-------------|----------------------------------------------------------|------------------------------|
| `url`       | `https://lstux.github.io/Slovingo-sk-fr-kids/api/` | folder of the course export  |
| `chosen`    | `false`                                                  | a course was picked (otherwise the list is shown) |
| `questions` | `10`                                                     | questions per exercise round |
| `name`      | `Líška`                                                  | name of the player, shown where the course has `[USER_NAME]` |

## What it does

Series list -> sheets -> *read* the sheet, or play its exercises.

- **Navigation**: the arrows at the right of the footer go to the previous or next series on
  the list of a series, and to the previous or next sheet on the menu of a sheet (the sheets
  follow the order of the course, so the end of a series leads to the next one). On the last
  page of a sheet the next arrow opens the next sheet, and the result of a quiz has a
  "Fiche suivante" button.
- **Reading** goes page by page, the arrows in the footer turn the pages, and so does a swipe
  (left: next page, right: previous page; only in the reader). Text blocks are
  put together, dialogue lines are chat bubbles (the fox, i.e. you, on the right, the others on the left,
  coloured name) with the translation under the line and the word by word glosses on a tap, and vocabulary tables become cards
  (word, then sound and meaning; the translations start hidden, a tap shows or hides one, a button shows or hides all
  of them to test yourself).
- **Exercises**: multiple choice, fill-in-the-blank and word ordering (the words of the
  sentence plus one or two that do not belong, tap them in order). The next question comes
  by itself after an answer, the progress and score are in the footer.
- **Menus**: each series has the pictogram and the colour of its theme (the `theme` of the
  sheets in `index.json`, the themes of the HTML site: basics, family, house, food, time,
  shopping, city, weather, mountains, easter, animals, games, sea; a series without theme gets
  the books). The home screen starts with the fox and the `greeting` of the course (with the
  name of the player and its translation), the menu of a sheet has the fox thinking, the
  loading screen a bouncing fox and the errors a sad one. The pictograms are the emoji of
  Noto Color Emoji (SIL OFL), reduced to 32 px by `tools/gen_assets.py`.
- **Mascots**: the fox (the player) sits in the footer of the quiz and smiles or cries
  at the answers, the watch buzzes a little (when "vibe feedback" is on); the result
  screen shows the fox, the rabbit from 70 %, and a little phrase of the course with its
  translation, drawn at random in the tier of the score (the `cheers` of `index.json`: perfect
  = 100 %, great from 90, good from 70, ok from 50, retry below). A course without `cheers`
  shows no phrase (see `docs/Lang-json.md` of the Slovingo engine).
- `[USER_NAME]` in the course is replaced by `name` of the config.
- **Standby**: while a quiz, the result or a sheet is shown, the watch does not jump back
  to the main tile when the screen goes off (the "block return to main tile" setting is
  restored when leaving). If it is left anyway, the series resumes where it was (an answered
  question moves on, an unanswered one starts again).

Not on the watch: audio (`listen` exercises), images and matching exercises.
Emojis and typographic symbols outside the font are dropped or replaced by ASCII
(`slovingo_sanitize()`); the emoji of a speaker is replaced by a name.

## Fonts, icon and mascots

The default fonts of the firmware have no `č ď ĺ ľ ň ô ŕ š ť ž`. This app ships its own
DejaVu Sans based fonts (Basic Latin, Latin-1 and Latin Extended-A, 16 and 22 px) in `font/`
and does not touch the global ones. `tools/gen_assets.py` regenerates fonts, icon and the
fox and rabbit sprites (needs Pillow and DejaVu Sans).

The app is built for every board except the small flash ones, where `support/apps_off.py`
leaves it out (`NOT_ESSENTIAL`).

## Progress

The best score of each sheet is kept in `/slovingo_progress.json` on the SPIFFS (about 16 bytes a
sheet, a hash of the course url and of the sheet id is the key). The file is written only at the end
of a quiz, and only when the score beats the old one. The sheet list shows the best score
(green at 100 %, gold from 70 %, white from 50 %, grey below), the series list shows
"sheets done/sheets" (done = 50 % or more), the sheet menu shows "Meilleur score" and the result
screen tells "Nouveau record !" or the record to beat. A course reached by another address
(for example the old `Slovingo-xx-xx` pages) has its own progress.

## Confetti and days in a row

A perfect quiz (100 %) lets confetti fall over the result screen. The days in a row are counted
in `/slovingo_streak.json` (last day played and streak, written at most once a day): playing the
day after the last one adds a day, a missed day starts again at 1. From 2 days the result screen
says "N jours de suite !". It needs the clock of the watch to be set (time sync), otherwise nothing is counted.
