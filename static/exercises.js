/**
 * exercises.js -- Slovingo v2 exercise engine.
 *
 * Selection screen + session for every exercise type, consuming the
 * merged l1/l2 format from smd2exercises.py (see areas/v2-rewrite):
 * one exercise object covers both practice directions, resolved into
 * a "view" (question/answer/choices) at render time rather than
 * stored twice.
 *
 * Types are declared ONCE, in EXERCISE_TYPES below -- everything else
 * in this file (the pool, the filters, the checkboxes, the session
 * player, the editor's forms) reads that registry instead of having
 * its own hardcoded list. Adding a new exercise type (e.g. "remettre
 * les mots dans l'ordre") means adding one entry here, not touching
 * a dozen files: this file, the standalone editeur-exercices.html,
 * and smd2exercises.py (which can generate the new type once it
 * knows its data shape) are the only places a type's *data shape* is
 * decided, and this registry is the single place that shape lives
 * for the front-end.
 *
 * Ported from v1's exercises.js. Differences:
 *   - No separate fetch: EXERCISES/DATA are already loaded by app.js's
 *     boot(), so this file just reads the globals.
 *   - Routed by hash (#/exercises, #/exercises/session?...) instead of
 *     a separate exercises.html page with query-string parameters.
 *   - Progress is keyed by sheet id, not by .md source filename.
 *   - Direction ("l1-l2" / "l2-l1" / "both") is resolved per question
 *     at render time from a single merged exercise, instead of
 *     filtering a pool that already contained two direction-tagged
 *     copies of everything -- so pool size no longer depends on the
 *     chosen direction (a genuine simplification over v1).
 *   - Exercise types are declared in one registry (EXERCISE_TYPES)
 *     instead of being scattered across resolveView()/renderX()/
 *     typeLabel()/typeIcon()/ALL_TYPES.
 *
 * Depends on app.js (speak(), targetVoiceAvailable, applyTheme(),
 * setKicker(), setPageTitle(), setToolbarButtons(),
 * setExercisesButtonMode(), setPagerNav(), hidePagerNav(),
 * flatSheetIds(), kickerFor(), splitTitle(), themeForGroup(),
 * categoryLabelFor(), subgroupLabelFor(), escapeHtml(), LANG, DATA,
 * EXERCISES) and progress.js (loadProgressStore(), getSheetProgress(),
 * pct(), scoreRatioClass(), averageScore()). Classic scripts sharing
 * one global scope, same pattern as v1.
 */

"use strict";

// ============================================================================
// 0. Exercise type registry -- the single source of truth for what an
//    exercise type IS: its data fields (read by editeur-exercices.html
//    to build its forms), how a merged exercise resolves into a
//    question/answer "view" for a given direction, and how that view
//    renders as an interactive card during a session.
//
//    A `fields` schema entry describes one editable facet of the
//    exercise's JSON shape, for a generic editor form:
//      - kind: "text"          -- a single-line string field (l1/l2).
//      - kind: "list"          -- an array of short strings
//                                  (distractors): add/remove/reorder.
//      - kind: "sentence-blank" -- a full sentence with one word
//                                  blanked out (fill-blank): the
//                                  sentence text (`key`), which word is
//                                  missing (`missingKey`), its token
//                                  index (`blankIndexKey`), and its
//                                  distractors (`choicesKey`).
//      - kind: "sentence-tokens" -- a full sentence (`key`) plus an
//                                  optional explicit tokenisation
//                                  (`tokensKey`) that can group words
//                                  that must stay together (e.g. "ne
//                                  ... pas"); falls back to a naive
//                                  split(" ") of the sentence when the
//                                  tokens array is absent.
//    This schema is intentionally generic rather than one bespoke
//    field per type, so the editor's form-builder is a single loop
//    over `fields`, not one branch per exercise type.
// ============================================================================

// Expected distractor counts, mirroring the constants of the same
// name in smd2exercises.py (QCM_CHOICES/FILL_BLANK_CHOICES/
// LISTEN_CHOICES there include the answer itself; here they're
// distractor-only, matching what choices_l1/choices_l2 actually
// store). Used only by the editor's validation banner -- the player
// itself works fine with more or fewer, so this is advisory.
const QCM_CHOICES = 4;
const FILL_BLANK_CHOICES = 3;
const LISTEN_CHOICES = 4;

const EXERCISE_TYPES = {
    qcm: {
        icon: "🔤",
        label: "QCM",
        expectedChoices: QCM_CHOICES - 1,
        fields: [
            { key: "l1", label: "Français", kind: "text" },
            { key: "l2", label: "Slovaque", kind: "text" },
            { key: "choices_l1", label: "Distracteurs (FR)", kind: "list" },
            { key: "choices_l2", label: "Distracteurs (SK)", kind: "list" },
        ],
        resolveView: resolveQcmView,
        render: renderQcm,
    },
    "fill-blank": {
        icon: "✏️",
        label: "Trou",
        expectedChoices: FILL_BLANK_CHOICES,
        fields: [
            {
                key: "l1", label: "Phrase (FR)", kind: "sentence-blank",
                missingKey: "missing_l1", blankIndexKey: "blank_index_l1", choicesKey: "choices_l1",
            },
            {
                key: "l2", label: "Phrase (SK)", kind: "sentence-blank",
                missingKey: "missing_l2", blankIndexKey: "blank_index_l2", choicesKey: "choices_l2",
            },
        ],
        resolveView: resolveFillBlankView,
        render: renderFillBlank,
    },
    listen: {
        icon: "🔊",
        label: "Écoute",
        needsVoice: true,
        expectedChoices: LISTEN_CHOICES - 1,
        fields: [
            { key: "l1", label: "Traduction (FR)", kind: "text" },
            { key: "l2", label: "Phrase (SK, écoutée)", kind: "text" },
            { key: "choices_l2", label: "Distracteurs (SK)", kind: "list" },
        ],
        resolveView: resolveListenView,
        render: renderListen,
    },
    order: {
        icon: "🔀",
        label: "Ordre",
        fields: [
            { key: "l1", label: "Traduction (FR)", kind: "sentence-tokens", tokensKey: "tokens_l1" },
            { key: "l2", label: "Phrase (SK, à remettre en ordre)", kind: "sentence-tokens", tokensKey: "tokens_l2" },
        ],
        resolveView: resolveOrderView,
        render: renderOrder,
    },
};

/** Declaration order of EXERCISE_TYPES, kept stable everywhere a fixed
 * display order matters (checkboxes, badges, summary). */
const ALL_TYPES = Object.keys(EXERCISE_TYPES);

