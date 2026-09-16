/**
 * app.js -- Slovingo v2 front-end.
 *
 * Single-page app: fetches lang.json / data.json / exercises.json
 * once, then renders everything client-side from those three
 * objects. Replaces the old per-sheet HTML generation (smd2html.py)
 * -- the JSON now carries structure, this file carries presentation.
 *
 * Sections below:
 *   1. Boot / data loading
 *   2. Router (hash-based)
 *   3. Text rendering (speakable + inline markdown)
 *   4. Content block renderers
 *   5. Sheet view
 *   6. Home view
 *   7. Exercises view (placeholder -- built in the next step)
 *   8. Text-to-speech (ported from v1, trimmed of server-side-only bits)
 *   9. Audio-card / speakable interactivity
 *  10. Small DOM helpers
 */

"use strict";

// ============================================================================
// 1. Boot / data loading
// ============================================================================

/** @type {object|null} Parsed lang.json. */
let LANG = null;
/** @type {object|null} Parsed data.json ({ groups: [...] }). */
let DATA = null;
/** @type {object|null} Parsed exercises.json ({ sheets: {...} }). */
let EXERCISES = null;

/**
 * Series subgroup slugs that have a dedicated color theme in
 * style.css (see the [data-theme="..."] rules). Anything else --
 * other categories, or a series subgroup without its own theme yet
 * (e.g. the Kronika story arcs) -- falls back to the neutral "uvod"
 * theme.
 */
const SERIES_THEME_KEYS = new Set([
    "rodina", "doma", "jedlo", "cas", "nakupy",
    "mesto", "pocasie", "tatry", "velkanoc",
]);

/** Fetch the three JSON sources, wire up navigation, then route. */
async function boot() {
    const content = document.getElementById("content");
    try {
        [LANG, DATA, EXERCISES] = await Promise.all([
            fetchJson("lang.json"),
            fetchJson("data.json"),
            fetchJson("exercises.json"),
        ]);
    } catch (err) {
        content.innerHTML = `<p class="loading">Failed to load data: ${escapeHtml(String(err))}</p>`;
        return;
    }

    if (LANG.site && LANG.site.title) {
        document.title = LANG.site.title;
    }
    applyUiLabels();
    initializeSpeechSynthesis();

    document.getElementById("nav-home").addEventListener("click", () => {
        window.location.hash = "#/";
    });
    document.getElementById("nav-exercises").addEventListener("click", handleExercisesButtonClick);
    document.getElementById("nav-translations").addEventListener("click", toggleAllTranslations);
    window.addEventListener("hashchange", route);

    route();
}

/** @param {string} path @returns {Promise<object>} */
async function fetchJson(path) {
    const response = await fetch(path);
    if (!response.ok) {
        throw new Error(`${path}: HTTP ${response.status}`);
    }
    return response.json();
}

/** Fill every [data-ui="key"] element's text from lang.json's `ui` map. */
function applyUiLabels() {
    document.querySelectorAll("[data-ui]").forEach((el) => {
        const key = el.dataset.ui;
        if (LANG.ui && LANG.ui[key]) {
            el.textContent = LANG.ui[key];
        }
    });
}

document.addEventListener("DOMContentLoaded", boot);

// ============================================================================
// 2. Router
// ============================================================================

/**
 * Hash-based routing:
 *   #/                  -- home (browse all sheets)
 *   #/sheet/<id>         -- one sheet
 *   #/exercises          -- exercises (placeholder for now)
 */
