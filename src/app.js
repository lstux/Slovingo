/*
 * app.js (ex-slovak.js)
 * ======================
 *
 * Fonctions interactives pour les mémos de langue. Générique : toute
 * valeur dépendant de la langue apprise (code TTS, indice de nom de
 * voix, textes d'avertissement) vient de window.LANG_CONFIG, injecté
 * par lang-config.js (généré depuis lang.json — voir gen_lang_config.py).
 * Ce fichier n'a plus besoin d'être modifié d'une langue à l'autre.
 */


// ==========================================================================
// Configuration langue (repli sûr si lang-config.js n'est pas chargé)
// ==========================================================================

const TARGET_LANG = (window.LANG_CONFIG && window.LANG_CONFIG.target_lang) || {
    name: "cible",
    tts_code: "en-US",
    voice_hint: "",
    headers: [],
    header_roots: [],
};

// ==========================================================================
// Variables globales
// ==========================================================================

let currentUtterance = null;
let selectedVoice = null;
let translationsVisible = true;

// Passe à true dès qu'une voix dans la langue cible a été trouvée.
// Tant que c'est false, le TTS est désactivé (pas question de lire
// la langue cible avec l'accent d'une voix de repli quelconque).
let targetVoiceAvailable = false;
let ttsWarningShown = false;

// ==========================================================================
// Synthèse vocale
// ==========================================================================

/*
 * Cherche la meilleure voix disponible pour la langue cible.
 * Priorité :
 * 1. Langue exacte (ex. sk-SK)
 * 2. Toute langue commençant par le même préfixe (ex. "sk")
 * 3. Nom de voix contenant l'indice configuré (voice_hint, ex. "Slovak")
 * 4. Aucune voix spécifique
 */

function findTargetVoice() {
    const voices = window.speechSynthesis.getVoices();
    if (!voices.length) { return null; }

    const ttsCode = (TARGET_LANG.tts_code || "").toLowerCase();
    const ttsPrefix = ttsCode.split("-")[0];

    // Langue exacte
    if (ttsCode) {
        const exactVoice = voices.find(voice => voice.lang.toLowerCase() === ttsCode);
        if (exactVoice) { return exactVoice; }
    }
    // Toute voix du même préfixe de langue
    if (ttsPrefix) {
        const prefixVoice = voices.find(voice => voice.lang.toLowerCase().startsWith(ttsPrefix));
        if (prefixVoice) { return prefixVoice; }
    }
    // Nom de voix contenant l'indice configuré (ex. "Slovak")
    const hint = (TARGET_LANG.voice_hint || "").toLowerCase();
    if (hint) {
        const namedVoice = voices.find(voice => voice.name.toLowerCase().includes(hint));
        if (namedVoice) { return namedVoice; }
    }
    return null;
}


// ==========================================================================
// Avertissement : pas de voix disponible dans la langue cible
// ==========================================================================

/*
 * Affiche un petit toast d'avertissement quand aucune voix de la
 * langue cible n'est disponible sur l'appareil.
 *
 * Ne s'affiche qu'une seule fois par chargement de page,
 * pour ne pas spammer l'utilisateur à chaque clic.
 */

function showTtsWarning() {
    if (ttsWarningShown) { return; }
    ttsWarningShown = true;

    const toast = document.createElement("div");
    toast.textContent =
        `⚠️ Aucune voix ${TARGET_LANG.name} disponible sur cet appareil : ` +
        "la lecture audio est désactivée (plutôt ça qu'un mauvais accent 😅).";

    Object.assign(
        toast.style,
        {
            position: "fixed",
            bottom: "16px",
            left: "50%",
            transform: "translateX(-50%)",
            maxWidth: "90%",
            padding: "10px 16px",
            borderRadius: "8px",
            background: "#c0392b",
            color: "#fff",
            fontSize: "0.9em",
            lineHeight: "1.4",
            textAlign: "center",
            zIndex: "1000",
            boxShadow: "0 2px 8px rgba(0,0,0,0.3)"
        }
    );

    document.body.appendChild(toast);
    setTimeout(() => toast.remove(), 6000);
}


// ==========================================================================
// Lire une phrase
// ==========================================================================

/*
 * Lit une phrase dans la langue cible.
 *
 * text :
 *     Texte à prononcer
 *
 * rate :
 *     0.9 = vitesse normale
 *     0.65 = vitesse lente
 */

function speak(text, rate = 0.9) {
    if (!("speechSynthesis" in window)) {
        alert("La synthèse vocale n'est pas disponible sur cet appareil.");
        return;
    }

    // Pas de voix dans la langue cible : on n'active pas le TTS,
    // on prévient juste une fois.
    if (!targetVoiceAvailable) {
        showTtsWarning();
        return;
    }

    // Arrête une éventuelle lecture précédente
    stopSpeaking();
    // Crée la phrase à prononcer
    currentUtterance = new SpeechSynthesisUtterance(text);
    // Langue cible
    currentUtterance.lang = TARGET_LANG.tts_code;
    // Vitesse
    currentUtterance.rate = rate;
    // Ton
    currentUtterance.pitch = 1.0;
    // Recherche de la voix
    selectedVoice = findTargetVoice();

    if (selectedVoice) {
        currentUtterance.voice = selectedVoice;
    }

    // Lecture
    window.speechSynthesis.speak(currentUtterance);
}


