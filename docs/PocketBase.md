# Synchronisation PocketBase (optionnelle)

Retrouver sa progression sur tous ses appareils, via un serveur
[PocketBase](https://pocketbase.io) personnel. **Désactivée par défaut** : un
build normal (et donc un déploiement GitHub Pages) ne contient rien de tout
ça, ne parle à aucun serveur et ne propose aucun compte.

## L'activer au build

```bash
python3 src/build.py   --langs-root langs --static-dir static --pocketbase
python3 src/publish.py --langs-root langs --execute --pocketbase-url https://store.example.org
```

- `--pocketbase` : inclut la synchro.
- `--pocketbase-url URL` : inclut la synchro **et** pré-remplit l'adresse du serveur
  dans le formulaire (écrite dans le `lang.json` de `dist/`, clé `pocketbase.url`).
  Sans elle, le formulaire propose `https://store.lslinux.org`.
- `src/build_site.py` (bundle de plusieurs cours sous un même site) accepte les mêmes deux options et les transmet à chaque cours : ils partagent alors une origine, donc une seule connexion.
- Un build sans l'option supprime `dist/pocketbase/` : rien ne reste d'un build précédent.

Ce que fait l'option : copie `static/pocketbase/` dans `dist/`, ajoute les deux
`<script>` après `backup.js` dans `dist/index.html` (le `static/index.html` du
dépôt n'est pas modifié), précharge les fichiers dans le service worker.

## Côté serveur

Même serveur et même collection `kv` que les [WebApps](https://github.com/lstux/WebApps/blob/main/docs/SYNCHRO.md)
(champs `owner`, `app`, `key`, `value` ; comptes créés par l'administrateur). Slovingo y écrit
avec `app = "slovingo"`, une ligne par cours : `key = "backup.<storage_prefix>"`.
CORS du serveur : doit autoriser l'origine du site.

## Côté élève

Réglages → Données → « Synchronisation (PocketBase) » : email + mot de passe. Un seul
compte pour tous les cours du même site (connexion partagée, aussi avec les WebApps
si elles sont sur la même origine). Synchro au démarrage, au retour en ligne, en
quittant/revenant sur l'onglet, et à la demande.

## Ce qui est synchronisé

| Synchronisé | Non synchronisé |
|---|---|
| Résultats d'exercices, fiches visitées, jours de visite (série) | Réglages (voix, vitesses, thème…) : propres à chaque appareil |
| Le prénom/pseudo, **seulement pour remplir un champ vide** (jamais d'écrasement) | Questions ratées à revoir (une fusion ferait revenir celles déjà réussies), dernière fiche ouverte |

Les données sont stockées **sans chiffrement** sur le serveur.

## Fonctionnement

Pas de « dernier écrit gagne » : chaque synchro télécharge la copie du serveur, la
**fusionne** à la progression locale avec la logique de `backup.js` (meilleurs scores
gardés, rien n'est jamais baissé), puis renvoie le résultat si différent. Deux
appareils ne peuvent donc pas s'effacer mutuellement. Si la progression locale change
à cause du serveur, un bandeau propose de recharger la page. Le serveur limite les
authentifications : la synchro automatique est espacée d'au moins 20 s.

Fichiers : `static/pocketbase/pbstore.js` (client PocketBase, partagé avec les WebApps)
et `static/pocketbase/pocketbase.js` (fusion, réglages, textes fr/en/sk/de).

## Notifications (optionnelles aussi)

Rappels et entraînements envoyés sur le téléphone, **uniquement pour les élèves connectés à la synchro**.
Il faut une option de build en plus et un petit service sur le serveur.

### Dans l'app

Réglages → Données → bloc de synchro, une fois connecté : « Notifications sur cet appareil », puis

- **Rappel** si aucune fiche depuis N jours, à une heure choisie (au plus une fois tous les N jours) ;
- **Sauve ta série** : rappel à une heure choisie si la série est en cours et que rien n'a été fait aujourd'hui ;
- **Heures d'entraînement** : horaires + jours de la semaine ; la notification ouvre une révision de 5 questions ;
- **Envoyer un test** : arrive dans la minute.

Les réglages sont **propres à chaque appareil et à chaque cours** (pas de doublons entre téléphone et tablette).
Sur iPhone (iOS 16.4+), l'app doit d'abord être ajoutée à l'écran d'accueil. Se déconnecter coupe les
notifications de l'appareil.

### Mise en place

1. **Clés VAPID**, une fois (ne plus jamais les changer : les abonnements en dépendent) :
   ```bash
   python3 -m venv /opt/slovingo-push/venv && /opt/slovingo-push/venv/bin/pip install pywebpush
   cp tools/slovingo_push.py /opt/slovingo-push/
   /opt/slovingo-push/venv/bin/python /opt/slovingo-push/slovingo_push.py genkeys --out /etc/slovingo-push/vapid.pem
   ```
   La commande affiche la **clé publique**.
2. **Build** avec cette clé (elle implique `--pocketbase`) :
   ```bash
   python3 src/publish.py --langs-root langs --execute --pocketbase-url https://store.example.org --vapid-public-key <clé publique>
   ```
   (même option pour `build.py` et `build_site.py`).
3. **Service d'envoi**, toutes les minutes : `tools/slovingo-push/env.example` → `/etc/slovingo-push/env`
   (URL PocketBase, compte **superuser**, chemin de la clé privée, `mailto:` de contact), puis
   `slovingo-push.service` et `slovingo-push.timer` dans `/etc/systemd/system/`, un utilisateur système
   `slovingo-push` propriétaire de `/etc/slovingo-push/`, et `systemctl enable --now slovingo-push.timer`.
   Essai sans rien envoyer : `slovingo_push.py run --dry-run`.

Rien à changer dans PocketBase (mêmes lignes `kv` : `notify.<cours>.<appareil>`) ni dans nginx ; le serveur doit
pouvoir joindre les services de push (Google, Mozilla, Apple). Les abonnements expirés (404/410) sont effacés
automatiquement. Une notification en retard de moins de 10 minutes est quand même envoyée, jamais deux fois
(état dans `/var/lib/slovingo-push/state.json`). Les heures suivent le fuseau de l'appareil de l'élève.

Fichiers : `static/pocketbase/notify.js` (réglages, abonnement), `static/pocketbase/sw-push.js` (importé par le
service worker), `tools/slovingo_push.py` (envoi), `tools/slovingo-push/` (exemples systemd).
