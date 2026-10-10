/**
 * deck.js -- "Pas à pas" view of a series sheet.
 *
 * Same JSON blocks as the "Page" view, shown one step at a time:
 *
 *   - intro   : the illustration
 *   - text    : a paragraph, list or blockquote. Under an "###"
 *               sub-title (in a series sheet: a grammar point and its
 *               one-sentence explanation) the fox looks thoughtful.
 *   - table   : a whole translate-table on ONE screen, target-language
 *               column first; tapping a word reads it aloud
 *   - phrase  : one example sentence, no card frame. It is read aloud
 *               on arrival; when the speech ends its translation and
 *               details fade in by themselves. A tap re-reads it.
 *   - dialogue: a run of lines with a speaker, shown as a chat: each
 *               line appears, is read in its character's voice, then its
 *               translation fades in under it. The list scrolls so the
 *               newest line stays at the bottom. Speakers alternate
 *               sides (first speaker left, second right). A tap re-reads
 *               a line.
 *   - end     : the rabbit, and the way on to the next sheet
 *
 * ("## " headings have no screen: they label the steps that follow.)
 *
 * The fox (same one as the exercises) sits in the corner of each card:
 * attentive while a sentence is read, happy when its translation shows.
 *
 * Navigation inside a sheet: the ‹ › buttons, the arrow keys, or a
 * horizontal swipe on the card. Horizontal swipe is therefore claimed
 * by the deck while it is shown; app.js's sheet-to-sheet swipe is
 * switched off in that case (see initSwipeNavigation), and changing
 * sheet goes through the pager buttons.
 *
 * Nothing is written to the SMD source or the build: this is only a
 * presentation of sheet.content, toggled per device in localStorage.
 * No text of its own (only symbols), so lang.json needs no new keys.
 *
 * Depends on (app.js): renderBlock, renderTable, renderImage, renderText,
 * initializeSpeakableElements, updateTtsAvailability, speak, stopSpeaking,
 * targetVoiceAvailable, effectiveCharacterVoice (settings.js), renderSheetBody;
 * and, if loaded (exo-mascots.js, exercises.js): el, mascotFace, reactFox.
 */
"use strict";

const VIEW_MODE_STORAGE_KEY = "slovingo.viewMode";

/** The sheet currently shown in deck mode, or null. */
let activeDeck = null;

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
 * Reference sheets have no lesson to walk through (word lists, grammar
 * tables, the one-line merci sheet, the film credits): they keep the
 * page view. Any other sheet that has sentences or prose can be a deck.
 */
const REFERENCE_CATEGORIES = ["vocabulary", "annex", "merci", "credits"];

function isDeckEligible(group, sheet) {
    if (!group || REFERENCE_CATEGORIES.includes(group.category)) return false;
    return sheet.content.some((b) =>
        b.type === "audio-card" || b.type === "paragraph" || b.type === "list" || b.type === "blockquote");
}

/** Stop everything the deck started (speech, timers) and forget it. */
function teardownDeck() {
    if (!activeDeck) return;
    activeDeck.playId++;
    clearTimeout(activeDeck.revealTimer);
    if (typeof stopSpeaking === "function") stopSpeaking();
    activeDeck = null;
}

/** The sheet last rendered by renderSheetBody(), for the toolbar switch. */
let deckSheetContext = null;

/**
 * The 🃏 button of the toolbar (left of Settings). Pressed = step by step.
 * It is only offered on eligible sheets, and hidden everywhere else.
 */
function deckShowToolbarSwitch(eligible, isDeck, sheet, group) {
    const btn = document.getElementById("nav-deck");
    deckSheetContext = eligible ? { sheet, group } : null;
    btn.classList.toggle("exo-toolbar-hidden", !eligible);
    btn.setAttribute("aria-pressed", String(eligible && isDeck));
}

/** Toolbar click: flip between page and step by step, keep the same sheet. */
function deckToggleFromToolbar() {
    if (!deckSheetContext) return;
    const { sheet, group } = deckSheetContext;
    storeViewMode(getStoredViewMode() === "deck" ? "page" : "deck");
    const content = document.getElementById("content");
    content.innerHTML = "";
    renderSheetBody(content, sheet, group);
}

