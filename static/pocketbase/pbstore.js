// pbstore.js : synchronisation optionnelle du localStorage vers PocketBase (https://store.lslinux.org).
// Sans dependance. localStorage reste la source de verite locale (l'app marche hors ligne,
// sans compte, sans reseau) ; PocketBase sert a retrouver les memes donnees sur les autres appareils.
//
//   <script src="pbstore.js"></script>
//   const store = new PBStore({ app: "slovingo", sharedAuth: true });   // sharedAuth : meme connexion pour toutes les apps de l'origine          // app : [a-z0-9][a-z0-9_-]*, <= 64 car.
//   await store.login("https://store.lslinux.org", email, motDePasse);   // ecran de reglages
//   store.set("progress", { level: 3 });                      // ecrit en local, pousse en arriere-plan
//   store.get("progress");                                     // lecture locale, synchrone
//   await store.sync();                                        // au demarrage de l'app et quand "online"
//   store.onChange = (key, value) => { ... };                  // une valeur a ete remplacee par la version distante
//
// Regle de fusion : la derniere ecriture gagne, par cle (horodatage local contre `updated` serveur).
// Une suppression est une ligne dont value vaut null (pierre tombale) : `set(key, null)` equivaut a `remove(key)`.
// Les horloges des appareils doivent etre a peu pres justes (le serveur donne `updated`, le client `Date.now()`).
// Le mot de passe n'est jamais stocke, seulement le jeton (localStorage, cle "pbstore:<app>:auth").
//
// Version « script classique » pour les pages de ce dépôt (comme common.js) : ouverte en file://, servie en http(s)
// ou insérée par `build.py --standalone`, elle définit window.PBStore. Le code de la classe est celui du module ES
// d'origine, seul le conteneur change : <script src="pbstore.js"></script>, puis new PBStore({ app: "...", prefix: "..." }).

