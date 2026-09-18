[English version](README.md) [Slovak version](README_sk.md)

# Slovingo

Apprendre une langue sans appli, sans compte, sans serveur. Juste des fiches texte, un script Python, et une page web. 

Conçu pour parler couramment (A2) sans s'égarer dans les déclinaisons — et ultra-simple à adapter à ta propre langue.

## 🎯 Pourquoi Slovingo

Les applications les plues connues n'ont pas de cours de Slovaque disponibles, j'avais essayé Ling qui le supporte, mais je n'étais pas tout à fait satisfait non plus. Je voulais pouvoir écouter les mots et les phrases en les lisant, créer mes propres fiches de vocabulaire ou de dialogue...

Ici tu ne dépends de personne. Pas de compte à créer, pas d'abonnement, pas de serveur applicatif qui peut disparaître demain. Juste des fichiers texte en Markdown, un petit script Python pour transformer le tout en un mini-site consultable hors-ligne, et tu as ton cours.

**[PLACEHOLDER: Screenshot 1 — Vue d'une fiche avec tableaux de vocabulaire]**
*Une fiche affiche vocabulaire, grammaire, audio-cards — tout cliquable et prononçable.*

**[PLACEHOLDER: Screenshot 2 — Audio-card en action]**
*Clic sur une phrase slovaque → traduction + décomposition mot à mot + lecture audio.*

**[PLACEHOLDER: Screenshot 3 — Exercices générés]**
*QCM, phrases à trous, compréhension audio — générés automatiquement à partir des fiches.*

**[PLACEHOLDER: Screenshot 4 — Mode hors-ligne + progression locale]**
*Ton avancement suivi en local (localStorage), tout fonctionne sans internet après une visite.*

---

## 🚀 Pour les apprenants

### Cours disponibles

- **[Slovaque pour francophones](https://www.lslinux.org/slovingo/sk-fr/)** — *Ahoj Slovenčina!*
- **[Français pour slovacophones](https://www.lslinux.org/slovingo/fr-sk/)** — *Dis bonjour!*

Chaque cours c'est :

✅ **Fiches de vocabulaire & grammaire** — audio-cards avec traduction et décomposition  
✅ **Mini-dialogues** — personnages récurrents, situations réelles  
✅ **Exercices illimités** — générés auto à partir du corpus  
✅ **Progression suivie** — en local (rien n'est envoyé nulle part)  
✅ **Hors-ligne** — une fois chargée, une fiche reste accessible sans réseau  

### Commencer en 30 sec

Clique sur un cours ci-dessus et c'est parti. Aucune inscription, aucune création de compte.

---

## 📚 Pour les créateurs de cours

Tu veux créer un cours pour **ta propre langue** ? Slovingo est un moteur générique — une langue = un fichier de config JSON + des fiches Markdown.

### Qu'est-ce qu'une fiche ?

C'est du **Markdown classique** + 3 petits trucs de syntaxe :

```markdown
# Vocabulaire basique

| Français | Ma langue |
|----------|-----------|
| Bonjour  | …         |
| Merci    | …         |

## Grammaire

! Une phrase example
> Traduction naturelle.
> Mot1 = traduction mot1
> Mot2 = traduction mot2
+ Note grammaticale si besoin.

Le mot {{spécial}} est prononçable au milieu du texte.
```

**C'est tout.** Le fichier reste lisible en texte brut. Aucune mise en forme, aucun HTML. Le moteur génère le HTML, le CSS gère l'apparence, le JS ajoute l'interactivité (son, boutons, recherche).

### Ajouter une langue

```bash
# 1. Clone le repo
git clone https://github.com/...
cd slovingo

# 2. Crée un dossier pour ta langue
cp -r langs/sk langs/ma-langue
cd langs/ma-langue

# 3. Édite lang.json avec tes métadonnées
# 4. Écris tes fiches en Markdown + SMD
# 5. Publie (voir INSTALL.md)
```

Zéro ligne de Python à toucher. Tout ce qui dépend de la langue (synthèse vocale, couleurs, titre) vit dans `lang.json`.

### Format des fiches

Slovingo supporte trois types de fiches :

1. **Séries** — parcours progressif thématique (5 fiches + 1 extra = 1 semaine)
2. **Dialogues** — conversations en contexte avec personnages
3. **Révisions** — situations quotidiennes pour recycler le vocabulaire

Tous les détails de structure → voir [Format-SMD.txt](https://github.com/...Format-SMD.txt), [Fiches-Serie.txt](https://github.com/...Fiches-Serie.txt), etc. dans le repo.

---

## ⚙️ Specs techniques

| Aspect | Détail |
|--------|--------|
| **Langage** | Python 3.8+ (smd2html.py) + vanilla JS |
| **Dépendances** | Zero — Python stdlib seulement |
| **Déploiement** | Static hosting (GitHub Pages, Netlify, ton serveur…) |
| **Taille** | ~500 KB compressé par cours |
| **Stockage** | localStorage côté navigateur (progression + préférences) |
| **Hors-ligne** | Service worker + PWA — fonctionne sans réseau après visite |
| **Navigateurs** | Chrome, Firefox, Safari, Edge (modernes) |
| **Synthèse vocale** | Web Speech API du navigateur (pas d'appel API externe) |
| **OS** | Linux, macOS, Windows — le script fonctionne partout |

### Architecture

```
slovingo/
├── src/              # Moteur générique (jamais dupliqué)
│   ├── smd2html.py   # Convertisseur SMD → HTML
│   ├── exercises.py  # Générateur QCM + exercices
│   └── …
├── langs/
│   ├── sk/           # Cours slovaque
│   │   ├── lang.json # Config : titre, couleurs, synthèse vocale…
│   │   └── fiches/   # Fichiers .md
│   └── bzh/          # Cours breton (même structure)
└── publish.sh        # Script de génération et déploiement
```

Ajouter une langue = dupliquer `langs/sk/`, renommer, modifier `lang.json`, écrire des fiches. C'est tout.

---

## 📖 Documentation

- **[INSTALL.md](INSTALL.md)** — Installation locale, lancer le dev server, générer les pages
- **[Format-SMD.txt](docs/Format-SMD.txt)** — Syntaxe complète du format SMD (audio-cards, tables, speakables)
- **[Fiches-Serie.txt](docs/Fiches-Serie.txt)** — Comment structurer un parcours progressif

---

## 🛠️ Installer & développer localement

### Prérequis
- Python 3.8+
- Navigateur moderne

### Setup en 2 min

```bash
git clone https://github.com/…/slovingo.git
cd slovingo
python3 src/smd2html.py langs/sk/
# Ouvre langs/sk/index.html dans ton navigateur
```

Chaque fois que tu modifies une fiche, relance le script → il régénère le HTML.

Pour le dev avec rechargement auto, voir [INSTALL.md](INSTALL.md).

---

## 🤝 Contribuer

### Ajouter des fiches à un cours existant

1. Fork le repo
2. Crée une fiche dans `langs/<code>/fiches/` (voir les fichiers existants comme modèle)
3. Teste localement : `python3 src/smd2html.py langs/<code>/`
4. Fais une PR

### Signaler un bug

Issues → section *Issues* du repo. Décris ce qui ne fonctionne pas + navigateur/OS.

### Proposer une nouvelle langue

Ouvre une issue avec :
- Langue + code ISO
- Pourquoi il n'existe pas ailleurs (comme pour slovaque/breton)
- Combien de fiches tu prévois

---

## 📝 Licence

GPL v3 — voir [LICENSE](LICENSE).

Tes fiches, tes exercices, tout ce que tu créés : à toi de choisir (CC-BY, GPL, domaine public…). Slovingo lui-même est GPL v3.

---

**Questions ?** Ouvre une issue, ou juste clone et essaie. Le meilleur README, c'est celui qu'on vit. 🚀
