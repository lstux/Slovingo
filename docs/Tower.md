# La tour de contrôle (`src/tower.py`)

Une interface console pour tout le flux de travail local : cours, déploiement,
dépendances, build, publication.

```
python3 src/tower.py               # interface interactive (un terminal est requis)
python3 src/tower.py --status      # l'écran d'accueil une fois, puis sortie
python3 src/tower.py --deps        # rapport de dépendances (code 1 s'il en manque une requise)
```

`rich` est facultatif (`pip install rich`) : avec lui, couleurs, cadres et
spinners ; sans lui (ou avec `--plain`), les mêmes écrans en texte simple.

La tour **n'ajoute aucune logique** au pipeline : elle assemble et lance
`langs.py` (`sync()`), `build.py` et `publish.py`, et affiche toujours la
commande équivalente avant de la lancer. Les seuls fichiers qu'elle écrit sont
`deploy.json` (écran de configuration) et `.tower.json` (ses préférences : langues
cochées, branches, options, dates des derniers envois ; ignoré par git).

## Deux entrées, un seul plan

```
accueil ──⏎──▶ fiche langue ──p / b / d──▶ plan ──⏎──▶ exécution + bilan
   └──p / b / d (langues cochées, ou liste à cocher)──▶ plan
```

**Accueil.** Un tableau des langues (branche, état git, âge du dernier build,
dernier envoi réel) avec un curseur, au-dessus d'un résumé du moteur, du
déploiement et des dépendances.

| Touche | Effet |
|---|---|
| `↑` `↓` | choisir une langue |
| `⏎` | ouvrir la fiche de la langue |
| `␣` / `a` | cocher la langue / tout cocher |
| `p` `b` `d` | pull / build / deploy sur les langues cochées ; si aucune n'est cochée, une liste à cocher s'ouvre (la langue sous le curseur est déjà cochée) |
| `c` | configuration du déploiement (`deploy.json`) |
| `s` | dépendances |
| `f` / `t` / `r` | `git fetch` / bascule HTTPS-SSH / rafraîchir |

**HTTPS / SSH.** `t` (ou `langs.py --ssh`) choisit le protocole. Il s'applique
aux nouveaux clones **et** aux clones existants : au prochain pull, l'`origin`
d'un `langs/<code>` est réécrit (`git remote set-url`) vers
`git@github.com:lstux/Slovingo-<code>.git` (ou l'inverse), ce qui permet de
`git push` depuis la tower sans repasser par un token HTTPS. Seuls les
`origin` de la forme `lstux/Slovingo-<code>` (HTTPS ou SSH) sont réécrits :
un fork, un miroir ou une base personnalisée (`--base`,
`SLOVINGO_REPO_BASE`) n'est jamais modifié. La fiche d'une langue affiche
l'`origin` réel. La commande `set-url` apparaît dans le plan (sans effet si l'`origin` est déjà dans le bon protocole).

**Fiche langue.** Dossier, dépôt, branche, état git, dernier commit, âge du
build, dernier envoi. `p` `b` `d` lancent l'action sur cette langue, `g`
change sa branche, `←` `→` passent à la langue voisine.

**Plan.** Même écran quel que soit le chemin pris : trois cases
**pull / build / deploy** (préremplies selon la touche : `p` = pull, `b` =
build, `d` = build + deploy), la branche du moteur (`e`), `--force-exercises`
(`x`), `--force-icons` (`i`), et pour un serveur distant `--delete` (`l`) et
**ENVOI RÉEL** (`v`). Les commandes exactes sont listées, `⏎` les lance.

- **Build par langue** : un build (ou un `publish.py`) par langue, jamais un
  build global.
- **Une langue en échec n'arrête pas les autres.** Le bilan donne une ligne
  par langue, puis un encadré **✘ ERREURS** qui récapitule chaque échec
  (langue, étape, message). Une langue qu'on ne peut pas traiter (dépôt
  modifié, non installé…) est signalée dans le plan puis dans le bilan.
  Seul un échec du changement de branche du moteur arrête tout.
- Déployer sans cocher *build* envoie `dist/` tel quel (`--skip-build`).

## Garde-fous

- Publier envoie à blanc (`rsync --dry-run`) tant que « ENVOI RÉEL » n'est pas
  coché, et un envoi réel demande une confirmation explicite.
- Aucun changement de branche, aucun `pull` sur un dépôt (cours ou moteur)
  ayant des modifications locales : le problème est signalé et l'action refusée.
- En mode local/LAN, une seule langue à la fois (le serveur bloque jusqu'à Ctrl+C,
  qui est alors la sortie normale).
- `publish.py` n'expanse pas `~` dans `identity_file` : l'éditeur enregistre un
  chemin absolu et la validation signale un chemin en `~` ou contenant un espace.
