/*
 * exercises.js
 * ============
 *
 * Moteur des exercices interactifs (QCM, phrases à trous, écoute).
 * Consomme les fichiers générés par smd2exercises.py :
 *
 *   - exercises-index.json     -> écran de sélection
 *   - NomFiche.exercises.json  -> une session d'exercices
 *
 * Dépend de app.js pour la synthèse vocale (fonction speak()).
 * N'a aucune dépendance externe.
 */

// ==========================================================================
// Configuration langue (repli sûr si lang-config.js n'est pas chargé)
// ==========================================================================

const EXO_TARGET_LANG = (window.LANG_CONFIG && window.LANG_CONFIG.target_lang) || {
    name: "cible", flag: "🎯", tts_code: "en-US",
};
const EXO_NATIVE_LANG = (window.LANG_CONFIG && window.LANG_CONFIG.native_lang) || {
    name: "native", flag: "🏠", tts_code: "fr-FR",
};
const EXERCISE_THEME_LABELS = (window.LANG_CONFIG && window.LANG_CONFIG.exercise_theme_labels) || {};

function langAbbrev(lang) {
    return (lang.tts_code || lang.name || "").split("-")[0].toUpperCase();
}

// ==========================================================================
// État global
// ==========================================================================

const state = {
    answerMode: "choice",   // "choice" ou "type"
    exercises: [],
    current: 0,
    score: 0,
    mistakes: [],
    byType: {},             // { qcm: {correct, total}, "fill-blank": {...}, listen: {...} }
    sourceFile: null,       // nom du .md, sert de clé de progression
};

const APP_ROOT = document.getElementById("exercise-app");

function params() {
    return new URLSearchParams(window.location.search);
}

// ==========================================================================
// Chargement des données
// ==========================================================================

async function fetchJson(path) {
    const response = await fetch(path);
    if (!response.ok) {
        throw new Error(`Impossible de charger ${path} (${response.status})`);
    }
    return response.json();
}

async function loadFiche(cardPath) {
    return fetchJson(cardPath);
}

async function loadIndex() {
    return fetchJson("exercises-index.json");
}

// Accepte aussi bien "NomFiche" que "NomFiche.exercises.json".
function resolveCardPath(card) {
    return card.endsWith(".json") ? card : `${card}.exercises.json`;
}

// Nom du fichier source (le .md) sans extension, pour construire ?card=...
function cardStem(sourceFilename) {
    return sourceFilename.replace(/\.md$/, "");
}

// ==========================================================================
// Thème visuel
// ==========================================================================

/*
 * Déduit le thème d'une fiche à partir de son nom de fichier, en
 * miroir de detect_theme() dans smd2html.py : une fiche de série
 * (Serie_08_Tatry_01_do-hor.md) suit le thème de son unité, tout le
 * reste retombe sur "uvod".
 */
function themeFromSource(sourceFilename) {
    const match = /^Serie_\d+_([A-Za-z]+)_/.exec(sourceFilename || "");
    return match ? match[1].toLowerCase() : "uvod";
}

function applyTheme(theme) {
    document.body.dataset.theme = theme;
}

// ==========================================================================
// Bouton "Fiche" de la barre d'outils
// ==========================================================================

/*
 * Le 3e bouton de la barre d'outils (🏠 Accueil / 🎯 Exercices / 📖 Fiche)
 * n'a de sens que pendant une session : on ne peut pas revenir "à la
 * fiche correspondante" quand on est encore sur l'écran de sélection.
 * Il est donc masqué tant qu'aucune fiche source n'est connue.
 */

function goToFiche() {
    if (state.sourceHtml) {
        window.location.href = state.sourceHtml;
    }
}

function updateFicheButton() {
    const btn = document.getElementById("exo-fiche-btn");
    if (!btn) return;
    btn.classList.toggle("exo-toolbar-hidden", !state.sourceHtml);
}

const ALL_TYPES = ["qcm", "fill-blank", "listen"];

// Filtre de types partagé entre l'écran de sélection et une session en cours.
let activeTypes = new Set(ALL_TYPES);

// Sens du QCM : "l2-l1" (langue cible -> native), "l1-l2" (native ->
// cible) ou "both". "l1-l2" par défaut : on part du mot qu'on connaît
// déjà (natif) pour retrouver celui qu'on apprend, plus utile à
// l'usage que l'inverse. Aligné avec les valeurs "direction" générées
// par smd2exercises.py.
let qcmDirection = "l1-l2";

// ==========================================================================
// Utilitaires
// ==========================================================================

function shuffle(array) {
    const copy = array.slice();
    for (let i = copy.length - 1; i > 0; i--) {
        const j = Math.floor(Math.random() * (i + 1));
        [copy[i], copy[j]] = [copy[j], copy[i]];
    }
    return copy;
}

function normalizeAnswer(text) {
    return text.trim().toLowerCase().replace(/\s+/g, " ");
}

function el(tag, options = {}, children = []) {
    const node = document.createElement(tag);
    if (options.className) node.className = options.className;
    if (options.text !== undefined) node.textContent = options.text;
    if (options.html !== undefined) node.innerHTML = options.html;
    if (options.onclick) node.addEventListener("click", options.onclick);
    if (options.attrs) {
        Object.entries(options.attrs).forEach(([k, v]) => node.setAttribute(k, v));
    }
    children.forEach(child => node.appendChild(child));
    return node;
}

function speakSafe(text, rate) {
    if (typeof speak === "function") {
        speak(text, rate);
    }
}

// targetVoiceAvailable est une variable globale déclarée dans app.js
// (partagée entre scripts classiques, pas un module) : true dès qu'une
// voix dans la langue cible a été trouvée. typeof la protège si jamais
// app.js n'était pas chargé sur la page.
function ttsAvailable() {
    return typeof targetVoiceAvailable !== "undefined" && targetVoiceAvailable;
}

// ==========================================================================
// Repli si progress.js n'est pas chargé
// ==========================================================================

