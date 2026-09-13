/*
 * index.js
 * ========
 *
 * Reporte, sur la page d'accueil, le score moyen des exercices déjà
 * faits pour chaque unité (section du parcours). S'appuie sur
 * progress.js (lecture de la progression, écrite par exercises.js).
 *
 * Ne dépend d'aucune donnée générée par smd2exercises.py : la
 * correspondance fiche -> clé de progression se déduit directement
 * des liens de la page (href="Nom.html" -> source "Nom.md"). Une
 * fiche sans exercices générés contribue simplement zéro donnée à la
 * moyenne de son unité — rien ne casse.
 */

function sectionSources(section) {
    return [...section.querySelectorAll(".index-card")]
        .map(card => card.getAttribute("href"))
        .filter(href => href && href.endsWith(".html"))
        .map(href => href.replace(/\.html$/, ".md"));
}

// Si progress.js n'a pas été chargé, on ne casse pas la page : pas
// de badge affiché, c'est tout (voir le même filet dans exercises.js).
if (typeof averageScore !== "function") {
    console.warn("progress.js manquant : pas de score sur l'index.");
    window.averageScore = () => null;
    window.getFicheProgress = () => null;
    window.scoreRatioClass = () => "bad";
}

// Score de la dernière session, sur la carte de chaque fiche —
// complète le score moyen déjà affiché sur le titre de l'unité.
function decorateCardScores(section) {
    section.querySelectorAll(".index-card").forEach(card => {
        const href = card.getAttribute("href");
        if (!href || !href.endsWith(".html")) return;

        const source = href.replace(/\.html$/, ".md");
        const progress = getFicheProgress(source);
        if (!progress || !progress.last) return;

        const badge = document.createElement("span");
        badge.className = `index-score-badge index-score-${scoreRatioClass(progress.last.score, progress.last.total)}`;
        badge.textContent = `${progress.last.score}/${progress.last.total}`;
        badge.title = `Dernier essai : ${new Date(progress.last.date).toLocaleDateString()}`;
        card.appendChild(badge);
    });
}

function renderIndexScores() {
    document.querySelectorAll(".index-section").forEach(section => {
        decorateCardScores(section);

        const heading = section.querySelector("h2");
        if (!heading) return;

        const sources = sectionSources(section);
        const score = averageScore(sources);
        if (!score) return;

        const plural = score.count > 1 ? "s" : "";
        const badge = document.createElement("span");
        badge.className = `index-score-badge index-score-${scoreRatioClass(score.avg, 100)}`;
        badge.textContent = `${score.avg}%`;
        badge.title = `Moyenne de l'unité : ${score.avg}% (${score.count}/${sources.length} fiche${plural} pratiquée${plural})`;
        heading.appendChild(badge);
    });
}

document.addEventListener("DOMContentLoaded", renderIndexScores);
