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