/*
 * progress.js (loadProgressStore, getFicheProgress, pct,
 * scoreRatioClass, averageScore) doit être inclus AVANT ce fichier
 * dans exercises.html. S'il manque (fichier pas déployé, balise
 * <script> oubliée...), on ne veut pas planter tout l'écran de
 * sélection pour autant : on retombe sur des fonctions neutres qui
 * désactivent juste l'affichage des scores, dans le même esprit que
 * la dégradation gracieuse déjà en place pour la synthèse vocale
 * (voir updateTtsAvailability() dans slovak.js).
 */

if (typeof loadProgressStore !== "function") {
    console.warn("progress.js manquant : les scores ne s'afficheront pas.");
    const fallbackPrefix = (window.LANG_CONFIG && window.LANG_CONFIG.site && window.LANG_CONFIG.site.storage_prefix) || "slovak";
    window.PROGRESS_KEY = window.PROGRESS_KEY || `${fallbackPrefix}-exercises-progress`;
    window.loadProgressStore = () => ({});
    window.getFicheProgress = () => null;
    window.pct = (correct, total) => (total ? Math.round((correct / total) * 100) : 0);
    window.scoreRatioClass = () => "bad";
    window.averageScore = () => null;
}

// ==========================================================================
// Progression (localStorage)
// ==========================================================================

/*
 * La lecture (loadProgressStore, getFicheProgress, pct,
 * scoreRatioClass, averageScore) vit dans progress.js, partagée avec
 * la page d'accueil. Ici : uniquement l'écriture.
 */

function saveProgressStore(store) {
    try {
        localStorage.setItem(PROGRESS_KEY, JSON.stringify(store));
    } catch (err) {
        // Stockage indisponible (navigation privée, quota...) : on continue
        // sans bloquer l'utilisateur, la progression ne sera juste pas gardée.
    }
}

// Enregistre le résultat d'une session terminée : dernier score + mise à
// jour du cumul (correct/total) par type d'exercice.
function recordSession(source, score, total, byType) {
    const store = loadProgressStore();
    const existing = store[source] || { cumulative: { byType: {}, sessionsCount: 0 } };

    existing.last = { date: new Date().toISOString(), score, total, byType };
    existing.cumulative.sessionsCount = (existing.cumulative.sessionsCount || 0) + 1;

    Object.entries(byType).forEach(([type, stats]) => {
        const cum = existing.cumulative.byType[type] || { correct: 0, total: 0 };
        cum.correct += stats.correct;
        cum.total += stats.total;
        existing.cumulative.byType[type] = cum;
    });

    store[source] = existing;
    saveProgressStore(store);
}

function resetProgress() {
    try {
        localStorage.removeItem(PROGRESS_KEY);
    } catch (err) { /* ignore */ }
}

// ==========================================================================
// Écran de sélection
// ==========================================================================

function categoryEmoji(category) {
    const map = {
        "Révisions": "📝",
        "Dialogues": "💬",
        "Situations": "💬",
        "Grammaire": "📖",
        "Vocabulaire": "📖",
        "Introduction": EXO_TARGET_LANG.flag,
        "Séries": "🧵",
    };
    return map[category] || "🗂️";
}

// Numérotation continue des cartes numérotées (voir renderEntryCard) :
// même logique que "num" dans publish-slovak.sh pour .index-num — un
// compteur global au fil du parcours, pas remis à zéro par unité.
let cardCounter = 0;

// Fiches cochées sur l'écran de sélection, par "stem" (nom du .md sans
// extension — voir cardStem()). Remis à zéro à chaque (re)rendu de
// l'écran de sélection.
let selectedSources = new Set();

// Liste complète des fiches disponibles (exercises-index.json), gardée
// pour les filtres rapides ("Déjà pratiquées"...) et le calcul du pool
// disponible dans le panneau de lancement.
let currentEntries = [];

async function renderSelectionScreen() {
    // Écran de sélection = neutre : la couleur arrive avec la fiche choisie.
    applyTheme("uvod");
    // Pas de fiche source ici : le 3e bouton de la barre d'outils reste masqué.
    state.sourceHtml = null;
    updateFicheButton();
    // Numérotation continue des cartes (voir renderEntryCard()), à
    // remettre à zéro à chaque (re)rendu de l'écran de sélection.
    cardCounter = 0;
    selectedSources = new Set();

    APP_ROOT.innerHTML = "";
    APP_ROOT.appendChild(el("p", { className: "exo-loading", text: "Chargement des fiches disponibles…" }));

    let data;
    try {
        data = await loadIndex();
    } catch (err) {
        APP_ROOT.innerHTML = "";
        APP_ROOT.appendChild(el("p", {
            className: "exo-error",
            text: "Impossible de charger exercises-index.json. As-tu lancé le script de génération ?",
        }));
        return;
    }

    // exercises-index.json est un objet {entries, groups} ; on accepte
    // encore l'ancien format (liste plate) par sécurité de migration.
    const entries = Array.isArray(data) ? data : (data.entries || []);
    const groups = Array.isArray(data) ? [] : (data.groups || []);
    currentEntries = entries;

    APP_ROOT.innerHTML = "";

    if (!entries.length) {
        APP_ROOT.appendChild(el("p", { text: "Aucune fiche d'exercices trouvée pour l'instant." }));
        return;
    }

    const header = el("div", { className: "exo-selection-header" }, [
        el("h2", { text: "Choisis tes fiches" }),
        el("p", { className: "exo-selection-hint", text: "Coche une ou plusieurs fiches (ou une série entière), puis lance un questionnaire mélangé." }),
    ]);
    header.appendChild(renderQuickFilters());
    const filterRow = el("div", { className: "exo-filter-row" }, [
        renderTypeCheckboxes(activeTypes, updateLaunchBar),
        renderDirectionSelector(updateLaunchBar),
    ]);
    header.appendChild(filterRow);
    APP_ROOT.appendChild(header);

    buildSections(entries, groups).forEach(section => {
        APP_ROOT.appendChild(renderSection(section));
    });

    APP_ROOT.appendChild(buildLaunchBar());

    APP_ROOT.appendChild(el("button", {
        className: "exo-reset-progress",
        text: "🗑️ Réinitialiser ma progression",
        onclick: () => {
            if (window.confirm("Effacer tous les scores enregistrés sur cet appareil ?")) {
                resetProgress();
                renderSelectionScreen();
            }
        },
    }));

    updateLaunchBar();
}

