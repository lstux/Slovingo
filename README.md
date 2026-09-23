[French version](README_fr.md) [Slovak version](README_sk.md)

# Slovingo

Learn a language without an app, without an account, without a server. Just text files, a Python script, and a web page.

Designed to speak fluently (A2) without getting lost in declensions — and ultra-simple to adapt to your own language.

## 🎯 Why Slovingo

The most well-known apps don't have Slovak courses available, at least in French. I tried Ling which supports it, but I wasn't quite satisfied either. I wanted to be able to listen to words and sentences while reading them, create my own vocabulary or dialogue cards...

Here you don't depend on anyone. No account to create, no subscription, no application server that could disappear tomorrow. Just text files in Markdown, a little Python script to transform it all into a mini-site you can consult offline, and you've got your course.

Some screenshots on mobile device :  

![Some screenshots](img/screenshots.png)  

- *Your progress tracked locally (localStorage), everything works without internet after a visit.*  
- *A card displays vocabulary, grammar, audio-cards — all clickable and pronounceable.*  
- *Click on a sentence → translation + word-by-word breakdown + audio playback.*  
- *Multiple choice, fill-in-the-blanks, listening comprehension — auto-generated from the cards.*  

---

## 🚀 For learners

### Available courses

- **[Slovak for French speakers](https://www.lslinux.org/slovingo/sk-fr/)** — *Ahoj Slovenčina!*
- **[French for Slovak speakers](https://www.lslinux.org/slovingo/fr-sk/)** — *Dis bonjour!*

Each course includes:

✅ **Vocabulary & grammar cards** — audio-cards with translation and breakdown  
✅ **Mini-dialogues** — recurring characters, real situations  
✅ **Unlimited exercises** — auto-generated from the corpus  
✅ **Progress tracking** — locally (nothing gets sent anywhere)  
✅ **Offline** — once loaded, a card stays accessible without internet  

### Get started in 30 seconds

Click on a course above and you're good to go. No sign-up, no account creation.

---

## 📚 For course creators

Want to create a course for **your own language**? Slovingo is a generic engine — one language = one JSON config file + Markdown cards.

### What's a card?

It's **regular Markdown** + 3 small syntax tricks:

```markdown
# Basic vocabulary

| Learn french | Native |
|--------|-------------|
| Bonjour | …           |
| Merci | …           |

## Grammar

! An example sentence
> Natural translation.
> Word1 = word1 translation
> Word2 = word2 translation
+ Grammar note if needed.

The word {{special}} is pronounceable in the middle of text.
```

**That's it.** The file stays readable as plain text. No formatting, no HTML. The engine generates the HTML, CSS handles the appearance, JS adds the interactivity (sound, buttons, search).

### Add a language

```bash
# 1. Clone the repo
git clone https://github.com/...
cd slovingo

# 2. Create a folder for your language
cp -r langs/sk langs/my-language
cd langs/my-language

# 3. Edit lang.json with your metadata
# 4. Write your cards in Markdown + SMD
# 5. Publish (see INSTALL.md)
```

Not a single line of Python to touch. Everything language-dependent (text-to-speech, colors, title) lives in `lang.json`.

---

## ⚙️ Technical specs

| Aspect | Detail |
|--------|--------|
| **Language** | Python 3.8+ (smd2html.py) + vanilla JS |
| **Dependencies** | Zero — Python stdlib only |
| **Deployment** | Static hosting (GitHub Pages, Netlify, your server…) |
| **Size** | ~500 KB compressed per course |
| **Storage** | Browser localStorage (progress + preferences) |
| **Offline** | Service worker + PWA — works without internet after a visit |
| **Browsers** | Chrome, Firefox, Safari, Edge (modern versions) |
| **Text-to-speech** | Browser's Web Speech API (no external API calls) |
| **OS** | Linux, macOS, Windows — script works everywhere |

### Architecture

```
slovingo/
├── src/              # Generic engine (never duplicated)
│   ├── smd2html.py   # SMD → HTML converter
│   ├── exercises.py  # Multiple choice + exercise generator
│   └── …
├── langs/
│   ├── sk/           # Slovak course
│   │   ├── lang.json # Config: title, colors, TTS…
│   │   └── fiches/   # .md files
│   └── bzh/          # Breton course (same structure)
└── publish.sh        # Build and deploy script
```

Adding a language = duplicate `langs/sk/`, rename, edit `lang.json`, write cards. That's all.

---

## 📖 Documentation

- **[INSTALL.md](INSTALL.md)** — Local setup, run dev server, generate pages
- **[Format-SMD.txt](docs/Format-SMD.txt)** — Full SMD syntax (audio-cards, tables, speakables)
- **[Fiches-Serie.txt](docs/Fiches-Serie.txt)** — How to structure a progressive progression

---

## 🛠️ Install & develop locally

### Requirements
- Python 3.8+
- Modern browser

### Setup in 2 minutes

```bash
git clone https://github.com/…/slovingo.git
cd slovingo
python3 src/smd2html.py langs/sk/
# Open langs/sk/index.html in your browser
```

Every time you modify a card, rerun the script → it regenerates the HTML.

For dev with auto-reload, see [INSTALL.md](INSTALL.md).

---

## 🤝 Contributing

### Add cards to an existing course

1. Fork the repo
2. Create a card in `langs/<code>/fiches/` (see existing files as model)
3. Test locally: `python3 src/smd2html.py langs/<code>/`
4. Make a PR

### Report a bug

Issues → *Issues* section of the repo. Describe what's not working + browser/OS.

### Propose a new language

Open an issue with:
- Language + ISO code
- Why it doesn't exist elsewhere (like for Slovak/Breton)
- How many cards you're planning

---

## 📝 License

GPL v3 — see [LICENSE](LICENSE).

Your cards, your exercises, everything you create: up to you to choose (CC-BY, GPL, public domain…). Slovingo itself is GPL v3.

---

**Questions?** Open an issue, or just clone and try. The best README is one you live. 🚀