function route() {
    const hash = window.location.hash || "#/";
    const sheetMatch = hash.match(/^#\/sheet\/(.+)$/);

    if (sheetMatch) {
        renderSheet(decodeURIComponent(sheetMatch[1]));
    } else if (hash.startsWith("#/exercises")) {
        if (typeof routeExercises === "function") {
            routeExercises(hash);
        } else {
            renderExercisesPlaceholder();
        }
    } else {
        renderHome();
    }
    window.scrollTo(0, 0);
}

// ============================================================================
// 3. Text rendering (speakable + inline markdown)
// ============================================================================

/**
 * Escape text for safe HTML insertion.
 * @param {string} text
 * @returns {string}
 */
function escapeHtml(text) {
    const div = document.createElement("div");
    div.textContent = text;
    return div.innerHTML;
}

/**
 * Render a content-block text field to HTML: resolves [[speakable]]
 * markers, **bold**, *italic*, `code`, and [text](url) links -- the
 * client-side mirror of smd2data.py's resolve_speakable() plus v1's
 * markdown_inline(). Speakable spans are protected with placeholders
 * before HTML-escaping, then restored last, exactly like the old
 * Python implementation, so escaping never touches their markup.
 *
 * @param {string|null|undefined} text
 * @returns {string} HTML string (safe to assign to innerHTML).
 */
function renderText(text) {
    if (text === null || text === undefined) {
        return "";
    }

    const speakables = [];
    let working = text.replace(/\[\[(.+?)\]\]/g, (_match, inner) => {
        const index = speakables.length;
        speakables.push(inner.trim());
        return `\u0000SPEAKABLE${index}\u0000`;
    });

    working = escapeHtml(working);
    working = working.replace(
        /\[([^\]]+)\]\((https?:\/\/[^)]+)\)/g,
        '<a href="$2" target="_blank" rel="noopener">$1</a>'
    );
    working = working.replace(/`([^`]+)`/g, "<code>$1</code>");
    working = working.replace(/\*\*(.+?)\*\*/g, "<strong>$1</strong>");
    working = working.replace(/\*(.+?)\*/g, "<em>$1</em>");

    speakables.forEach((inner, index) => {
        const placeholder = `\u0000SPEAKABLE${index}\u0000`;
        const replacement = `<span class="speakable speakable-hint">${escapeHtml(inner)}</span>`;
        working = working.split(placeholder).join(replacement);
    });

    return working;
}

// ============================================================================
// 4. Content block renderers
// ============================================================================

/**
 * Render one content block (see smd2data.py for the block shapes) to
 * a DOM element.
 * @param {object} block
 * @returns {HTMLElement}
 */
function renderBlock(block) {
    switch (block.type) {
        case "heading":
            return renderHeading(block);
        case "paragraph":
            return renderParagraph(block);
        case "list":
            return renderList(block);
        case "blockquote":
            return renderBlockquote(block);
        case "hr":
            return document.createElement("hr");
        case "image":
            return renderImage(block);
        case "table":
            return renderTable(block);
        case "audio-card":
            return renderAudioCard(block);
        default: {
            console.warn(`Unknown content block type: ${block.type}`);
            const fallback = document.createElement("p");
            fallback.textContent = `[unsupported block: ${block.type}]`;
            return fallback;
        }
    }
}

function renderHeading(block) {
    const level = Math.min(Math.max(block.level, 1), 6);
    const heading = document.createElement(`h${level}`);
    heading.innerHTML = renderText(block.text);
    return heading;
}

function renderParagraph(block) {
    const p = document.createElement("p");
    p.innerHTML = renderText(block.text);
    return p;
}

function renderList(block) {
    const list = document.createElement(block.ordered ? "ol" : "ul");
    block.items.forEach((item) => {
        const li = document.createElement("li");
        li.innerHTML = renderText(item);
        list.appendChild(li);
    });
    return list;
}

function renderBlockquote(block) {
    const quote = document.createElement("blockquote");
    block.lines.forEach((line) => {
        const p = document.createElement("p");
        p.innerHTML = renderText(line);
        quote.appendChild(p);
    });
    return quote;
}

function renderImage(block) {
    const figure = document.createElement("figure");
    figure.className = "fiche-illustration";
    const img = document.createElement("img");
    img.src = block.src;
    img.alt = block.caption || "";
    img.loading = "lazy";
    figure.appendChild(img);
    if (block.caption) {
        const caption = document.createElement("figcaption");
        caption.innerHTML = renderText(block.caption);
        figure.appendChild(caption);
    }
    return figure;
}

/**
 * Render a `table` block. `speakable_column` (set by smd2data.py
 * when a header matches the target language) marks that column as
 * both the target-language color (LANG.target_lang.css_class) and
 * clickable-to-speak ("speakable"). For an ordinary 2-column table,
 * the other column is assumed to be the native-language side and
 * gets the native color -- the same assumption smd2exercises.py
 * makes when pairing vocabulary. Tables with no target column at all
 * (e.g. a purely informational table) render as plain tables.
 */
function renderTable(block) {
    const hasTargetColumn = block.speakable_column !== null && block.speakable_column !== undefined;
    const nativeColumn = hasTargetColumn && block.columns.length === 2
        ? 1 - block.speakable_column
        : null;

    const table = document.createElement("table");
    if (hasTargetColumn) {
        table.className = "translate-table";
    }

    const thead = document.createElement("thead");
    const headRow = document.createElement("tr");
    block.columns.forEach((column, index) => {
        const th = document.createElement("th");
        if (index === block.speakable_column) {
            th.classList.add(LANG.target_lang.css_class);
            th.innerHTML = (LANG.target_lang.flag || "") + renderText(column);
        } else if (index === nativeColumn) {
            th.classList.add(LANG.native_lang.css_class);
            th.innerHTML = (LANG.native_lang.flag || "") + renderText(column);
        } else {
            th.innerHTML = renderText(column);
        }
        headRow.appendChild(th);
    });
    thead.appendChild(headRow);
    table.appendChild(thead);

    const tbody = document.createElement("tbody");
    block.rows.forEach((row) => {
        const tr = document.createElement("tr");
        row.forEach((cell, index) => {
            const td = document.createElement("td");
            if (index === block.speakable_column) {
                td.classList.add(LANG.target_lang.css_class, "speakable");
            } else if (index === nativeColumn) {
                td.classList.add(LANG.native_lang.css_class);
            }
            td.innerHTML = renderText(cell);
            tr.appendChild(td);
        });
        tbody.appendChild(tr);
    });
    table.appendChild(tbody);

    return table;
}

/**
 * Render an `audio-card` block. The speaker marker (if any) is
 * re-prefixed into the visible text -- same visual result as v1,
 * which never separated it -- but is kept as a data attribute too,
 * so a future feature (one TTS voice per character) has something
 * to key off without re-parsing the sentence.
 *
 * Natural translation and literal breakdown are deliberately rendered
 * as one flat sequence of <p> elements, with no visual distinction
 * between them -- matching v1's output exactly (see
 * smd2html.py's audio-card handling).
 */
function renderAudioCard(block) {
    const card = document.createElement("div");
    card.className = "audio-card";
    if (block.speaker) {
        card.dataset.speaker = block.speaker;
    }

    const text = document.createElement("div");
    text.className = "audio-text";
    const prefix = block.speaker ? `${block.speaker} ` : "";
    text.innerHTML = prefix + renderText(block.phrase);
    card.appendChild(text);

    const hasTranslation = block.natural || block.literal.length || block.notes.length;
    if (hasTranslation) {
        const translation = document.createElement("div");
        translation.className = "audio-translation";
        if (block.natural) {
            const p = document.createElement("p");
            p.innerHTML = renderText(block.natural);
            translation.appendChild(p);
        }
        block.literal.forEach((line) => {
            const p = document.createElement("p");
            p.innerHTML = renderText(line);
            translation.appendChild(p);
        });
        block.notes.forEach((note) => {
            const p = document.createElement("p");
            p.className = "comments";
            p.innerHTML = renderText(note);
            translation.appendChild(p);
        });
        card.appendChild(translation);
    }

    return card;
}

// ============================================================================
// 5. Sheet view
// ============================================================================

/**
 * Find the group a sheet belongs to.
 * @param {string} sheetId
 * @returns {object|undefined}
 */
function findGroupForSheet(sheetId) {
    return DATA.groups.find((group) => group.sheets.some((sheet) => sheet.id === sheetId));
}

/** @param {object|undefined} group @returns {string} A theme key for [data-theme]. */
function themeForGroup(group) {
    if (group && group.category === "series" && SERIES_THEME_KEYS.has(group.subgroup)) {
        return group.subgroup;
    }
    return "uvod";
}

/**
 * Resolve a group's display label: an override from lang.json's
 * `subgroups` map (for diacritics an ASCII filename can't carry, e.g.
 * "velkanoc" -> "Veľká noc"), falling back to the label smd2data.py
 * derived automatically from the filename's original casing.
 */
function subgroupLabelFor(group) {
    if (!group.subgroup) {
        return "";
    }
    if (LANG.subgroups && LANG.subgroups[group.subgroup]) {
        return LANG.subgroups[group.subgroup];
    }
    return group.subgroup_label || group.subgroup;
}

/** @param {object} group @returns {string} The category's display label. */
function categoryLabelFor(group) {
    return (LANG.categories && LANG.categories[group.category]) || group.category;
}

/** Build the small breadcrumb line shown above a sheet's title. */
function kickerFor(group, sheet) {
    if (group.category === "series") {
        const index = group.sheets.findIndex((s) => s.id === sheet.id) + 1;
        return `${subgroupLabelFor(group)} \u00b7 fiche ${String(index).padStart(2, "0")}`;
    }
    return categoryLabelFor(group);
}

/**
 * Render one sheet: illustration (if any) followed by its content
 * blocks, in order.
 * @param {string} sheetId
 */
function renderSheet(sheetId) {
    const group = findGroupForSheet(sheetId);
    const sheet = group ? group.sheets.find((s) => s.id === sheetId) : null;

    if (!sheet) {
        renderNotFound(sheetId);
        return;
    }

    applyTheme(themeForGroup(group));
    setKicker(kickerFor(group, sheet));
    setPageTitle(sheet.title);
    updateToolbarForSheetView(true);
    setExercisesButtonMode("launch", sheet.id);

    const content = document.getElementById("content");
    content.innerHTML = "";
    if (sheet.image) {
        content.appendChild(renderImage(sheet.image));
    }
    sheet.content.forEach((block) => {
        content.appendChild(renderBlock(block));
    });

    initializeAudioCards();
    initializeSpeakableElements();
    updateTtsAvailability();
}

function renderNotFound(sheetId) {
    applyTheme("uvod");
    setKicker("");
    setPageTitle("404");
    updateToolbarForSheetView(false);
    setExercisesButtonMode("selection");
    const content = document.getElementById("content");
    content.innerHTML = "";
    const p = document.createElement("p");
    p.className = "loading";
    p.textContent = `Sheet not found: ${sheetId}`;
    content.appendChild(p);
}

// ============================================================================
// 6. Home view
// ============================================================================

/** Render the home screen: every group, each with its sheets. */
function renderHome() {
    applyTheme("uvod");
    setKicker("");
    setPageTitle((LANG.site && LANG.site.title) || "Slovingo");
    updateToolbarForSheetView(false);
    setExercisesButtonMode("selection");

    const content = document.getElementById("content");
    content.innerHTML = "";
    // Running counter for .index-num, shared across every group on the
    // page -- matches v1, which numbered sheets continuously along the
    // whole curriculum rather than restarting at 1 in each series.
    let cardCounter = 0;
    DATA.groups.forEach((group) => {
        const section = renderGroupSection(group, () => {
            cardCounter += 1;
            return cardCounter;
        });
        content.appendChild(section);
    });
}

/**
 * Render one group as a collapsible section, mirroring the
 * .index-section / .index-grid / .index-card markup already styled
 * in style.css for the old index/exercises selection screens.
 */
function renderGroupSection(group, nextCardNumber) {
    const section = document.createElement("details");
    section.className = "index-section";
    section.dataset.theme = themeForGroup(group);

    const summary = document.createElement("summary");
    const heading = document.createElement("h2");
    const categoryLabel = categoryLabelFor(group);
    const subgroupLabel = subgroupLabelFor(group);
    heading.textContent = subgroupLabel ? `${categoryLabel} \u00b7 ${subgroupLabel}` : categoryLabel;
    summary.appendChild(heading);
    section.appendChild(summary);

    const grid = document.createElement("div");
    grid.className = "index-grid";
    group.sheets.forEach((sheet) => {
        grid.appendChild(renderSheetCard(sheet, group, nextCardNumber));
    });
    section.appendChild(grid);

    return section;
}

function renderSheetCard(sheet, group, nextCardNumber) {
    const card = document.createElement("a");
    card.className = "index-card";
    card.href = `#/sheet/${encodeURIComponent(sheet.id)}`;
    card.dataset.theme = themeForGroup(group);

    if (group.category === "series") {
        const num = document.createElement("span");
        num.className = "index-num";
        num.textContent = String(nextCardNumber()).padStart(2, "0");
        card.appendChild(num);
    }

    const title = document.createElement("span");
    title.className = "index-card-title";
    title.textContent = sheet.title;
    card.appendChild(title);

    return card;
}