// Boutons "Tout / Déjà pratiquées / Non commencées / Aucune" : cochent
// en masse à partir des données déjà connues (progress.js), sans rien
// stocker de nouveau. Une action ponctuelle, pas un mode de filtre
// permanent — on peut toujours retoucher des cases à la main ensuite.
function renderQuickFilters() {
    const row = el("div", { className: "exo-quick-filters" });

    const setAll = (predicate) => {
        selectedSources = new Set(
            currentEntries.filter(predicate).map(entry => cardStem(entry.source))
        );
        refreshCheckboxes();
        updateLaunchBar();
    };

    row.appendChild(el("button", { className: "chip", text: "Tout", onclick: () => setAll(() => true) }));
    row.appendChild(el("button", {
        className: "chip",
        text: "Déjà pratiquées",
        onclick: () => setAll(entry => !!(getFicheProgress(entry.source) || {}).last),
    }));
    row.appendChild(el("button", {
        className: "chip",
        text: "Non commencées",
        onclick: () => setAll(entry => !(getFicheProgress(entry.source) || {}).last),
    }));
    row.appendChild(el("button", { className: "chip", text: "Aucune", onclick: () => setAll(() => false) }));
    return row;
}

// Recale l'état coché/décoché de toutes les cases (fiches + cases
// "série") sur selectedSources — source de vérité unique, qu'on vienne
// de cocher une case individuelle, une case série, ou un filtre rapide.
function refreshCheckboxes() {
    document.querySelectorAll(".index-card[data-stem]").forEach(card => {
        const checkbox = card.querySelector("input[type=checkbox]");
        if (checkbox) checkbox.checked = selectedSources.has(card.dataset.stem);
    });
    document.querySelectorAll(".index-section").forEach(section => {
        const boxes = [...section.querySelectorAll(".index-card[data-stem] input[type=checkbox]")];
        const seriesCheckbox = section.querySelector("summary input[type=checkbox]");
        if (!seriesCheckbox || !boxes.length) return;
        const checkedCount = boxes.filter(b => b.checked).length;
        seriesCheckbox.checked = checkedCount === boxes.length;
        seriesCheckbox.indeterminate = checkedCount > 0 && checkedCount < boxes.length;
    });
}

/*
 * Organise les fiches d'exercices en sections, dans l'ordre du
 * parcours pédagogique (même sommaire que la page d'accueil, voir
 * src/parcours.txt) : une fiche de l'index correspond à une entrée
 * "files" d'une unité du parcours par son nom de fichier source.
 *
 * Une fiche listée dans le parcours mais sans exercices générés est
 * simplement absente (rien à afficher). Une fiche avec exercices
 * mais absente du parcours atterrit dans "À classer", en miroir du
 * comportement de l'index principal.
 *
 * Repli : si aucun parcours n'a été fourni au générateur (--parcours
 * absent), on retombe sur l'ancien regroupement par catégorie.
 */

function buildSections(entries, groups) {
    if (!groups.length) {
        const byCategory = {};
        entries.forEach(entry => {
            (byCategory[entry.category] = byCategory[entry.category] || []).push(entry);
        });
        return Object.entries(byCategory).map(([category, list]) => ({
            label: `${categoryEmoji(category)} ${category}`,
            entries: list,
            numbered: false,
        }));
    }

    const bySource = new Map(entries.map(entry => [cardStem(entry.source), entry]));
    const used = new Set();
    const sections = [];

    groups.forEach(group => {
        const list = [];
        group.files.forEach(stem => {
            const entry = bySource.get(stem);
            if (entry) {
                list.push(entry);
                used.add(stem);
            }
        });
        if (list.length) {
            sections.push({ label: group.label, entries: list, numbered: true });
        }
    });

    // Comme sur la page d'accueil : les fiches du sommaire sont
    // numérotées (.index-num), celles reléguées à "À classer" non.
    const orphans = entries.filter(entry => !used.has(cardStem(entry.source)));
    if (orphans.length) {
        sections.push({ label: "🗂️ À classer", entries: orphans, numbered: false });
    }

    return sections;
}

// Réutilise .index-section / .index-grid / .index-card de la page
// d'accueil (voir style.css) : même apparence, une seule définition
// à maintenir des deux côtés. Repliée par défaut ; le clic sur le
// titre (natif à <details>/<summary>) l'ouvre ou la referme. La case
// à cocher de l'en-tête sélectionne/désélectionne toute la série d'un
// coup, sans avoir besoin de déplier.
function renderSection(section) {
    // Thème de l'unité déduit de sa première fiche, comme theme_key()
    // côté publish-slovak.sh pour la page d'accueil.
    const theme = themeFromSource(section.entries[0].source);

    const details = el("details", { className: "index-section" });
    details.dataset.theme = theme;

    const h2 = el("h2", { text: `${section.label} (${section.entries.length})` });

    // Moyenne des scores des fiches de l'unité (voir averageScore()
    // dans progress.js) : chaque fiche pratiquée compte pour une voix.
    const score = averageScore(section.entries.map(entry => entry.source));
    if (score) {
        const plural = score.count > 1 ? "s" : "";
        h2.appendChild(el("span", {
            className: `index-score-badge index-score-${scoreRatioClass(score.avg, 100)}`,
            text: `${score.avg}%`,
            attrs: { title: `Moyenne de l'unité : ${score.avg}% (${score.count}/${section.entries.length} fiche${plural} pratiquée${plural})` },
        }));
    }

    const seriesCheckbox = el("input", { attrs: { type: "checkbox", "aria-label": `Sélectionner toute la série ${section.label}` } });
    // Empêche le clic sur la case de (dé)plier le <details> — sans
    // ça, cocher la case ouvrirait/fermerait la section à chaque fois.
    seriesCheckbox.addEventListener("click", (e) => e.stopPropagation());
    seriesCheckbox.addEventListener("change", () => {
        section.entries.forEach(entry => {
            const stem = cardStem(entry.source);
            if (seriesCheckbox.checked) selectedSources.add(stem); else selectedSources.delete(stem);
        });
        refreshCheckboxes();
        updateLaunchBar();
    });

    details.appendChild(el("summary", {}, [seriesCheckbox, h2]));

    const grid = el("div", { className: "index-grid" });
    section.entries.forEach(entry => grid.appendChild(renderEntryCard(entry, section.numbered)));
    details.appendChild(grid);

    return details;
}

