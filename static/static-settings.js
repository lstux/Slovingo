/**
 * settings.js -- Slovingo v2 user settings.
 *
 * Voice, playback speed, and exercise defaults (question count,
 * direction, types, answer mode), stored in localStorage next to
 * progress.js's store. Read/write pattern mirrors progress.js:
 * settingsKey() is computed lazily (LANG isn't loaded yet when this
 * script first parses), and SETTINGS itself is populated once, by
 * app.js's boot() right after LANG is fetched -- not at this file's
 * own parse time, since exercises.js's initial `state` needs
 * SETTINGS to already be a plain object by the time app.js calls
 * applyExerciseDefaultsFromSettings() (see boot()).
 *
 * Depends on app.js (applyTheme(), setKicker(), setPageTitle(),
 * setToolbarButtons(), setExercisesButtonMode(), hidePagerNav(),
 * escapeHtml(), LANG) and exercises.js (ALL_TYPES, el(),
 * renderTypeCheckboxes(), renderDirectionSelector(), resetProgress(),
 * typeLabel()) -- all called at runtime, so load order relative to
 * this file doesn't matter (see those files' own docstrings for why).
 */

"use strict";

const DEFAULT_SETTINGS = {
    voiceURI: null,             // null = automatic best match for the target language
    rate: 0.9,                   // normal TTS playback rate
    slowRatio: 0.7,               // "slow" rate = rate * slowRatio
    defaultQuestionCount: 12,     // selection screen AND quick-launch use the same value
    defaultDirection: "l1-l2",
    defaultTypes: ["qcm", "fill-blank", "listen"],
    defaultAnswerMode: "choice",
};

/**
 * The active settings, merged over DEFAULT_SETTINGS. Populated by
 * app.js's boot(); null before that.
 * @type {object|null}
 */
let SETTINGS = null;

function settingsKey() {
    const prefix = (LANG && LANG.site && LANG.site.storage_prefix) || "slovingo";
    return `${prefix}-settings`;
}

/** @returns {object} DEFAULT_SETTINGS merged with whatever's stored. */
function loadSettings() {
    try {
        const stored = JSON.parse(localStorage.getItem(settingsKey()));
        return { ...DEFAULT_SETTINGS, ...(stored || {}) };
    } catch (err) {
        return { ...DEFAULT_SETTINGS };
    }
}

/**
 * Merge `patch` into SETTINGS and persist the result.
 * @param {object} patch
 */
function saveSettings(patch) {
    SETTINGS = { ...SETTINGS, ...patch };
    try {
        localStorage.setItem(settingsKey(), JSON.stringify(SETTINGS));
    } catch (err) {
        // Storage unavailable (private browsing, quota...) -- the
        // setting still applies for this session, just isn't kept.
    }
}

function resetSettingsToDefaults() {
    try {
        localStorage.removeItem(settingsKey());
    } catch (err) {
        // ignore
    }
    SETTINGS = { ...DEFAULT_SETTINGS };
}

// ============================================================================
// Settings screen
// ============================================================================

function renderSettingsScreen() {
    applyTheme("uvod");
    setKicker("");
    setPageTitle((LANG.ui && LANG.ui.settings) || "Settings");
    setToolbarButtons({});
    setExercisesButtonMode("selection");
    hidePagerNav();

    const content = document.getElementById("content");
    content.innerHTML = "";

    content.appendChild(renderAudioSettingsSection());
    content.appendChild(renderExerciseSettingsSection());
    content.appendChild(renderDataSettingsSection());
}

function settingsSection(titleText, children) {
    const section = el("section", { className: "settings-section" });
    section.appendChild(el("h2", { text: titleText }));
    children.forEach((child) => section.appendChild(child));
    return section;
}

/**
 * Voice picker + rate slider + slow-rate ratio slider, each with a
 * "Test" button so the effect is heard immediately rather than
 * guessed from a number.
 */