// ============================================================================
// 7. Exercises view (placeholder)
// ============================================================================

/**
 * Placeholder screen for #/exercises. The real exercise engine
 * (selection screen + session, ported from exercises.js) is a
 * separate step -- this just avoids a dead link in the toolbar.
 */
function renderExercisesPlaceholder() {
    applyTheme("uvod");
    setKicker("");
    setPageTitle((LANG.site && LANG.site.title) || "Slovingo");
    updateToolbarForSheetView(false);
    setExercisesButtonMode("selection");

    const content = document.getElementById("content");
    content.innerHTML = "";
    const p = document.createElement("p");
    p.className = "loading";
    p.textContent = (LANG.ui && LANG.ui.exercises_coming_soon) || "Coming soon.";
    content.appendChild(p);
}

// ============================================================================
// 8. Text-to-speech (ported from v1's app.js; header-detection helpers
//    dropped since speakable_column is now decided once, in Python)
// ============================================================================

let currentUtterance = null;
let targetVoiceAvailable = false;
let ttsWarningShown = false;

/**
 * Find the best available voice for the target language.
 * Priority: exact tts_code -> same language prefix -> voice name
 * containing voice_hint -> none.
 */
function findTargetVoice() {
    const voices = window.speechSynthesis.getVoices();
    if (!voices.length) {
        return null;
    }

    const ttsCode = (LANG.target_lang.tts_code || "").toLowerCase();
    const ttsPrefix = ttsCode.split("-")[0];

    if (ttsCode) {
        const exact = voices.find((v) => v.lang.toLowerCase() === ttsCode);
        if (exact) return exact;
    }
    if (ttsPrefix) {
        const prefixed = voices.find((v) => v.lang.toLowerCase().startsWith(ttsPrefix));
        if (prefixed) return prefixed;
    }
    const hint = (LANG.target_lang.voice_hint || "").toLowerCase();
    if (hint) {
        const named = voices.find((v) => v.name.toLowerCase().includes(hint));
        if (named) return named;
    }
    return null;
}