// Carte d'une fiche sur l'écran de sélection — réutilise .index-card
// (voir style.css), avec en plus les compteurs d'exercices et le
// score, poussés à droite via .index-card-badges. C'est un <label>
// entourant une case à cocher plutôt qu'un bouton de navigation
// directe : la sélection est multiple, le lancement se fait depuis le
// panneau du bas (voir buildLaunchBar()).
function renderEntryCard(entry, numbered) {
    const stem = cardStem(entry.source);
    const themeLabel = entry.theme ? (EXERCISE_THEME_LABELS[entry.theme] || entry.theme) : null;
    const progress = getFicheProgress(entry.source);

    const badges = [];
    ["qcm", "fill-blank", "listen"].forEach(type => {
        if (entry.counts[type]) {
            badges.push(el("span", {
                className: "exo-badge",
                text: `${typeIcon(type)} ${entry.counts[type]}`,
                attrs: { title: typeLabel(type) },
            }));
        }
    });
    if (progress && progress.last) {
        badges.push(el("span", {
            className: `index-score-badge index-score-${scoreRatioClass(progress.last.score, progress.last.total)}`,
            text: `${progress.last.score}/${progress.last.total}`,
            attrs: { title: `Dernier essai : ${new Date(progress.last.date).toLocaleDateString()}` },
        }));
    }

    const checkbox = el("input", { attrs: { type: "checkbox" } });
    checkbox.checked = selectedSources.has(stem);
    checkbox.addEventListener("change", () => {
        if (checkbox.checked) selectedSources.add(stem); else selectedSources.delete(stem);
        refreshCheckboxes();
        updateLaunchBar();
    });

    const card = el("label", { className: "index-card" }, [
        checkbox,
        ...(numbered ? [(() => {
            cardCounter += 1;
            return el("span", { className: "index-num", text: String(cardCounter).padStart(2, "0") });
        })()] : []),
        el("span", { className: "index-card-title", text: entry.title }),
        ...(themeLabel ? [el("span", { className: "index-card-theme", text: ` · ${themeLabel}` })] : []),
        el("span", { className: "index-card-badges" }, badges),
    ]);
    // Liseré de couleur propre à l'unité de la fiche, pour repérer
    // le thème d'un coup d'œil avant même d'avoir coché quoi que ce soit.
    card.dataset.theme = themeFromSource(entry.source);
    card.dataset.stem = stem;
    return card;
}

function typeLabel(type) {
    return { qcm: "🔤 QCM", "fill-blank": "✏️ Trous", listen: "🔊 Écoute" }[type] || type;
}

function typeIcon(type) {
    return { qcm: "🔤", "fill-blank": "✏️", listen: "🔊" }[type] || "❔";
}

// ==========================================================================
// Panneau de lancement (bas de l'écran de sélection)
// ==========================================================================

function buildLaunchBar() {
    const bar = el("div", { className: "exo-launch-bar", attrs: { id: "exo-launch-bar" } });

    bar.appendChild(el("p", { className: "exo-launch-summary", attrs: { id: "exo-launch-summary" } }));

    bar.appendChild(el("div", { className: "exo-launch-qcount" }, [
        el("label", { text: "Nombre de questions", attrs: { for: "exo-launch-n" } }),
        el("input", { attrs: { type: "number", id: "exo-launch-n", min: "1", value: "20" } }),
    ]));

    bar.appendChild(el("button", {
        className: "exo-launch-btn",
        text: "▶️ Lancer la session",
        attrs: { id: "exo-launch-btn" },
        onclick: startMixedFromSelection,
    }));

    return bar;
}

// Recalcule le texte récapitulatif et le pool disponible à chaque
// changement de sélection ou de filtre de type. Le nombre de questions
// n'est qu'une approximation ("en gros") : pour le QCM, le compteur de
// la fiche inclut les deux sens à la fois, donc le vrai pool est un peu
// plus petit dès qu'un seul sens est choisi — même imprécision qu'avant
// sur l'écran mono-fiche, pas une régression introduite ici.
function updateLaunchBar() {
    const bar = document.getElementById("exo-launch-bar");
    if (!bar) return;

    const selectedEntries = currentEntries.filter(entry => selectedSources.has(cardStem(entry.source)));
    const effectiveTypes = ALL_TYPES.filter(t => activeTypes.has(t) && (t !== "listen" || ttsAvailable()));
    const pool = selectedEntries.reduce((sum, entry) => (
        sum + effectiveTypes.reduce((s, t) => s + (entry.counts[t] || 0), 0)
    ), 0);

    const summaryEl = document.getElementById("exo-launch-summary");
    if (selectedEntries.length) {
        const fp = selectedEntries.length > 1 ? "s" : "";
        const ep = pool > 1 ? "s" : "";
        summaryEl.textContent = `${selectedEntries.length} fiche${fp} sélectionnée${fp} · ≈ ${pool} exercice${ep} disponible${ep}`;
    } else {
        summaryEl.textContent = "Sélectionne au moins une fiche pour commencer.";
    }

    const input = document.getElementById("exo-launch-n");
    const btn = document.getElementById("exo-launch-btn");
    if (pool > 0) {
        input.max = pool;
        input.disabled = false;
        if (!input.value || parseInt(input.value, 10) > pool) {
            input.value = Math.min(20, pool);
        }
    } else {
        input.disabled = true;
    }
    btn.disabled = selectedEntries.length === 0 || pool === 0;
}