function typeLabel(type) {
    const entry = EXERCISE_TYPES[type];
    return entry ? `${entry.icon} ${entry.label}` : type;
}

function typeIcon(type) {
    return (EXERCISE_TYPES[type] && EXERCISE_TYPES[type].icon) || "❔";
}

/** Whether `type` can currently be played -- false only for a
 * voice-dependent type (listen) when no target-language voice is
 * available on this device. */
function typePlayable(type) {
    const entry = EXERCISE_TYPES[type];
    return !(entry && entry.needsVoice && !ttsAvailable());
}

/** Session state, rebuilt each time a session starts. */
const state = {
    answerMode: "choice",       // "choice" or "type"
    allExercises: [],
    exercises: [],
    current: 0,
    currentView: null,
    score: 0,
    mistakes: [],
    byType: {},                 // { qcm: {correct, total}, "fill-blank": {...}, ... }
    sheetId: null,               // single-sheet session -- progress recording key
    sheetTitle: "",
    sessionSize: 10,
    direction: "l1-l2",
    activeTypes: new Set(ALL_TYPES),
};

/**
 * Overwrite state's session-shape defaults (size, direction, active
 * types, answer mode) with the person's saved Settings, once SETTINGS
 * is loaded. Called by app.js's boot() right after that -- before the
 * first route() -- so even the very first render of the selection
 * screen already reflects them, not just sessions started after a
 * visit to Settings.
 */
function applyExerciseDefaultsFromSettings() {
    state.sessionSize = SETTINGS.defaultQuestionCount;
    state.direction = SETTINGS.defaultDirection;
    state.activeTypes = new Set(SETTINGS.defaultTypes);
    state.answerMode = SETTINGS.defaultAnswerMode;
}

// Selection-screen-only state, reset each time that screen is (re)rendered.
let selectedSheetIds = new Set();
let cardCounter = 0;

// ============================================================================
// Small DOM / utility helpers (exercises.js's own -- app.js's are for
// content rendering and not reused here to keep the two files
// independent of each other's internals).
// ============================================================================

function shuffle(array) {
    const copy = array.slice();
    for (let i = copy.length - 1; i > 0; i--) {
        const j = Math.floor(Math.random() * (i + 1));
        [copy[i], copy[j]] = [copy[j], copy[i]];
    }
    return copy;
}

function normalizeAnswer(text) {
    return text.trim().toLowerCase().replace(/\s+/g, " ");
}

/** Small hyperscript-ish helper, ported from v1 unchanged. */
function el(tag, options = {}, children = []) {
    const node = document.createElement(tag);
    if (options.className) node.className = options.className;
    if (options.text !== undefined) node.textContent = options.text;
    if (options.html !== undefined) node.innerHTML = options.html;
    if (options.onclick) node.addEventListener("click", options.onclick);
    if (options.attrs) {
        Object.entries(options.attrs).forEach(([k, v]) => node.setAttribute(k, v));
    }
    children.forEach((child) => node.appendChild(child));
    return node;
}

function speakSafe(text, rate, pitch, voiceURI) {
    if (typeof speak === "function") {
        speak(text, rate, pitch, voiceURI);
    }
}

/** targetVoiceAvailable is a global declared in app.js. */
function ttsAvailable() {
    return typeof targetVoiceAvailable !== "undefined" && targetVoiceAvailable;
}

// ============================================================================
// Router entry point (called from app.js's route())
// ============================================================================

/**
 * Handle any #/exercises... hash. A session hash carries its
 * parameters in a query string appended to the hash itself, e.g.
 * "#/exercises/session?cards=a,b&n=20&types=qcm,listen&direction=l1-l2".
 * @param {string} hash
 */
function routeExercises(hash) {
    const queryIndex = hash.indexOf("?");
    const path = queryIndex === -1 ? hash : hash.slice(0, queryIndex);
    const query = new URLSearchParams(queryIndex === -1 ? "" : hash.slice(queryIndex + 1));

    if (path === "#/exercises/session") {
        startSessionFromQuery(query);
    } else {
        renderSelectionScreen();
    }
}

// ============================================================================
// Progress (localStorage) -- write side; progress.js has the read side.
// ============================================================================

function saveProgressStore(store) {
    try {
        localStorage.setItem(progressKey(), JSON.stringify(store));
    } catch (err) {
        // Storage unavailable (private browsing, quota...) -- carry on
        // without blocking the person, progress just won't be kept.
    }
}

/** Record a finished session's result: last score + cumulative by type. */
function recordSession(sheetId, score, total, byType) {
    const store = loadProgressStore();
    const existing = store[sheetId] || { cumulative: { byType: {}, sessionsCount: 0 } };

    existing.last = { date: new Date().toISOString(), score, total, byType };
    existing.cumulative.sessionsCount = (existing.cumulative.sessionsCount || 0) + 1;

    Object.entries(byType).forEach(([type, stats]) => {
        const cum = existing.cumulative.byType[type] || { correct: 0, total: 0 };
        cum.correct += stats.correct;
        cum.total += stats.total;
        existing.cumulative.byType[type] = cum;
    });

    store[sheetId] = existing;
    saveProgressStore(store);
}

function resetProgress() {
    try {
        localStorage.removeItem(progressKey());
    } catch (err) {
        // ignore
    }
}

// ============================================================================
// Selection screen
// ============================================================================

/**
 * Every DATA group that has at least one sheet with exercises,
 * narrowed to just those sheets.
 * @returns {{group: object, sheets: object[]}[]}
 */
function buildExerciseSections() {
    return DATA.groups
        .map((group) => ({
            group,
            sheets: group.sheets.filter((sheet) => EXERCISES.sheets[sheet.id]),
        }))
        .filter(({ sheets }) => sheets.length > 0);
}

async function renderSelectionScreen() {
    applyTheme("uvod");
    setKicker("");
    setPageTitle((LANG.site && LANG.site.title) || "Slovingo");
    setToolbarButtons({});
    setExercisesButtonMode("selection");
    hidePagerNav();

    cardCounter = 0;
    selectedSheetIds = new Set();

    const content = document.getElementById("content");
    content.innerHTML = "";

    const sections = buildExerciseSections();
    if (!sections.length) {
        content.appendChild(el("p", { text: "No exercises available yet." }));
        return;
    }

    const header = el("div", { className: "exo-selection-header" }, [
        el("h2", { text: "Choose your sheets" }),
        el("p", {
            className: "exo-selection-hint",
            text: "Check one or more sheets (or a whole group), then start a mixed quiz.",
        }),
    ]);
    header.appendChild(renderQuickFilters(sections));
    const filterRow = el("div", { className: "exo-filter-row" }, [
        renderTypeCheckboxes(state.activeTypes, updateLaunchBar),
        renderDirectionSelector(state.direction, (value) => {
            state.direction = value;
            updateLaunchBar();
        }),
    ]);
    header.appendChild(filterRow);
    content.appendChild(header);

    sections.forEach(({ group, sheets }) => {
        content.appendChild(renderGroupSelectionSection(group, sheets));
    });

    content.appendChild(buildLaunchBar());
    // "Reset my progress" now lives in Settings (#/settings) rather
    // than duplicated here.

    updateLaunchBar();
}

