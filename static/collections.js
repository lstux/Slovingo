// collections.js -- "collection" index pages: a grid of cards (the sheet's
// illustration in a square thumbnail + its title) for a set of sheets that
// is not part of the story path: the vocabulary sheets, the Kronika series,
// conjugation sheets later...
//
// Collections are detected automatically from data.json (see
// detectCollections()); lang.json's optional `collections` key only renames
// or hides them. Loaded after app.js and progress.js: it uses their globals
// (DATA, LANG, getSheetProgress, splitTitle, renderSheetCard's helpers...).

// Categories that are the course path (or its frame), never a collection.
const COLLECTION_EXCLUDED_CATEGORIES = ["introduction", "series", "annex", "credits"];

let collectionsCache = null;
let collectionsCacheFor = null;

function capitalizeFirst(text) {
    return text ? text.charAt(0).toUpperCase() + text.slice(1) : text;
}

/**
 * Work out the course's collections from DATA.groups.
 *
 * Two rules:
 *   - a category other than introduction / series / annex / credits that
 *     holds at least 2 sheets (vocabulary, dialog, conjugation...);
 *   - a subgroup-name prefix shared by at least 2 subgroups ("kronika_stur",
 *     "kronika_pionieri"... -> "kronika"), whatever their category.
 *
 * lang.json may carry a `collections` key:
 *   false                                 no collection pages at all;
 *   { "<id>": "Label" }                   rename a collection;
 *   { "<id>": { "label": "L", "hide": true } }   rename and/or hide it.
 * The id is the category ("vocabulary") or the prefix ("kronika").
 * Without an override the label is lang.json's `categories[<category>]`
 * (or `subgroups[<prefix>]`), else the key with a capital.
 *
 * @returns {{id: string, label: string, entries: {sheet: object, group: object}[]}[]}
 */
function detectCollections() {
    if (!DATA || !Array.isArray(DATA.groups)) {
        return [];
    }
    if (collectionsCache && collectionsCacheFor === DATA) {
        return collectionsCache;
    }
    const config = LANG ? LANG.collections : undefined;
    if (config === false) {
        collectionsCache = [];
        collectionsCacheFor = DATA;
        return collectionsCache;
    }
    const overrides = config && typeof config === "object" ? config : {};

    const byCategory = new Map();
    const byPrefix = new Map();
    const touch = (map, key, firstIndex) => {
        if (!map.has(key)) {
            map.set(key, { key, firstIndex, subgroups: new Set(), entries: [] });
        }
        return map.get(key);
    };

    DATA.groups.forEach((group, index) => {
        if (!COLLECTION_EXCLUDED_CATEGORIES.includes(group.category)) {
            const item = touch(byCategory, group.category, index);
            group.sheets.forEach((sheet) => item.entries.push({ sheet, group }));
            item.subgroups.add(group.subgroup || "");
        }
        const prefix = group.subgroup && group.subgroup.includes("_") ? group.subgroup.split("_")[0] : null;
        if (prefix) {
            const item = touch(byPrefix, prefix, index);
            group.sheets.forEach((sheet) => item.entries.push({ sheet, group }));
            item.subgroups.add(group.subgroup);
        }
    });

    const found = [];
    byCategory.forEach((item) => {
        if (item.entries.length >= 2) {
            const label = (LANG.categories && LANG.categories[item.key]) || capitalizeFirst(item.key);
            found.push({ id: item.key, key: item.key, label, firstIndex: item.firstIndex, entries: item.entries });
        }
    });
    byPrefix.forEach((item) => {
        if (item.subgroups.size >= 2) {
            // A prefix equal to a category id would clash in the URL.
            const id = byCategory.has(item.key) ? `${item.key}-2` : item.key;
            const label = (LANG.subgroups && LANG.subgroups[item.key]) || capitalizeFirst(item.key);
            found.push({ id, key: item.key, label, firstIndex: item.firstIndex, entries: item.entries });
        }
    });

    collectionsCache = found
        .map((collection) => {
            const override = overrides[collection.id];
            if (override === false || (override && override.hide === true)) {
                return null;
            }
            if (typeof override === "string") {
                collection.label = override;
            } else if (override && typeof override.label === "string") {
                collection.label = override.label;
            }
            return collection;
        })
        .filter(Boolean)
        .sort((a, b) => a.firstIndex - b.firstIndex);
    collectionsCacheFor = DATA;
    return collectionsCache;
}

/**
 * True when a collection page already stands for a map "resource" link
 * (the links under the map that scroll to a category or subgroup of the
 * sheet list): the home screen then shows the collection's button only.
 * @param {{id: string, subgroup?: string}} resource A map.json resource.
 */
function collectionCoversResource(resource) {
    return detectCollections().some((collection) =>
        resource.subgroup
            ? collection.entries.some((e) => e.group.category === "series" && e.group.subgroup === resource.subgroup
                || e.group.subgroup === resource.subgroup && e.group.category === resource.id)
            : collection.id === resource.id && collection.entries.every((e) => e.group.category === resource.id)
    );
}