function startMixedFromSelection() {
    const n = parseInt(document.getElementById("exo-launch-n").value, 10) || 10;
    const url = new URL(window.location.href);
    url.searchParams.delete("card");
    url.searchParams.set("cards", [...selectedSources].join(","));
    url.searchParams.set("n", n);
    if (activeTypes.size < ALL_TYPES.length) {
        url.searchParams.set("types", [...activeTypes].join(","));
    } else {
        url.searchParams.delete("types");
    }
    if (qcmDirection !== "l1-l2") {
        url.searchParams.set("direction", qcmDirection);
    } else {
        url.searchParams.delete("direction");
    }
    window.location.href = url.toString();
}

// Case à cocher réutilisée sur l'écran de sélection et dans une session.
// "Écoute" est désactivée et décochée d'office s'il n'y a aucune voix
// disponible pour la langue cible (voir ttsAvailable()) : inutile de
// proposer des questions qu'on ne peut pas entendre.
function renderTypeCheckboxes(selectedSet, onChange) {
    const row = el("div", { className: "exo-type-filter" });
    const inputs = [];
    ALL_TYPES.forEach(type => {
        const disabled = type === "listen" && !ttsAvailable();
        if (disabled) selectedSet.delete(type);

        const input = el("input", { attrs: { type: "checkbox" } });
        input.checked = selectedSet.has(type);
        input.disabled = disabled;
        input.addEventListener("change", () => {
            if (input.checked) {
                selectedSet.add(type);
            } else if (selectedSet.size > 1) {
                selectedSet.delete(type);
            } else {
                input.checked = true; // on garde toujours au moins un type actif
            }
            onChange();
        });
        const label = el("label", {
            attrs: { title: disabled ? "Écoute désactivée : aucune voix disponible sur cet appareil" : typeLabel(type) },
        }, [
            input,
            document.createTextNode(` ${typeIcon(type)}`),
        ]);
        if (disabled) label.classList.add("exo-type-disabled");
        row.appendChild(label);
        inputs.push({ type, input });
    });

    // Cas limite : un vieux lien "?types=listen" tout seul, sur un
    // appareil sans voix disponible, viderait selectedSet entièrement
    // (le seul type demandé vient d'être retiré ci-dessus). On retombe
    // sur tous les types encore utilisables plutôt que de se retrouver
    // avec zéro type actif.
    if (selectedSet.size === 0) {
        inputs.forEach(({ type, input }) => {
            if (!input.disabled) {
                selectedSet.add(type);
                input.checked = true;
            }
        });
    }

    return row;
}

// Sélecteur du sens du QCM (SK→FR / FR→SK / Les deux), réutilisé sur
// l'écran de sélection et dans une session. Menu déroulant pour rester
// compact en hauteur.
function renderDirectionSelector(onChange) {
    const select = el("select", { className: "exo-direction-select" });
    const options = [
        { value: "l1-l2", label: `${EXO_NATIVE_LANG.flag}→${EXO_TARGET_LANG.flag} ${langAbbrev(EXO_NATIVE_LANG)}→${langAbbrev(EXO_TARGET_LANG)}` },
        { value: "l2-l1", label: `${EXO_TARGET_LANG.flag}→${EXO_NATIVE_LANG.flag} ${langAbbrev(EXO_TARGET_LANG)}→${langAbbrev(EXO_NATIVE_LANG)}` },
        { value: "both", label: "🔀 Les deux sens" },
    ];
    options.forEach(({ value, label }) => {
        const option = el("option", { text: label, attrs: { value } });
        if (qcmDirection === value) option.selected = true;
        select.appendChild(option);
    });
    select.addEventListener("change", () => {
        qcmDirection = select.value;
        onChange();
    });
    return select;
}

// ==========================================================================
// Démarrage d'une session
// ==========================================================================

async function startSession() {
    const p = params();
    const cardParam = p.get("card");
    const cardsParam = p.get("cards");
    const requestedN = p.get("n") ? parseInt(p.get("n"), 10) : 10;

    if (cardsParam) {
        await startMergedSession(cardsParam.split(",").filter(Boolean), requestedN);
        return;
    }

    activeTypes = new Set(p.get("types") ? p.get("types").split(",") : ALL_TYPES);
    qcmDirection = p.get("direction") || "l1-l2";
    state.answerMode = p.get("mode") === "type" ? "type" : "choice";
    state.sessionSize = requestedN;

    APP_ROOT.innerHTML = "";
    APP_ROOT.appendChild(el("p", { className: "exo-loading", text: "Préparation des exercices…" }));

    let data;
    try {
        data = await loadFiche(resolveCardPath(cardParam));
    } catch (err) {
        APP_ROOT.innerHTML = "";
        APP_ROOT.appendChild(el("p", { className: "exo-error", text: `Impossible de charger la fiche "${cardParam}".` }));
        return;
    }

    state.ficheTitle = data.title;
    state.sourceFile = data.source;
    state.allExercises = data.exercises;
    state.sourceHtml = data.source ? data.source.replace(/\.md$/, ".html") : null;
    updateFicheButton();

    // La session prend la couleur de la fiche travaillée.
    applyTheme(themeFromSource(data.source));

    APP_ROOT.innerHTML = "";
    APP_ROOT.appendChild(buildToolbar());
    APP_ROOT.appendChild(el("div", { className: "exo-progress-bar" }, [
        el("span", { attrs: { id: "exo-progress-fill" } }),
    ]));
    APP_ROOT.appendChild(el("div", { className: "exo-progress-text", attrs: { id: "exo-progress" } }));
    APP_ROOT.appendChild(el("div", { className: "exo-card-area", attrs: { id: "exo-card-area" } }));

    applyTypeFilter();
}

