/**
 * notify.js -- optional push notifications (needs the PocketBase sync).
 *
 * Only in builds made with --vapid-public-key (see docs/PocketBase.md).
 * Settings are per device AND per course: one `kv` record
 * "notify.<storage_prefix>.<deviceId>" holding the push subscription and the
 * learner's choices. tools/slovingo_push.py (on the server, every minute)
 * reads those records and sends the notifications; this file never sends any.
 *
 * Needs: pocketbase.js (PBSlovingo, pbText helpers), exercises.js (el),
 * settings.js (renderSettingsScreen), LANG.pocketbase.vapid.
 */

"use strict";

const NOTIFY_STRINGS = {
    en: {
        title: "Notifications", enable: "Notifications on this device",
        reminder: "Reminder when no sheet for", days: "days, at", streak: "Save my streak: reminder at",
        streak_hint: "if nothing done today", training: "Training times", training_hint: "a 5-question review",
        add: "Add a time", remove: "Remove", test: "Send a test (within a minute)", test_sent: "Test requested: it should arrive within a minute.",
        denied: "Notifications are blocked for this site in the browser settings.",
        ios: "On iPhone: add the app to the home screen first (Share → Add to Home Screen).",
        unsupported: "This browser cannot receive notifications.", saved: "Saved.", fail: "Could not save",
    },
    fr: {
        title: "Notifications", enable: "Notifications sur cet appareil",
        reminder: "Rappel si aucune fiche depuis", days: "jours, à", streak: "Sauve ta série : rappel à",
        streak_hint: "si rien n'a été fait aujourd'hui", training: "Heures d'entraînement", training_hint: "une révision de 5 questions",
        add: "Ajouter un horaire", remove: "Supprimer", test: "Envoyer un test (dans la minute)", test_sent: "Test demandé : il devrait arriver dans la minute.",
        denied: "Les notifications sont bloquées pour ce site dans les réglages du navigateur.",
        ios: "Sur iPhone : ajoute d'abord l'app à l'écran d'accueil (Partager → Sur l'écran d'accueil).",
        unsupported: "Ce navigateur ne peut pas recevoir de notifications.", saved: "Enregistré.", fail: "Enregistrement impossible",
    },
    sk: {
        title: "Upozornenia", enable: "Upozornenia na tomto zariadení",
        reminder: "Pripomienka, ak žiadna karta už", days: "dní, o", streak: "Zachráň sériu: pripomienka o",
        streak_hint: "ak dnes ešte nič", training: "Časy tréningu", training_hint: "opakovanie s 5 otázkami",
        add: "Pridať čas", remove: "Odstrániť", test: "Poslať test (do minúty)", test_sent: "Test vyžiadaný: mal by prísť do minúty.",
        denied: "Upozornenia sú pre túto stránku zablokované v nastaveniach prehliadača.",
        ios: "Na iPhone: najprv pridaj aplikáciu na plochu (Zdieľať → Pridať na plochu).",
        unsupported: "Tento prehliadač nemôže prijímať upozornenia.", saved: "Uložené.", fail: "Uloženie zlyhalo",
    },
    de: {
        title: "Benachrichtigungen", enable: "Benachrichtigungen auf diesem Gerät",
        reminder: "Erinnerung, wenn keine Karte seit", days: "Tagen, um", streak: "Serie retten: Erinnerung um",
        streak_hint: "wenn heute noch nichts gemacht", training: "Trainingszeiten", training_hint: "eine Wiederholung mit 5 Fragen",
        add: "Zeit hinzufügen", remove: "Entfernen", test: "Test senden (innerhalb einer Minute)", test_sent: "Test angefordert: sollte innerhalb einer Minute ankommen.",
        denied: "Benachrichtigungen sind für diese Seite in den Browsereinstellungen blockiert.",
        ios: "Auf dem iPhone: die App zuerst zum Home-Bildschirm hinzufügen (Teilen → Zum Home-Bildschirm).",
        unsupported: "Dieser Browser kann keine Benachrichtigungen empfangen.", saved: "Gespeichert.", fail: "Speichern fehlgeschlagen",
    },
};

function ntText(key) {
    const code = (typeof LANG === "object" && LANG && LANG.native_lang && LANG.native_lang.code) || "en";
    return (NOTIFY_STRINGS[code] || NOTIFY_STRINGS.en)[key] || NOTIFY_STRINGS.en[key] || key;
}

const ntVapid = () => (typeof LANG === "object" && LANG && LANG.pocketbase && LANG.pocketbase.vapid) || "";
const ntSupported = () => "serviceWorker" in navigator && "PushManager" in window && "Notification" in window;
const ntLocalKey = () => `${backupStoragePrefix()}-notify`;

