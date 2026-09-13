// Service worker — généré depuis service-worker.template.js par
// gen_service_worker.py (voir lang.json -> site.storage_prefix /
// site.url_path). Ne pas éditer service-worker.js à la main : modifier
// le gabarit, puis régénérer.
// Stratégie : precache du "noyau" (shell) au moment de l'install,
// puis cache-first + mise à jour en arrière-plan pour tout le reste
// (fiches, images, exercices...) au fur et à mesure de la navigation.
// Ça évite d'avoir à lister ici les 50+ fiches à la main : tout ce que
// tu visites une fois reste ensuite disponible hors-ligne.

const CACHE_VERSION = "__CACHE_VERSION__";
const SCOPE = "__SCOPE__";

// Fichiers indispensables au démarrage de l'app, à adapter si les noms
// ou l'emplacement changent chez toi.
const CORE_ASSETS = [
  SCOPE,
  SCOPE + "index.html",
  SCOPE + "exercises.html",
  SCOPE + "style.css",
  SCOPE + "exercises.css",
  SCOPE + "lang-config.js",
  SCOPE + "app.js",
  SCOPE + "exercises.js",
  SCOPE + "progress.js",
  SCOPE + "manifest.json",
  SCOPE + "icons/icon-192.png",
  SCOPE + "icons/icon-512.png",
];

self.addEventListener("install", (event) => {
  event.waitUntil(
    caches.open(CACHE_VERSION).then((cache) =>
      cache.addAll(CORE_ASSETS).catch((err) => {
        // Si un des fichiers listés n'existe pas encore / a un autre nom,
        // on log l'erreur mais on ne bloque pas l'installation du SW.
        console.warn("[SW] precache partiel :", err);
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

  // On ne gère que le GET, même origine, et uniquement sous /slovak/
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
          // On met à jour le cache silencieusement à chaque visite en ligne
          if (res && res.status === 200) {
            const copy = res.clone();
            caches.open(CACHE_VERSION).then((cache) => cache.put(req, copy));
          }
          return res;
        })
        .catch(() => cached); // hors-ligne : on retombe sur le cache

      // cache-first : réponse immédiate si on l'a déjà, sinon on attend le réseau
      return cached || network;
    })
  );
});
