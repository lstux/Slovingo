/**
 * exo-mascots.js -- the fox and the rabbit of the exercise screens.
 *
 * Ported from MathPulse. Faces live in mascots/<who>-<mood>.svg
 * (moods: content | bravo | think | comfort). No text, so no lang.json key.
 *
 *   - The FOX sits in a corner during a session and reacts to each
 *     answer: bravo + hop (correct), comfort + tilt (wrong). It goes
 *     back to "content" when the next question shows up.
 *   - The RABBIT shows up on the result screen, with a mood that
 *     follows the score, and bounces in.
 *
 * Animations are in exo-mascots.css (neutralised by prefers-reduced-motion).
 * Depends on el() (exercises.js), called lazily.
 */
"use strict";

const MASCOT_MOODS = ["content", "bravo", "think", "comfort"];

function mascotFace(who, mood) {
    return el("img", {
        className: `exo-face exo-face-${who}`,
        attrs: { src: `mascots/${who}-${mood}.svg`, alt: "", width: "96", height: "96", "aria-hidden": "true", draggable: "false" },
    });
}

/** Put the session fox in the page (once per session screen). */
function mountSessionFox(container) {
    const existing = document.getElementById("exo-fox");
    if (existing) existing.remove();
    const box = el("div", { className: "exo-fox", attrs: { id: "exo-fox", "data-mood": "content", "aria-hidden": "true" } },
        [mascotFace("fox", "content")]);
    container.appendChild(box);
}

/** Change the fox's face and replay a one-shot animation (hop | tilt | nod). */
function reactFox(mood, anim) {
    const box = document.getElementById("exo-fox");
    if (!box || !MASCOT_MOODS.includes(mood)) return;
    box.querySelector("img").src = `mascots/fox-${mood}.svg`;
    box.dataset.mood = mood;
    box.classList.remove("hop", "tilt", "nod");
    void box.offsetWidth; // restart the CSS animation
    if (anim) box.classList.add(anim);
}

/** Result screen: hide the fox, return the rabbit for this score. */
function summaryRabbit(correct, total) {
    const box = document.getElementById("exo-fox");
    if (box) box.remove();
    const ratio = total ? correct / total : 0;
    const mood = ratio >= 0.8 ? "bravo" : ratio >= 0.5 ? "content" : "comfort";
    return el("div", { className: "exo-rabbit", attrs: { "data-mood": mood, "aria-hidden": "true" } },
        [mascotFace("rabbit", mood)]);
}