/** Random id of this browser, shared by the courses of the origin. */
function ntDeviceId() {
    let id = localStorage.getItem("pbstore:device");
    if (!id) {
        id = Math.random().toString(36).slice(2, 10) + Date.now().toString(36);
        localStorage.setItem("pbstore:device", id);
    }
    return id;
}

const ntRecordKey = () => `notify.${backupStoragePrefix()}.${ntDeviceId()}`;

function ntDefaults() {
    return { enabled: false, reminder: { on: true, days: 3, time: "18:00" }, streak: { on: false, time: "20:00" }, training: [] };
}

function ntLoad() {
    try { return Object.assign(ntDefaults(), JSON.parse(localStorage.getItem(ntLocalKey())) || {}); } catch (e) { return ntDefaults(); }
}

function ntStore(cfg) { localStorage.setItem(ntLocalKey(), JSON.stringify(cfg)); }

function ntKeyBytes(b64) {
    const s = (b64 + "=".repeat((4 - (b64.length % 4)) % 4)).replace(/-/g, "+").replace(/_/g, "/");
    return Uint8Array.from(atob(s), (c) => c.charCodeAt(0));
}

/** The push subscription of this course's service worker (created if needed). */
async function ntSubscription(create) {
    const reg = await navigator.serviceWorker.ready;
    let sub = await reg.pushManager.getSubscription();
    if (!sub && create) sub = await reg.pushManager.subscribe({ userVisibleOnly: true, applicationServerKey: ntKeyBytes(ntVapid()) });
    return sub;
}

/** Send this device's settings to the server (null value when disabled). */
async function ntPush(cfg, extra) {
    const pb = new PBSlovingo();
    if (!pb.connected) return;
    const key = ntRecordKey();
    let value = null;
    if (cfg.enabled) {
        const sub = await ntSubscription(true);
        value = {
            v: 1,
            course: { prefix: backupStoragePrefix(), title: (LANG.site && LANG.site.title) || "Slovingo", url: new URL(LANG.site.url_path || "./", location.href).href },
            lang: (LANG.native_lang && LANG.native_lang.code) || "en",
            tz: Intl.DateTimeFormat().resolvedOptions().timeZone || "UTC",
            subscription: sub.toJSON(),
            reminder: cfg.reminder, streak: cfg.streak, training: cfg.training,
            ...(extra || {}),
        };
    }
    const rec = await pb.remoteGet(key);
    if (!rec && value === null) return;
    await pb.remotePut(key, value, rec);
}

let ntTimer = 0;
function ntSave(cfg, status) {
    ntStore(cfg);
    clearTimeout(ntTimer);
    ntTimer = setTimeout(() => {
        ntPush(cfg).then(() => status && status(ntText("saved"))).catch((e) => status && status(`${ntText("fail")}: ${e.message || e}`, true));
    }, 800);
}

/** Before logging out: stop notifications for this device and course. */
async function pbNotifyOff() {
    const cfg = ntLoad();
    if (!cfg.enabled) return;
    cfg.enabled = false; ntStore(cfg);
    try { await ntPush(cfg); } catch (e) { /* best effort */ }
    try { const sub = await ntSubscription(false); if (sub) await sub.unsubscribe(); } catch (e) { /* ignore */ }
}

/** The subscription can change (browser renewal): re-send it, at most hourly. */
let ntLastCheck = 0;
async function ntRefresh() {
    const cfg = ntLoad();
    if (!cfg.enabled || !ntSupported() || Notification.permission !== "granted" || Date.now() - ntLastCheck < 3600000) return;
    ntLastCheck = Date.now();
    const sub = await ntSubscription(true);
    const json = JSON.stringify(sub.toJSON());
    if (json !== cfg.lastSub) { await ntPush(cfg); cfg.lastSub = json; ntStore(cfg); }
}

/** Weekday names, Monday first (index 0 = Monday, like Python's weekday()). */
function ntWeekdays() {
    const fmt = new Intl.DateTimeFormat((LANG.native_lang && LANG.native_lang.tts_code) || undefined, { weekday: "narrow" });
    return [...Array(7)].map((_, i) => fmt.format(new Date(Date.UTC(2024, 0, 1 + i, 12))));
}

function ntTimeInput(value, onChange) {
    const input = el("input", { attrs: { type: "time", value, step: "300" } });
    input.addEventListener("change", () => { if (input.value) onChange(input.value); });
    return input;
}