/**
 * "All / Already practiced / Not started / None" buttons: bulk-check
 * from what's already known (progress.js), without storing anything
 * new. A one-off action, not a persistent filter mode -- individual
 * boxes can still be adjusted by hand afterwards.
 */
function renderQuickFilters(sections) {
    const row = el("div", { className: "exo-quick-filters" });
    const allSheets = sections.flatMap(({ sheets }) => sheets);

    const setAll = (predicate) => {
        selectedSheetIds = new Set(allSheets.filter(predicate).map((s) => s.id));
        refreshCheckboxes();
        updateLaunchBar();
    };

    row.appendChild(el("button", { className: "chip", text: "All", onclick: () => setAll(() => true) }));
    row.appendChild(el("button", {
        className: "chip",
        text: "Already practiced",
        onclick: () => setAll((s) => !!(getSheetProgress(s.id) || {}).last),
    }));
    row.appendChild(el("button", {
        className: "chip",
        text: "Not started",
        onclick: () => setAll((s) => !(getSheetProgress(s.id) || {}).last),
    }));
    row.appendChild(el("button", { className: "chip", text: "None", onclick: () => setAll(() => false) }));
    return row;
}

/**
 * Re-sync every checkbox's checked state (sheets + group headers) on
 * selectedSheetIds -- the single source of truth, whether it just
 * changed from an individual box, a group box, or a quick filter.
 */
function refreshCheckboxes() {
    document.querySelectorAll(".index-card[data-sheet-id]").forEach((card) => {
        const checkbox = card.querySelector("input[type=checkbox]");
        if (checkbox) checkbox.checked = selectedSheetIds.has(card.dataset.sheetId);
    });
    document.querySelectorAll(".index-section").forEach((section) => {
        const boxes = [...section.querySelectorAll(".index-card[data-sheet-id] input[type=checkbox]")];
        const groupCheckbox = section.querySelector("summary input[type=checkbox]");
        if (!groupCheckbox || !boxes.length) return;
        const checkedCount = boxes.filter((b) => b.checked).length;
        groupCheckbox.checked = checkedCount === boxes.length;
        groupCheckbox.indeterminate = checkedCount > 0 && checkedCount < boxes.length;
    });
}

/**
 * One group as a collapsible section, reusing .index-section /
 * .index-grid / .index-card from the home view (see style.css) --
 * same look, one definition to maintain on both screens.
 */
function renderGroupSelectionSection(group, sheets) {
    const details = el("details", { className: "index-section" });
    details.dataset.theme = themeForGroup(group);

    const categoryLabel = categoryLabelFor(group);
    const subgroupLabel = subgroupLabelFor(group);
    const label = subgroupLabel ? `${categoryLabel} - ${subgroupLabel}` : categoryLabel;
    const heading = el("h2", { text: `${label} (${sheets.length})` });

    const score = averageScore(sheets.map((s) => s.id));
    if (score) {
        heading.appendChild(el("span", {
            className: `index-score-badge index-score-${scoreRatioClass(score.avg, 100)}`,
            text: `${score.avg}%`,
            attrs: { title: `Group average: ${score.avg}% (${score.count}/${sheets.length} sheet(s) practiced)` },
        }));
    }

    const groupCheckbox = el("input", { attrs: { type: "checkbox", "aria-label": `Select all of ${label}` } });
    // Stop the checkbox click from also toggling the <details> open/closed.
    groupCheckbox.addEventListener("click", (e) => e.stopPropagation());
    groupCheckbox.addEventListener("change", () => {
        sheets.forEach((sheet) => {
            if (groupCheckbox.checked) selectedSheetIds.add(sheet.id);
            else selectedSheetIds.delete(sheet.id);
        });
        refreshCheckboxes();
        updateLaunchBar();
    });

    details.appendChild(el("summary", {}, [groupCheckbox, heading]));

    const grid = el("div", { className: "index-grid" });
    const numbered = group.category === "series";
    sheets.forEach((sheet) => grid.appendChild(renderSelectionCard(sheet, group, numbered)));
    details.appendChild(grid);

    return details;
}

function renderSelectionCard(sheet, group, numbered) {
    const entry = EXERCISES.sheets[sheet.id];
    const progress = getSheetProgress(sheet.id);

    const badges = [];
    ALL_TYPES.forEach((type) => {
        if (entry.counts[type]) {
            badges.push(el("span", {
                className: "exo-badge",
                text: `${typeIcon(type)} ${entry.counts[type]}`,
                attrs: { title: typeLabel(type) },
            }));
        }
    });
    if (progress && progress.last) {
        badges.push(el("span", {
            className: `index-score-badge index-score-${scoreRatioClass(progress.last.score, progress.last.total)}`,
            text: `${progress.last.score}/${progress.last.total}`,
            attrs: { title: `Last try: ${new Date(progress.last.date).toLocaleDateString()}` },
        }));
    }

    const checkbox = el("input", { attrs: { type: "checkbox" } });
    checkbox.checked = selectedSheetIds.has(sheet.id);
    checkbox.addEventListener("change", () => {
        if (checkbox.checked) selectedSheetIds.add(sheet.id);
        else selectedSheetIds.delete(sheet.id);
        refreshCheckboxes();
        updateLaunchBar();
    });

    const children = [checkbox];
    if (numbered) {
        cardCounter += 1;
        children.push(el("span", { className: "index-num", text: String(cardCounter).padStart(2, "0") }));
    }
    children.push(el("span", { className: "index-card-title", text: splitTitle(sheet.title).title }));
    children.push(el("span", { className: "index-card-badges" }, badges));

    const card = el("label", { className: "index-card" }, children);
    card.dataset.theme = themeForGroup(group);
    card.dataset.sheetId = sheet.id;
    return card;
}

// ============================================================================
// Launch bar (bottom of the selection screen)
// ============================================================================

