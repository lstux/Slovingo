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
 * escapeHtml(), LANG, DATA.characters) and exercises.js (ALL_TYPES,
 * el(), renderTypeCheckboxes(), renderDirectionSelector(),
 * resetProgress(), typeLabel()) -- all called at runtime, so load
 * order relative to this file doesn't matter (see those files' own
 * docstrings for why).
 */

"use strict";

const DEFAULT_SETTINGS = {
    voiceURI: null,             // null = automatic best match for the target language
    rate: 0.9,                   // normal TTS playback rate
    slowRatio: 0.7,               // "slow" rate = rate * slowRatio
    pitch: 1.0,                   // voice pitch (Web Speech API range: 0-2, 1 = natural)
    defaultQuestionCount: 12,     // selection screen AND quick-launch use the same value
    defaultDirection: "l1-l2",
    defaultTypes: ["qcm", "fill-blank", "listen"],
    defaultAnswerMode: "choice",
    characterVoices: {},          // { "\ud83d\udc66": { voiceURI, rate, pitch } }, each field
                                    // null/absent = inherit the setting above
    dialoguePlaybackMode: "auto",  // "auto" = auto-play in sequence, "manual" = wait for click
    dialoguePauseDuration: 2,      // pause duration between dialogue lines (seconds)
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
// Theme mode (light/dark/auto)
// ============================================================================

function themeSettingKey() {
    const prefix = (LANG && LANG.site && LANG.site.storage_prefix) || "slovingo";
    return `${prefix}-theme-mode`;
}

/**
 * Load the stored theme mode, defaulting to "auto" (system preference).
 * @returns {string} "auto" | "light" | "dark"
 */
function loadThemeSetting() {
    try {
        const stored = localStorage.getItem(themeSettingKey());
        return (stored === "light" || stored === "dark") ? stored : "auto";
    } catch (err) {
        return "auto";
    }
}

/**
 * Save theme mode to localStorage.
 * @param {string} mode "auto" | "light" | "dark"
 */
function saveThemeSetting(mode) {
    try {
        localStorage.setItem(themeSettingKey(), mode);
    } catch (err) {
        // ignore
    }
}

/**
 * Apply theme mode to <html[data-mode]> and trigger CSS color-scheme change.
 * @param {string} mode "auto" | "light" | "dark"
 */
function applyThemeSetting(mode) {
    const html = document.documentElement;
    if (mode === "auto") {
        html.removeAttribute("data-mode");
    } else {
        html.setAttribute("data-mode", mode);
    }
}

/**
 * Merge `patch` into one character's overrides (voiceURI/rate/pitch),
 * leaving the others untouched, and persist. A field set to null
 * means "inherit the corresponding global Settings value" -- see
 * effectiveCharacterVoice() once step 3 wires this into actual
 * playback.
 * @param {string} emoji
 * @param {{voiceURI?: string|null, rate?: number|null, pitch?: number|null}} patch
 */
function setCharacterOverride(emoji, patch) {
    const current = (SETTINGS.characterVoices && SETTINGS.characterVoices[emoji]) || {};
    const updated = { voiceURI: null, rate: null, pitch: null, ...current, ...patch };
    saveSettings({ characterVoices: { ...(SETTINGS.characterVoices || {}), [emoji]: updated } });
}

/** A DOM-id-safe token for an emoji (ids can't contain some emoji sequences reliably). */
function emojiSlug(emoji) {
    return Array.from(emoji).map((ch) => ch.codePointAt(0).toString(16)).join("-");
}

/**
 * Resolve the voice/rate/pitch to actually use for a dialogue line,
 * given its speaker emoji (audio-card.speaker, e.g. "\ud83d\udc66") --
 * that character's own override for each field when set, the global
 * Setting otherwise. No speaker (undefined/null, e.g. a non-dialogue
 * audio-card) resolves straight to the global settings.
 * @param {string|null|undefined} speaker
 * @returns {{voiceURI: string|null, rate: number, pitch: number}}
 */
function effectiveCharacterVoice(speaker) {
    const override = (speaker && SETTINGS.characterVoices && SETTINGS.characterVoices[speaker]) || {};
    return {
        voiceURI: override.voiceURI != null ? override.voiceURI : SETTINGS.voiceURI,
        rate: override.rate != null ? override.rate : SETTINGS.rate,
        pitch: override.pitch != null ? override.pitch : SETTINGS.pitch,
    };
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

    content.appendChild(renderThemeSettingsSection());
    content.appendChild(renderAudioSettingsSection());
    content.appendChild(renderDialoguePlaybackSection());
    const characterSection = renderCharacterSettingsSection();
    if (characterSection) content.appendChild(characterSection);
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
 * Theme selector (light/dark/auto), persisted in localStorage and applied to <html[data-mode]>.
 */
function renderThemeSettingsSection() {
    const rows = [];

    const themeRow = el("div", { className: "settings-row" });
    themeRow.appendChild(el("label", { text: (LANG.ui && LANG.ui.theme_mode) || "Theme", attrs: { for: "settings-theme" } }));
    const themeSelect = el("select", { attrs: { id: "settings-theme" } });
    
    const options = [
        { value: "auto", label: (LANG.ui && LANG.ui.theme_system) || "System default" },
        { value: "light", label: (LANG.ui && LANG.ui.theme_light) || "Light" },
        { value: "dark", label: (LANG.ui && LANG.ui.theme_dark) || "Dark" }
    ];
    
    options.forEach(({ value, label }) => {
        const option = el("option", { text: label, attrs: { value } });
        themeSelect.appendChild(option);
    });
    
    const currentTheme = loadThemeSetting();
    themeSelect.value = currentTheme;
    
    themeSelect.addEventListener("change", () => {
        saveThemeSetting(themeSelect.value);
        applyThemeSetting(themeSelect.value);
    });
    
    themeRow.appendChild(themeSelect);
    rows.push(themeRow);

    return settingsSection((LANG.ui && LANG.ui.appearance) || "Appearance", rows);
}

/**
 * Voice picker + rate slider + slow-rate ratio slider + pitch slider,
 * each with a "Test" button so the effect is heard immediately
 * rather than guessed from a number.
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
        // Test the selected voice with the sample phrase
        const samplePhrase = (LANG.target_lang && LANG.target_lang.tts_sample_phrase) || (LANG.target_lang && LANG.target_lang.name) || "Test";
        speakSafe(samplePhrase, SETTINGS.rate, SETTINGS.pitch, voiceSelect.value || null);
    });

    // Normal playback rate.
    const samplePhrase = (LANG.target_lang && LANG.target_lang.tts_sample_phrase) || (LANG.target_lang && LANG.target_lang.name) || "Test";
    rows.push(renderSlider({
        id: "settings-rate",
        labelText: (LANG.ui && LANG.ui.playback_speed) || "Playback speed",
        min: 0.5, max: 1.5, step: 0.05,
        value: SETTINGS.rate,
        formatValue: (v) => `${Math.round(v * 100)}%`,
        onChange: (v) => saveSettings({ rate: v }),
        onTest: () => speakSafe(samplePhrase, SETTINGS.rate),
    }));

    // Slow playback rate, expressed as a percentage OF the normal
    // rate above (not an absolute value) -- so raising/lowering the
    // normal speed carries the "slow" speed along with it.
    rows.push(renderSlider({
        id: "settings-slow-ratio",
        labelText: (LANG.ui && LANG.ui.slow_playback_speed) || "Slow playback speed",
        min: 0.3, max: 1, step: 0.05,
        value: SETTINGS.slowRatio,
        formatValue: (v) => `${Math.round(v * 100)}% ${(LANG.ui && LANG.ui.of_normal_speed) || "of normal speed"}`,
        onChange: (v) => saveSettings({ slowRatio: v }),
        onTest: () => speakSafe(samplePhrase, SETTINGS.rate * SETTINGS.slowRatio),
    }));

    // Voice pitch. 1.0 is the browser/voice's own natural pitch (Web
    // Speech API range: 0-2); shown as a percentage of that natural
    // pitch, same idiom as the two speed sliders above.
    rows.push(renderSlider({
        id: "settings-pitch",
        labelText: (LANG.ui && LANG.ui.voice_pitch) || "Voice pitch",
        min: 0, max: 2, step: 0.1,
        value: SETTINGS.pitch,
        formatValue: (v) => `${Math.round(v * 100)}%`,
        onChange: (v) => saveSettings({ pitch: v }),
        onTest: () => speakSafe(samplePhrase, SETTINGS.rate, SETTINGS.pitch),
    }));

    return settingsSection((LANG.ui && LANG.ui.settings_audio) || "Audio", rows);
}

/**
 * Dialogue playback settings: auto/manual mode toggle and pause duration slider.
 */
function renderDialoguePlaybackSection() {
    const rows = [];

    // Auto-play mode toggle
    const modeRow = el("div", { className: "settings-row" });
    modeRow.appendChild(el("label", { text: (LANG.ui && LANG.ui.dialogue_mode) || "Playback mode", attrs: { for: "settings-dialogue-mode" } }));
    const modeSelect = el("select", { attrs: { id: "settings-dialogue-mode" } });
    
    const modeOptions = [
        { value: "auto", label: (LANG.ui && LANG.ui.dialogue_auto) || "Auto-play (sequence)" },
        { value: "manual", label: (LANG.ui && LANG.ui.dialogue_manual) || "Manual (click to continue)" }
    ];
    
    modeOptions.forEach(({ value, label }) => {
        const option = el("option", { text: label, attrs: { value } });
        modeSelect.appendChild(option);
    });
    
    modeSelect.value = SETTINGS.dialoguePlaybackMode || "auto";
    modeSelect.addEventListener("change", () => {
        saveSettings({ dialoguePlaybackMode: modeSelect.value });
    });
    
    modeRow.appendChild(modeSelect);
    rows.push(modeRow);

    // Pause duration slider
    rows.push(renderSlider({
        id: "settings-dialogue-pause",
        labelText: (LANG.ui && LANG.ui.dialogue_pause) || "Pause between lines",
        min: 0.5, max: 5, step: 0.25,
        value: SETTINGS.dialoguePauseDuration || 2,
        formatValue: (v) => `${v.toFixed(2)}s`,
        onChange: (v) => saveSettings({ dialoguePauseDuration: v }),
    }));

    return settingsSection((LANG.ui && LANG.ui.settings_dialogue) || "Dialogue playback", rows);
}

/**
 * One row per dialogue character found in the corpus (see
 * assemble.py's build_characters_map(), shipped as DATA.characters),
 * each collapsible to keep the page manageable once there are more
 * than a couple. Omitted entirely if the corpus has no characters.
 *
 * The "Test" button previews rate/pitch only for now -- it still
 * plays through the globally-resolved voice, not yet the character's
 * own chosen one. Wiring an actual per-character voice into playback
 * (dialogue audio-cards) is the next step, not this one.
 */
function renderCharacterSettingsSection() {
    const characters = (DATA && DATA.characters) || {};
    const entries = Object.entries(characters);
    if (!entries.length) return null;

    const rows = entries.map(([emoji, name]) => renderCharacterRow(emoji, name));
    return settingsSection((LANG.ui && LANG.ui.settings_characters) || "Characters", rows);
}

function renderCharacterRow(emoji, name) {
    const slug = emojiSlug(emoji);
    const details = el("details", { className: "settings-character" });
    details.appendChild(el("summary", { text: `${emoji} ${name}` }));

    const override = () => (SETTINGS.characterVoices && SETTINGS.characterVoices[emoji]) || {};

    // Voice picker: same target-language filtering as the global one,
    // plus a leading "use the default voice" option.
    const voiceRow = el("div", { className: "settings-row" });
    voiceRow.appendChild(el("label", {
        text: (LANG.ui && LANG.ui.voice) || "Voice",
        attrs: { for: `settings-char-voice-${slug}` },
    }));
    const voiceSelect = el("select", { attrs: { id: `settings-char-voice-${slug}` } });
    voiceRow.appendChild(voiceSelect);
    details.appendChild(voiceRow);

    const populateVoiceOptions = () => {
        voiceSelect.innerHTML = "";
        voiceSelect.appendChild(el("option", {
            text: (LANG.ui && LANG.ui.voice_use_default) || "Use default voice",
            attrs: { value: "" },
        }));
        const ttsPrefix = (LANG.target_lang.tts_code || "").split("-")[0].toLowerCase();
        const voices = ("speechSynthesis" in window ? window.speechSynthesis.getVoices() : [])
            .filter((v) => v.lang.toLowerCase().startsWith(ttsPrefix));
        voices.forEach((voice) => {
            voiceSelect.appendChild(el("option", { text: `${voice.name} (${voice.lang})`, attrs: { value: voice.voiceURI } }));
        });
        voiceSelect.value = override().voiceURI || "";
    };
    populateVoiceOptions();
    if ("speechSynthesis" in window) {
        window.speechSynthesis.addEventListener("voiceschanged", populateVoiceOptions);
    }
    voiceSelect.addEventListener("change", () => {
        setCharacterOverride(emoji, { voiceURI: voiceSelect.value || null });
    });

    // Rate and pitch each start at this character's override if it
    // has one, else at the current global value -- and can be reset
    // back to "inherit" independently of one another.
    details.appendChild(renderSlider({
        id: `settings-char-rate-${slug}`,
        labelText: (LANG.ui && LANG.ui.playback_speed) || "Playback speed",
        min: 0.5, max: 1.5, step: 0.05,
        value: override().rate != null ? override().rate : SETTINGS.rate,
        formatValue: (v) => `${Math.round(v * 100)}%`,
        onChange: (v) => setCharacterOverride(emoji, { rate: v }),
        onTest: () => {
            const v = effectiveCharacterVoice(emoji);
            speakSafe(name, override().rate != null ? override().rate : SETTINGS.rate, v.pitch, v.voiceURI);
        },
        isOverridden: override().rate != null,
        onReset: () => { setCharacterOverride(emoji, { rate: null }); return SETTINGS.rate; },
    }));

    details.appendChild(renderSlider({
        id: `settings-char-pitch-${slug}`,
        labelText: (LANG.ui && LANG.ui.voice_pitch) || "Voice pitch",
        min: 0, max: 2, step: 0.1,
        value: override().pitch != null ? override().pitch : SETTINGS.pitch,
        formatValue: (v) => `${Math.round(v * 100)}%`,
        onChange: (v) => setCharacterOverride(emoji, { pitch: v }),
        onTest: () => {
            const v = effectiveCharacterVoice(emoji);
            speakSafe(name, v.rate, override().pitch != null ? override().pitch : SETTINGS.pitch, v.voiceURI);
        },
        isOverridden: override().pitch != null,
        onReset: () => { setCharacterOverride(emoji, { pitch: null }); return SETTINGS.pitch; },
    }));

    return details;
}

/**
 * One labelled range input with a live readout, a "Test" button that
 * speaks LANG.target_lang.name with whatever rate/pitch `onTest`
 * chooses to preview, and an optional "reset to inherited default"
 * button (used by character rows; the global sliders above don't
 * pass onReset and get none).
 *
 * onReset, when provided, must apply the underlying change (clear
 * the override) AND return the value the slider should now display
 * -- the reset button updates the input/readout itself rather than
 * triggering a full re-render, so an open <details> panel doesn't
 * snap shut on click.
 */
function renderSlider({ id, labelText, min, max, step, value, formatValue, onChange, onTest, onReset, isOverridden }) {
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
        onclick: onTest,
    });

    let resetBtn = null;
    if (onReset) {
        resetBtn = el("button", {
            className: "settings-reset-btn",
            text: "\u21a9",
            attrs: { type: "button", "aria-label": "Reset to default" },
            onclick: () => {
                const resetValue = onReset();
                input.value = String(resetValue);
                readout.textContent = formatValue(resetValue);
                resetBtn.disabled = true;
            },
        });
        resetBtn.disabled = !isOverridden;
    }

    input.addEventListener("input", () => {
        readout.textContent = formatValue(parseFloat(input.value));
    });
    input.addEventListener("change", () => {
        onChange(parseFloat(input.value));
        if (resetBtn) resetBtn.disabled = false;
    });

    wrap.appendChild(input);
    wrap.appendChild(readout);
    wrap.appendChild(testBtn);
    if (resetBtn) wrap.appendChild(resetBtn);
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