/** Show a one-time toast when no target-language voice is available. */
function showTtsWarning() {
    if (ttsWarningShown) return;
    ttsWarningShown = true;

    const template = (LANG.ui && LANG.ui.tts_unavailable)
        || "No {target_name} voice is available on this device.";
    const message = template.replace("{target_name}", LANG.target_lang.name);

    const toast = document.createElement("div");
    toast.className = "tts-toast";
    toast.textContent = message;
    document.body.appendChild(toast);
    setTimeout(() => toast.remove(), 6000);
}

/**
 * Speak `text` in the target language.
 * @param {string} text
 * @param {number} [rate=0.9] 0.9 normal, 0.65 slow.
 */
function speak(text, rate = 0.9) {
    if (!("speechSynthesis" in window)) {
        alert("Speech synthesis is not available on this device.");
        return;
    }
    if (!targetVoiceAvailable) {
        showTtsWarning();
        return;
    }

    stopSpeaking();
    currentUtterance = new SpeechSynthesisUtterance(text);
    currentUtterance.lang = LANG.target_lang.tts_code;
    currentUtterance.rate = rate;
    currentUtterance.pitch = 1.0;
    const voice = findTargetVoice();
    if (voice) {
        currentUtterance.voice = voice;
    }
    window.speechSynthesis.speak(currentUtterance);
}