function buildLaunchBar() {
    const bar = el("div", { className: "exo-launch-bar", attrs: { id: "exo-launch-bar" } });

    bar.appendChild(el("p", { className: "exo-launch-summary", attrs: { id: "exo-launch-summary" } }));

    bar.appendChild(el("div", { className: "exo-launch-qcount" }, [
        el("label", { text: "Number of questions", attrs: { for: "exo-launch-n" } }),
        el("input", { attrs: { type: "number", id: "exo-launch-n", min: "1", value: String(SETTINGS.defaultQuestionCount) } }),
    ]));

    bar.appendChild(el("button", {
        className: "exo-launch-btn",
        text: "▶️ Start the session",
        attrs: { id: "exo-launch-btn" },
        onclick: launchSession,
    }));

    return bar;
}

/**
 * Recompute the summary text and available pool whenever the
 * selection or type filter changes. Unlike v1, this count is now
 * exact for any direction (qcm/fill-blank counts are no longer
 * doubled per direction in the data -- direction only changes how a
 * question is presented, not the pool size).
 */
function updateLaunchBar() {
    const bar = document.getElementById("exo-launch-bar");
    if (!bar) return;

    const selectedSheets = [...selectedSheetIds];
    const effectiveTypes = ALL_TYPES.filter((t) => state.activeTypes.has(t) && typePlayable(t));
    const pool = selectedSheets.reduce((sum, id) => {
        const counts = EXERCISES.sheets[id].counts;
        return sum + effectiveTypes.reduce((s, t) => s + (counts[t] || 0), 0);
    }, 0);

    const summaryEl = document.getElementById("exo-launch-summary");
    if (selectedSheets.length) {
        summaryEl.textContent = `${selectedSheets.length} sheet(s) selected - ${pool} exercise(s) available`;
    } else {
        summaryEl.textContent = "Select at least one sheet to get started.";
    }

    const input = document.getElementById("exo-launch-n");
    const btn = document.getElementById("exo-launch-btn");
    if (pool > 0) {
        input.max = pool;
        input.disabled = false;
        if (!input.value || parseInt(input.value, 10) > pool) {
            input.value = Math.min(SETTINGS.defaultQuestionCount, pool);
        }
    } else {
        input.disabled = true;
    }
    btn.disabled = selectedSheets.length === 0 || pool === 0;
}

function launchSession() {
    const n = parseInt(document.getElementById("exo-launch-n").value, 10) || 10;
    const query = new URLSearchParams();
    query.set("cards", [...selectedSheetIds].join(","));
    query.set("n", String(n));
    if (state.activeTypes.size < ALL_TYPES.length) {
        query.set("types", [...state.activeTypes].join(","));
    }
    if (state.direction !== "l1-l2") {
        query.set("direction", state.direction);
    }
    window.location.hash = `#/exercises/session?${query.toString()}`;
}

/**
 * Checkbox row shared between the selection screen and an active
 * session, one checkbox per entry of EXERCISE_TYPES -- adding a type
 * to the registry adds its checkbox here automatically. A
 * voice-dependent type (listen) is disabled (and unchecked) whenever
 * no target-language voice is available -- no point offering
 * questions that can't be heard.
 */
function renderTypeCheckboxes(selectedSet, onChange) {
    const row = el("div", { className: "exo-type-filter" });
    const inputs = [];
    ALL_TYPES.forEach((type) => {
        const disabled = !typePlayable(type);
        if (disabled) selectedSet.delete(type);

        const input = el("input", { attrs: { type: "checkbox" } });
        input.checked = selectedSet.has(type);
        input.disabled = disabled;
        input.addEventListener("change", () => {
            if (input.checked) {
                selectedSet.add(type);
            } else if (selectedSet.size > 1) {
                selectedSet.delete(type);
            } else {
                input.checked = true; // always keep at least one active type
            }
            onChange();
        });
        const label = el("label", {
            attrs: { title: disabled ? "Listening disabled: no voice available on this device" : typeLabel(type) },
        }, [input, document.createTextNode(` ${typeIcon(type)}`)]);
        if (disabled) label.classList.add("exo-type-disabled");
        row.appendChild(label);
        inputs.push({ type, input });
    });

    if (selectedSet.size === 0) {
        inputs.forEach(({ type, input }) => {
            if (!input.disabled) {
                selectedSet.add(type);
                input.checked = true;
            }
        });
    }

    return row;
}

/** Direction selector (native->target / target->native / both). */
function renderDirectionSelector(current, onChange) {
    const select = el("select", { className: "exo-direction-select" });
    const nativeAbbr = ((LANG.native_lang.tts_code || LANG.native_lang.name || "").split("-")[0]).toUpperCase();
    const targetAbbr = ((LANG.target_lang.tts_code || LANG.target_lang.name || "").split("-")[0]).toUpperCase();
    const options = [
        { value: "l1-l2", label: `${LANG.native_lang.flag}→${LANG.target_lang.flag} ${nativeAbbr}→${targetAbbr}` },
        { value: "l2-l1", label: `${LANG.target_lang.flag}→${LANG.native_lang.flag} ${targetAbbr}→${nativeAbbr}` },
        { value: "both", label: "🔀 Both directions" },
    ];
    options.forEach(({ value, label }) => {
        const option = el("option", { text: label, attrs: { value } });
        if (current === value) option.selected = true;
        select.appendChild(option);
    });
    select.addEventListener("change", () => onChange(select.value));
    return select;
}

// ============================================================================
// Starting a session
// ============================================================================

/**
 * The nearest sheet before/after `sheetId`, in the whole-course
 * order (see app.js's flatSheetIds()), that actually HAS exercises --
 * skipping over sheets that don't (Introduction, or an Annex sheet
 * with no audio-card/translate-table to build exercises from), so
 * the pager never lands on a dead end.
 * @param {string} sheetId
 * @param {1|-1} direction
 * @returns {string|null}
 */
function adjacentExerciseSheetId(sheetId, direction) {
    const ids = flatSheetIds();
    let index = ids.indexOf(sheetId) + direction;
    while (index >= 0 && index < ids.length) {
        if (EXERCISES.sheets[ids[index]]) return ids[index];
        index += direction;
    }
    return null;
}

