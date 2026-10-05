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
`deploy.json` (écran Déploiement) et `.tower.json` (ses préférences : cours
cochés, branches, options ; ignoré par git).

## Écrans

| Touche | Écran | Rôle |
|---|---|---|
| (accueil) | État | branche et propreté du moteur, déploiement, dépendances, et un tableau des cours : branche, propre/modifié, ↑ avance / ↓ retard sur `origin`, âge du dernier build |
| `1` | Cours | cocher des cours, choisir une branche par cours, cloner / mettre à jour (`git pull --ff-only`, jamais sur un dépôt modifié), `fetch` pour actualiser les retards, bascule HTTPS/SSH |
| `2` | Déploiement | modes **local** (127.0.0.1), **lan** (visible sur le Wi-Fi) ou **distant** (rsync via SSH) ; édition de `host`, `user`, `ssh_port`, `remote_root`, `identity_file`, `port` ; validation ; test de connexion SSH (clé, dossier distant, rsync côté serveur) |
| `3` | Dépendances | `git`, Python, Pillow (requis) ; `rsync` et `ssh` (requis pour publier à distance) ; ImageMagick (`fetch_images.py`) et `rich` (facultatifs) ; version détectée et commande d'installation adaptée à ton système |
| `4` | Build & publication | action (build seul / build + publication / publication sans rebuild), branche du moteur, pull avant build, `--force-exercises`, `--force-icons`, `--delete`, envoi réel ou essai à blanc ; liste des problèmes, commandes prévues, puis exécution étape par étape (arrêt à la première erreur) |

## Garde-fous

- Publier envoie à blanc (`rsync --dry-run`) tant que « ENVOI RÉEL » n'est pas
  coché, et un envoi réel demande une confirmation explicite.
- Aucun changement de branche, aucun `pull` sur un dépôt (cours ou moteur)
  ayant des modifications locales : le problème est signalé et l'action refusée.
- En mode local/LAN, un seul cours à la fois (le serveur bloque jusqu'à Ctrl+C,
  qui est alors la sortie normale).
- `publish.py` n'expanse pas `~` dans `identity_file` : l'éditeur enregistre un
  chemin absolu et la validation signale un chemin en `~` ou contenant un espace.
