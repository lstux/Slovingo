/**
 * backup.js -- Slovingo v2: export / import of settings and progress.
 *
 * Everything the app keeps lives in localStorage, under keys prefixed
 * with LANG.site.storage_prefix (see progress.js / settings.js / app.js).
 * This file turns those keys into ONE readable JSON file (backup, or
 * moving to another device) and back.
 *
 * File layout (version 1) -- settings and progress are kept apart:
 *
 *   {
 *     "format": "slovingo-backup", "version": 1,
 *     "exportedAt": "2026-10-06T16:00:00.000Z",
 *     "course": { "storagePrefix": "...", "targetLang": "sk", "nativeLang": "fr" },
 *     "settings": {
 *       "userName": "...",          // the only personal datum
 *       "themeMode": "auto",         // auto | light | dark
 *       "app": { ...SETTINGS... }    // voices, speeds, exercise defaults...
 *     },
 *     "progress": {
 *       "exercises": { "<sheetId>": { last, cumulative } },
 *       "visitedSheets": { "<sheetId>": true },
 *       "visitDays": ["YYYY-MM-DD", ...],   // streak
 *       "lastSheetId": "...", "lastSeriesSheetId": "..."
 *     }
 *   }
 *
 * Import rules:
 *   - progress is MERGED, keeping the best of both sides (see
 *     mergeExerciseEntry()); nothing local is ever lowered;
 *   - settings are REPLACED by the file's (a backup is a snapshot);
 *   - older / partial files are accepted and adapted (normalizeBackup()).
 *
 * The merge logic is pure (no DOM, no localStorage) so it can be unit
 * tested in Node. Depends at runtime on LANG, DEFAULT_SETTINGS
 * (settings.js) and uiLabel()/el() (exercises.js) -- all called lazily.
 */

"use strict";

const BACKUP_FORMAT = "slovingo-backup";
const BACKUP_VERSION = 1;
const THEME_MODES = ["auto", "light", "dark"];

// ============================================================================
// Storage keys (all derived from the course's storage prefix)
// ============================================================================

function backupStoragePrefix() {
    return (typeof LANG === "object" && LANG && LANG.site && LANG.site.storage_prefix) || "slovingo";
}

function backupKeys() {
    const p = backupStoragePrefix();
    return {
        progress: progressKey(),
        visitDays: `${p}-last-visits`,
        visitedSheets: `${p}-visited-sheets`,
        lastSheet: `${p}-last-sheet`,
        lastSeriesSheet: `${p}-last-series-sheet`,
        settings: `${p}-settings`,
        userName: `${p}-user-name`,
        themeMode: `${p}-theme-mode`,
    };
}

function readJsonKey(key, fallback) {
    try {
        const raw = localStorage.getItem(key);
        return raw == null ? fallback : (JSON.parse(raw) ?? fallback);
    } catch (err) {
        return fallback;
    }
}

// ============================================================================
// Export
// ============================================================================

/** @returns {object} The current settings + progress as a backup object. */
function buildBackup() {
    const k = backupKeys();
    const lastSheet = readJsonKey(k.lastSheet, null);
    const lastSeries = readJsonKey(k.lastSeriesSheet, null);
    const themeStored = localStorage.getItem(k.themeMode);
    return {
        format: BACKUP_FORMAT,
        version: BACKUP_VERSION,
        exportedAt: new Date().toISOString(),
        course: {
            storagePrefix: backupStoragePrefix(),
            targetLang: (LANG.target_lang && LANG.target_lang.code) || null,
            nativeLang: (LANG.native_lang && LANG.native_lang.code) || null,
        },
        settings: {
            userName: localStorage.getItem(k.userName) || "",
            themeMode: THEME_MODES.includes(themeStored) ? themeStored : "auto",
            app: readJsonKey(k.settings, {}),
        },
        progress: {
            exercises: readJsonKey(k.progress, {}),
            visitedSheets: readJsonKey(k.visitedSheets, {}),
            visitDays: Object.keys(readJsonKey(k.visitDays, {})).sort(),
            lastSheetId: (lastSheet && lastSheet.sheetId) || null,
            lastSeriesSheetId: (lastSeries && lastSeries.sheetId) || null,
        },
    };
}