document.getElementById("nav-deck").addEventListener("click", deckToggleFromToolbar);

// Leaving a sheet: stop the deck, and hide its switch until a sheet renders it again.
// (Registered before app.js's router, so it runs first.)
window.addEventListener("hashchange", () => {
    teardownDeck();
    deckShowToolbarSwitch(false, false, null, null);
});

// ----------------------------------------------------------------------------
// Step building
// ----------------------------------------------------------------------------

/**
 * A two-column translate-table is shown target language first. If the
 * table already has it first, or is not a translate-table, it is kept.
 */
function targetFirst(block) {
    if (block.columns.length === 2 && block.speakable_column === 1) {
        return {
            ...block,
            columns: [block.columns[1], block.columns[0]],
            rows: block.rows.map((cells) => [cells[1], cells[0]]),
            speakable_column: 0,
        };
    }
    return block;
}

/** The translation and details of an audio-card, initially hidden. */
function deckBuildDetailNode(block, options = {}) {
    const detail = document.createElement("div");
    detail.className = "deck-phrase-detail";
    if (block.natural) {
        const p = document.createElement("p");
        p.className = "deck-phrase-natural";
        p.innerHTML = renderText(block.natural);
        detail.appendChild(p);
    }
    // options.naturalOnly: just the translation (dialogue lines), no
    // word-by-word breakdown and no notes.
    if (options.naturalOnly) return detail;
    block.literal.forEach((line) => {
        const p = document.createElement("p");
        p.className = "deck-phrase-literal";
        p.innerHTML = renderText(line);
        detail.appendChild(p);
    });
    block.notes.forEach((note) => {
        const p = document.createElement("p");
        p.className = "comments";
        p.innerHTML = renderText(note);
        detail.appendChild(p);
    });
    return detail;
}

/** An example sentence: the phrase, and its (initially hidden) details. */
function buildPhraseNode(block) {
    const wrap = document.createElement("div");
    wrap.className = "deck-phrase";

    const text = document.createElement("p");
    text.className = "deck-phrase-text";
    text.innerHTML = renderText(block.phrase);
    wrap.appendChild(text);
    wrap.appendChild(deckBuildDetailNode(block));
    return wrap;
}

/**
 * A dialogue as a chat: one bubble per line, speakers alternate sides in
 * the order they first speak. Lines are hidden until deckPlayDialogue() shows
 * them one by one.
 */
function deckBuildDialogueNode(lines) {
    const speakers = [];
    lines.forEach((b) => { if (!speakers.includes(b.speaker)) speakers.push(b.speaker); });

    const box = document.createElement("div");
    box.className = "deck-dialogue";
    lines.forEach((block) => {
        const line = document.createElement("div");
        line.className = "deck-line";
        line.dataset.side = speakers.indexOf(block.speaker) % 2 === 0 ? "left" : "right";
        line.dataset.speaker = block.speaker;

        const bubble = document.createElement("div");
        bubble.className = "deck-bubble";
        const who = document.createElement("span");
        who.className = "deck-speaker";
        who.setAttribute("aria-hidden", "true");
        who.textContent = block.speaker;
        const text = document.createElement("p");
        text.className = "deck-line-text";
        text.innerHTML = renderText(block.phrase);
        // A tap re-reads this line only.
        text.addEventListener("click", () => deckReplayDialogueLine(line));
        bubble.append(who, text, deckBuildDetailNode(block, { naturalOnly: true }));
        line.appendChild(bubble);
        box.appendChild(line);
    });
    return box;
}

/**
 * Turn sheet.content into a flat list of steps. Each step is
 * { kind, chapter, sub, nodes, isGrammar? }.
 */
/** Longest run of plain text (characters) shown on one step. */
const TEXT_STEP_MAX_CHARS = 400;