/** The "Notifications" part of the sync block (connected accounts only). */
function renderNotifyControls() {
    const box = el("div", { className: "pb-notify" });
    box.appendChild(el("h3", { text: ntText("title") }));
    const msg = el("p", { className: "pb-status" });
    const status = (text, err) => { msg.textContent = text || ""; msg.style.color = err ? "var(--danger, #c0392b)" : ""; };

    if (!ntSupported()) {
        const ios = /iPhone|iPad|iPod/.test(navigator.userAgent) && !window.navigator.standalone;
        box.appendChild(el("p", { text: ntText(ios ? "ios" : "unsupported") }));
        return box;
    }

    const cfg = ntLoad();
    const save = () => ntSave(cfg, status);
    const check = (label, checked, onChange) => {
        const input = el("input", { attrs: { type: "checkbox" } });
        input.checked = checked;
        input.addEventListener("change", () => onChange(input.checked, input));
        const lab = el("label", { className: "pb-line" });
        lab.append(input, document.createTextNode(` ${label}`));
        return lab;
    };

    box.appendChild(check(ntText("enable"), cfg.enabled && Notification.permission === "granted", async (on, input) => {
        if (on) {
            const perm = await Notification.requestPermission();
            if (perm !== "granted") { input.checked = false; status(ntText("denied"), true); return; }
        }
        cfg.enabled = on; ntStore(cfg);
        try {
            await ntPush(cfg);
            if (!on) { const sub = await ntSubscription(false); if (sub) await sub.unsubscribe(); }
        } catch (e) { status(`${ntText("fail")}: ${e.message || e}`, true); }
        renderSettingsScreen();
    }));
    if (Notification.permission === "denied") box.appendChild(el("p", { text: ntText("denied") }));

    if (cfg.enabled && Notification.permission === "granted") {
        // Reminder after N days without a sheet.
        const rem = check(ntText("reminder"), cfg.reminder.on, (on) => { cfg.reminder.on = on; save(); });
        const days = el("input", { attrs: { type: "number", min: "1", max: "30", value: String(cfg.reminder.days) } });
        days.style.width = "4em";
        days.addEventListener("change", () => { cfg.reminder.days = Math.max(1, Math.min(30, parseInt(days.value, 10) || 3)); save(); });
        rem.append(" ", days, ` ${ntText("days")} `, ntTimeInput(cfg.reminder.time, (v) => { cfg.reminder.time = v; save(); }));
        box.appendChild(rem);

        // Streak saver.
        const st = check(ntText("streak"), cfg.streak.on, (on) => { cfg.streak.on = on; save(); });
        st.append(" ", ntTimeInput(cfg.streak.time, (v) => { cfg.streak.time = v; save(); }), ` (${ntText("streak_hint")})`);
        box.appendChild(st);

        // Training times.
        box.appendChild(el("p", { className: "pb-line", text: `${ntText("training")} : ${ntText("training_hint")}` }));
        const names = ntWeekdays();
        cfg.training.forEach((slot, i) => {
            const row = el("div", { className: "pb-slot" });
            row.appendChild(ntTimeInput(slot.time, (v) => { slot.time = v; save(); }));
            names.forEach((name, d) => {
                const b = el("button", { className: "pb-day", text: name, attrs: { type: "button", "aria-pressed": String(slot.days.includes(d)) } });
                b.addEventListener("click", () => {
                    slot.days = slot.days.includes(d) ? slot.days.filter((x) => x !== d) : [...slot.days, d].sort();
                    b.setAttribute("aria-pressed", String(slot.days.includes(d))); save();
                });
                row.appendChild(b);
            });
            const del = el("button", { className: "pb-day", text: "✕", attrs: { type: "button", "aria-label": ntText("remove"), title: ntText("remove") } });
            del.addEventListener("click", () => { cfg.training.splice(i, 1); ntSave(cfg, status); renderSettingsScreen(); });
            row.appendChild(del);
            box.appendChild(row);
        });
        const add = el("button", { className: "settings-backup-btn", text: `+ ${ntText("add")}`, attrs: { type: "button" } });
        add.addEventListener("click", () => { cfg.training.push({ time: "18:00", days: [0, 1, 2, 3, 4] }); ntSave(cfg, status); renderSettingsScreen(); });
        box.appendChild(add);

        const test = el("button", { className: "settings-backup-btn", text: ntText("test"), attrs: { type: "button" } });
        test.addEventListener("click", async () => {
            test.disabled = true;
            try { await ntPush(cfg, { testAt: Date.now() }); status(ntText("test_sent")); } catch (e) { status(`${ntText("fail")}: ${e.message || e}`, true); }
            test.disabled = false;
        });
        box.appendChild(test);
    }
    box.appendChild(msg);
    return box;
}

if (typeof window !== "undefined") {
    document.addEventListener("visibilitychange", () => { if (!document.hidden) ntRefresh().catch((e) => console.warn("notify:", e)); });
    setTimeout(() => { if (typeof LANG === "object" && LANG) ntRefresh().catch((e) => console.warn("notify:", e)); }, 4000);
}
