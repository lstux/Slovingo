/**
 * credits.js -- a "credits" sheet (file prefix 99_Credits_) is shown like
 * the end credits of a film: its content is centred, spaced out, and
 * scrolls by itself from the bottom of the screen up and out at the top,
 * then loops, between red cinema curtains.
 *
 * The sheet is rendered by app.js as usual; setupCredits() only wraps
 * what is already in #content. Nothing here is required to read the
 * sheet: with reduced motion (or if anything fails) it stays a plain,
 * normally scrollable page.
 *
 * Controls: a tap, a click or the Space key pauses / resumes; the
 * button does the same. The scroll also pauses while the tab is hidden.
 */
(function () {
"use strict";

const SPEED = 38; // px per second: slow enough to read comfortably
const MAX_FRAME = 0.1; // s: a long frame (tab in background) must not jump the text

function prefersReducedMotion() {
    return typeof matchMedia === "function" && matchMedia("(prefers-reduced-motion: reduce)").matches;
}

function label(key, fallback) {
    if (typeof uiLabel === "function") {
        return uiLabel(key, fallback);
    }
    return fallback;
}

let stopCurrent = null;

/**
 * Turn the sheet currently in `content` into the scrolling credits.
 * Any previous credits scroll is stopped first.
 * @param {HTMLElement} content The #content element, already filled.
 */
function setupCredits(content) {
    teardownCredits();
    if (!content || !content.firstElementChild) {
        return;
    }

    const stage = document.createElement("div");
    stage.className = "credits-stage";
    const reel = document.createElement("div");
    reel.className = "credits-reel";
    const roll = document.createElement("div");
    roll.className = "credits-roll";

    // Everything the sheet rendered goes on the roll, except the
    // exercise status line app.js appends afterwards (not part of a film).
    while (content.firstChild) {
        roll.appendChild(content.firstChild);
    }
    reel.appendChild(roll);
    stage.appendChild(reel);
    ["left", "right", "top"].forEach((side) => {
        const curtain = document.createElement("div");
        curtain.className = `credits-curtain credits-curtain--${side}`;
        curtain.setAttribute("aria-hidden", "true");
        stage.appendChild(curtain);
    });

    const button = document.createElement("button");
    button.type = "button";
    button.className = "credits-pause";
    stage.appendChild(button);
    content.appendChild(stage);
    document.body.classList.add("is-credits");

    if (prefersReducedMotion()) {
        // A calm page: no motion, the whole text readable by scrolling.
        stage.classList.add("is-static");
        button.hidden = true;
        stopCurrent = () => {
            document.body.classList.remove("is-credits");
        };
        return;
    }

    let paused = false;
    let offset = 0; // px the roll has travelled up from its start position
    let last = performance.now();
    let frame = 0;
    let rollHeight = 0;
    let stageHeight = 0;

    const measure = () => {
        stageHeight = reel.clientHeight;
        rollHeight = roll.offsetHeight;
    };
    const restart = () => {
        measure();
        offset = 0; // starts just below the visible area (see CSS), then rises
    };
    const syncButton = () => {
        button.textContent = paused ? `▶ ${label("credits_resume", "Resume")}` : `⏸ ${label("credits_pause", "Pause")}`;
        button.setAttribute("aria-pressed", paused ? "true" : "false");
    };
    const toggle = () => {
        paused = !paused;
        syncButton();
    };

    const tick = (now) => {
        if (!stage.isConnected) {
            stopCurrent && stopCurrent();
            return;
        }
        const dt = Math.min((now - last) / 1000, MAX_FRAME);
        last = now;
        if (!paused && !document.hidden) {
            offset += SPEED * dt;
            // The roll starts below the stage (translateY = stageHeight) and
            // is gone once its bottom passes the top edge.
            if (offset > stageHeight + rollHeight) {
                offset = 0;
            }
        }
        roll.style.transform = `translate3d(0, ${Math.round((stageHeight - offset) * 10) / 10}px, 0)`;
        frame = requestAnimationFrame(tick);
    };

    const onStageClick = (event) => {
        if (event.target.closest("a, button")) {
            return; // links in the credits keep working; the button has its own handler
        }
        toggle();
    };
    const onKey = (event) => {
        if (event.key === " " && !/^(input|textarea|select|button|a)$/i.test(event.target.tagName)) {
            event.preventDefault();
            toggle();
        }
    };
    const onResize = () => {
        measure();
    };

    button.addEventListener("click", (event) => {
        event.stopPropagation();
        toggle();
    });
    stage.addEventListener("click", onStageClick);
    document.addEventListener("keydown", onKey);
    window.addEventListener("resize", onResize);

    stopCurrent = () => {
        cancelAnimationFrame(frame);
        document.removeEventListener("keydown", onKey);
        window.removeEventListener("resize", onResize);
        document.body.classList.remove("is-credits");
        stopCurrent = null;
    };

    syncButton();
    restart();
    // Fonts / images may change the roll's height after first paint.
    setTimeout(measure, 400);
    frame = requestAnimationFrame((now) => {
        last = now;
        tick(now);
    });
}

/** Stop the credits scroll, if one is running (called on every route change). */
function teardownCredits() {
    if (stopCurrent) {
        stopCurrent();
    }
    document.body.classList.remove("is-credits");
}

window.setupCredits = setupCredits;
window.teardownCredits = teardownCredits;
})();
