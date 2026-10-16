// Service worker de la page d'accueil Slovingo.
//
// Il ne gère QUE la coquille de la page d'accueil (index.html, courses.json,
// manifest, icônes). Tout le reste -- en particulier les cours, publiés dans
// des sous-dossiers de ce même scope -- est laissé au réseau et aux service
// workers propres des cours (scope plus précis, donc prioritaires).
//
// Cache : nom propre ("slovingo-launcher-vN"), et le nettoyage ne touche que
// les anciennes versions de CE cache, jamais ceux des cours.

const CACHE = "slovingo-launcher-v1";
const CACHE_PREFIX = "slovingo-launcher-";

const SHELL = ["./", "index.html", "courses.json", "manifest.json", "icons/icon-192.png", "icons/icon-512.png"];
const SHELL_PATHS = new Set(SHELL.map((p) => new URL(p, self.registration.scope).pathname));

self.addEventListener("install", (event) => {
  event.waitUntil(
    caches
      .open(CACHE)
      .then((cache) => cache.addAll(SHELL.map((p) => new URL(p, self.registration.scope).href)))
      .catch((err) => console.warn("[launcher SW] partial precache:", err))
  );
  self.skipWaiting();
});

self.addEventListener("activate", (event) => {
  event.waitUntil(
    caches
      .keys()
      .then((keys) =>
        Promise.all(keys.filter((k) => k.startsWith(CACHE_PREFIX) && k !== CACHE).map((k) => caches.delete(k)))
      )
      .then(() => self.clients.claim())
  );
});

self.addEventListener("fetch", (event) => {
  const req = event.request;
  if (req.method !== "GET") return;
  const url = new URL(req.url);
  if (url.origin !== self.location.origin || !SHELL_PATHS.has(url.pathname)) return;

  // Stale-while-revalidate. La clé de cache ignore la query (?choose).
  const key = url.origin + url.pathname;
  event.respondWith(
    caches.open(CACHE).then((cache) =>
      cache.match(key).then((cached) => {
        const network = fetch(req)
          .then((res) => {
            if (res && res.status === 200) cache.put(key, res.clone());
            return res;
          })
          .catch(() => cached);
        return cached || network;
      })
    )
  );
});