function stopSpeaking() {
    if ("speechSynthesis" in window) {
        window.speechSynthesis.cancel();
    }
    currentUtterance = null;
}

/**
 * Re-check voice availability and grey out TTS controls when none is
 * found -- called on load and again on the `voiceschanged` event,
 * since some browsers (notably Android) populate the voice list
 * asynchronously.
 */
function updateTtsAvailability() {
    targetVoiceAvailable = !!findTargetVoice();

    document.querySelectorAll(".audio-buttons button, .speakable").forEach((el) => {
        if (targetVoiceAvailable) {
            el.style.opacity = "";
            el.style.cursor = "";
            if (el.classList.contains("speakable")) {
                el.title = "Click to listen";
            }
        } else {
            el.style.opacity = "0.4";
            el.style.cursor = "not-allowed";
            el.title = `No ${LANG.target_lang.name} voice available`;
        }
    });
}

function initializeSpeechSynthesis() {
    if (!("speechSynthesis" in window)) {
        return;
    }
    updateTtsAvailability();
    window.speechSynthesis.addEventListener("voiceschanged", updateTtsAvailability);
}

// ============================================================================
// 9. Audio-card / speakable interactivity
// ============================================================================

function toggleTranslation(el) {
    el.classList.toggle("hidden-translation");
}

/** Whether audio-card translations are currently shown globally. */
let translationsVisible = false;

