/**
 * exo-mascots.js -- the fox and the rabbit of the exercise screens.
 *
 * Ported from MathPulse. Default faces live in mascots/<fox|rabbit>-<mood>.svg
 * (moods: content | bravo | think | comfort). A course can replace any of
 * them with its own pictures (img/mascots/, listed in data.json "mascot_pictures"
 * by src/mascots.py): see mascotSrc(). "rabbit" and "hare" are the same
 * character.
 *
 *   - The FOX sits in the top-right corner of the question card (so it
 *     never covers an answer or the check button) and reacts to the
 *     answer: bravo + hop (correct), comfort + tilt (wrong). Each new
 *     question brings a fresh card, hence a fresh "content" fox.
 *   - The RABBIT shows up on the result screen, with a mood that
 *     follows the score, and bounces in. If lang.json has the optional
 *     "cheers" block, it also says a short phrase (a bubble you can tap
 *     to hear it): see summaryCheer().
 *
 * Animations are in exo-mascots.css (neutralised by prefers-reduced-motion).
 * Depends on el() and speakSafe() (exercises.js), called lazily.
 */
"use strict";

const MASCOT_MOODS = ["content", "bravo", "think", "comfort"];

/** "rabbit" (the engine's historical name) and "hare" are one character. */
function mascotWho(who) {
    return who === "rabbit" ? "hare" : who;
}

/** The course's own picture for a key ("fox-bravo", "duo-walk"), or null. */
function courseMascot(key) {
    const list = typeof DATA !== "undefined" && DATA && DATA.mascot_pictures;
    return (list && list[key]) || null;
}

/** Picture of a mascot in a mood: the course's own, else the default SVG. */
function mascotSrc(who, mood) {
    const w = mascotWho(who);
    const m = MASCOT_MOODS.includes(mood) ? mood : "content";
    return courseMascot(`${w}-${m}`) || `mascots/${w === "hare" ? "rabbit" : w}-${m}.svg`;
}

function mascotFace(who, mood) {
    return el("img", {
        className: `exo-face exo-face-${who}`,
        attrs: { src: mascotSrc(who, mood), alt: "", width: "96", height: "96", "aria-hidden": "true", draggable: "false" },
    });
}

/** Put the fox in the top-right corner of a question card (one per card). */
function mountFoxInCard(card) {
    card.classList.add("exo-has-fox");
    card.appendChild(el("div", { className: "exo-fox", attrs: { id: "exo-fox", "data-mood": "content", "aria-hidden": "true" } },
        [mascotFace("fox", "content")]));
}

/** Change the fox's face and replay a one-shot animation (hop | tilt | nod). */
function reactFox(mood, anim) {
    const box = document.getElementById("exo-fox");
    if (!box || !MASCOT_MOODS.includes(mood)) return;
    box.querySelector("img").src = mascotSrc("fox", mood);
    box.dataset.mood = mood;
    box.classList.remove("hop", "tilt", "nod");
    void box.offsetWidth; // restart the CSS animation
    if (anim) box.classList.add(anim);
}

/** Mood of the result screen for a score (same bounds as before). */
function summaryMood(correct, total) {
    const ratio = total ? correct / total : 0;
    return ratio >= 0.8 ? "bravo" : ratio >= 0.5 ? "content" : "comfort";
}

/**
 * The hare and the fox together, in one mood: the hare comes in from the
 * left, the fox from the right; then a double hop (bravo), or the fox
 * leans towards the hare (comfort). Used by the result screen and by the
 * end screen of a sheet in step-by-step mode.
 */
function mascotDuo(mood) {
    return el("div", { className: "exo-duo", attrs: { "data-mood": mood, "aria-hidden": "true" } }, [
        el("div", { className: "exo-rabbit", attrs: { "data-mood": mood } }, [mascotFace("hare", mood)]),
        el("div", { className: "exo-duo-fox", attrs: { "data-mood": mood } }, [mascotFace("fox", mood)]),
    ]);
}

/** Result screen: the duo for this score. */
function summaryRabbit(correct, total) {
    return mascotDuo(summaryMood(correct, total));
}

/** Tier of lang.json's "cheers" for a score (same bounds as docs/Lang-json.md). */
function cheerTier(correct, total) {
    const ratio = total ? correct / total : 0;
    if (ratio >= 1) return "perfect";
    if (ratio >= 0.9) return "great";
    if (ratio >= 0.7) return "good";
    if (ratio >= 0.5) return "ok";
    return "retry";
}

/**
 * Result screen: what the rabbit says. One phrase drawn at random in the tier of
 * the optional lang.json "cheers" that matches the score ("t" in the target
 * language, "n" its translation). Null when the course has none. Tapping the
 * bubble reads the phrase aloud, when speech is available.
 */