// Session "mélangée" : plusieurs fiches choisies sur l'écran de
// sélection, fusionnées en un seul questionnaire. Pas de fiche source
// unique, donc pas de progression enregistrée (renderSummary() est déjà
// conditionné sur state.sourceFile) ni de bouton "📖 Fiche" (idem pour
// updateFicheButton()) — comportement gratuit, pas de branche à ajouter
// ailleurs.
async function startMergedSession(stems, requestedN) {
    const p = params();
    activeTypes = new Set(p.get("types") ? p.get("types").split(",") : ALL_TYPES);
    qcmDirection = p.get("direction") || "l1-l2";
    state.answerMode = p.get("mode") === "type" ? "type" : "choice";
    state.sessionSize = requestedN;

    APP_ROOT.innerHTML = "";
    APP_ROOT.appendChild(el("p", { className: "exo-loading", text: "Préparation du questionnaire mélangé…" }));

    let fiches;
    try {
        fiches = await Promise.all(stems.map(stem => loadFiche(resolveCardPath(stem))));
    } catch (err) {
        APP_ROOT.innerHTML = "";
        APP_ROOT.appendChild(el("p", { className: "exo-error", text: "Impossible de charger une des fiches sélectionnées." }));
        return;
    }

    state.ficheTitle = `Session mélangée (${fiches.length} fiche${fiches.length > 1 ? "s" : ""})`;
    state.sourceFile = null;
    state.allExercises = fiches.flatMap(f => f.exercises);
    state.sourceHtml = null;
    updateFicheButton();

    // Pas de thème unique pertinent pour un mélange de fiches.
    applyTheme("uvod");

    APP_ROOT.innerHTML = "";
    APP_ROOT.appendChild(buildToolbar());
    APP_ROOT.appendChild(el("div", { className: "exo-progress-bar" }, [
        el("span", { attrs: { id: "exo-progress-fill" } }),
    ]));
    APP_ROOT.appendChild(el("div", { className: "exo-progress-text", attrs: { id: "exo-progress" } }));
    APP_ROOT.appendChild(el("div", { className: "exo-card-area", attrs: { id: "exo-card-area" } }));

    applyTypeFilter();
}

// Recalcule le pool d'exercices depuis la liste complète en fonction du
// filtre de types actif, et (re)démarre une session dessus.
function applyTypeFilter() {
    let pool = state.allExercises.filter(ex => activeTypes.has(ex.type));
    // Filet de sécurité, indépendant de activeTypes : même avec un
    // vieux lien "?types=listen" en favori, pas de question audio
    // sans voix disponible.
    if (!ttsAvailable()) {
        pool = pool.filter(ex => ex.type !== "listen");
    }
    if (qcmDirection !== "both") {
        pool = pool.filter(ex => ex.type !== "qcm" || ex.direction === qcmDirection);
    }

    if (!pool.length) {
        document.getElementById("exo-card-area").innerHTML =
            '<p class="exo-error">Aucun exercice disponible pour ce filtre.</p>';
        document.getElementById("exo-progress").textContent = "";
        return;
    }

    state.exercises = shuffle(pool).slice(0, state.sessionSize || 10);
    state.current = 0;
    state.score = 0;
    state.mistakes = [];
    state.byType = {};

    const url = new URL(window.location.href);
    if (activeTypes.size < ALL_TYPES.length) {
        url.searchParams.set("types", [...activeTypes].join(","));
    } else {
        url.searchParams.delete("types");
    }
    if (qcmDirection !== "l1-l2") {
        url.searchParams.set("direction", qcmDirection);
    } else {
        url.searchParams.delete("direction");
    }
    window.history.replaceState(null, "", url.toString());

    renderCurrent();
}

function buildToolbar() {
    // Le lien vers la fiche vit désormais dans la barre d'outils du
    // haut (bouton 📖 Fiche, voir updateFicheButton()) : harmonisé avec
    // le triptyque Accueil / Traductions / Exercices des fiches normales.
    const toolbar = el("div", { className: "exo-toolbar" }, [
        el("h2", { text: `🎯 ${state.ficheTitle}` }),
    ]);

    const controlsRow = el("div", { className: "exo-filter-row" }, [
        renderTypeCheckboxes(activeTypes, applyTypeFilter),
        renderDirectionSelector(applyTypeFilter),
    ]);

    const modeToggle = el("div", { className: "exo-mode-toggle" }, [
        el("label", {}, [
            el("input", { attrs: { type: "radio", name: "exo-mode", value: "choice", ...(state.answerMode === "choice" ? { checked: "checked" } : {}) } }),
            document.createTextNode(" QCM"),
        ]),
        el("label", {}, [
            el("input", { attrs: { type: "radio", name: "exo-mode", value: "type", ...(state.answerMode === "type" ? { checked: "checked" } : {}) } }),
            document.createTextNode(" Saisie libre"),
        ]),
    ]);
    modeToggle.querySelectorAll("input").forEach(input => {
        input.addEventListener("change", (e) => {
            state.answerMode = e.target.value;
            renderCurrent();
        });
    });
    controlsRow.appendChild(modeToggle);
    toolbar.appendChild(controlsRow);
    return toolbar;
}

function updateProgress() {
    const total = state.exercises.length;
    const current = state.current + 1;

    const fill = document.getElementById("exo-progress-fill");
    if (fill) fill.style.width = `${Math.round((state.current / total) * 100)}%`;

    const text = document.getElementById("exo-progress");
    text.innerHTML = "";
    text.appendChild(el("span", { text: `Question ${current} / ${total}` }));
    text.appendChild(el("span", { text: `Score ${state.score}` }));
}

// ==========================================================================
// Rendu d'un exercice
// ==========================================================================

function renderCurrent() {
    if (state.current >= state.exercises.length) {
        renderSummary();
        return;
    }
    updateProgress();
    const area = document.getElementById("exo-card-area");
    area.innerHTML = "";
    const ex = state.exercises[state.current];

    if (ex.type === "qcm") {
        area.appendChild(renderQcm(ex));
    } else if (ex.type === "fill-blank") {
        area.appendChild(renderFillBlank(ex));
    } else if (ex.type === "listen") {
        area.appendChild(renderListen(ex));
    }
}

