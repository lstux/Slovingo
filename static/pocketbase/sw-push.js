/**
 * sw-push.js -- push notifications for the service worker (imported by
 * service-worker.js only in builds made with --vapid-public-key).
 * The payload comes from tools/slovingo_push.py: { title, body, url, tag }.
 */
self.addEventListener("push", (event) => {
  let data = {};
  try { data = event.data ? event.data.json() : {}; } catch (e) { data = { body: event.data && event.data.text() }; }
  const scope = self.registration.scope;
  event.waitUntil(self.registration.showNotification(data.title || "Slovingo", {
    body: data.body || "",
    tag: data.tag || "slovingo",
    icon: scope + "icons/icon-192.png",
    badge: scope + "icons/icon-192.png",
    data: { url: data.url || scope },
  }));
});

self.addEventListener("notificationclick", (event) => {
  event.notification.close();
  const url = (event.notification.data && event.notification.data.url) || self.registration.scope;
  event.waitUntil(self.clients.matchAll({ type: "window", includeUncontrolled: true }).then((wins) => {
    const win = wins.find((w) => w.url.startsWith(self.registration.scope));
    if (win) return win.navigate(url).then((w) => (w || win).focus());
    return self.clients.openWindow(url);
  }));
});
