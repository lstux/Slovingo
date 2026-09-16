// Service worker -- generated from service-worker.template.js by
// gen_service_worker.py (see lang.json -> site.storage_prefix /
// site.url_path). Do not edit service-worker.js by hand: edit the
// template, then regenerate.
//
// Strategy: precache the "shell" (everything the app needs to boot,
// including data.json/exercises.json -- see below) on install, then
// cache-first + background refresh for anything else requested while
// navigating (images...).
//
// v2 note: precaching data.json and exercises.json (not just the
// static shell) is a deliberate choice, different from v1's lazy
// per-page caching. Since v2 is a single index.html that fetches all
// content up front, the app is simply unusable offline on a first
// visit unless these two are part of the shell -- there is no
// separate per-sheet HTML page left to fall back on.

const CACHE_VERSION = "__CACHE_VERSION__";
const SCOPE = "__SCOPE__";

// Files required for the app to boot and work fully offline. Update
// this list if filenames or their location change.
const CORE_ASSETS = [
  SCOPE,
  SCOPE + "index.html",
  SCOPE + "app.js",
  SCOPE + "exercises.js",
  SCOPE + "progress.js",
  SCOPE + "style.css",
  SCOPE + "exercises.css",
  SCOPE + "lang.json",
  SCOPE + "data.json",
  SCOPE + "exercises.json",
  SCOPE + "manifest.json",
  SCOPE + "icons/icon-192.png",
  SCOPE + "icons/icon-512.png",
];

self.addEventListener("install", (event) => {
  event.waitUntil(
    caches.open(CACHE_VERSION).then((cache) =>
      cache.addAll(CORE_ASSETS).catch((err) => {
        // If one of the listed files doesn't exist yet / has a
        // different name, log it but don't block SW installation.
        console.warn("[SW] partial precache:", err);
      })
    )
  );
  self.skipWaiting();
});

self.addEventListener("activate", (event) => {
  event.waitUntil(
    caches.keys().then((keys) =>
      Promise.all(
        keys
          .filter((key) => key !== CACHE_VERSION)
          .map((key) => caches.delete(key))
      )
    )
  );
  self.clients.claim();
});

self.addEventListener("fetch", (event) => {
  const req = event.request;

  // Only handle GET, same-origin, and only within this course's scope.
  if (req.method !== "GET" || !req.url.startsWith(self.location.origin)) {
    return;
  }
  if (!new URL(req.url).pathname.startsWith(SCOPE)) {
    return;
  }

  event.respondWith(
    caches.match(req).then((cached) => {
      const network = fetch(req)
        .then((res) => {
          // Silently refresh the cache on every online visit.
          if (res && res.status === 200) {
            const copy = res.clone();
            caches.open(CACHE_VERSION).then((cache) => cache.put(req, copy));
          }
          return res;
        })
        .catch(() => cached); // offline: fall back to the cache

      // cache-first: answer immediately if we already have it, else wait for the network
      return cached || network;
    })
  );
});
