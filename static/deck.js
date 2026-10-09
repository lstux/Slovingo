/**
 * deck.js -- "Pas à pas" view of a series sheet.
 *
 * Same JSON blocks as the "Page" view, shown one step at a time:
 *
 *   - intro      : the illustration and the opening lines
 *   - chapter    : a divider for each "## " heading
 *   - flip       : one row of a translate-table; tap to reveal the
 *                  target column, which is spoken on reveal
 *   - phrase     : one audio-card; the sentence is spoken on arrival,
 *                  its translation stays hidden until 👀
 *   - text       : a paragraph, list or blockquote ("⚠️" pitfalls are
 *                  flagged so the CSS can highlight them)
 *
 * Navigation inside a sheet: the ‹ › buttons, the arrow keys, or a
 * horizontal swipe on the card. Horizontal swipe is therefore claimed
 * by the deck while it is shown; app.js's sheet-to-sheet swipe is
 * switched off in that case (see initSwipeNavigation), and changing
 * sheet goes through the pager buttons.
 *
 * Nothing is written to the SMD source or the build: this is only a
 * presentation of sheet.content, toggled per device in localStorage.
 *
 * Depends on (app.js): renderBlock, renderText, renderAudioCard,
 * initializeAudioCards, initializeSpeakableElements, speak,
 * effectiveCharacterVoice (settings.js), LANG, renderSheet, renderSheetBody.
 */
"use strict";

const VIEW_MODE_STORAGE_KEY = "slovingo.viewMode";

/** The sheet currently shown in deck mode, or null. */
let activeDeck = null;

function deckLabel(key, fallback) {
    return (LANG.ui && LANG.ui[key]) || fallback;
}

function getStoredViewMode() {
    try {
        return localStorage.getItem(VIEW_MODE_STORAGE_KEY) === "deck" ? "deck" : "page";
    } catch (_) {
        return "page";
    }
}

function storeViewMode(mode) {
    try {
        localStorage.setItem(VIEW_MODE_STORAGE_KEY, mode);
    } catch (_) { /* private mode: the choice just isn't remembered */ }
}

/**
 * Deck mode is offered on series sheets only, and not on sheets that
 * contain a dialogue (speaker emoji): those read better as a page.
 */
function isDeckEligible(group, sheet) {
    if (!group || group.category !== "series") return false;
    return !sheet.content.some((block) => block.type === "audio-card" && block.speaker);
}

/** Two-button switch "Page | Pas à pas", shown at the top of the sheet. */
function buildViewModeToggle(sheet, group, current) {
    const bar = document.createElement("div");
    bar.className = "view-mode-toggle";
    bar.setAttribute("role", "group");

    // Icons only: no text to translate, the lang.json ui map stays untouched.
    [
        ["page", "📄"],
        ["deck", "🃏"],
    ].forEach(([mode, icon]) => {
        const btn = document.createElement("button");
        btn.type = "button";
        btn.className = "view-mode-btn" + (mode === current ? " is-active" : "");
        btn.setAttribute("aria-pressed", String(mode === current));
        btn.textContent = icon;
        btn.onclick = () => {
            if (mode === current) return;
            storeViewMode(mode);
            const content = document.getElementById("content");
            content.innerHTML = "";
            renderSheetBody(content, sheet, group);
        };
        bar.appendChild(btn);
    });
    return bar;
}

// ----------------------------------------------------------------------------
// Step building
// ----------------------------------------------------------------------------

/** Plain text of an element, for speech. */
function plainText(el) {
    return el.textContent.replace(/\s+/g, " ").trim();
}

/**
 * Turn sheet.content into a flat list of steps. Each step is
 * { kind, chapter, sub, node, backNode?, speakText?, isPiege? }.
 */