/**
 * Home view: one button per collection, under the map and the story link.
 * @param {HTMLElement} container The home view's content element.
 */
function renderCollectionLinks(container) {
    const collections = detectCollections();
    if (!collections.length) {
        return;
    }
    const nav = document.createElement("nav");
    nav.className = "collection-links";
    collections.forEach((collection) => {
        const link = document.createElement("a");
        link.className = "collection-link";
        link.href = `#/collection/${encodeURIComponent(collection.id)}`;
        link.textContent = collection.label;
        nav.appendChild(link);
    });
    container.appendChild(nav);
}

/**
 * One collection card: the illustration as a square thumbnail (the
 * generated thumbs/ file when the build had Pillow, else the full image
 * cropped square by CSS, else a themed placeholder), the title, and the
 * last exercise score like on the home cards.
 */
function renderCollectionCard(entry) {
    const { sheet, group } = entry;
    const card = document.createElement("a");
    card.className = "collection-card";
    card.href = `#/sheet/${encodeURIComponent(sheet.id)}`;
    card.dataset.theme = themeForGroup(group);

    const frame = document.createElement("span");
    frame.className = "collection-thumb";
    const source = sheet.thumb || (sheet.image && sheet.image.src);
    if (source) {
        const img = document.createElement("img");
        img.src = source;
        img.alt = "";
        img.loading = "lazy";
        img.decoding = "async";
        // The page-wide image lightbox must not hijack a click that
        // is meant to open the sheet.
        img.dataset.lightboxEnabled = "true";
        img.style.cursor = "inherit";
        frame.appendChild(img);
        if (/\.svg(\?|$)/i.test(source)) {
            // A logo or a map: show it whole instead of cropping it.
            frame.classList.add("collection-thumb-vector");
        }
    } else {
        frame.classList.add("collection-thumb-empty");
        frame.textContent = "📖";
    }
    card.appendChild(frame);

    const title = document.createElement("span");
    title.className = "collection-card-title";
    title.textContent = splitTitle(sheet.title).title;
    card.appendChild(title);

    const progress = getSheetProgress(sheet.id);
    if (progress && progress.last) {
        const badge = document.createElement("span");
        badge.className = `index-score-badge index-score-${scoreRatioClass(progress.last.score, progress.last.total)}`;
        badge.textContent = `${progress.last.score}/${progress.last.total}`;
        badge.title = new Date(progress.last.date).toLocaleDateString();
        frame.appendChild(badge);
    }
    return card;
}

/**
 * Heading of one sub-series section: the subgroup's label, minus the
 * collection's own name when it starts with it ("Kronika : Pionieri" under
 * the "Kronika" page -> "Pionieri").
 */
function collectionSectionTitle(collection, group) {
    const full = subgroupLabelFor(group) || group.subgroup || "";
    const names = [collection.label, collection.key].filter(Boolean)
        .map((name) => name.replace(/[.*+?^${}()|[\]\\]/g, "\\$&"));
    const stripped = full.replace(new RegExp(`^(?:${names.join("|")})\\s*[:\\-\u2013\u2014]\\s*`, "i"), "").trim();
    return stripped || full;
}

/** Render `#/collection/<id>`; an unknown id goes back to the home screen. */
function renderCollection(collectionId) {
    const collection = detectCollections().find((c) => c.id === collectionId);
    if (!collection) {
        window.location.hash = "#/";
        return;
    }
    applyTheme(DEFAULT_THEME);
    setKicker("");
    setPageTitle(collection.label);
    updateToolbarForSheetView(false);
    setExercisesButtonMode("selection");
    hidePagerNav();

    const content = document.getElementById("content");
    content.innerHTML = "";

    const heading = document.createElement("h2");
    heading.className = "collection-heading";
    heading.textContent = collection.label;
    content.appendChild(heading);

    // Sheets from several subgroups (the Kronika sub-series): one titled
    // section per subgroup, all open, on this same page. A single group
    // (vocabulary) stays one flat grid with no section titles.
    const sections = [];
    collection.entries.forEach((entry) => {
        const last = sections[sections.length - 1];
        if (last && last.group.subgroup === entry.group.subgroup) {
            last.entries.push(entry);
        } else {
            sections.push({ group: entry.group, entries: [entry] });
        }
    });
    const titled = new Set(sections.map((s) => s.group.subgroup || "")).size > 1;

    sections.forEach((section) => {
        if (titled) {
            const title = document.createElement("h3");
            title.className = "collection-section-title";
            title.textContent = collectionSectionTitle(collection, section.group);
            content.appendChild(title);
        }
        const grid = document.createElement("div");
        grid.className = "collection-grid";
        section.entries.forEach((entry) => grid.appendChild(renderCollectionCard(entry)));
        content.appendChild(grid);
    });
}