function summaryCheer(correct, total) {
    const list = typeof LANG !== "undefined" && LANG && LANG.cheers && LANG.cheers[cheerTier(correct, total)];
    const choices = Array.isArray(list) ? list.filter((c) => c && c.t) : [];
    if (!choices.length) return null;
    const cheer = choices[Math.floor(Math.random() * choices.length)];

    const children = [el("strong", { className: "exo-cheer-text", text: cheer.t })];
    if (cheer.n) children.push(el("span", { className: "exo-cheer-native", text: cheer.n }));
    const canSpeak = "speechSynthesis" in window && typeof speakSafe === "function";
    return el(canSpeak ? "button" : "div", {
        className: "exo-cheer",
        onclick: canSpeak ? () => speakSafe(cheer.t) : undefined,
        attrs: canSpeak ? { type: "button", "aria-label": cheer.t } : {},
    }, children);
}

// -- Launch greeting --------------------------------------------------------

/**
 * Once per launch of the app, and only if it opens on the home screen: the
 * hare slips in from the edge of the screen with a short phrase, stays a
 * few seconds, then leaves. Coming back to the home screen later (from a
 * sheet, the exercises...) never brings her back: the flag lives in
 * memory, so only a fresh start of the app resets it.
 *
 * The phrases come from lang.json "mascots.greetings", either a list or an
 * object by time of day ({"morning": [...], "day": [...], "evening": [...],
 * optional "night": [...]},
 * plus "any" for phrases that fit every hour). Each phrase is {"t": target
 * language, "n": translation, "who": "hare" | "fox" (default "hare")}.
 * Without that list, the course's single "greeting" line is used. No
 * phrase at all, no greeting. [USER_NAME] is the learner's first name
 * (dropped, with its comma, when there is none). Tapping the bubble reads the phrase aloud;
 * tapping the mascot sends her away.
 */
const GREETING_STAY_MS = 7000;
let mascotGreetingDone = false;

function greetingChoices(now = new Date()) {
    if (typeof LANG === "undefined" || !LANG) return [];
    const cfg = (LANG.mascots && LANG.mascots.greetings) || (LANG.greeting ? [LANG.greeting] : null);
    if (!cfg) return [];
    let list;
    if (Array.isArray(cfg)) {
        list = cfg;
    } else {
        const h = now.getHours();
        let slot = h >= 5 && h < 12 ? "morning" : h >= 12 && h < 18 ? "day" : "evening";
        // Late at night (22 h - 5 h), "night" if the course has it, else "evening".
        if ((h >= 22 || h < 5) && Array.isArray(cfg.night) && cfg.night.length) slot = "night";
        list = [].concat(cfg[slot] || [], cfg.any || []);
    }
    return list.filter((g) => g && g.t);
}

/** Put the learner's first name in a phrase, or drop the placeholder. */
function withUserName(text) {
    const name = window.USER_NAME || (LANG.site && LANG.site.user_name_default) || "";
    return name ? text.replace(/\[USER_NAME\]/g, name) : text.replace(/[,\s]*\[USER_NAME\]/g, "");
}

/** Called by route() after each screen; only the very first call counts. */
function maybeGreetOnLaunch(onHome) {
    if (mascotGreetingDone) return;
    mascotGreetingDone = true;
    if (!onHome) return;
    const choices = greetingChoices();
    if (!choices.length) return;
    const pick = choices[Math.floor(Math.random() * choices.length)];
    const g = { t: withUserName(pick.t), n: pick.n ? withUserName(pick.n) : "", who: pick.who };
    const who = mascotWho(g.who === "fox" ? "fox" : "hare");

    const wrap = document.createElement("div");
    wrap.className = `mascot-greeting mascot-greeting--${who}`;
    const bubble = document.createElement("button");
    bubble.type = "button";
    bubble.className = "mascot-greeting__bubble";
    const t = document.createElement("strong");
    t.textContent = g.t;
    bubble.appendChild(t);
    if (g.n) {
        const n = document.createElement("span");
        n.textContent = g.n;
        bubble.appendChild(n);
    }
    const pic = document.createElement("img");
    pic.className = "mascot-pic mascot-greeting__pic";
    pic.src = mascotSrc(who, "content");
    pic.alt = "";
    pic.draggable = false;
    wrap.append(bubble, pic);

    let timer = null;
    const leave = () => {
        clearTimeout(timer);
        window.removeEventListener("hashchange", leave);
        if (!wrap.isConnected) return;
        wrap.classList.add("is-leaving");
        setTimeout(() => wrap.remove(), 600);
    };
    bubble.addEventListener("click", () => {
        if ("speechSynthesis" in window && typeof speakSafe === "function") speakSafe(g.t);
        clearTimeout(timer);
        timer = setTimeout(leave, GREETING_STAY_MS);
    });
    pic.addEventListener("click", leave);
    window.addEventListener("hashchange", leave);
    document.body.appendChild(wrap);
    timer = setTimeout(leave, GREETING_STAY_MS);
}