function buildDeckSteps(sheet) {
    const steps = [];
    let chapter = "";
    let sub = "";

    // The intro picture opens the sheet; the plain text right after it
    // (before the first exercise-like block) is shown on the same screen.
    let introStep = null;
    if (sheet.image) {
        introStep = { kind: "intro", chapter: "", sub: "", nodes: [renderImage(sheet.image)], chars: 0 };
        steps.push(introStep);
    }

    sheet.content.forEach((block) => {
        if (block.type === "heading") {
            if (block.level <= 2) {
                // No screen of its own: shown as the kicker of the next steps.
                chapter = block.text;
                sub = "";
            } else if (block.level === 3) {
                sub = block.text;
            }
            return;
        }

        if (block.type === "image" || block.type === "hr") return;

        const isPlainText = ["paragraph", "list", "blockquote"].includes(block.type);
        if (isPlainText) {
            // Short paragraphs of one section share a screen, up to a
            // comfortable reading length. The intro picture's screen takes
            // the text that follows it the same way.
            const node = renderBlock(block);
            const chars = node.textContent.length;
            const last = steps[steps.length - 1];
            if (last && last.kind === "text" && last.chapter === chapter && last.sub === sub
                && !sub && last.chars + chars <= TEXT_STEP_MAX_CHARS) {
                last.nodes.push(node);
                last.chars += chars;
                return;
            }
            if (introStep && last === introStep && !chapter && !sub
                && introStep.chars + chars <= TEXT_STEP_MAX_CHARS) {
                introStep.nodes.push(node);
                introStep.chars += chars;
                return;
            }
            steps.push({ kind: "text", chapter, sub, nodes: [node], chars, isGrammar: !!sub });
            sub = "";
            introStep = null;
            return;
        }
        introStep = null;

        if (block.type === "table") {
            steps.push({ kind: "table", chapter, sub, nodes: [renderTable(targetFirst(block))] });
            return;
        }

        if (block.type === "audio-card" && block.speaker) {
            // Consecutive speaker lines in the same section form one dialogue.
            const last = steps[steps.length - 1];
            if (last && last.kind === "dialogue" && last.chapter === chapter && last.sub === sub) {
                last.lines.push(block);
            } else {
                steps.push({ kind: "dialogue", chapter, sub, lines: [block] });
            }
            return;
        }

        if (block.type === "audio-card") {
            steps.push({ kind: "phrase", chapter, sub, nodes: [buildPhraseNode(block)] });
            return;
        }

        // geomap
        steps.push({ kind: "text", chapter, sub, nodes: [renderBlock(block)], isGrammar: !!sub });
        sub = "";
    });

    // Dialogue nodes are built once the runs of lines are known.
    steps.forEach((step) => {
        if (step.kind === "dialogue") step.nodes = [deckBuildDialogueNode(step.lines)];
    });

    steps.push({ kind: "end", chapter: "", sub: "", nodes: [] });
    return steps;
}

/** The rabbit of the end screen, and its way on to the next sheet. */
function buildEndNodes() {
    const nodes = [];
    if (typeof el === "function" && typeof mascotFace === "function") {
        nodes.push(el("div", { className: "exo-rabbit", attrs: { "data-mood": "bravo", "aria-hidden": "true" } },
            [mascotFace("rabbit", "bravo")]));
    }
    const pagerNext = document.getElementById("pager-top-next");
    if (pagerNext && !pagerNext.disabled) {
        const btn = document.createElement("button");
        btn.type = "button";
        btn.className = "deck-end-next";
        btn.textContent = "›";
        btn.onclick = () => deckNext();
        nodes.push(btn);
    }
    return nodes;
}

/** Build the DOM of one step, hidden until shown. */
function buildStepElement(step, index) {
    const stepEl = document.createElement("section");
    stepEl.className = `deck-step deck-step-${step.kind}`;
    stepEl.dataset.index = String(index);

    if (step.kind !== "end") {
        // Header row: the section title on the left, the fox's corner on the right.
        const head = document.createElement("div");
        head.className = "deck-head";
        const kicker = document.createElement("p");
        kicker.className = "deck-kicker";
        kicker.textContent = [step.chapter, step.sub].filter(Boolean).join(" · ");
        head.appendChild(kicker);
        stepEl.appendChild(head);
    }

    const body = document.createElement("div");
    body.className = "deck-body";
    (step.kind === "end" ? buildEndNodes() : step.nodes).forEach((n) => body.appendChild(n));
    stepEl.appendChild(body);

    if (step.kind === "phrase") {
        body.querySelector(".deck-phrase-text").addEventListener("click", () => playPhrase(index));
    }
    return stepEl;
}