/** "Slovingo-{userName}-{YYYY-MM-DD}.json", the name made filename-safe. */
function backupFilename(backup) {
    const now = new Date();
    const pad = (n) => String(n).padStart(2, "0");
    const date = `${now.getFullYear()}-${pad(now.getMonth() + 1)}-${pad(now.getDate())}`;
    const name = String((backup.settings && backup.settings.userName) || "")
        .trim()
        .replace(/[\\/:*?"<>|\u0000-\u001f]/g, "")
        .replace(/\s+/g, "_");
    return name ? `Slovingo-${name}-${date}.json` : `Slovingo-${date}.json`;
}

function downloadBackup() {
    const backup = buildBackup();
    const blob = new Blob([JSON.stringify(backup, null, 2)], { type: "application/json" });
    const url = URL.createObjectURL(blob);
    const link = document.createElement("a");
    link.href = url;
    link.download = backupFilename(backup);
    document.body.appendChild(link);
    link.click();
    link.remove();
    setTimeout(() => URL.revokeObjectURL(url), 1000);
}

// ============================================================================
// Import: validation / adaptation
// ============================================================================

function isPlainObject(v) {
    return v !== null && typeof v === "object" && !Array.isArray(v);
}

/** An exercise entry's `last` attempt, or null when missing / unusable. */
function validLast(last) {
    if (!isPlainObject(last)) return null;
    if (!Number.isFinite(last.score) || !Number.isFinite(last.total) || last.total <= 0) return null;
    return last;
}

/**
 * Turn whatever was read from a file into the canonical shape, or throw
 * an Error whose message is a short reason. Tolerant on purpose: missing
 * sections are fine, "progression" is accepted for "progress", a flat
 * `settings` object is read as the app settings, visit days may be an
 * array or a {date: true} map. A newer `version` is attempted anyway.
 *
 * @param {*} raw Parsed JSON.
 * @returns {{settings: object, progress: object, course: object|null, version: number|null, skipped: number}}
 */
function normalizeBackup(raw) {
    if (!isPlainObject(raw)) throw new Error("not a JSON object");
    const rawProgress = raw.progress || raw.progression;
    if (!isPlainObject(raw.settings) && !isPlainObject(rawProgress)) {
        throw new Error("no settings or progress found");
    }

    // --- settings ---
    const s = isPlainObject(raw.settings) ? raw.settings : {};
    const appSource = isPlainObject(s.app)
        ? s.app
        : Object.fromEntries(Object.entries(s).filter(([key]) => key !== "userName" && key !== "themeMode"));
    const known = typeof DEFAULT_SETTINGS === "object" ? DEFAULT_SETTINGS : null;
    const app = {};
    Object.entries(appSource).forEach(([key, value]) => {
        if (!known || key in known) app[key] = value;
    });
    const settings = {
        userName: typeof s.userName === "string" ? s.userName : "",
        themeMode: THEME_MODES.includes(s.themeMode) ? s.themeMode : null,
        app: Object.keys(app).length || isPlainObject(s.app) ? app : null,
    };

    // --- progress ---
    const p = isPlainObject(rawProgress) ? rawProgress : {};
    let skipped = 0;
    const exercises = {};
    if (isPlainObject(p.exercises)) {
        Object.entries(p.exercises).forEach(([sheetId, entry]) => {
            if (!isPlainObject(entry)) { skipped += 1; return; }
            const cleaned = { ...entry };
            if ("last" in cleaned && !validLast(cleaned.last)) delete cleaned.last;
            if (!isPlainObject(cleaned.cumulative)) delete cleaned.cumulative;
            exercises[sheetId] = cleaned;
        });
    }
    const days = Array.isArray(p.visitDays)
        ? p.visitDays
        : isPlainObject(p.visitDays) ? Object.keys(p.visitDays) : [];
    const progress = {
        exercises,
        visitedSheets: isPlainObject(p.visitedSheets) ? p.visitedSheets : {},
        visitDays: days.filter((d) => typeof d === "string" && /^\d{4}-\d{2}-\d{2}$/.test(d)),
        lastSheetId: typeof p.lastSheetId === "string" ? p.lastSheetId : null,
        lastSeriesSheetId: typeof p.lastSeriesSheetId === "string" ? p.lastSeriesSheetId : null,
    };

    return {
        settings,
        progress,
        course: isPlainObject(raw.course) ? raw.course : null,
        version: Number.isFinite(raw.version) ? raw.version : null,
        skipped,
    };
}

// ============================================================================
// Import: merge (pure)
// ============================================================================

function lastRatio(last) {
    return last ? last.score / last.total : -1;
}

/**
 * Merge two progress entries of the same sheet, keeping the best:
 *  - `last`: the attempt with the higher score ratio (the more recent
 *    one on a tie) -- so merging never lowers a displayed score;
 *  - `cumulative`: field-by-field maximum. Summing would double-count
 *    when the same backup is imported twice (sessions have no ids), the
 *    maximum is idempotent and never loses what either device recorded.
 */
function mergeExerciseEntry(a, b) {
    const lastA = validLast(a && a.last);
    const lastB = validLast(b && b.last);
    let last = lastA || lastB || undefined;
    if (lastA && lastB) {
        const ra = lastRatio(lastA);
        const rb = lastRatio(lastB);
        if (rb > ra || (rb === ra && String(lastB.date || "") > String(lastA.date || ""))) last = lastB;
        else last = lastA;
    }

    const cumA = (a && a.cumulative) || {};
    const cumB = (b && b.cumulative) || {};
    const byType = {};
    new Set([...Object.keys(cumA.byType || {}), ...Object.keys(cumB.byType || {})]).forEach((type) => {
        const x = (cumA.byType && cumA.byType[type]) || {};
        const y = (cumB.byType && cumB.byType[type]) || {};
        byType[type] = {
            correct: Math.max(x.correct || 0, y.correct || 0),
            total: Math.max(x.total || 0, y.total || 0),
        };
    });

    const merged = { ...(a || {}), ...(b || {}), cumulative: {
        ...(cumA), ...(cumB),
        byType,
        sessionsCount: Math.max(cumA.sessionsCount || 0, cumB.sessionsCount || 0),
    } };
    if (last) merged.last = last; else delete merged.last;
    return merged;
}

/**
 * Merge imported progress into local progress. Pure: returns the merged
 * progress plus counts for the confirmation message.
 *
 * lastSheetId / lastSeriesSheetId: the local value wins when there is
 * one (it is the device in use); the file's fills in on an empty device.
 */
function mergeProgress(local, imported) {
    const exercises = { ...local.exercises };
    const stats = { added: 0, improved: 0, unchanged: 0 };
    Object.entries(imported.exercises).forEach(([sheetId, entry]) => {
        const existing = local.exercises[sheetId];
        if (!existing) {
            exercises[sheetId] = mergeExerciseEntry(null, entry);
            stats.added += 1;
            return;
        }
        const merged = mergeExerciseEntry(existing, entry);
        exercises[sheetId] = merged;
        if (JSON.stringify(merged) === JSON.stringify(existing)) stats.unchanged += 1;
        else stats.improved += 1;
    });

    return {
        progress: {
            exercises,
            visitedSheets: { ...local.visitedSheets, ...imported.visitedSheets },
            visitDays: [...new Set([...local.visitDays, ...imported.visitDays])].sort(),
            lastSheetId: local.lastSheetId || imported.lastSheetId,
            lastSeriesSheetId: local.lastSeriesSheetId || imported.lastSeriesSheetId,
        },
        stats,
    };
}

// ============================================================================
// Import: apply to localStorage
// ============================================================================

/**
 * Write a normalized backup into localStorage (progress merged, settings
 * replaced). The caller reloads the page afterwards, so the boot sequence
 * re-reads everything (theme, font, user name, streak...).
 * @returns {{added: number, improved: number, unchanged: number, skipped: number, settingsApplied: boolean}}
 */
function applyBackup(normalized) {
    const k = backupKeys();
    const current = buildBackup().progress;
    const { progress, stats } = mergeProgress(current, normalized.progress);

    localStorage.setItem(k.progress, JSON.stringify(progress.exercises));
    localStorage.setItem(k.visitedSheets, JSON.stringify(progress.visitedSheets));
    localStorage.setItem(k.visitDays, JSON.stringify(Object.fromEntries(progress.visitDays.map((d) => [d, true]))));
    if (progress.lastSheetId) localStorage.setItem(k.lastSheet, JSON.stringify({ sheetId: progress.lastSheetId }));
    if (progress.lastSeriesSheetId) localStorage.setItem(k.lastSeriesSheet, JSON.stringify({ sheetId: progress.lastSeriesSheetId }));

    const s = normalized.settings;
    let settingsApplied = false;
    if (s.app) { localStorage.setItem(k.settings, JSON.stringify(s.app)); settingsApplied = true; }
    if (s.themeMode) { localStorage.setItem(k.themeMode, s.themeMode); settingsApplied = true; }
    if (s.userName) { localStorage.setItem(k.userName, s.userName); settingsApplied = true; }

    return { ...stats, skipped: normalized.skipped, settingsApplied };
}

// ============================================================================
// Settings screen glue
// ============================================================================

/** Read a File as JSON, confirm, apply, report, reload. Errors never leave the page half-imported. */
async function importBackupFile(file) {
    let normalized;
    try {
        normalized = normalizeBackup(JSON.parse(await file.text()));
    } catch (err) {
        const msg = uiLabel("backup_import_invalid", "This file is not a valid Slovingo backup.");
        window.alert(`${msg} (${err.message})`);
        return;
    }

    let question = uiLabel(
        "backup_import_confirm",
        "Import this backup? Progress will be merged (best scores kept); settings will be replaced by the ones in the file."
    );
    const filePrefix = normalized.course && normalized.course.storagePrefix;
    if (filePrefix && filePrefix !== backupStoragePrefix()) {
        question = `${uiLabel("backup_import_other_course", "Warning: this backup comes from another course.")}\n\n${question}`;
    }
    if (!window.confirm(question)) return;

    let summary;
    try {
        summary = applyBackup(normalized);
    } catch (err) {
        window.alert(`${uiLabel("backup_import_failed", "Import failed (storage full or unavailable).")} (${err.message})`);
        return;
    }

    window.alert(uiLabel(
        "backup_import_done",
        "Import done: {added} new sheet(s), {improved} improved, {unchanged} unchanged."
    ).replace("{added}", summary.added).replace("{improved}", summary.improved).replace("{unchanged}", summary.unchanged));
    window.location.reload();
}

/** The two buttons (+ hidden file input) shown in Settings > Data. */
function renderBackupControls() {
    const exportBtn = el("button", {
        className: "settings-backup-btn",
        text: uiLabel("backup_export", "📥 Export settings and progress (JSON)"),
        attrs: { type: "button" },
        onclick: downloadBackup,
    });

    const fileInput = el("input", { attrs: { type: "file", accept: "application/json,.json", hidden: "" } });
    fileInput.addEventListener("change", () => {
        const file = fileInput.files && fileInput.files[0];
        fileInput.value = "";
        if (file) importBackupFile(file);
    });

    const importBtn = el("button", {
        className: "settings-backup-btn",
        text: uiLabel("backup_import", "📤 Import a backup (JSON)"),
        attrs: { type: "button" },
        onclick: () => fileInput.click(),
    });

    return [exportBtn, importBtn, fileInput];
}

// Node (unit tests) can require() the pure parts; the browser ignores this.
if (typeof module !== "undefined" && module.exports) {
    module.exports = { normalizeBackup, mergeExerciseEntry, mergeProgress, backupFilename };
}