function renderAudioSettingsSection() {
    const rows = [];

    // Voice picker -- lists every voice whose lang matches the target
    // language, by tts_code prefix (same matching logic as
    // findTargetVoice()'s prefix fallback). Populated once voices are
    // available; some browsers report an empty list until the
    // `voiceschanged` event fires, so this re-renders when it does.
    const voiceRow = el("div", { className: "settings-row" });
    voiceRow.appendChild(el("label", { text: (LANG.ui && LANG.ui.voice) || "Voice", attrs: { for: "settings-voice" } }));
    const voiceSelect = el("select", { attrs: { id: "settings-voice" } });
    voiceRow.appendChild(voiceSelect);
    rows.push(voiceRow);

    const populateVoiceOptions = () => {
        voiceSelect.innerHTML = "";
        const auto = el("option", { text: (LANG.ui && LANG.ui.voice_automatic) || "Automatic", attrs: { value: "" } });
        voiceSelect.appendChild(auto);

        const ttsPrefix = (LANG.target_lang.tts_code || "").split("-")[0].toLowerCase();
        const voices = ("speechSynthesis" in window ? window.speechSynthesis.getVoices() : [])
            .filter((v) => v.lang.toLowerCase().startsWith(ttsPrefix));
        voices.forEach((voice) => {
            const option = el("option", { text: `${voice.name} (${voice.lang})`, attrs: { value: voice.voiceURI } });
            voiceSelect.appendChild(option);
        });
        voiceSelect.value = SETTINGS.voiceURI || "";
    };
    populateVoiceOptions();
    if ("speechSynthesis" in window) {
        window.speechSynthesis.addEventListener("voiceschanged", populateVoiceOptions);
    }
    voiceSelect.addEventListener("change", () => {
        saveSettings({ voiceURI: voiceSelect.value || null });
        updateTtsAvailability();
    });

    // Normal playback rate.
    rows.push(renderRateSlider({
        id: "settings-rate",
        labelText: (LANG.ui && LANG.ui.playback_speed) || "Playback speed",
        min: 0.5, max: 1.5, step: 0.05,
        value: SETTINGS.rate,
        formatValue: (v) => `${Math.round(v * 100)}%`,
        onChange: (v) => saveSettings({ rate: v }),
        testRate: () => SETTINGS.rate,
    }));

    // Slow playback rate, expressed as a percentage OF the normal
    // rate above (not an absolute value) -- so raising/lowering the
    // normal speed carries the "slow" speed along with it.
    rows.push(renderRateSlider({
        id: "settings-slow-ratio",
        labelText: (LANG.ui && LANG.ui.slow_playback_speed) || "Slow playback speed",
        min: 0.3, max: 1, step: 0.05,
        value: SETTINGS.slowRatio,
        formatValue: (v) => `${Math.round(v * 100)}% ${(LANG.ui && LANG.ui.of_normal_speed) || "of normal speed"}`,
        onChange: (v) => saveSettings({ slowRatio: v }),
        testRate: () => SETTINGS.rate * SETTINGS.slowRatio,
    }));

    return settingsSection((LANG.ui && LANG.ui.settings_audio) || "Audio", rows);
}

/**
 * One labelled range input with a live percentage readout and a
 * "Test" button that speaks LANG.target_lang.name at the resulting
 * rate -- generic across languages (no hardcoded sample phrase to
 * translate for every course).
 */
function renderRateSlider({ id, labelText, min, max, step, value, formatValue, onChange, testRate }) {
    const row = el("div", { className: "settings-row settings-row-slider" });
    row.appendChild(el("label", { text: labelText, attrs: { for: id } }));

    const wrap = el("div", { className: "settings-slider-wrap" });
    const input = el("input", {
        attrs: { type: "range", id, min: String(min), max: String(max), step: String(step), value: String(value) },
    });
    const readout = el("span", { className: "settings-readout", text: formatValue(value) });
    const testBtn = el("button", {
        className: "settings-test-btn",
        text: "\ud83d\udd0a",
        attrs: { type: "button", "aria-label": "Test" },
        onclick: () => speakSafe(LANG.target_lang.name, testRate()),
    });

    input.addEventListener("input", () => {
        readout.textContent = formatValue(parseFloat(input.value));
    });
    input.addEventListener("change", () => {
        onChange(parseFloat(input.value));
    });

    wrap.appendChild(input);
    wrap.appendChild(readout);
    wrap.appendChild(testBtn);
    row.appendChild(wrap);
    return row;
}