/** @param {URLSearchParams} query */
function startSessionFromQuery(query) {
    const cardIds = (query.get("cards") || "").split(",").filter(Boolean);
    const n = query.get("n") ? parseInt(query.get("n"), 10) : SETTINGS.defaultQuestionCount;

    state.activeTypes = new Set(query.get("types") ? query.get("types").split(",") : SETTINGS.defaultTypes);
    state.direction = query.get("direction") || SETTINGS.defaultDirection;
    state.answerMode = query.get("mode") || SETTINGS.defaultAnswerMode;
    state.sessionSize = n;

    if (!cardIds.length) {
        renderSelectionScreen();
        return;
    }

    const validIds = cardIds.filter((id) => EXERCISES.sheets[id]);
    if (!validIds.length) {
        const content = document.getElementById("content");
        content.innerHTML = "";
        content.appendChild(el("p", { className: "exo-error", text: "Could not load the selected sheet(s)." }));
        return;
    }

    state.allExercises = validIds.flatMap((id) => EXERCISES.sheets[id].exercises);
    state.sheetId = validIds.length === 1 ? validIds[0] : null;

    const group = state.sheetId ? findGroupForSheet(state.sheetId) : null;
    applyTheme(group ? themeForGroup(group) : "uvod");

    // Single sheet: same kicker/title split as the sheet page itself
    // (kickerFor()/splitTitle(), both in app.js) -- "Série Rodina
    // (1/5)" as the kicker, "Moja rodina" as the title, not the raw
    // "Série Rodina (1/5) -- Moja rodina" string. A mixed session has
    // no one sheet to derive a kicker from, so it keeps its own label.
    if (state.sheetId) {
        const rawTitle = EXERCISES.sheets[state.sheetId].title;
        setKicker(kickerFor(group, { id: state.sheetId, title: rawTitle }));
        state.sheetTitle = splitTitle(rawTitle).title;
    } else {
        setKicker("");
        state.sheetTitle = `Mixed session (${validIds.length} sheets)`;
    }
    setPageTitle(state.sheetTitle);
    setToolbarButtons({});
    setExercisesButtonMode(state.sheetId ? "back-to-sheet" : "selection", state.sheetId);

    // Pager: only meaningful for a single-sheet session -- jumps
    // straight to the previous/next practicable sheet's own session
    // (same question count as the "Exercises" button's direct
    // launch, from Settings), skipping the selection screen
    // entirely. Hidden for a mixed session, same as on the selection
    // screen itself.
    if (state.sheetId) {
        const prevId = adjacentExerciseSheetId(state.sheetId, -1);
        const nextId = adjacentExerciseSheetId(state.sheetId, 1);
        const n = SETTINGS.defaultQuestionCount;
        setPagerNav(
            prevId ? `#/exercises/session?cards=${encodeURIComponent(prevId)}&n=${n}` : null,
            nextId ? `#/exercises/session?cards=${encodeURIComponent(nextId)}&n=${n}` : null,
        );
    } else {
        hidePagerNav();
    }

    const content = document.getElementById("content");
    content.innerHTML = "";
    content.appendChild(buildSessionToolbar());
    content.appendChild(el("div", { className: "exo-progress-bar" }, [
        el("span", { attrs: { id: "exo-progress-fill" } }),
    ]));
    content.appendChild(el("div", { className: "exo-progress-text", attrs: { id: "exo-progress" } }));
    content.appendChild(el("div", { className: "exo-card-area", attrs: { id: "exo-card-area" } }));

    startPool();
}

/**
 * Pick `n` exercises out of `pool` aiming for an even split across the
 * types present in it, instead of a flat shuffle-then-slice (which lets
 * whichever type happens to be biggest in the pool dominate a session).
 *
 * Each type gets pool.length / typeCount slots (remainder handed out to
 * random types); a type short on exercises gives its unfilled slots back,
 * which get redistributed round-robin to types that still have spare
 * exercises. The picks are then interleaved (see interleaveNoRepeat) so
 * the session doesn't run several questions of the same type in a row.
 */
function buildBalancedPool(pool, n) {
    const byType = new Map();
    for (const ex of pool) {
        if (!byType.has(ex.type)) byType.set(ex.type, []);
        byType.get(ex.type).push(ex);
    }
    const types = [...byType.keys()];
    if (types.length <= 1) return shuffle(pool).slice(0, n);

    const shuffled = new Map(types.map((t) => [t, shuffle(byType.get(t))]));

    const base = Math.floor(n / types.length);
    let remainder = n - base * types.length;
    const quota = new Map(types.map((t) => [t, base]));
    const order = shuffle(types);
    for (let i = 0; i < remainder; i++) {
        const t = order[i % order.length];
        quota.set(t, quota.get(t) + 1);
    }

    const picked = new Map();
    let deficit = 0;
    for (const t of types) {
        const avail = shuffled.get(t).length;
        const want = quota.get(t);
        const take = Math.min(avail, want);
        picked.set(t, take);
        deficit += want - take;
    }

    while (deficit > 0) {
        let progress = false;
        for (const t of types) {
            if (deficit <= 0) break;
            const avail = shuffled.get(t).length;
            const cur = picked.get(t);
            if (cur < avail) {
                picked.set(t, cur + 1);
                deficit -= 1;
                progress = true;
            }
        }
        if (!progress) break; // no type has any spare left
    }

    const buckets = new Map(types.map((t) => [t, shuffled.get(t).slice(0, picked.get(t))]));
    return interleaveNoRepeat(buckets);
}

/**
 * Flattens per-type buckets (each already shuffled) into one sequence,
 * picking randomly among the types that still have exercises left but
 * avoiding a 3rd-in-a-row of the same type whenever another type still
 * has something to offer. When only one type has anything left, its
 * exercises are used anyway (a run is unavoidable at that point).
 */
function interleaveNoRepeat(buckets, maxRun = 2) {
    const remaining = new Map([...buckets.entries()].map(([t, arr]) => [t, arr.slice()]));
    const result = [];
    let lastType = null;
    let runLength = 0;
    let totalLeft = [...remaining.values()].reduce((sum, arr) => sum + arr.length, 0);

    while (totalLeft > 0) {
        let candidates = [...remaining.entries()].filter(([, arr]) => arr.length > 0);
        if (runLength >= maxRun) {
            const avoiding = candidates.filter(([t]) => t !== lastType);
            if (avoiding.length) candidates = avoiding;
        }

        const totalWeight = candidates.reduce((sum, [, arr]) => sum + arr.length, 0);
        let r = Math.random() * totalWeight;
        let chosen = candidates[0];
        for (const candidate of candidates) {
            r -= candidate[1].length;
            if (r <= 0) {
                chosen = candidate;
                break;
            }
        }

        const [type, arr] = chosen;
        result.push(arr.shift());
        totalLeft--;
        if (type === lastType) {
            runLength++;
        } else {
            lastType = type;
            runLength = 1;
        }
    }

    return result;
}

/** Build the filtered, shuffled, sized pool for this run and show question 1. */
function startPool() {
    let pool = state.allExercises.filter((ex) => state.activeTypes.has(ex.type));
    pool = pool.filter((ex) => typePlayable(ex.type));

    if (!pool.length) {
        document.getElementById("exo-card-area").innerHTML =
            '<p class="exo-error">No exercise available for this filter.</p>';
        document.getElementById("exo-progress").textContent = "";
        return;
    }

    state.exercises = buildBalancedPool(pool, state.sessionSize || 10);
    state.score = 0;
    state.mistakes = [];
    state.byType = {};

    goToExercise(0);
}

