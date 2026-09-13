/*
 * progress.js
 * ===========
 *
 * Lecture de la progression des exercices (localStorage), partagée
 * entre la page d'accueil (badges de score par unité) et
 * exercises.js (écran de sélection, session).
 *
 * Ce fichier ne fait que LIRE le stockage : c'est exercises.js qui
 * l'écrit (recordSession(), au moment où une session se termine).
 *
 * Chargé par index.html ET exercises.html — toute page qui affiche
 * un score doit l'inclure avant son propre script.
 */

// Préfixe configurable (lang.json -> site.storage_prefix), avec repli
// sur "slovak" si lang-config.js n'est pas chargé sur cette page —
// comportement strictement identique à avant tant que la page ne
// charge pas encore lang-config.js (voir publish.sh, étape 3).
const STORAGE_PREFIX =
    (window.LANG_CONFIG && window.LANG_CONFIG.site && window.LANG_CONFIG.site.storage_prefix)
        || "slovak";
const PROGRESS_KEY = `${STORAGE_PREFIX}-exercises-progress`;

function loadProgressStore() {
    try {
        return JSON.parse(localStorage.getItem(PROGRESS_KEY)) || {};
    } catch (err) {
        return {};
    }
}

function getFicheProgress(source) {
    return loadProgressStore()[source] || null;
}

function pct(correct, total) {
    return total ? Math.round((correct / total) * 100) : 0;
}

/*
 * "good" / "mid" / "bad" selon le ratio correct/total — à préfixer
 * selon le contexte d'affichage (voir exo-score-* dans exercises.css,
 * index-score-* dans style.css).
 */
function scoreRatioClass(correct, total) {
    const ratio = total ? correct / total : 0;
    if (ratio >= 0.8) return "good";
    if (ratio >= 0.5) return "mid";
    return "bad";
}

/*
 * Score moyen d'un ensemble de fiches, à partir de leur nom de
 * fichier source (ex. "Serie_01_Rodina_01_moja-rodina.md").
 *
 * C'est la MOYENNE DES POURCENTAGES DE TOUTES LES FICHES DE L'UNITÉ,
 * pratiquées ou non — une fiche jamais pratiquée compte pour 0, elle
 * n'est pas simplement ignorée. Sur une unité de 5 fiches où une
 * seule a été faite avec 10/10, la moyenne est donc 20% (10+0+0+0+0
 * sur 5), pas 100% : elle reflète la progression sur l'unité entière,
 * pas seulement la qualité de ce qui a été tenté.
 *
 * Seul le DERNIER essai de chaque fiche pratiquée compte.
 * Retourne null si AUCUNE fiche de l'ensemble n'a jamais été
 * pratiquée (pas de badge à afficher dans ce cas — un 0% partout
 * n'apporterait rien).
 */
function averageScore(sources) {
    let sum = 0;
    let practicedCount = 0;
    sources.forEach(source => {
        const progress = getFicheProgress(source);
        if (progress && progress.last) {
            sum += pct(progress.last.score, progress.last.total);
            practicedCount += 1;
        }
        // Fiche jamais pratiquée : contribue 0 à la somme, sans
        // incrémenter practicedCount (utilisé seulement pour décider
        // s'il y a quoi que ce soit à afficher).
    });
    if (!practicedCount) return null;
    const avg = Math.round(sum / sources.length);
    return { avg, count: practicedCount };
}