/**
 * Exercise defaults: question count, direction, active types, answer
 * mode -- reusing exercises.js's own renderDirectionSelector() /
 * renderTypeCheckboxes() so the controls look and behave exactly
 * like the ones on the selection screen, rather than a second
 * implementation to keep in sync.
 */
function renderExerciseSettingsSection() {
    const rows = [];

    const countRow = el("div", { className: "settings-row" });
    countRow.appendChild(el("label", {
        text: (LANG.ui && LANG.ui.default_question_count) || "Default number of questions",
        attrs: { for: "settings-question-count" },
    }));
    const countInput = el("input", {
        attrs: { type: "number", id: "settings-question-count", min: "1", value: String(SETTINGS.defaultQuestionCount) },
    });
    countInput.addEventListener("change", () => {
        const n = parseInt(countInput.value, 10);
        if (n > 0) saveSettings({ defaultQuestionCount: n });
    });
    countRow.appendChild(countInput);
    rows.push(countRow);

    const directionRow = el("div", { className: "settings-row" });
    directionRow.appendChild(el("label", { text: (LANG.ui && LANG.ui.default_direction) || "Default direction" }));
    directionRow.appendChild(renderDirectionSelector(SETTINGS.defaultDirection, (value) => {
        saveSettings({ defaultDirection: value });
    }));
    rows.push(directionRow);

    const typesRow = el("div", { className: "settings-row" });
    typesRow.appendChild(el("label", { text: (LANG.ui && LANG.ui.default_types) || "Default exercise types" }));
    const typesSet = new Set(SETTINGS.defaultTypes);
    typesRow.appendChild(renderTypeCheckboxes(typesSet, () => {
        saveSettings({ defaultTypes: [...typesSet] });
    }));
    rows.push(typesRow);

    const modeRow = el("div", { className: "settings-row" });
    modeRow.appendChild(el("label", { text: (LANG.ui && LANG.ui.default_answer_mode) || "Default answer mode" }));
    const modeSelect = el("select", {}, [
        el("option", { text: (LANG.ui && LANG.ui.answer_mode_choice) || "Choices", attrs: { value: "choice" } }),
        el("option", { text: (LANG.ui && LANG.ui.answer_mode_type) || "Free typing", attrs: { value: "type" } }),
    ]);
    modeSelect.value = SETTINGS.defaultAnswerMode;
    modeSelect.addEventListener("change", () => saveSettings({ defaultAnswerMode: modeSelect.value }));
    modeRow.appendChild(modeSelect);
    rows.push(modeRow);

    return settingsSection((LANG.ui && LANG.ui.settings_exercises) || "Exercises", rows);
}

function renderDataSettingsSection() {
    const resetProgressBtn = el("button", {
        className: "settings-danger-btn",
        text: (LANG.ui && LANG.ui.reset_progress) || "\ud83d\uddd1\ufe0f Reset my progress",
        onclick: () => {
            const message = (LANG.ui && LANG.ui.reset_progress_confirm) || "Erase every score saved on this device?";
            if (window.confirm(message)) {
                resetProgress();
            }
        },
    });

    const resetSettingsBtn = el("button", {
        className: "settings-danger-btn",
        text: (LANG.ui && LANG.ui.reset_settings) || "\u21a9\ufe0f Reset settings to defaults",
        onclick: () => {
            const message = (LANG.ui && LANG.ui.reset_settings_confirm) || "Reset every setting on this page to its default?";
            if (window.confirm(message)) {
                resetSettingsToDefaults();
                renderSettingsScreen();
            }
        },
    });

    return settingsSection((LANG.ui && LANG.ui.settings_data) || "Data", [resetProgressBtn, resetSettingsBtn]);
}