function buildSessionToolbar() {
    const toolbar = el("div", { className: "exo-toolbar" }, [
        el("h2", { text: `🎯 ${state.sheetTitle}` }),
    ]);

    const controlsRow = el("div", { className: "exo-filter-row" }, [
        renderTypeCheckboxes(state.activeTypes, () => {
            startPool();
        }),
        renderDirectionSelector(state.direction, (value) => {
            state.direction = value;
            startPool();
        }),
    ]);

    const modeToggle = el("div", { className: "exo-mode-toggle" }, [
        el("label", {}, [
            el("input", {
                attrs: {
                    type: "radio", name: "exo-mode", value: "choice",
                    ...(state.answerMode === "choice" ? { checked: "checked" } : {}),
                },
            }),
            document.createTextNode(" Choices"),
        ]),
        el("label", {}, [
            el("input", {
                attrs: {
                    type: "radio", name: "exo-mode", value: "type",
                    ...(state.answerMode === "type" ? { checked: "checked" } : {}),
                },
            }),
            document.createTextNode(" Free typing"),
        ]),
    ]);
    modeToggle.querySelectorAll("input").forEach((input) => {
        input.addEventListener("change", (e) => {
            state.answerMode = e.target.value;
            renderCurrentView();
        });
    });
    controlsRow.appendChild(modeToggle);
    toolbar.appendChild(controlsRow);
    return toolbar;
}

// ============================================================================
// Direction resolution: turn one merged exercise into a question/answer
// "view" for the direction currently in effect. One resolveXView()
// function per registered type (see EXERCISE_TYPES above); resolveView()
// itself just dispatches to whichever the exercise's type points to.
// ============================================================================

function resolveDirection(direction) {
    if (direction === "both") {
        return Math.random() < 0.5 ? "l1-l2" : "l2-l1";
    }
    return direction;
}

function resolveQcmView(ex, direction) {
    const dir = resolveDirection(direction);
    if (dir === "l1-l2") {
        return {
            type: "qcm", direction: dir,
            question: ex.l1, answer: ex.l2,
            choices: shuffle([ex.l2, ...ex.choices_l2]),
            audio: ex.l2,
        };
    }
    return {
        type: "qcm", direction: dir,
        question: ex.l2, answer: ex.l1,
        choices: shuffle([ex.l1, ...ex.choices_l1]),
        audio: ex.l2,
    };
}

function resolveFillBlankView(ex, direction) {
    const dir = resolveDirection(direction);
    if (dir === "l1-l2") {
        return {
            type: "fill-blank", direction: dir,
            tokens: ex.l2.split(" "), blankIndex: ex.blank_index_l2,
            answer: ex.missing_l2, choices: shuffle([ex.missing_l2, ...ex.choices_l2]),
            audio: ex.l2, translation: ex.l1,
        };
    }
    return {
        type: "fill-blank", direction: dir,
        tokens: ex.l1.split(" "), blankIndex: ex.blank_index_l1,
        answer: ex.missing_l1, choices: shuffle([ex.missing_l1, ...ex.choices_l1]),
        audio: ex.l2, translation: ex.l2,
    };
}

function resolveListenView(ex) {
    return {
        type: "listen", direction: null,
        audio: ex.l2, answer: ex.l2,
        choices: shuffle([ex.l2, ...ex.choices_l2]),
        translation: ex.l1,
    };
}

/**
 * Tokens for `sentence`, from the exercise's explicit tokenisation
 * (`tokensField`, e.g. tokens_l2) when present -- so a generator or
 * the editor can keep a group like "ne ... pas" together as one
 * chip -- falling back to a naive split(" ") otherwise.
 */
function orderTokensFor(ex, sentenceField, tokensField) {
    if (Array.isArray(ex[tokensField]) && ex[tokensField].length) {
        return ex[tokensField];
    }
    return (ex[sentenceField] || "").split(" ").filter(Boolean);
}

/**
 * "Remettre les mots dans l'ordre" -- rebuild the target sentence by
 * placing its shuffled tokens (words, or hand-grouped short phrases)
 * in the right order. Correctness compares the tokens joined by a
 * single space to the original sentence, normalizeAnswer()'d the
 * same way free-typed answers are -- good enough for this exercise
 * without a second, parallel "is this sentence equal" definition.
 */
function resolveOrderView(ex, direction) {
    const dir = resolveDirection(direction);
    if (dir === "l1-l2") {
        const tokens = orderTokensFor(ex, "l2", "tokens_l2");
        return {
            type: "order", direction: dir,
            tokens: shuffle(tokens), orderedTokens: tokens,
            answer: ex.l2, audio: ex.l2, translation: ex.l1,
        };
    }
    const tokens = orderTokensFor(ex, "l1", "tokens_l1");
    return {
        type: "order", direction: dir,
        tokens: shuffle(tokens), orderedTokens: tokens,
        answer: ex.l1, audio: ex.l2, translation: ex.l2,
    };
}

function resolveView(ex, direction) {
    const entry = EXERCISE_TYPES[ex.type];
    return entry ? entry.resolveView(ex, direction) : resolveListenView(ex);
}

// ============================================================================
// Rendering one exercise
// ============================================================================

/**
 * Move to exercise `index`, resolving its view (direction, shuffled
 * choices) once -- the resolved view is kept in state.currentView so
 * that toggling the answer mode (choice/free typing) re-renders the
 * SAME question instead of re-rolling a fresh "both" direction.
 */
function goToExercise(index) {
    state.current = index;
    if (index >= state.exercises.length) {
        renderSummary();
        return;
    }
    state.currentView = resolveView(state.exercises[index], state.direction);
    renderCurrentView();
}

function updateProgress() {
    const total = state.exercises.length;
    const current = state.current + 1;

    const fill = document.getElementById("exo-progress-fill");
    if (fill) fill.style.width = `${Math.round((state.current / total) * 100)}%`;

    const text = document.getElementById("exo-progress");
    text.innerHTML = "";
    text.appendChild(el("span", { text: `Question ${current} / ${total}` }));
    text.appendChild(el("span", { text: `Score ${state.score}` }));
}

function renderCurrentView() {
    updateProgress();
    const area = document.getElementById("exo-card-area");
    area.innerHTML = "";
    const view = state.currentView;
    const entry = EXERCISE_TYPES[view.type];
    area.appendChild(entry ? entry.render(view) : renderListen(view));
}