// Détermine le texte à (ré)écouter après une réponse, et null si ça
// n'a pas de sens de le proposer (voir markAnswer()).
function replayText(ex) {
    if (ex.type === "qcm") {
        // ex.answer n'est dans la langue cible (donc prononçable
        // correctement par la voix configurée) que dans le sens
        // langue native -> langue cible. Dans l'autre sens, c'est du
        // texte en langue native : pas de bouton dans ce cas.
        return ex.direction === "l1-l2" ? ex.answer : null;
    }
    if (ex.type === "fill-blank") {
        // La phrase entière (celle où se trouvait le mot manquant),
        // pas juste le mot seul — ex.audio, toujours en langue cible.
        return ex.audio;
    }
    // "listen" garde ses propres gros boutons dédiés tout au long de
    // l'exercice : pas besoin d'en dupliquer un petit ici.
    return null;
}

function markAnswer(container, isCorrect, ex, userAnswer) {
    container.classList.add(isCorrect ? "exo-correct" : "exo-incorrect");

    if (!state.byType[ex.type]) state.byType[ex.type] = { correct: 0, total: 0 };
    state.byType[ex.type].total += 1;

    if (isCorrect) {
        state.score += 1;
        state.byType[ex.type].correct += 1;
    } else {
        state.mistakes.push({ ex, userAnswer });
    }

    const replay = replayText(ex);
    const canReplay = !!replay && ttsAvailable();

    const feedbackLine = [
        el("span", {
            className: isCorrect ? "exo-feedback-ok" : "exo-feedback-ko",
            text: isCorrect ? "✅ Correct !" : `❌ Raté — réponse : ${ex.answer}`,
        }),
    ];
    if (canReplay) {
        feedbackLine.push(el("button", {
            className: "exo-feedback-audio",
            text: "🔊",
            attrs: { type: "button", "aria-label": "Écouter la prononciation" },
            onclick: () => speakSafe(replay, 0.9),
        }));
    }

    const feedback = el("div", { className: "exo-feedback" }, [
        el("p", { className: "exo-feedback-line" }, feedbackLine),
        ...(ex.translation ? [el("p", { className: "exo-translation", text: ex.translation })] : []),
    ]);
    container.appendChild(feedback);

    const next = el("button", {
        className: "exo-next",
        text: state.current + 1 < state.exercises.length ? "Suivant →" : "Voir le score →",
        onclick: () => {
            state.current += 1;
            renderCurrent();
        },
    });
    container.appendChild(next);

    // Après "next" : si jamais speak() plante (voix inattendue...), le
    // bouton Suivant reste malgré tout disponible.
    if (canReplay) {
        speakSafe(replay, 0.9);
    }
}

// -- QCM ---------------------------------------------------------------

function renderQcm(ex) {
    const container = el("div", { className: "exo-card exo-qcm" });
    container.appendChild(el("div", { className: "exo-question-label", text: "Traduis :" }));
    container.appendChild(el("div", { className: "exo-question", text: ex.question }));

    if (state.answerMode === "choice") {
        const choicesEl = el("div", { className: "exo-choices" });
        ex.choices.forEach(choice => {
            const btn = el("button", {
                className: "exo-choice",
                text: choice,
                onclick: () => {
                    disableChoices(choicesEl);
                    const correct = choice === ex.answer;
                    btn.classList.add(correct ? "exo-choice-correct" : "exo-choice-wrong");
                    if (!correct) highlightCorrectChoice(choicesEl, ex.answer);
                    markAnswer(container, correct, ex, choice);
                },
            });
            choicesEl.appendChild(btn);
        });
        container.appendChild(choicesEl);
    } else {
        container.appendChild(renderTypeInput(container, ex));
    }

    return container;
}

// -- Phrase à trous ------------------------------------------------------

function renderFillBlank(ex) {
    const container = el("div", { className: "exo-card exo-fill-blank" });
    container.appendChild(el("div", { className: "exo-question-label", text: "Complète la phrase :" }));

    const sentence = el("div", { className: "exo-sentence" });
    const blank = el("span", { className: "exo-blank", text: "___" });
    ex.tokens.forEach((token, i) => {
        if (i === ex.blank_index) {
            sentence.appendChild(blank);
        } else {
            sentence.appendChild(document.createTextNode(token));
        }
        sentence.appendChild(document.createTextNode(" "));
    });
    container.appendChild(sentence);

    if (state.answerMode === "choice") {
        const choicesEl = el("div", { className: "exo-choices" });
        ex.choices.forEach(choice => {
            const btn = el("button", {
                className: "exo-choice",
                text: choice,
                onclick: () => {
                    disableChoices(choicesEl);
                    blank.textContent = choice;
                    const correct = choice === ex.answer;
                    blank.classList.add(correct ? "exo-choice-correct" : "exo-choice-wrong");
                    btn.classList.add(correct ? "exo-choice-correct" : "exo-choice-wrong");
                    markAnswer(container, correct, ex, choice);
                },
            });
            choicesEl.appendChild(btn);
        });
        container.appendChild(choicesEl);
    } else {
        container.appendChild(renderTypeInput(container, ex, (value) => { blank.textContent = value; }));
    }

    return container;
}

// -- Écoute et devine ------------------------------------------------------