// ----------------------------------------------------------------------------
// Mascots
// ----------------------------------------------------------------------------

/** Change the fox's mood, if the mascots are loaded. */
function deckFox(mood, anim) {
    if (typeof reactFox === "function") reactFox(mood, anim);
}

/** Move the (single) fox into the header of the given step. */
function mountDeckFox(stepEl) {
    const d = activeDeck;
    if (typeof el !== "function" || typeof mascotFace !== "function") return;
    if (!d.fox) {
        d.fox = el("div", { className: "exo-fox", attrs: { id: "exo-fox", "data-mood": "content", "aria-hidden": "true" } },
            [mascotFace("fox", "content")]);
    }
    const head = stepEl.querySelector(".deck-head");
    if (head) head.appendChild(d.fox);
    else d.fox.remove();
}

// ----------------------------------------------------------------------------
// Speech
// ----------------------------------------------------------------------------

/** Whether a target-language voice can be used (no alert, no warning). */
function canSpeakTarget() {
    return "speechSynthesis" in window && typeof targetVoiceAvailable !== "undefined" && targetVoiceAvailable;
}

/**
 * Read one dialogue line aloud (in its character's voice), then reveal
 * its translation. onDone runs after the reveal, if still wanted.
 */
function deckReadDialogueLine(d, line, isAlive, onDone) {
    const textEl = line.querySelector(".deck-line-text");
    const detail = line.querySelector(".deck-phrase-detail");
    const reveal = () => {
        if (!isAlive()) return;
        clearTimeout(d.revealTimer);
        detail.classList.add("is-shown");
        deckFox("content", "nod");
        if (onDone) onDone();
    };

    if (!canSpeakTarget()) {
        reveal();
        return;
    }

    deckFox("think", null);
    const text = textEl.textContent.replace(/\s+/g, " ").trim();
    const voice = effectiveCharacterVoice(line.dataset.speaker);
    d.revealTimer = setTimeout(reveal, 3000 + text.length * 150);
    speak(text, voice.rate, voice.pitch, voice.voiceURI, textEl, reveal);
}

/**
 * Play the lines of the dialogue step at `index`, top to bottom. Each line
 * appears, is read, then its translation shows; the list keeps the newest
 * line at the bottom. A new step or a re-read cancels the chain (playId).
 */
function deckPlayDialogue(index) {
    const d = activeDeck;
    if (!d) return;
    const box = d.elements[index].querySelector(".deck-dialogue");
    const lines = [...box.querySelectorAll(".deck-line")];
    const myId = ++d.playId;
    const alive = () => !!activeDeck && d.playId === myId;

    lines.forEach((l) => {
        l.classList.remove("is-shown");
        l.querySelector(".deck-phrase-detail").classList.remove("is-shown");
    });

    const showFrom = (i) => {
        if (!alive()) return;
        if (i >= lines.length) {
            deckFox("content", "nod");
            return;
        }
        const line = lines[i];
        line.classList.add("is-shown");
        box.scrollTop = box.scrollHeight;
        deckReadDialogueLine(d, line, alive, () => {
            box.scrollTop = box.scrollHeight;
            setTimeout(() => showFrom(i + 1), 450);
        });
    };
    showFrom(0);
}

/** Tap on a dialogue line: read that line again, and stop the chain. */
function deckReplayDialogueLine(line) {
    const d = activeDeck;
    if (!d) return;
    const myId = ++d.playId;
    deckReadDialogueLine(d, line, () => !!activeDeck && d.playId === myId, null);
}