/** Toolbar "Translations" button: show/hide every translation at once. */
function toggleAllTranslations() {
    translationsVisible = !translationsVisible;
    document.querySelectorAll(".audio-translation").forEach((el) => {
        el.classList.toggle("hidden-translation", !translationsVisible);
    });
}

/**
 * Wire up every .audio-card currently in the DOM: click-to-speak on
 * the sentence, a toggle button for its translation, hidden by
 * default.
 */
function initializeAudioCards() {
    document.querySelectorAll(".audio-card").forEach((card) => {
        const text = card.querySelector(".audio-text");
        if (!text) return;
        text.onclick = () => speak(text.textContent.trim(), 0.9);

        const translation = card.querySelector(".audio-translation");
        if (translation) {
            translation.classList.add("hidden-translation");
        }

        const buttons = document.createElement("div");
        buttons.className = "audio-buttons";
        if (translation) {
            const toggle = document.createElement("button");
            toggle.textContent = "\ud83d\udc40";
            toggle.title = "Show/hide translation";
            toggle.type = "button";
            toggle.onclick = () => toggleTranslation(translation);
            buttons.appendChild(toggle);
        }
        card.appendChild(buttons);
    });
}

/** Wire up every .speakable element currently in the DOM. */
function initializeSpeakableElements() {
    document.querySelectorAll(".speakable").forEach((el) => {
        el.title = targetVoiceAvailable
            ? "Click to listen"
            : `No ${LANG.target_lang.name} voice available`;
        el.addEventListener("click", () => {
            const text = el.textContent.trim();
            if (text) speak(text, 0.9);
        });
    });
}

// ============================================================================
// 10. Small DOM helpers
// ============================================================================

function applyTheme(theme) {
    document.body.dataset.theme = theme;
}

function setKicker(text) {
    document.getElementById("kicker").textContent = text;
}

function setPageTitle(text) {
    document.getElementById("page-title").textContent = text;
}

/**
 * Show/hide the toolbar's "Translations" button (sheet view only --
 * the "Exercises"/"Sheet" button is always visible, see
 * setExercisesButtonMode() below).
 * @param {{translations?: boolean}} [options]
 */
function setToolbarButtons({ translations = false } = {}) {
    document.getElementById("nav-translations").classList.toggle("exo-toolbar-hidden", !translations);
}

/** Backward-compatible alias used by the sheet/home views above. */
function updateToolbarForSheetView(isSheet) {
    setToolbarButtons({ translations: isSheet });
}

/**
 * The toolbar's 2nd/3rd button doubles as "Exercises" and "Sheet"
 * depending on context, instead of two separate buttons that were
 * never shown at the same time anyway:
 *
 *   - "launch": on a sheet -- clicking starts a 12-question session
 *     for that sheet directly, skipping the selection screen.
 *   - "back-to-sheet": in an exercise session tied to a single sheet
 *     -- clicking returns to that sheet.
 *   - "selection" (default): everywhere else -- clicking goes to the
 *     exercises selection screen.
 *
 * Called by renderSheet()/renderHome() here, and by
 * renderSelectionScreen()/startSessionFromQuery() in exercises.js.
 *
 * @param {"launch"|"back-to-sheet"|"selection"} mode
 * @param {string} [sheetId] Required for "launch" and "back-to-sheet".
 */
function setExercisesButtonMode(mode, sheetId) {
    const btn = document.getElementById("nav-exercises");
    btn.dataset.mode = mode;
    btn.dataset.sheetId = sheetId || "";

    if (mode === "back-to-sheet") {
        btn.innerHTML = `\ud83d\udcd6 <span>${(LANG.ui && LANG.ui.sheet) || "Sheet"}</span>`;
    } else {
        btn.innerHTML = `\ud83c\udfaf <span>${(LANG.ui && LANG.ui.exercises) || "Exercises"}</span>`;
    }
}

function handleExercisesButtonClick() {
    const btn = document.getElementById("nav-exercises");
    const mode = btn.dataset.mode;
    const sheetId = btn.dataset.sheetId;

    if (mode === "back-to-sheet" && sheetId) {
        window.location.hash = `#/sheet/${encodeURIComponent(sheetId)}`;
    } else if (mode === "launch" && sheetId) {
        window.location.hash = `#/exercises/session?cards=${encodeURIComponent(sheetId)}&n=12`;
    } else {
        window.location.hash = "#/exercises";
    }
}
