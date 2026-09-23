[English version](README.md) [Slovak version](README_sk.md)

# Slovingo

Apprendre une langue sans appli, sans compte, sans serveur. Juste des fiches texte et un script Python qui génère un mini site en html.

Directement disponible sur **[www.lslinux.org/slovingo](https://www.lslinux.org/slovingo)**.

## 🎯 Pourquoi Slovingo

Les applis connues n'ont pas de cours de slovaque. Ling a été testé, sans convaincre (et payant après le premier cours). D'où ce projet : pouvoir écouter les mots en les lisant, et créer ses propres fiches de vocabulaire ou de dialogue.

Pas de compte, pas d'abonnement, pas de serveur applicatif qui peut disparaître demain. Juste des fichiers texte en Markdown + un petit script Python qui en fait un mini-site consultable hors-ligne.

![Quelques captures d'écran](img/screenshots.png)

- Avancement suivi en local (rien n'est envoyé nulle part)
- Une fiche affiche vocabulaire, grammaire, audio — tout cliquable et prononçable
- Clic sur une phrase → traduction + mot à mot + lecture audio
- QCM, phrases à trous, compréhension audio — générés automatiquement à partir des fiches
- Une fois installé en PWA, le site reste utilisable hors-ligne même si lslinux.org venait à disparaître un jour

## ⚠️ Contenu généré par IA

Les fiches ont été écrites avec l'aide de l'IA.

- **Slovaque pour francophones** : relu et corrigé au fur et à mesure du suivi du cours.
- **Français pour slovacophones** : bien plus difficile à relire sérieusement (pas de relecture native slovaque disponible ici). Une PR ou une issue en cas de bourde repérée est la bienvenue 🙏

## 🚀 Pour apprendre

- **[Slovaque pour francophones](https://www.lslinux.org/slovingo/sk-fr/)** — *Ahoj Slovenčina!*
- **[Français pour slovacophones](https://www.lslinux.org/slovingo/fr-sk/)** — *Dis bonjour!*
- **Breton pour francophones** — embryon de cours, sans synthèse vocale disponible pour l'instant

Un clic suffit, aucune inscription.

## 📚 Pour créer un cours dans une autre langue

Slovingo est un moteur générique : une langue = un fichier `lang.json` + des fiches Markdown.

Une fiche, c'est du Markdown classique + quelques trucs de syntaxe (le format **SMD**) :

```markdown
# Vocabulaire basique

| Langue apprise | Français |
|-----------------|----------|
| …               | Bonjour  |
| …               | Merci    |

! Une phrase exemple
> Traduction naturelle.
> Mot1 = traduction mot1
+ Note grammaticale si besoin.
```

Rien à toucher côté Python : tout ce qui dépend de la langue (voix, couleurs, titre) vit dans `lang.json`. Détail complet du format dans [Format-SMD.txt](docs/Format-SMD.txt).

```bash
git clone https://github.com/…/slovingo.git
cd slovingo
cp -r langs/sk-fr langs/ma-langue
python3 src/publish.py --lang-dir langs/ma-langue
```

## 🛠️ Installer en local

```bash
git clone https://github.com/…/slovingo.git
cd slovingo
python3 src/publish.py --lang-dir langs/sk-fr
# Ouvrir langs/sk-fr/dist/index.html dans un navigateur
```

Python 3.8+, aucune dépendance externe, fonctionne partout (Linux/macOS/Windows). Détails dans [INSTALL.md](INSTALL.md).

## 🤝 Contribuer

Les contributions sont les bienvenues, en particulier pour relire et corriger le cours de français !

- **Corriger/ajouter une fiche** : fork → édition dans `langs/<code>/fiches/` → test avec `python3 src/publish.py --lang-dir langs/<code>` → PR
- **Signaler un bug** : ouvrir une issue (ce qui ne fonctionne pas + navigateur/OS)
- **Proposer une langue** : ouvrir une issue (langue, code ISO, pourquoi elle manque ailleurs)

## 📝 Licence

Le dépôt GitHub — moteur, fiches et exercices inclus — est sous GPL v3, voir [LICENSE](LICENSE). En dehors du dépôt, chacun reste libre de choisir la licence de ses propres fiches (CC-BY, domaine public…).

---

**Des questions ?** Ouvrir une issue, ou cloner et essayer directement. 🚀