// ==========================================================================
// Arrêter la lecture
// ==========================================================================

function stopSpeaking() {
    if ("speechSynthesis" in window) {
        window.speechSynthesis.cancel();
    }
    currentUtterance = null;
}

// ==========================================================================
// Lire toutes les phrases
// ==========================================================================

/*
 * Lit toutes les phrases audio-card
 * les unes après les autres.
 *
 * Les éléments masqués par une recherche
 * ne sont pas lus.
 */

function speakAll() {
    // Pas de voix dans la langue cible : on n'active pas le TTS,
    // on prévient juste une fois.
    if (!targetVoiceAvailable) {
        showTtsWarning();
        return;
    }

    stopSpeaking();
    const cards = document.querySelectorAll(".audio-text:not(.search-hidden)");
    if (!cards.length) { return; }

    let index = 0;
    function next() {
        // Fin de la liste
        if (index >= cards.length) {
            currentUtterance = null;
            return;
        }

        const card = cards[index];
        // Récupère le texte
        const text = card.innerText;
        // Crée la phrase
        const utterance = new SpeechSynthesisUtterance(text);
        // Langue
        utterance.lang = TARGET_LANG.tts_code;
        // Vitesse normale
        utterance.rate = 0.9;
        // Voix
        const voice = findTargetVoice();

        if (voice) {
            utterance.voice = voice;
        }

        // Une fois la phrase terminée,
        // passe à la suivante
        utterance.onend =
            function() {
                index++;
                // Petite pause entre les phrases
                setTimeout(next, 250);
            };

        currentUtterance = utterance;
        window.speechSynthesis.speak(utterance);
    }
    next();
}


// ==========================================================================
// Afficher / masquer les traductions
// ==========================================================================

/*
 * Masque ou affiche tous les éléments
 * blockquote utilisés comme traductions.
 *
 * Markdown :
 *     > Bonjour, comment allez-vous?
 * devient :
 *     <blockquote>
 *         <p>Bonjour, comment allez-vous?</p>
 *     </blockquote>
 */

function toggleAllTranslations() {
    translationsVisible = !translationsVisible;
    const blockquotes = document.querySelectorAll(".audio-translation");
    blockquotes.forEach(
        blockquote => {
            if (translationsVisible) {
                blockquote.classList.remove("hidden-translation");
            } else {
                blockquote.classList.add("hidden-translation");
            }
        }
    );
}

function toggleTranslation(element) {
    if (element.classList.contains("hidden-translation")) {
        element.classList.remove("hidden-translation");
    }
    else {
        element.classList.add("hidden-translation");
    }
}


// ==========================================================================
// Recherche
// ==========================================================================

/*
 * Filtre les phrases audio en fonction
 * du contenu de la barre de recherche.
 *
 * La recherche porte sur :
 *
 * - Le texte dans la langue cible
 * - La traduction
 */

function searchContent() {
    const searchInput = document.getElementById("search");
    if (!searchInput) {
        return;
    }

    const query = searchInput.value.toLowerCase().trim();
    const cards = document.querySelectorAll(".audio-card");

    cards.forEach(
        card => {
            const text = card.innerText.toLowerCase();
            if (!query || text.includes(query)) {
                card.classList.remove("search-hidden");
            } else {
                card.classList.add("search-hidden");
            }
        }
    );
}

// ==========================================================================
// Initialisation des éléments de classe speakable
// ==========================================================================

/*
 * Ajoute un événément onclick sur tous les éléments
 * de classe "speakable" permettant d'écouter le
 * contenu par synthès vocale
 */

function initializeSpeakableElements() {
    const elements = document.querySelectorAll(".speakable");
    elements.forEach(
        element => {
            element.title = targetVoiceAvailable
                ? "cliquer pour écouter"
                : `Aucune voix ${TARGET_LANG.name} disponible`;
            element.addEventListener(
                "click",
                function() {
                    const text = this.textContent.trim();
                    if (text) {
                        speak(text, 0.9);
                    }
                }
            )
        }
    )
}

// ==========================================================================
// Initialisation des translate-tables
// ==========================================================================

/*
 * Normalise un texte d'en-tête : accents retirés, tout ce qui n'est
 * pas une lettre a-z retiré. Utilisé pour comparer un header aux
 * mots-clés de lang.json indépendamment des accents/majuscules.
 */

function normalizeHeaderText(rawText) {
    return rawText
        .normalize("NFD")
        .replace(/[\u0300-\u036f]/g, "")
        .toLowerCase()
        .replace(/[^a-z]/g, "");
}