(function (global) {
  "use strict";
  const DEFAULT_URL = "https://store.lslinux.org";

  class PBStore {
    constructor({ app, prefix = "", sharedAuth = false, storage = globalThis.localStorage, fetchImpl = globalThis.fetch.bind(globalThis) }) {
      if (!/^[a-z0-9][a-z0-9_-]{0,63}$/.test(app)) throw new Error("pbstore : nom d'app invalide");
      this.app = app;
      this.prefix = prefix;          // prefixe des cles localStorage de l'app (ex. "slovingo.")
      this.ls = storage;
      this.fetch = fetchImpl;
      this.onChange = null;
      this.onError = null;           // (erreur) => void ; sinon console.warn
      this._timers = new Map();
      // sharedAuth : la connexion est gardee sous une cle commune a toutes les apps de la meme origine
      // (« pbstore:auth ») : un seul login pour toutes. Une connexion deja faite sous la cle de l'app est reprise.
      this._shared = sharedAuth;
      this._authKey = sharedAuth ? "pbstore:auth" : this._k("auth");
      this._auth = this._readJSON(this._authKey);       // { url, token, id, email }
      if (!this._auth && sharedAuth) {
        this._auth = this._readJSON(this._k("auth"));
        if (this._auth) { this._writeJSON(this._authKey, this._auth); this.ls.removeItem(this._k("auth")); }
      }
    }

    // ---- etat -------------------------------------------------------------------------------
    get connected() { return !!(this._auth && this._auth.token); }
    get email() { return this._auth ? this._auth.email : null; }
    get url() { return this._auth ? this._auth.url : DEFAULT_URL; }

    // ---- lecture / ecriture locales (synchrones) ---------------------------------------------
    get(key) {
      const raw = this.ls.getItem(this.prefix + key);
      if (raw === null) return null;
      try { return JSON.parse(raw); } catch { return raw; }
    }

    set(key, value) {
      if (value === null || value === undefined) return this.remove(key);
      this.ls.setItem(this.prefix + key, typeof value === "string" ? value : JSON.stringify(value));
      this._touch(key);
      this._schedulePush(key);
    }

    remove(key) {
      this.ls.removeItem(this.prefix + key);
      this._touch(key);
      this._schedulePush(key);       // pousse une suppression (pierre tombale locale)
    }

    // ---- compte -------------------------------------------------------------------------------
    async login(url, email, password) {
      url = String(url || DEFAULT_URL).replace(/\/+$/, "");
      const r = await this._raw(url, "POST", "/api/collections/users/auth-with-password", null, { identity: email, password });
      this._auth = { url, token: r.token, id: r.record.id, email: r.record.email || email };
      this._writeJSON(this._authKey, this._auth);
      return this._auth.email;
    }

    logout() {
      this._auth = null;
      this.ls.removeItem(this._authKey);
    }

    // Declare des cles deja presentes dans localStorage avant l'activation de la synchro (donnees d'avant PocketBase).
    // Elles sont poussees si le serveur ne les a pas, mais la version du serveur gagne en cas de conflit.
    adopt(keys) {
      const all = this._meta();
      for (const k of keys) if (!(k in all) && this.ls.getItem(this.prefix + k) !== null) all[k] = { t: 1 };
      this._writeJSON(this._k("meta"), all);
    }

    // ---- synchronisation ------------------------------------------------------------------------
    // Pousse les cles locales plus recentes, rapatrie les cles distantes plus recentes. Renvoie { pushed, pulled }.
    async sync() {
      if (this._shared) this._auth = this._readJSON(this._authKey);   // connexion/deconnexion faite dans une autre app
      if (!this.connected) return { pushed: 0, pulled: 0 };
      await this._refresh();
      const remote = new Map();
      for (let page = 1; ; page++) {
        const q = `/api/collections/kv/records?perPage=200&page=${page}&filter=${encodeURIComponent(`app='${this.app}'`)}`;
        const r = await this._api("GET", q);
        for (const it of r.items) remote.set(it.key, it);
        if (page >= r.totalPages) break;
      }
      const meta = this._meta();
      let pushed = 0, pulled = 0;
      const keys = new Set([...remote.keys(), ...Object.keys(meta)]);
      for (const key of keys) {
        const rec = remote.get(key);
        const lt = meta[key] ? meta[key].t : 0;                  // derniere ecriture locale (ms)
        const rt = rec ? Date.parse(rec.updated.replace(" ", "T")) : 0;
        if (rec && rt > lt) {                                     // distant plus recent (ou cle inconnue localement)
          if (rec.value === null) {                               // pierre tombale : supprime sur un autre appareil
            const had = this.ls.getItem(this.prefix + key) !== null;
            this.ls.removeItem(this.prefix + key);
            this._setMeta(key, { t: rt, del: true });
            if (had) { pulled++; if (this.onChange) this.onChange(key, null); }
          } else {
            this.ls.setItem(this.prefix + key, typeof rec.value === "string" ? rec.value : JSON.stringify(rec.value));
            this._setMeta(key, { t: rt });
            pulled++;
            if (this.onChange) this.onChange(key, rec.value);
          }
        } else if (lt > rt) {                                     // local plus recent
          await this._push(key, rec);
          pushed++;
        }
      }
      return { pushed, pulled };
    }

    // ---- interne : poussee ----------------------------------------------------------------------
    _schedulePush(key) {
      if (!this.connected) return;
      clearTimeout(this._timers.get(key));
      this._timers.set(key, setTimeout(() => {
        this._timers.delete(key);
        this._push(key).catch((e) => this._err(e));
      }, 1500));                     // regroupe les ecritures rapprochees
    }

    async _push(key, known) {
      if (!this.connected) return;
      const raw = this.ls.getItem(this.prefix + key);
      let rec = known;
      if (rec === undefined) rec = await this._find(key);
      if (raw === null) {                                         // suppression : la ligne reste, value = null (pierre tombale)
        if (!rec) { this._clearMeta(key); return; }               // jamais envoyee : rien a propager
        const saved = await this._api("PATCH", `/api/collections/kv/records/${rec.id}`, { value: null });
        this._setMeta(key, { t: Date.parse(saved.updated.replace(" ", "T")), del: true });
        return;
      }
      let value; try { value = JSON.parse(raw); } catch { value = raw; }
      const body = { value };
      let saved;
      if (rec) saved = await this._api("PATCH", `/api/collections/kv/records/${rec.id}`, body);
      else {
        try { saved = await this._api("POST", "/api/collections/kv/records", { owner: this._auth.id, app: this.app, key, value }); }
        catch (e) {                                               // cree entre-temps par un autre appareil : on met a jour
          if (e.status !== 400) throw e;
          const again = await this._find(key);
          if (!again) throw e;
          saved = await this._api("PATCH", `/api/collections/kv/records/${again.id}`, body);
        }
      }
      this._setMeta(key, { t: Date.parse(saved.updated.replace(" ", "T")) });
    }

    async _find(key) {
      const f = encodeURIComponent(`app='${this.app}' && key='${String(key).replace(/\\/g, "\\\\").replace(/'/g, "\\'")}'`);
      const r = await this._api("GET", `/api/collections/kv/records?perPage=1&filter=${f}`);
      return r.items[0] || null;
    }

    async _refresh() {
      try {
        const r = await this._api("POST", "/api/collections/users/auth-refresh");
        this._auth.token = r.token;
        this._writeJSON(this._authKey, this._auth);
      } catch (e) {
        if (e.status === 401 || e.status === 403) { this.logout(); }   // jeton expire : se reconnecter
        throw e;
      }
    }

    // ---- interne : HTTP -------------------------------------------------------------------------
    async _api(method, path, body) { return this._raw(this._auth.url, method, path, this._auth.token, body); }

    async _raw(url, method, path, token, body) {
      const res = await this.fetch(url + path, {
        method,
        headers: { ...(token ? { Authorization: token } : {}), ...(body ? { "Content-Type": "application/json" } : {}) },
        body: body ? JSON.stringify(body) : undefined,
      });
      if (res.status === 204) return null;
      const data = await res.json().catch(() => ({}));
      if (!res.ok) { const e = new Error(data.message || `HTTP ${res.status}`); e.status = res.status; throw e; }
      return data;
    }

    // ---- interne : metadonnees locales (horodatage par cle) ------------------------------------
    _k(s) { return `pbstore:${this.app}:${s}`; }
    _meta() { return this._readJSON(this._k("meta")) || {}; }
    _setMeta(key, m) { const all = this._meta(); all[key] = m; this._writeJSON(this._k("meta"), all); }
    _clearMeta(key) { const all = this._meta(); delete all[key]; this._writeJSON(this._k("meta"), all); }
    _touch(key) { this._setMeta(key, { t: Date.now() }); }
    _readJSON(k) { try { return JSON.parse(this.ls.getItem(k)); } catch { return null; } }
    _writeJSON(k, v) { try { this.ls.setItem(k, JSON.stringify(v)); } catch (e) { this._err(e); } }
    _err(e) { if (this.onError) this.onError(e); else console.warn("pbstore :", e.message || e); }
  }

  global.PBStore = PBStore;
  if (typeof module !== "undefined" && module.exports) module.exports = { PBStore };
})(typeof window !== "undefined" ? window : globalThis);