function buildDeckSteps(sheet) {
    const steps = [];
    let chapter = "";
    let sub = "";
    let seenHeading = false;

    if (sheet.image) {
        steps.push({ kind: "intro", chapter: "", sub: "", nodes: [renderImage(sheet.image)] });
    }

    sheet.content.forEach((block) => {
        if (block.type === "heading") {
            seenHeading = true;
            if (block.level <= 2) {
                chapter = block.text;
                sub = "";
                steps.push({ kind: "chapter", chapter, sub: "", nodes: [renderBlock(block)] });
            } else if (block.level === 3) {
                // H3 is a kicker shown above the next step, not a screen of its own.
                sub = block.text;
            }
            return;
        }

        if (block.type === "image" || block.type === "hr") return;

        if (block.type === "table") {
            const target = block.speakable_column !== undefined && block.speakable_column !== null
                ? block.speakable_column
                : 1;
            const source = target === 0 ? 1 : 0;
            block.rows.forEach((cells) => {
                const front = document.createElement("div");
                front.className = "deck-flip-front";
                front.innerHTML = renderText(cells[source]);

                const back = document.createElement("div");
                back.className = "deck-flip-back";
                back.innerHTML = renderText(cells[target]);

                steps.push({
                    kind: "flip",
                    chapter,
                    sub,
                    nodes: [front],
                    backNode: back,
                    speakText: plainText(back),
                });
            });
            return;
        }

        if (block.type === "audio-card") {
            steps.push({ kind: "phrase", chapter, sub, nodes: [renderAudioCard(block)] });
            return;
        }

        // paragraph, list, blockquote, geomap
        const node = renderBlock(block);
        const isPiege = node.textContent.trimStart().startsWith("⚠️");
        steps.push({ kind: "text", chapter, sub, nodes: [node], isPiege });
        sub = "";
    });

    // A sheet with no heading at all: the intro is simply the first step.
    if (!seenHeading && steps.length === 0) {
        steps.push({ kind: "intro", chapter: "", sub: "", nodes: [] });
    }
    return steps;
}

/** Build the DOM of one step (header + body), hidden until shown. */
function buildStepElement(step, total, index) {
    const el = document.createElement("section");
    el.className = `deck-step deck-step-${step.kind}`;
    el.dataset.index = String(index);

    if (step.chapter || step.sub) {
        const kicker = document.createElement("p");
        kicker.className = "deck-kicker";
        kicker.textContent = [step.chapter, step.sub].filter(Boolean).join(" · ");
        el.appendChild(kicker);
    }

    const body = document.createElement("div");
    body.className = "deck-body" + (step.isPiege ? " deck-piege" : "");
    step.nodes.forEach((n) => body.appendChild(n));

    if (step.kind === "flip") {
        // Front stays visible; tapping the card reveals the back.
        body.classList.add("deck-flip");
        body.appendChild(step.backNode);
        body.addEventListener("click", () => {
            const flipped = body.classList.toggle("is-flipped");
            if (flipped && step.speakText) speakDeckText(step.speakText, body);
        });
        const hint = document.createElement("p");
        hint.className = "deck-hint";
        hint.textContent = "👆";
        el.appendChild(body);
        el.appendChild(hint);
        return el;
    }

    el.appendChild(body);
    return el;
}

function speakDeckText(text, highlightEl) {
    const voice = effectiveCharacterVoice(undefined);
    speak(text, voice.rate, voice.pitch, voice.voiceURI, highlightEl);
}

// ----------------------------------------------------------------------------
// Rendering and navigation
// ----------------------------------------------------------------------------

/**
 * Render the deck inside #content (the toggle is already in place).
 * All step elements are built and wired up once, then shown one by one.
 */