/**
 * Record the answer, show feedback, and offer to move on.
 * `view.audio` is always a meaningful target-language sentence now
 * (a genuine simplification over v1, where the replay button was
 * only ever available in the l1->l2 direction).
 */
function markAnswer(container, isCorrect, view, userAnswer) {
    container.classList.add(isCorrect ? "exo-correct" : "exo-incorrect");

    if (!state.byType[view.type]) state.byType[view.type] = { correct: 0, total: 0 };
    state.byType[view.type].total += 1;

    if (isCorrect) {
        state.score += 1;
        state.byType[view.type].correct += 1;
    } else {
        state.mistakes.push({ view, userAnswer });
    }

    const canReplay = ttsAvailable();
    const feedbackLine = [
        el("span", {
            className: isCorrect ? "exo-feedback-ok" : "exo-feedback-ko",
            text: isCorrect ? "✅ Correct!" : `❌ Missed — answer: ${view.answer}`,
        }),
    ];
    if (canReplay) {
        feedbackLine.push(el("button", {
            className: "exo-feedback-audio",
            text: "🔊",
            attrs: { type: "button", "aria-label": "Listen to the pronunciation" },
            onclick: () => speakSafe(view.audio),
        }));
    }

    const feedback = el("div", { className: "exo-feedback" }, [
        el("p", { className: "exo-feedback-line" }, feedbackLine),
        ...(view.translation ? [el("p", { className: "exo-translation", text: view.translation })] : []),
    ]);
    container.appendChild(feedback);

    container.appendChild(el("button", {
        className: "exo-next",
        text: state.current + 1 < state.exercises.length ? "Next →" : "See score →",
        onclick: () => goToExercise(state.current + 1),
    }));

    if (canReplay) {
        speakSafe(view.audio);
    }
}

// -- QCM ----------------------------------------------------------------

function renderQcm(view) {
    const container = el("div", { className: "exo-card exo-qcm" });
    container.appendChild(el("div", { className: "exo-question-label", text: "Translate:" }));
    container.appendChild(el("div", { className: "exo-question", text: view.question }));

    if (state.answerMode === "choice") {
        const choicesEl = el("div", { className: "exo-choices" });
        view.choices.forEach((choice) => {
            const btn = el("button", {
                className: "exo-choice",
                text: choice,
                onclick: () => {
                    disableChoices(choicesEl);
                    const correct = choice === view.answer;
                    btn.classList.add(correct ? "exo-choice-correct" : "exo-choice-wrong");
                    if (!correct) highlightCorrectChoice(choicesEl, view.answer);
                    markAnswer(container, correct, view, choice);
                },
            });
            choicesEl.appendChild(btn);
        });
        container.appendChild(choicesEl);
    } else {
        container.appendChild(renderTypeInput(container, view));
    }

    return container;
}

// -- Fill in the blank ----------------------------------------------------

function renderFillBlank(view) {
    const container = el("div", { className: "exo-card exo-fill-blank" });
    container.appendChild(el("div", { className: "exo-question-label", text: "Complete the sentence:" }));

    const sentence = el("div", { className: "exo-sentence" });
    const blank = el("span", { className: "exo-blank", text: "___" });
    view.tokens.forEach((token, i) => {
        if (i === view.blankIndex) {
            sentence.appendChild(blank);
        } else {
            sentence.appendChild(document.createTextNode(token));
        }
        sentence.appendChild(document.createTextNode(" "));
    });
    container.appendChild(sentence);

    if (state.answerMode === "choice") {
        const choicesEl = el("div", { className: "exo-choices" });
        view.choices.forEach((choice) => {
            const btn = el("button", {
                className: "exo-choice",
                text: choice,
                onclick: () => {
                    disableChoices(choicesEl);
                    blank.textContent = choice;
                    const correct = choice === view.answer;
                    blank.classList.add(correct ? "exo-choice-correct" : "exo-choice-wrong");
                    btn.classList.add(correct ? "exo-choice-correct" : "exo-choice-wrong");
                    markAnswer(container, correct, view, choice);
                },
            });
            choicesEl.appendChild(btn);
        });
        container.appendChild(choicesEl);
    } else {
        container.appendChild(renderTypeInput(container, view, (value) => { blank.textContent = value; }));
    }

    return container;
}

// -- Listen and guess ------------------------------------------------------

function renderListen(view) {
    const container = el("div", { className: "exo-card exo-listen" });
    container.appendChild(el("div", { className: "exo-question-label", text: "Listen and find the sentence:" }));

    container.appendChild(el("button", {
        className: "exo-audio-btn exo-audio-btn-big",
        text: "🔊 Listen",
        onclick: () => speakSafe(view.audio),
    }));
    container.appendChild(el("button", {
        className: "exo-audio-btn",
        text: "🐢 Slowly",
        onclick: () => speakSafe(view.audio, SETTINGS.rate * SETTINGS.slowRatio),
    }));

    if (state.answerMode === "choice") {
        const choicesEl = el("div", { className: "exo-choices exo-choices-vertical" });
        view.choices.forEach((choice) => {
            const btn = el("button", {
                className: "exo-choice",
                text: choice,
                onclick: () => {
                    disableChoices(choicesEl);
                    const correct = choice === view.answer;
                    btn.classList.add(correct ? "exo-choice-correct" : "exo-choice-wrong");
                    if (!correct) highlightCorrectChoice(choicesEl, view.answer);
                    markAnswer(container, correct, view, choice);
                },
            });
            choicesEl.appendChild(btn);
        });
        container.appendChild(choicesEl);
    } else {
        container.appendChild(renderTypeInput(container, view));
    }

    speakSafe(view.audio);
    return container;
}

// -- Remettre les mots dans l'ordre ----------------------------------------

/**
 * Click tokens (from the shuffled bank) to append them to the answer
 * strip, in order; click a token already placed to send it back. No
 * drag-and-drop -- works the same with a mouse, a touchscreen, or a
 * keyboard tab+enter, and needs no extra library.
 */