/*
 * Vérifie si un texte d'en-tête désigne la colonne "langue cible".
 *
 * Alignée avec la détection de smd2html.py / smd2exercises.py
 * (lang.json -> target_lang.headers / target_lang.header_roots),
 * pour couvrir "Slovenský", "Slovenská", "Slovenské",
 * "slovakština", etc. — ou l'équivalent pour une autre langue.
 */

function isTargetHeader(rawText) {
    const text = normalizeHeaderText(rawText);
    const headers = (TARGET_LANG.headers || []).map(normalizeHeaderText);
    if (headers.includes(text)) { return true; }
    const roots = (TARGET_LANG.header_roots || []).map(normalizeHeaderText);
    return roots.some(root => text.includes(root));
}

/*
 * Ajoute un événément onclick sur tous les éléments
 * des colonnes ayant pour header un des mots-clefs
 */

function initializeTranslateTables() {
    const tables = document.querySelectorAll(".translate-table");
    tables.forEach(table => {
        const firstRow = table.rows[0];
        if (!firstRow) return;
        let column = -1;

        [...firstRow.cells].forEach((cell, index) => {
            if (isTargetHeader(cell.textContent)) { column = index; }
        });
        if (column < 0) return;

        [...table.rows].slice(1).forEach(row => {
            if (row.cells.length <= column) return;
            row.cells[column].classList.add("speakable");
        });
    });
}



// ==========================================================================
// Initialisation des audio-cards
// ==========================================================================

/*
 * Ajoute des boutons pour écouter/écouter lentement/voir la traduction
 */

function initializeAudioCards() {
    const cards = document.querySelectorAll(".audio-card");
    cards.forEach(card => {
        // Rendre le "audio-text" cliquable (écoute)
        const text = card.querySelector(".audio-text");
        if (!text) return;
        text.onclick = () => speak(text.textContent.trim(), 0.9);
        // Cache la traduction
        const translate = card.querySelector(".audio-translation");
        if (translate) translate.classList.add("hidden-translation");

        const buttons = document.createElement("div");
        buttons.className = "audio-buttons";
        /*
        // Ajouter le bouton écoute
        const play = document.createElement("button");
        play.textContent = "🔊";
        play.title = "Ecouter";
        play.onclick = () => speak(text.textContent.trim(), 0.9);
        buttons.appendChild(play);
        // Ajouter le bouton écoute lente
        const slow = document.createElement("button");
        slow.textContent = "🐢";
        slow.title = "Ecouter lentement";
        slow.onclick = () => speak(text.textContent.trim(), 0.65);
        buttons.appendChild(slow);
        */
        if (translate) {
            // Ajouter le bouton afficher la traduction
            const toggle = document.createElement("button");
            toggle.textContent = "👀";
            toggle.title = "Afficher/Masquer la traduction";
            toggle.onclick = () => {
              toggleTranslation(translate);
            }
            buttons.appendChild(toggle);
        }
        card.appendChild(buttons);
    });
}

// ==========================================================================
// Initialisation des voix
// ==========================================================================

/*
 * Sur certains navigateurs, notamment Android,
 * la liste des voix n'est disponible qu'un peu
 * après le chargement de la page.
 *
 * L'événement voiceschanged permet de récupérer
 * la liste dès qu'elle est disponible.
 */

function updateTtsAvailability() {
    selectedVoice = findTargetVoice();
    targetVoiceAvailable = !!selectedVoice;

    // Grise visuellement les éléments TTS quand aucune voix de la
    // langue cible n'est disponible, pour que ce soit clair avant
    // même de cliquer.
    const elements = document.querySelectorAll(".audio-buttons button, .speakable");
    elements.forEach(
        element => {
            if (targetVoiceAvailable) {
                element.style.opacity = "";
                element.style.cursor = "";
                element.title = element.classList.contains("speakable")
                    ? "cliquer pour écouter"
                    : element.title;
            } else {
                element.style.opacity = "0.4";
                element.style.cursor = "not-allowed";
                element.title = `Aucune voix ${TARGET_LANG.name} disponible`;
            }
        }
    );
}

function initializeSpeechSynthesis() {
    if (!("speechSynthesis" in window)) {
        return;
    }
    updateTtsAvailability();
    // Sur certains navigateurs (notamment Android), la liste
    // des voix n'arrive qu'après le chargement initial.
    window.speechSynthesis.addEventListener(
            "voiceschanged",
            updateTtsAvailability
        );
}


// ==========================================================================
// Lancement
// ==========================================================================
document.addEventListener(
    "DOMContentLoaded",
    function() {
        initializeSpeechSynthesis();
        initializeTranslateTables();
        initializeAudioCards();
        initializeSpeakableElements();
        toggleAllTranslations();
        // Les boutons des audio-cards sont créés après le premier
        // appel à updateTtsAvailability() : on la relance pour
        // qu'ils héritent bien de l'état (grisés ou non).
        updateTtsAvailability();
    }
)
