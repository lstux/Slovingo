# Slovingo

Un petit moteur pour apprendre une langue avec des fiches qu'on peut
lire, écouter, et réviser — sans dépendre d'une appli tierce, sans
serveur applicatif, sans compte à créer. Juste des fichiers texte, un
script Python, et une page web.

Conçu au départ pour apprendre le slovaque en vue d'une conversation
courante (A2, formules toutes faites, on laisse les déclinaisons pour
plus tard), le moteur est aujourd'hui générique : une langue = un
fichier de configuration, jamais une ligne de code à toucher. Le
slovaque a servi de premier cas d'usage ; le breton est le deuxième.

**Sur le nom** : oui, "Slovingo" sur un moteur qui sert aussi de cours
breton, c'est un peu absurde — c'est fait exprès. C'est un private
joke qui trahit l'origine du projet plutôt qu'un vrai nom de marque
par langue (chaque cours a son propre titre dans son `lang.json` :
"Ahoj Slovenčina", "Demat Brezhoneg"...). Un peu comme le format SMD
ci-dessous, qui garde lui aussi une trace de ses origines slovaques.

Pour l'installer et t'en servir : voir **[INSTALL.md](INSTALL.md)**.

---

## Ce que ça fait

- **Des fiches** en Markdown augmenté (le format SMD, voir plus bas),
  converties en pages HTML autonomes : vocabulaire, grammaire,
  dialogues, tout est cliquable et prononçable via la synthèse vocale
  du navigateur.
- **Des exercices générés automatiquement** (QCM, phrases à trous,
  écoute) à partir du vocabulaire déjà présent dans tes fiches — pas
  besoin de les écrire à la main, le corpus grandit et les exercices
  suivent.
- **Une progression suivie en local** (localStorage), pour savoir où
  tu en es fiche par fiche.
- **Un mode hors-ligne** (PWA + service worker) : une fois visitée,
  une fiche reste consultable sans réseau.
- **Un moteur générique** : tout ce qui dépend de la langue apprise
  (code de synthèse vocale, mots-clés de détection de colonne, titre,
  couleurs, déploiement) vit dans un seul `lang.json` par cours. Le
  code, lui, ne connaît aucune langue.

## Le format SMD

**SMD veut dire *Speakable Markdown*** — parce que l'idée centrale du
format, c'est qu'un mot ou une phrase, une fois écrit, doit pouvoir se
faire lire à voix haute d'un clic. (Le sigle existait déjà à l'époque
où le projet ne parlait que slovaque, sous le nom *Slovak Markdown* —
il se trouve que ça tombait juste, alors on l'a gardé. Même logique
que pour le nom du projet.)

C'est du Markdown classique, avec trois ajouts :

**1. Une table de traduction devient prononçable automatiquement**, dès
qu'une colonne a un en-tête reconnu comme la langue apprise :

```markdown
| Français | Slovaque |
|----------|-----------|
| Bonjour  | Dobrý deň |
| Merci    | Ďakujem   |
```

**2. Une "audio-card"** représente une phrase, sa traduction, et sa
décomposition mot à mot :

```markdown
! Chýbaš mi.
> Tu me manques.
> Chýbaš = tu manques
> mi = à moi
+ La construction est inversée par rapport au français.
```

`!` ouvre la carte, le premier `>` est la traduction naturelle, les
`>` suivants décomposent la phrase, `+` ajoute une remarque
grammaticale quand il y en a une à faire.

**3. Un mot isolé devient prononçable** au milieu d'un paragraphe :

```markdown
Le mot {{jeden}} signifie « un ».
```

Le fichier `.md` reste lisible tel quel, sans aucune information de
mise en forme : c'est le convertisseur (`smd2html.py`) qui produit du
HTML sémantique, le CSS qui gère l'apparence, et le JS qui ajoute
l'interactivité (lecture audio, boutons, recherche). Changer l'un ne
demande jamais de toucher aux fiches.

## Architecture en un coup d'œil

```
src/         moteur générique — jamais dupliqué, jamais édité par langue
langs/
  sk/        cours slovaque : lang.json + fiches .md + sommaire
  bzh/       cours breton
  <autre>/   n'importe quelle langue future, même principe
publish.sh   génère les pages HTML + exercices, publie sur un serveur
```

Ajouter une langue : copier un dossier `langs/<code>/`, changer son
`lang.json`, écrire ses fiches. Rien d'autre.

Détails d'installation, de configuration et de publication : voir
**[INSTALL.md](INSTALL.md)**.

## Licence

GPL v3 — voir [LICENSE](LICENSE).