function renderOrder(view) {
    const container = el("div", { className: "exo-card exo-order" });
    container.appendChild(el("div", { className: "exo-question-label", text: "Put the words in order:" }));
    if (view.translation) {
        container.appendChild(el("div", { className: "exo-question", text: view.translation }));
    }

    const answerStrip = el("div", { className: "exo-order-answer" });
    const bank = el("div", { className: "exo-order-bank" });
    const submit = el("button", { className: "exo-submit", text: "Check", attrs: { disabled: "disabled" } });

    const placed = [];
    const MIN_PLACED_TOKENS = 3;  // Allow check only if at least this many tokens are placed

    const chips = view.tokens.map((token) => {
        const chip = el("button", {
            className: "exo-order-chip",
            text: token,
            attrs: { type: "button" },
        });
        chip.onclick = () => {
            if (chip.disabled) return;
            chip.disabled = true;
            chip.classList.add("exo-order-chip-placed");
            placed.push(token);
            answerStrip.appendChild(makePlacedChip(token, chip, placed));
            submit.disabled = placed.length < MIN_PLACED_TOKENS;
        };
        return chip;
    });
    chips.forEach((chip) => bank.appendChild(chip));

    function makePlacedChip(token, sourceChip, placedArr) {
        const placedChip = el("button", {
            className: "exo-order-chip exo-order-chip-answer",
            text: token,
            attrs: { type: "button" },
        });
        placedChip.onclick = () => {
            if (submit.disabled === false && submit.dataset.locked === "1") return;
            const idx = placedArr.indexOf(token);
            if (idx !== -1) placedArr.splice(idx, 1);
            placedChip.remove();
            sourceChip.disabled = false;
            sourceChip.classList.remove("exo-order-chip-placed");
            submit.disabled = placedArr.length < MIN_PLACED_TOKENS;
        };
        return placedChip;
    }

    submit.onclick = () => {
        submit.dataset.locked = "1";
        chips.forEach((c) => { c.disabled = true; });
        [...answerStrip.children].forEach((c) => { c.disabled = true; });
        submit.disabled = true;
        const userAnswer = placed.join(" ");
        const correct = normalizeAnswer(userAnswer) === normalizeAnswer(view.answer);
        answerStrip.classList.add(correct ? "exo-choice-correct" : "exo-choice-wrong");
        markAnswer(container, correct, view, userAnswer);
    };

    container.appendChild(el("div", { className: "exo-order-strip-label", text: "Your answer:" }));
    container.appendChild(answerStrip);
    container.appendChild(el("div", { className: "exo-order-strip-label", text: "Available words:" }));
    container.appendChild(bank);
    container.appendChild(submit);

    return container;
}

// -- Free typing (shared by qcm/fill-blank/listen) -------------------------

function renderTypeInput(container, view, onValidate) {
    const wrap = el("div", { className: "exo-type-input" });
    const input = el("input", { attrs: { type: "text", placeholder: "Your answer…", autocomplete: "off" } });
    const submit = el("button", {
        className: "exo-submit",
        text: "Check",
        onclick: () => {
            const value = input.value;
            input.disabled = true;
            submit.disabled = true;
            const correct = normalizeAnswer(value) === normalizeAnswer(view.answer);
            input.classList.add(correct ? "exo-choice-correct" : "exo-choice-wrong");
            if (onValidate) onValidate(correct ? view.answer : value);
            markAnswer(container, correct, view, value);
        },
    });
    input.addEventListener("keydown", (e) => {
        if (e.key === "Enter") submit.click();
    });
    wrap.appendChild(input);
    wrap.appendChild(submit);
    return wrap;
}

function disableChoices(choicesEl) {
    choicesEl.querySelectorAll("button").forEach((b) => { b.disabled = true; });
}

function highlightCorrectChoice(choicesEl, answer) {
    choicesEl.querySelectorAll("button").forEach((b) => {
        if (b.textContent === answer) b.classList.add("exo-choice-correct");
    });
}

// ============================================================================
// End-of-session summary
// ============================================================================

function renderSummary() {
    const area = document.getElementById("exo-card-area");
    document.getElementById("exo-progress").textContent = "";
    const fill = document.getElementById("exo-progress-fill");
    if (fill) fill.style.width = "100%";
    area.innerHTML = "";

    const total = state.exercises.length;
    const scorePct = pct(state.score, total);

    // Fetch history BEFORE recording this session, to show "before / after".
    const previous = state.sheetId ? getSheetProgress(state.sheetId) : null;
    if (state.sheetId) {
        recordSession(state.sheetId, state.score, total, state.byType);
    }
    const updated = state.sheetId ? getSheetProgress(state.sheetId) : null;

    const summary = el("div", { className: "exo-card exo-summary" }, [
        el("h3", { text: "Result" }),
        el("p", { className: "exo-score", text: `${state.score} / ${total} (${scorePct}%)` }),
    ]);

    if (previous && previous.last) {
        const prevDate = new Date(previous.last.date).toLocaleDateString();
        summary.appendChild(el("p", {
            className: "exo-history-line",
            text: `Previous try: ${previous.last.score}/${previous.last.total} (${prevDate})`,
        }));
    }

    if (updated && updated.cumulative && updated.cumulative.sessionsCount > 1) {
        const cumLine = el("div", { className: "exo-cumulative" });
        cumLine.appendChild(el("p", { className: "exo-history-line", text: "Cumulative average:" }));
        const row = el("div", { className: "exo-cumulative-row" });
        ALL_TYPES.forEach((type) => {
            const stats = updated.cumulative.byType[type];
            if (!stats || !stats.total) return;
            row.appendChild(el("span", {
                className: "exo-badge",
                text: `${typeIcon(type)} ${pct(stats.correct, stats.total)}%`,
                attrs: { title: `${typeLabel(type)} — ${stats.correct}/${stats.total} over ${updated.cumulative.sessionsCount} sessions` },
            }));
        });
        cumLine.appendChild(row);
        summary.appendChild(cumLine);
    }

    if (state.mistakes.length) {
        const list = el("ul", { className: "exo-mistake-list" });
        state.mistakes.forEach(({ view, userAnswer }) => {
            const label = view.type === "qcm" ? view.question : (view.audio || "");
            list.appendChild(el("li", {
                html: `<strong>${escapeHtml(label)}</strong> → ${escapeHtml(view.answer)}` +
                      (userAnswer ? ` <span class="exo-your-answer">(you: ${escapeHtml(userAnswer)})</span>` : ""),
            }));
        });
        summary.appendChild(el("p", { text: "To review:" }));
        summary.appendChild(list);
    }

    const actions = el("div", { className: "exo-summary-actions" }, [
        el("button", { className: "exo-next", text: "🔁 Start over", onclick: () => startPool() }),
        el("button", {
            className: "exo-secondary", text: "⬅ New selection",
            onclick: () => { window.location.hash = "#/exercises"; },
        }),
    ]);
    summary.appendChild(actions);

    area.appendChild(summary);
}

// Exposed for the standalone editor (editeur-exercices.html), loaded
// via <script src="exercises.js"> next to it: the registry, plus the
// small pure functions it reuses for validation and preview so the
// editor never re-implements this file's rules under a second name.
if (typeof window !== "undefined") {
    window.EXERCISE_TYPES = EXERCISE_TYPES;
    window.ALL_TYPES = ALL_TYPES;
    window.orderTokensFor = orderTokensFor;
}