function renderListen(ex) {
    const container = el("div", { className: "exo-card exo-listen" });
    container.appendChild(el("div", { className: "exo-question-label", text: "Écoute et retrouve la phrase :" }));

    const playBtn = el("button", {
        className: "exo-audio-btn exo-audio-btn-big",
        text: "🔊 Écouter",
        onclick: () => speakSafe(ex.audio, 0.9),
    });
    container.appendChild(playBtn);
    container.appendChild(el("button", {
        className: "exo-audio-btn",
        text: "🐢 Lentement",
        onclick: () => speakSafe(ex.audio, 0.65),
    }));

    if (state.answerMode === "choice") {
        const choicesEl = el("div", { className: "exo-choices exo-choices-vertical" });
        ex.choices.forEach(choice => {
            const btn = el("button", {
                className: "exo-choice",
                text: choice,
                onclick: () => {
                    disableChoices(choicesEl);
                    const correct = choice === ex.answer;
                    btn.classList.add(correct ? "exo-choice-correct" : "exo-choice-wrong");
                    if (!correct) highlightCorrectChoice(choicesEl, ex.answer);
                    markAnswer(container, correct, ex, choice);
                },
            });
            choicesEl.appendChild(btn);
        });
        container.appendChild(choicesEl);
    } else {
        container.appendChild(renderTypeInput(container, ex));
    }

    // Joue la phrase automatiquement à l'arrivée sur la question.
    speakSafe(ex.audio, 0.9);

    return container;
}

// -- Saisie libre (commune aux 3 types) -----------------------------------

function renderTypeInput(container, ex, onValidate) {
    const wrap = el("div", { className: "exo-type-input" });
    const input = el("input", { attrs: { type: "text", placeholder: "Ta réponse…", autocomplete: "off" } });
    const submit = el("button", {
        className: "exo-submit",
        text: "Valider",
        onclick: () => {
            const value = input.value;
            input.disabled = true;
            submit.disabled = true;
            const correct = normalizeAnswer(value) === normalizeAnswer(ex.answer);
            input.classList.add(correct ? "exo-choice-correct" : "exo-choice-wrong");
            if (onValidate) onValidate(correct ? ex.answer : value);
            markAnswer(container, correct, ex, value);
        },
    });
    input.addEventListener("keydown", (e) => {
        if (e.key === "Enter") submit.click();
    });
    wrap.appendChild(input);
    wrap.appendChild(submit);
    return wrap;
}

function disableChoices(choicesEl) {
    choicesEl.querySelectorAll("button").forEach(b => { b.disabled = true; });
}

function highlightCorrectChoice(choicesEl, answer) {
    choicesEl.querySelectorAll("button").forEach(b => {
        if (b.textContent === answer) b.classList.add("exo-choice-correct");
    });
}

// ==========================================================================
// Récapitulatif de fin de session
// ==========================================================================

function renderSummary() {
    const area = document.getElementById("exo-card-area");
    document.getElementById("exo-progress").textContent = "";
    const fill = document.getElementById("exo-progress-fill");
    if (fill) fill.style.width = "100%";
    area.innerHTML = "";

    const total = state.exercises.length;
    const scorePct = pct(state.score, total);

    // On récupère l'historique AVANT d'enregistrer la session courante,
    // pour pouvoir afficher "avant / après".
    const previous = state.sourceFile ? getFicheProgress(state.sourceFile) : null;
    if (state.sourceFile) {
        recordSession(state.sourceFile, state.score, total, state.byType);
    }
    const updated = state.sourceFile ? getFicheProgress(state.sourceFile) : null;

    const summary = el("div", { className: "exo-card exo-summary" }, [
        el("h3", { text: "Résultat" }),
        el("p", { className: "exo-score", text: `${state.score} / ${total} (${scorePct}%)` }),
    ]);

    if (previous && previous.last) {
        const prevDate = new Date(previous.last.date).toLocaleDateString();
        summary.appendChild(el("p", {
            className: "exo-history-line",
            text: `Essai précédent : ${previous.last.score}/${previous.last.total} (${prevDate})`,
        }));
    }

    if (updated && updated.cumulative && updated.cumulative.sessionsCount > 1) {
        const cumLine = el("div", { className: "exo-cumulative" });
        cumLine.appendChild(el("p", { className: "exo-history-line", text: "Moyenne cumulée :" }));
        const row = el("div", { className: "exo-cumulative-row" });
        ALL_TYPES.forEach(type => {
            const stats = updated.cumulative.byType[type];
            if (!stats || !stats.total) return;
            row.appendChild(el("span", {
                className: "exo-badge",
                text: `${typeIcon(type)} ${pct(stats.correct, stats.total)}%`,
                attrs: { title: `${typeLabel(type)} — ${stats.correct}/${stats.total} sur ${updated.cumulative.sessionsCount} sessions` },
            }));
        });
        cumLine.appendChild(row);
        summary.appendChild(cumLine);
    }

    if (state.mistakes.length) {
        const list = el("ul", { className: "exo-mistake-list" });
        state.mistakes.forEach(({ ex, userAnswer }) => {
            const label = ex.type === "qcm" ? ex.question : (ex.audio || "");
            list.appendChild(el("li", {
                html: `<strong>${escapeHtml(label)}</strong> → ${escapeHtml(ex.answer)}` +
                      (userAnswer ? ` <span class="exo-your-answer">(toi : ${escapeHtml(userAnswer)})</span>` : ""),
            }));
        });
        summary.appendChild(el("p", { text: "À revoir :" }));
        summary.appendChild(list);
    }

    const actions = el("div", { className: "exo-summary-actions" }, [
        el("button", { className: "exo-next", text: "🔁 Recommencer", onclick: () => startSession() }),
        el("button", { className: "exo-secondary", text: "⬅ Nouvelle sélection", onclick: () => {
            const url = new URL(window.location.href);
            url.searchParams.delete("card");
            url.searchParams.delete("cards");
            url.searchParams.delete("n");
            url.searchParams.delete("types");
            url.searchParams.delete("direction");
            window.location.href = url.toString();
        } }),
    ]);
    summary.appendChild(actions);

    area.appendChild(summary);
}

function escapeHtml(text) {
    const div = document.createElement("div");
    div.textContent = text;
    return div.innerHTML;
}

// ==========================================================================
// Lancement
// ==========================================================================

document.addEventListener("DOMContentLoaded", () => {
    const p = params();
    if (p.get("card") || p.get("cards")) {
        startSession();
    } else {
        renderSelectionScreen();
    }
});