function renderDeck(content, sheet) {
    const steps = buildDeckSteps(sheet);

    const stage = document.createElement("div");
    stage.className = "deck-stage";

    const track = document.createElement("div");
    track.className = "deck-track";
    stage.appendChild(track);

    const elements = steps.map((step, i) => {
        const el = buildStepElement(step, steps.length, i);
        track.appendChild(el);
        return el;
    });

    // The audio-cards inside the deck get their buttons and handlers here,
    // exactly as in page mode (they are in the DOM now).
    initializeAudioCards();
    initializeSpeakableElements();

    const controls = document.createElement("nav");
    controls.className = "deck-controls";
    controls.innerHTML = `
        <button type="button" class="deck-prev">‹</button>
        <div class="deck-meter">
            <div class="deck-bar"><div class="deck-bar-fill"></div></div>
            <span class="deck-count"></span>
        </div>
        <button type="button" class="deck-next"></button>
    `;

    content.appendChild(stage);
    content.appendChild(controls);

    activeDeck = { stage, elements, steps, index: 0, sheet, controls };
    controls.querySelector(".deck-prev").onclick = () => deckGo(-1);
    controls.querySelector(".deck-next").onclick = () => deckNext();

    attachDeckSwipe(stage);
    deckShow(0);
}

function deckNext() {
    if (!activeDeck) return;
    if (activeDeck.index >= activeDeck.elements.length - 1) {
        // End of the sheet: hand over to the pager's "next sheet" (if any).
        const pagerNext = document.getElementById("pager-top-next");
        if (pagerNext && !pagerNext.disabled) pagerNext.click();
        return;
    }
    deckGo(1);
}

function deckGo(delta) {
    if (!activeDeck) return;
    const target = activeDeck.index + delta;
    if (target < 0 || target >= activeDeck.elements.length) return;
    deckShow(target);
}

function deckShow(index) {
    const d = activeDeck;
    d.index = index;
    d.elements.forEach((el, i) => el.classList.toggle("is-current", i === index));

    const current = d.elements[index];
    const step = d.steps[index];

    // Phrases are spoken on arrival: the audio-text's handler does the speaking.
    if (step.kind === "phrase") {
        const text = current.querySelector(".audio-text");
        if (text) text.click();
    }

    const total = d.elements.length;
    d.controls.querySelector(".deck-count").textContent = `${index + 1} / ${total}`;
    d.controls.querySelector(".deck-bar-fill").style.width = `${((index + 1) / total) * 100}%`;
    d.controls.querySelector(".deck-prev").disabled = index === 0;

    // Last step: "›" moves on to the next sheet if there is one, "✔" otherwise.
    const next = d.controls.querySelector(".deck-next");
    const isLast = index === total - 1;
    const pagerNext = document.getElementById("pager-top-next");
    const hasNextSheet = !!pagerNext && !pagerNext.disabled;
    next.textContent = isLast && !hasNextSheet ? "✔" : "›";
    next.disabled = isLast && !hasNextSheet;

    current.scrollIntoView({ block: "nearest" });
}

/** Horizontal swipe on the deck card moves between steps. */
function attachDeckSwipe(stage) {
    const MIN_PX = 50;
    let start = null;

    stage.addEventListener("touchstart", (e) => {
        if (e.touches.length !== 1) { start = null; return; }
        start = { x: e.touches[0].clientX, y: e.touches[0].clientY };
    }, { passive: true });

    stage.addEventListener("touchend", (e) => {
        if (!start) return;
        const t = e.changedTouches[0];
        const dx = t.clientX - start.x;
        const dy = t.clientY - start.y;
        start = null;
        if (Math.abs(dx) < MIN_PX || Math.abs(dx) < Math.abs(dy) * 1.5) return;
        deckGo(dx < 0 ? 1 : -1);
    }, { passive: true });
}

/** Arrow keys, while a deck is on screen and nothing is being typed in. */
document.addEventListener("keydown", (e) => {
    if (!activeDeck || !activeDeck.stage.isConnected) return;
    if (/^(INPUT|TEXTAREA|SELECT)$/.test(document.activeElement && document.activeElement.tagName)) return;
    if (e.key === "ArrowRight") { e.preventDefault(); deckNext(); }
    if (e.key === "ArrowLeft") { e.preventDefault(); deckGo(-1); }
});