/**
 * Read a phrase aloud, then reveal its translation and details. Called
 * on arrival at the step and on a tap on the sentence. Each call
 * invalidates the previous one (playId), so a cancelled or replayed
 * utterance cannot reveal anything late or on the wrong step.
 */
function playPhrase(index) {
    const d = activeDeck;
    if (!d) return;
    const stepEl = d.elements[index];
    const textEl = stepEl.querySelector(".deck-phrase-text");
    const detail = stepEl.querySelector(".deck-phrase-detail");
    const myId = ++d.playId;
    clearTimeout(d.revealTimer);
    detail.classList.remove("is-shown");

    const reveal = () => {
        if (!activeDeck || d.playId !== myId) return;
        clearTimeout(d.revealTimer);
        detail.classList.add("is-shown");
        deckFox("content", "nod");
    };

    // No voice: nothing to wait for, show the translation straight away.
    if (!canSpeakTarget()) {
        reveal();
        return;
    }

    deckFox("think", null);
    const text = textEl.textContent.replace(/\s+/g, " ").trim();
    const voice = effectiveCharacterVoice(undefined);
    // Safety net for engines that never fire "end" (long utterances on some browsers).
    d.revealTimer = setTimeout(reveal, 3000 + text.length * 150);
    speak(text, voice.rate, voice.pitch, voice.voiceURI, textEl, reveal);
}

// ----------------------------------------------------------------------------
// Rendering and navigation
// ----------------------------------------------------------------------------

/**
 * Render the deck inside #content (the toggle is already in place).
 * All steps are built once, then shown one by one.
 */
function renderDeck(content, sheet) {
    const steps = buildDeckSteps(sheet);

    const stage = document.createElement("div");
    stage.className = "deck-stage";

    const track = document.createElement("div");
    track.className = "deck-track";
    stage.appendChild(track);

    const elements = steps.map((step, i) => {
        const stepEl = buildStepElement(step, i);
        track.appendChild(stepEl);
        return stepEl;
    });

    const controls = document.createElement("nav");
    controls.className = "deck-controls";
    controls.innerHTML = `
        <button type="button" class="deck-prev">‹</button>
        <div class="deck-meter">
            <div class="deck-bar"><div class="deck-bar-fill"></div></div>
            <span class="deck-count"></span>
        </div>
        <button type="button" class="deck-next">›</button>
    `;

    content.appendChild(stage);
    content.appendChild(controls);

    // Tables carry clickable words: wire them like in page mode. They must
    // already be in the document, initializeSpeakableElements() queries it.
    initializeSpeakableElements();
    updateTtsAvailability();

    activeDeck = { stage, elements, steps, index: 0, sheet, controls, fox: null, playId: 0, revealTimer: null };
    controls.querySelector(".deck-prev").onclick = () => deckGo(-1);
    controls.querySelector(".deck-next").onclick = () => deckNext();

    // The progress bar is a map of the cards: a tap jumps to the card
    // under the finger (card i covers the i-th slice of the bar).
    const bar = controls.querySelector(".deck-bar");
    bar.addEventListener("click", (e) => {
        if (!activeDeck) return;
        const rect = bar.getBoundingClientRect();
        if (rect.width === 0) return;
        const fraction = (e.clientX - rect.left) / rect.width;
        const total = activeDeck.elements.length;
        const target = Math.min(total - 1, Math.max(0, Math.floor(fraction * total)));
        deckShow(target);
    });

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
    // Whatever was being said or waited for belongs to the previous step.
    d.playId++;
    clearTimeout(d.revealTimer);
    if (typeof stopSpeaking === "function") stopSpeaking();

    d.index = index;
    d.elements.forEach((stepEl, i) => stepEl.classList.toggle("is-current", i === index));

    const current = d.elements[index];
    const step = d.steps[index];

    mountDeckFox(current);
    // A grammar explanation: the fox ponders it with a little head tilt.
    if (step.isGrammar) deckFox("think", "tilt");
    else deckFox("content", null);

    if (step.kind === "phrase") playPhrase(index);
    if (step.kind === "dialogue") deckPlayDialogue(index);

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
