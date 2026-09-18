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

    SETTINGS = loadSettings();
    if (typeof applyExerciseDefaultsFromSettings === "function") {
        applyExerciseDefaultsFromSettings();
    }

    // Apply saved theme preference (light/dark/auto)
    const themeMode = typeof loadThemeSetting === "function" ? loadThemeSetting() : "auto";
    if (typeof applyThemeSetting === "function") {
        applyThemeSetting(themeMode);
    }

    // Apply saved display font preference (serif/mono/grotesk)
    if (typeof applyDisplayFontSetting === "function") {
        applyDisplayFontSetting(SETTINGS.displayFont || "grotesk");
    }

    // Load user name from localStorage
    if (typeof loadUserName === "function") {
        window.USER_NAME = loadUserName();
    }

    initializeSpeechSynthesis();

    document.getElementById("nav-home").addEventListener("click", () => {
        window.location.hash = "#/";
    });
    document.getElementById("nav-exercises").addEventListener("click", handleExercisesButtonClick);
    document.getElementById("nav-translations").addEventListener("click", toggleAllTranslations);
    document.getElementById("nav-settings").addEventListener("click", () => {
        window.location.hash = "#/settings";
    });
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
    } else if (hash === "#/settings") {
        renderSettingsScreen();
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

    // Get a safe fallback for USER_NAME (from LANG or empty string)
    const userNameFallback = (LANG && LANG.site && LANG.site.user_name_default) || "";
    const displayName = window.USER_NAME || userNameFallback;

    // Replace [USER_NAME] placeholder with a span that can be updated live
    let working = text.replace(/\[USER_NAME\]/g, `__USER_NAME_MARKER__${escapeHtml(displayName)}__USER_NAME_MARKER__`);

    // Protect [ASK_USER_NAME] with a sentinel that survives escapeHtml
    let askUserNameCount = 0;
    working = working.replace(/\[ASK_USER_NAME\]/g, () => {
        return `__ASK_USER_NAME_PLACEHOLDER_${askUserNameCount++}__`;
    });

    const speakables = [];
    working = working.replace(/\[\[(.+?)\]\]/g, (_match, inner) => {
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

    // Replace speakable sentinels back
    speakables.forEach((inner, index) => {
        const placeholder = `\u0000SPEAKABLE${index}\u0000`;
        const replacement = `<span class="speakable speakable-hint">${escapeHtml(inner)}</span>`;
        working = working.split(placeholder).join(replacement);
    });

    // Replace ASK_USER_NAME sentinels back with a marker that replaceAskUserNameInputs can find
    // Use a marker element that is findable but won't break rendering
    for (let i = 0; i < askUserNameCount; i++) {
        const placeholder = `__ASK_USER_NAME_PLACEHOLDER_${i}__`;
        const replacement = `<span data-ask-user-name-marker="${i}"></span>`;
        working = working.split(placeholder).join(replacement);
    }

    // Replace USER_NAME markers back with updateable spans
    working = working.replace(/__USER_NAME_MARKER__(.+?)__USER_NAME_MARKER__/g, 
        '<span class="user-name-display">$1</span>');

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

/**
 * Every sheet id across every group, flattened in display order --
 * the single continuous flow the "previous/next" pager follows,
 * regardless of category or group boundaries.
 * @returns {string[]}
 */
function flatSheetIds() {
    return DATA.groups.flatMap((group) => group.sheets.map((sheet) => sheet.id));
}

/**
 * The sheet immediately before/after `sheetId` in the flattened,
 * whole-course order.
 * @param {string} sheetId
 * @param {1|-1} direction
 * @returns {string|null} null at either end of the course.
 */
function adjacentSheetId(sheetId, direction) {
    const ids = flatSheetIds();
    const index = ids.indexOf(sheetId);
    if (index === -1) return null;
    const target = index + direction;
    return target >= 0 && target < ids.length ? ids[target] : null;
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

/**
 * Split a sheet's raw title into a kicker (breadcrumb-style context)
 * and the actual title, on whichever dash separates them -- em dash
 * "\u2014", en dash "\u2013", or a plain hyphen, as long as it's
 * flanked by spaces (so a hyphenated single word like
 * "Aspects-verbaux" is never mistaken for a separator). The corpus
 * isn't consistent about which dash it uses from one sheet to the
 * next, so all three are accepted.
 *
 * "Série Rodina (1/5) \u2014 Moja rodina" -> kicker "Série Rodina
 * (1/5)", title "Moja rodina". A title with no such separator (e.g.
 * most Vocabulary sheets) returns kicker: null, title unchanged.
 *
 * @param {string} rawTitle
 * @returns {{kicker: string|null, title: string}}
 */
function splitTitle(rawTitle) {
    const match = rawTitle.match(/^(.+?)\s+[\u2014\u2013-]\s+(.+)$/);
    if (match) {
        return { kicker: match[1].trim(), title: match[2].trim() };
    }
    return { kicker: null, title: rawTitle };
}

/**
 * Build the small breadcrumb line shown above a sheet's title: the
 * kicker parsed out of the sheet's own title (splitTitle()) when it
 * has one, falling back to the series' "Subgroup - fiche NN" or the
 * plain category label otherwise.
 */
function kickerFor(group, sheet) {
    const { kicker } = splitTitle(sheet.title);
    if (kicker) {
        return kicker;
    }
    if (group.category === "series") {
        const index = group.sheets.findIndex((s) => s.id === sheet.id) + 1;
        return `${subgroupLabelFor(group)} - fiche ${String(index).padStart(2, "0")}`;
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
    setPageTitle(splitTitle(sheet.title).title);
    updateToolbarForSheetView(true);
    setExercisesButtonMode("launch", sheet.id);
    setPagerNav(
        adjacentSheetId(sheet.id, -1) ? `#/sheet/${encodeURIComponent(adjacentSheetId(sheet.id, -1))}` : null,
        adjacentSheetId(sheet.id, 1) ? `#/sheet/${encodeURIComponent(adjacentSheetId(sheet.id, 1))}` : null,
    );

    const content = document.getElementById("content");
    content.innerHTML = "";
    if (sheet.image) {
        content.appendChild(renderImage(sheet.image));
    }
    sheet.content.forEach((block) => {
        content.appendChild(renderBlock(block));
    });

    // Replace [ASK_USER_NAME] marker spans with actual input elements
    replaceAskUserNameInputs(content);

    initializeAudioCards();
    initializeDialoguePlayback(content);
    initializeSpeakableElements();
    updateTtsAvailability();
}

function renderNotFound(sheetId) {
    applyTheme("uvod");
    setKicker("");
    setPageTitle("404");
    updateToolbarForSheetView(false);
    setExercisesButtonMode("selection");
    hidePagerNav();
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
    hidePagerNav();

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
/**
 * Render one group as a collapsible section, mirroring the
 * .index-section / .index-grid / .index-card markup already styled
 * in style.css for the old index/exercises selection screens.
 *
 * Shows a group-average score badge, same as the exercises selection
 * screen (see exercises.js's renderGroupSelectionSection()) -- only
 * sheets that actually HAVE exercises count towards it, so a group
 * with an unplayable sheet (e.g. an Annex sheet with no audio-card or
 * translate-table to build exercises from) isn't unfairly dragged
 * down by a sheet that can never be practiced.
 */
function renderGroupSection(group, nextCardNumber) {
    const section = document.createElement("details");
    section.className = "index-section";
    section.dataset.theme = themeForGroup(group);

    const summary = document.createElement("summary");
    const heading = document.createElement("h2");
    const categoryLabel = categoryLabelFor(group);
    const subgroupLabel = subgroupLabelFor(group);
    heading.textContent = subgroupLabel ? `${categoryLabel} - ${subgroupLabel}` : categoryLabel;

    const scorableIds = group.sheets.filter((sheet) => EXERCISES.sheets[sheet.id]).map((sheet) => sheet.id);
    const score = averageScore(scorableIds);
    if (score) {
        const badge = document.createElement("span");
        badge.className = `index-score-badge index-score-${scoreRatioClass(score.avg, 100)}`;
        badge.textContent = `${score.avg}%`;
        badge.title = `${score.count}/${scorableIds.length}`;
        heading.appendChild(badge);
    }

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
    title.textContent = splitTitle(sheet.title).title;
    card.appendChild(title);

    const progress = getSheetProgress(sheet.id);
    if (progress && progress.last) {
        const badge = document.createElement("span");
        badge.className = `index-score-badge index-score-${scoreRatioClass(progress.last.score, progress.last.total)}`;
        badge.textContent = `${progress.last.score}/${progress.last.total}`;
        badge.title = new Date(progress.last.date).toLocaleDateString();
        card.appendChild(badge);
    }

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
    hidePagerNav();

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
 * Find the voice to speak with. If the person picked a specific one
 * in Settings (SETTINGS.voiceURI), honor it as long as it's still
 * available; otherwise auto-detect: exact tts_code -> same language
 * prefix -> voice name containing voice_hint -> none.
 */
function findTargetVoice(preferredVoiceURI) {
    const voices = window.speechSynthesis.getVoices();
    if (!voices.length) {
        return null;
    }

    const uri = preferredVoiceURI !== undefined ? preferredVoiceURI : (SETTINGS && SETTINGS.voiceURI);
    if (uri) {
        const chosen = voices.find((v) => v.voiceURI === uri);
        if (chosen) return chosen;
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
 * @param {number} [rate] Defaults to SETTINGS.rate (the "normal"
 *     speed from Settings) when omitted. Pass an explicit rate for
 *     "slow" playback (SETTINGS.rate * SETTINGS.slowRatio), a
 *     character override, or the Settings screen's live preview.
 * @param {number} [pitch] Defaults to SETTINGS.pitch when omitted.
 * @param {string} [voiceURI] Defaults to SETTINGS.voiceURI (the
 *     global voice choice, itself "automatic" when null) when
 *     omitted. Pass a specific voiceURI to speak as one dialogue
 *     character (see effectiveCharacterVoice() in settings.js).
 */
/**
 * Speak text using Web Speech API.
 * @param {string} text The text to speak
 * @param {number} [rate] Playback rate (default from SETTINGS)
 * @param {number} [pitch] Voice pitch (default from SETTINGS)
 * @param {string} [voiceURI] Voice URI (default automatic)
 * @param {HTMLElement} [highlightElement] Element to add .speaking class during playback
 */
function speak(text, rate, pitch, voiceURI, highlightElement) {
    if (!("speechSynthesis" in window)) {
        alert("Speech synthesis is not available on this device.");
        return;
    }
    if (!targetVoiceAvailable) {
        showTtsWarning();
        return;
    }

    const effectiveRate = rate === undefined ? (SETTINGS ? SETTINGS.rate : 0.9) : rate;
    const effectivePitch = pitch === undefined ? (SETTINGS ? SETTINGS.pitch : 1.0) : pitch;

    stopSpeaking();
    currentUtterance = new SpeechSynthesisUtterance(text);
    currentUtterance.lang = LANG.target_lang.tts_code;
    currentUtterance.rate = effectiveRate;
    currentUtterance.pitch = effectivePitch;
    const voice = findTargetVoice(voiceURI);
    if (voice) {
        currentUtterance.voice = voice;
    }
    
    // Handle visual feedback: add .speaking class to the text element during playback
    if (highlightElement) {
        currentUtterance.onstart = () => {
            highlightElement.classList.add("speaking");
        };
        currentUtterance.onend = () => {
            highlightElement.classList.remove("speaking");
        };
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
 * default. Cards from a dialogue carry a `speaker` emoji
 * (card.dataset.speaker, set by renderAudioCard()) -- their voice,
 * rate and pitch resolve through effectiveCharacterVoice() (in
 * settings.js), so a dialogue plays each character in their own
 * configured voice if the person set one, and the global voice
 * otherwise.
 */
function initializeAudioCards() {
    document.querySelectorAll(".audio-card").forEach((card) => {
        const text = card.querySelector(".audio-text");
        if (!text) return;
        const voice = effectiveCharacterVoice(card.dataset.speaker);
        text.onclick = () => speak(text.textContent.trim(), voice.rate, voice.pitch, voice.voiceURI, text);

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

/**
 * Detect dialogue sections (headings followed by only audio-cards with speakers)
 * and add a "Play dialogue" button before the first audio-card in each section.
 */
function initializeDialoguePlayback(contentElement) {
    const headings = contentElement.querySelectorAll("h2, h3");
    
    headings.forEach((heading) => {
        let nextElement = heading.nextElementSibling;
        const audioCards = [];
        let isDialogue = true;
        
        // Collect all audio-cards until we hit a heading or non-audio-card
        while (nextElement && nextElement.classList) {
            if (nextElement.tagName.match(/^H[2-6]$/)) break;
            if (nextElement.classList.contains("audio-card")) {
                audioCards.push(nextElement);
                nextElement = nextElement.nextElementSibling;
            } else if (nextElement.classList.contains("paragraph") || 
                       nextElement.classList.contains("list") ||
                       nextElement.classList.contains("blockquote")) {
                // Non-audio-card content ends the dialogue section
                isDialogue = false;
                break;
            } else {
                nextElement = nextElement.nextElementSibling;
            }
        }
        
        // Check if ALL audio-cards have speakers (are dialogue lines)
        if (audioCards.length > 0 && isDialogue) {
            const allHaveSpeaker = audioCards.every((card) => {
                const textEl = card.querySelector(".audio-text");
                // Check if the text starts with an emoji (speaker marker)
                return textEl && /^\p{Emoji}/u.test(textEl.textContent);
            });
            
            if (allHaveSpeaker) {
                // Add "Play dialogue" button before the first audio-card
                const playButton = document.createElement("button");
                playButton.className = "dialogue-play-btn";
                playButton.innerHTML = "▶ " + ((LANG.ui && LANG.ui.play_dialogue) || "Play dialogue");
                playButton.type = "button";
                playButton.onclick = () => playDialogue(audioCards);
                
                audioCards[0].insertAdjacentElement("beforebegin", playButton);
            }
        }
    });
}

/**
 * Play a sequence of audio-cards (dialogue lines) in order.
 * Mode: "auto" = play all in sequence with pauses, "manual" = wait for user
 */
function playDialogue(audioCards) {
    let currentIndex = 0;
    let isPlaying = true;
    let continueButton = null;
    
    const clearContinueButton = () => {
        if (continueButton && continueButton.parentNode) {
            continueButton.remove();
        }
        continueButton = null;
    };
    
    const playNextCard = () => {
        if (currentIndex >= audioCards.length || !isPlaying) {
            isPlaying = false;
            clearContinueButton();
            if (window._dialogueCleanup) {
                window._dialogueCleanup();
                window._dialogueCleanup = null;
            }
            return;
        }
        
        const card = audioCards[currentIndex];
        const textEl = card.querySelector(".audio-text");
        const voice = effectiveCharacterVoice(card.dataset.speaker);
        
        if (!textEl) {
            currentIndex++;
            playNextCard();
            return;
        }
        
        const text = textEl.textContent.trim();
        
        stopSpeaking();
        clearContinueButton();
        currentUtterance = new SpeechSynthesisUtterance(text);
        currentUtterance.lang = LANG.target_lang.tts_code;
        currentUtterance.rate = voice.rate || (SETTINGS ? SETTINGS.rate : 0.9);
        currentUtterance.pitch = voice.pitch || (SETTINGS ? SETTINGS.pitch : 1.0);
        
        const voiceObj = findTargetVoice(voice.voiceURI);
        if (voiceObj) {
            currentUtterance.voice = voiceObj;
        }
        
        textEl.classList.add("speaking");
        
        currentUtterance.onend = () => {
            textEl.classList.remove("speaking");
            currentIndex++;
            
            const mode = (SETTINGS && SETTINGS.dialoguePlaybackMode) || "auto";
            const pauseDuration = (SETTINGS && SETTINGS.dialoguePauseDuration) || 2;
            
            if (mode === "auto") {
                setTimeout(playNextCard, pauseDuration * 1000);
            } else if (currentIndex < audioCards.length) {
                // Manual mode: show "Continue" button
                const nextCard = audioCards[currentIndex];
                continueButton = document.createElement("button");
                continueButton.className = "dialogue-continue-btn";
                continueButton.textContent = (LANG.ui && LANG.ui.dialogue_continue) || "Continue (Space)";
                continueButton.type = "button";
                continueButton.onclick = () => playNextCard();
                nextCard.insertAdjacentElement("beforebegin", continueButton);
            }
        };
        
        window.speechSynthesis.speak(currentUtterance);
    };
    
    // Handle Space key for manual mode
    const mode = (SETTINGS && SETTINGS.dialoguePlaybackMode) || "auto";
    if (mode === "manual") {
        const handleSpace = (e) => {
            if (e.code === "Space" && isPlaying && currentIndex < audioCards.length) {
                e.preventDefault();
                playNextCard();
            }
        };
        window.addEventListener("keydown", handleSpace);
        window._dialogueCleanup = () => {
            window.removeEventListener("keydown", handleSpace);
            clearContinueButton();
        };
    }
    
    playNextCard();
}

/**
 * Replace [ASK_USER_NAME] marker spans with actual input elements.
 * Call this after appending new blocks to the DOM.
 */
function replaceAskUserNameInputs(containerElement) {
    // Find all span markers created by renderText
    const markers = containerElement.querySelectorAll("[data-ask-user-name-marker]");
    
    markers.forEach((marker) => {
        const input = document.createElement("input");
        input.className = "ask-user-name-input";
        input.type = "text";
        input.placeholder = (LANG.site && LANG.site.user_name_placeholder) || "Your name";
        input.value = window.USER_NAME || "";

        input.addEventListener("input", () => {
            saveUserName(input.value);
        });

        input.addEventListener("keydown", (e) => {
            if (e.key === "Enter") {
                saveUserName(input.value);
                input.blur();
            }
        });

        // Replace the marker span with the input
        marker.replaceWith(input);
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
            if (text) speak(text);
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
 * Show the previous/next pager (both the top row, flanking the
 * title, and the pair of buttons flanking the footer text) wired to
 * the given hashes. A null hash disables that button (still visible,
 * greyed out) rather than hiding it -- reached the start/end of the
 * flow, not "no pager here at all" (see hidePagerNav() for that
 * case). The footer itself (#pager-bottom) is permanent chrome and is
 * never hidden -- only its two buttons are.
 * @param {string|null} prevHash
 * @param {string|null} nextHash
 */
function setPagerNav(prevHash, nextHash) {
    wirePagerButton("pager-top-prev", prevHash);
    wirePagerButton("pager-top-next", nextHash);
    wirePagerButton("pager-bottom-prev", prevHash);
    wirePagerButton("pager-bottom-next", nextHash);
}

/**
 * Hide the pager's four buttons -- for views with no "previous/next"
 * sense (home, the exercises selection screen, a mixed multi-sheet
 * session). The title and the footer text stay exactly as they are;
 * only the buttons around them disappear.
 */
function hidePagerNav() {
    ["pager-top-prev", "pager-top-next", "pager-bottom-prev", "pager-bottom-next"].forEach((id) => {
        document.getElementById(id).classList.add("exo-toolbar-hidden");
    });
}

function wirePagerButton(id, hash) {
    const btn = document.getElementById(id);
    btn.classList.remove("exo-toolbar-hidden");
    btn.disabled = !hash;
    btn.onclick = hash ? () => { window.location.hash = hash; } : null;
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
        const n = (SETTINGS && SETTINGS.defaultQuestionCount) || 12;
        window.location.hash = `#/exercises/session?cards=${encodeURIComponent(sheetId)}&n=${n}`;
    } else {
        window.location.hash = "#/exercises";
    }
}
