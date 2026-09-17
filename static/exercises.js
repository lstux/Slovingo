/**
 * exercises.js -- Slovingo v2 exercise engine.
 *
 * Selection screen + session for qcm / fill-blank / listen exercises,
 * consuming the merged l1/l2 format from smd2exercises.py (see
 * areas/v2-rewrite): one exercise object covers both practice
 * directions, resolved into a "view" (question/answer/choices) at
 * render time rather than stored twice.
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

const ALL_TYPES = ["qcm", "fill-blank", "listen"];

/** Session state, rebuilt each time a session starts. */
const state = {
    answerMode: "choice",       // "choice" or "type"
    allExercises: [],
    exercises: [],
    current: 0,
    currentView: null,
    score: 0,
    mistakes: [],
    byType: {},                 // { qcm: {correct, total}, "fill-blank": {...}, listen: {...} }
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

function typeLabel(type) {
    return { qcm: "\ud83d\udd24 QCM", "fill-blank": "\u270f\ufe0f Fill in the blank", listen: "\ud83d\udd0a Listen" }[type] || type;
}

function typeIcon(type) {
    return { qcm: "\ud83d\udd24", "fill-blank": "\u270f\ufe0f", listen: "\ud83d\udd0a" }[type] || "\u2754";
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
        text: "\u25b6\ufe0f Start the session",
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
    const effectiveTypes = ALL_TYPES.filter((t) => state.activeTypes.has(t) && (t !== "listen" || ttsAvailable()));
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
 * session. "Listen" is disabled (and unchecked) whenever no
 * target-language voice is available -- no point offering questions
 * that can't be heard.
 */
function renderTypeCheckboxes(selectedSet, onChange) {
    const row = el("div", { className: "exo-type-filter" });
    const inputs = [];
    ALL_TYPES.forEach((type) => {
        const disabled = type === "listen" && !ttsAvailable();
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
        { value: "l1-l2", label: `${LANG.native_lang.flag}\u2192${LANG.target_lang.flag} ${nativeAbbr}\u2192${targetAbbr}` },
        { value: "l2-l1", label: `${LANG.target_lang.flag}\u2192${LANG.native_lang.flag} ${targetAbbr}\u2192${nativeAbbr}` },
        { value: "both", label: "\ud83d\udd00 Both directions" },
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

/** Build the filtered, shuffled, sized pool for this run and show question 1. */
function startPool() {
    let pool = state.allExercises.filter((ex) => state.activeTypes.has(ex.type));
    if (!ttsAvailable()) {
        pool = pool.filter((ex) => ex.type !== "listen");
    }

    if (!pool.length) {
        document.getElementById("exo-card-area").innerHTML =
            '<p class="exo-error">No exercise available for this filter.</p>';
        document.getElementById("exo-progress").textContent = "";
        return;
    }

    state.exercises = shuffle(pool).slice(0, state.sessionSize || 10);
    state.score = 0;
    state.mistakes = [];
    state.byType = {};

    goToExercise(0);
}

function buildSessionToolbar() {
    const toolbar = el("div", { className: "exo-toolbar" }, [
        el("h2", { text: `\ud83c\udfaf ${state.sheetTitle}` }),
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
// "view" for the direction currently in effect.
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

function resolveView(ex, direction) {
    if (ex.type === "qcm") return resolveQcmView(ex, direction);
    if (ex.type === "fill-blank") return resolveFillBlankView(ex, direction);
    return resolveListenView(ex);
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

    if (view.type === "qcm") area.appendChild(renderQcm(view));
    else if (view.type === "fill-blank") area.appendChild(renderFillBlank(view));
    else area.appendChild(renderListen(view));
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
            text: isCorrect ? "\u2705 Correct!" : `\u274c Missed \u2014 answer: ${view.answer}`,
        }),
    ];
    if (canReplay) {
        feedbackLine.push(el("button", {
            className: "exo-feedback-audio",
            text: "\ud83d\udd0a",
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
        text: state.current + 1 < state.exercises.length ? "Next \u2192" : "See score \u2192",
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
        text: "\ud83d\udd0a Listen",
        onclick: () => speakSafe(view.audio),
    }));
    container.appendChild(el("button", {
        className: "exo-audio-btn",
        text: "\ud83d\udc22 Slowly",
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

// -- Free typing (shared by all 3 types) -----------------------------------

function renderTypeInput(container, view, onValidate) {
    const wrap = el("div", { className: "exo-type-input" });
    const input = el("input", { attrs: { type: "text", placeholder: "Your answer\u2026", autocomplete: "off" } });
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
                attrs: { title: `${typeLabel(type)} \u2014 ${stats.correct}/${stats.total} over ${updated.cumulative.sessionsCount} sessions` },
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
                html: `<strong>${escapeHtml(label)}</strong> \u2192 ${escapeHtml(view.answer)}` +
                      (userAnswer ? ` <span class="exo-your-answer">(you: ${escapeHtml(userAnswer)})</span>` : ""),
            }));
        });
        summary.appendChild(el("p", { text: "To review:" }));
        summary.appendChild(list);
    }

    const actions = el("div", { className: "exo-summary-actions" }, [
        el("button", { className: "exo-next", text: "\ud83d\udd01 Start over", onclick: () => startPool() }),
        el("button", {
            className: "exo-secondary", text: "\u2b05 New selection",
            onclick: () => { window.location.hash = "#/exercises"; },
        }),
    ]);
    summary.appendChild(actions);

    area.appendChild(summary);
}
