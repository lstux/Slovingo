/**
 * progress.js -- Slovingo v2.
 *
 * Reads exercise progress from localStorage. Shared between the home
 * view (score badges per group) and exercises.js (selection screen,
 * session summary). This file only READS the store; exercises.js
 * writes it (recordSession(), when a session ends).
 *
 * Progress is keyed by sheet id now, not by the .md source filename
 * (see the v2 rewrite notes) -- ids are stable across renumbering,
 * filenames aren't.
 *
 * progressKey() reads LANG lazily rather than at parse time: this
 * script loads before app.js's boot() has fetched lang.json, so the
 * storage prefix isn't known yet when the file is first evaluated.
 */

"use strict";

/** @returns {string} The localStorage key for the progress store. */
function progressKey() {
    const prefix = (LANG && LANG.site && LANG.site.storage_prefix) || "slovingo";
    return `${prefix}-exercises-progress`;
}

/** @returns {object} The whole progress store, keyed by sheet id. */
function loadProgressStore() {
    try {
        return JSON.parse(localStorage.getItem(progressKey())) || {};
    } catch (err) {
        return {};
    }
}

/**
 * @param {string} sheetId
 * @returns {object|null} That sheet's progress entry, or null.
 */
function getSheetProgress(sheetId) {
    return loadProgressStore()[sheetId] || null;
}

/** @param {number} correct @param {number} total @returns {number} Rounded percentage. */
function pct(correct, total) {
    return total ? Math.round((correct / total) * 100) : 0;
}

/**
 * "good" / "mid" / "bad" by correct/total ratio -- prefix with the
 * display context (see .exo-score-* / .index-score-* in the CSS).
 */
function scoreRatioClass(correct, total) {
    const ratio = total ? correct / total : 0;
    if (ratio >= 0.8) return "good";
    if (ratio >= 0.5) return "mid";
    return "bad";
}

/**
 * Average score across a set of sheets, identified by id.
 *
 * This is the average of ALL sheets' percentages, practiced or not --
 * an unpracticed sheet counts as 0, it is not simply skipped. On a
 * 5-sheet group where only one sheet was practiced at 10/10, the
 * average is therefore 20% (10+0+0+0+0 over 5), not 100%: it reflects
 * progress across the whole group, not just the quality of what was
 * attempted. Only the LAST attempt of each practiced sheet counts.
 *
 * @param {string[]} sheetIds
 * @returns {{avg: number, count: number}|null} null if no sheet in
 *     the set has ever been practiced (nothing worth showing).
 */
function averageScore(sheetIds) {
    let sum = 0;
    let practicedCount = 0;
    sheetIds.forEach((id) => {
        const progress = getSheetProgress(id);
        if (progress && progress.last) {
            sum += pct(progress.last.score, progress.last.total);
            practicedCount += 1;
        }
    });
    if (!practicedCount) return null;
    const avg = Math.round(sum / sheetIds.length);
    return { avg, count: practicedCount };
}
