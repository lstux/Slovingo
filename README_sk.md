[English version](README.md) [French version](README_fr.md)

# Slovingo

Uč sa jazyk bez aplikácie, bez účtu, bez servera. Len textové kartičky, skript v Python a webová stránka.

Navrhnuté tak, aby si sa vedel plynule dorozumieť (A2) bez toho, aby si sa stratil v skloňovaní — a ultra jednoduché na prispôsobenie tvojmu vlastnému jazyku.

## 🎯 Prečo Slovingo

Najznámejšie aplikácie nemajú kurz slovenčiny. Skúšal som Ling, ktorý ju podporuje, ale ani ten ma úplne nenadchol. Chcel som si slová a vety počas čítania aj počúvať a vytvárať si vlastné kartičky so slovnou zásobou či dialógmi...

Tu nie si na nikom závislý. Žiadny účet, žiadne predplatné, žiadny aplikačný server, ktorý môže zajtra zmiznúť. Len textové súbory v Markdowne, malý skript v Python, ktorý z nich vyrobí mini-web použiteľný offline, a máš svoj kurz.

Niekoľko snímok obrazovky v mobilnej verzii :

![Some screenshots](img/screenshots.png)  

- *Tvoj pokrok sa ukladá lokálne (localStorage), všetko funguje bez internetu po prvej návšteve.*  
- *Kartička zobrazuje slovnú zásobu, gramatiku, audio-karty — všetko sa dá kliknúť a vysloviť.*  
- *Klik na slovenskú vetu → preklad + rozklad slovo po slove + prehratie zvuku.*  
- *Testy s výberom odpovede, vety s medzerami, porozumenie sluchom — generované automaticky z kartičiek.*  

---

## 🚀 Pre študentov

### Dostupné kurzy

- **[Slovenčina pre frankofónov](https://www.lslinux.org/slovingo/sk-fr/)** — *Ahoj Slovenčina!*
- **[Francúzština pre Slovákov](https://www.lslinux.org/slovingo/fr-sk/)** — *Dis bonjour!*

Každý kurz obsahuje:

✅ **Kartičky so slovnou zásobou a gramatikou** — audio-karty s prekladom a rozkladom  
✅ **Mini-dialógy** — opakujúce sa postavy, reálne situácie  
✅ **Neobmedzené cvičenia** — automaticky generované z korpusu  
✅ **Sledovanie pokroku** — lokálne (nič sa nikam neodosiela)  
✅ **Offline** — po načítaní zostane kartička dostupná aj bez siete  

### Začni za 30 sekúnd

Klikni na kurz vyššie a ide sa. Žiadna registrácia, žiadne zakladanie účtu.

---

## 📚 Pre tvorcov kurzov

Chceš vytvoriť kurz pre **svoj vlastný jazyk**? Slovingo je generický engine — jeden jazyk = jeden konfiguračný súbor JSON + kartičky v Markdowne.

### Čo je to kartička?

Je to **klasický Markdown** + 3 malé syntaktické vychytávky:

```markdown
# Základná slovná zásoba

| Môj jazyk | Slovenčina |
|--------------|-----------|
| …         | Dobrý deň |
| …         | Ďakujem |

## Gramatika

! Ukážková veta
> Prirodzený preklad.
> Slovo1 = preklad slova 1
> Slovo2 = preklad slova 2
+ Gramatická poznámka, ak je potrebná.

Slovo {{špeciálne}} sa dá vysloviť aj uprostred textu.
```

**To je všetko.** Súbor zostáva čitateľný ako čistý text. Žiadne formátovanie, žiadny HTML. Engine vygeneruje HTML, CSS sa postará o vzhľad, JS pridá interaktivitu (zvuk, tlačidlá, vyhľadávanie).

### Pridanie jazyka

```bash
# 1. Naklonuj repozitár
git clone https://github.com/...
cd slovingo

# 2. Vytvor priečinok pre svoj jazyk
cp -r langs/sk langs/môj-jazyk
cd langs/môj-jazyk

# 3. Uprav lang.json podľa svojich metadát
# 4. Napíš svoje kartičky v Markdowne + SMD
# 5. Publikuj (pozri INSTALL.md)
```

Ani jeden riadok Python netreba meniť. Všetko, čo závisí od jazyka (syntéza reči, farby, názov), žije v `lang.json`.

## ⚙️ Technické špecifikácie

| Aspekt | Detail |
|--------|--------|
| **Jazyk** | Python 3.8+ (smd2html.py) + vanilla JS |
| **Závislosti** | Žiadne — iba štandardná knižnica Python |
| **Nasadenie** | Statický hosting (GitHub Pages, Netlify, tvoj server…) |
| **Veľkosť** | ~500 KB komprimované na kurz |
| **Úložisko** | localStorage v prehliadači (pokrok + preferencie) |
| **Offline** | Service worker + PWA — funguje bez siete po prvej návšteve |
| **Prehliadače** | Chrome, Firefox, Safari, Edge (moderné verzie) |
| **Syntéza reči** | Web Speech API prehliadača (žiadne volanie externého API) |
| **OS** | Linux, macOS, Windows — skript funguje všade |

### Architektúra

```
slovingo/
├── src/              # Generický engine (nikdy sa nekopíruje)
│   ├── smd2html.py   # Konvertor SMD → HTML
│   ├── exercises.py  # Generátor testov a cvičení
│   └── …
├── langs/
│   ├── sk/           # Slovenský kurz
│   │   ├── lang.json # Konfigurácia: názov, farby, syntéza reči…
│   │   └── fiches/   # Súbory .md
│   └── bzh/          # Bretónsky kurz (rovnaká štruktúra)
└── publish.sh        # Skript na generovanie a nasadenie
```

Pridať jazyk = skopírovať `langs/sk/`, premenovať, upraviť `lang.json`, napísať kartičky. To je všetko.

---

## 📖 Dokumentácia

- **[INSTALL.md](INSTALL.md)** — Lokálna inštalácia, spustenie dev servera, generovanie stránok
- **[Format-SMD.txt](docs/Format-SMD.txt)** — Úplná syntax formátu SMD (audio-karty, tabuľky, vysloviteľné prvky)
- **[Fiches-Serie.txt](docs/Fiches-Serie.txt)** — Ako štruktúrovať postupný kurz

---

## 🛠️ Inštalácia a lokálny vývoj

### Požiadavky
- Python 3.8+
- Moderný prehliadač

### Nastavenie za 2 minúty

```bash
git clone https://github.com/…/slovingo.git
cd slovingo
python3 src/smd2html.py langs/sk/
# Otvor langs/sk/index.html vo svojom prehliadači
```

Zakaždým, keď upravíš kartičku, spusti skript znova → vygeneruje HTML nanovo.

Pre vývoj s automatickým znovunačítaním pozri [INSTALL.md](INSTALL.md).

---

## 🤝 Prispievanie

### Pridanie kartičiek do existujúceho kurzu

1. Sforkuj repozitár
2. Vytvor kartičku v `langs/<kód>/fiches/` (ako vzor použi existujúce súbory)
3. Otestuj lokálne: `python3 src/smd2html.py langs/<kód>/`
4. Otvor PR

### Nahlásenie chyby

Issues → sekcia *Issues* v repozitári. Opíš, čo nefunguje, + prehliadač/OS.

### Návrh nového jazyka

Otvor issue s týmito údajmi:
- Jazyk + ISO kód
- Prečo inde neexistuje (ako v prípade slovenčiny/bretónčiny)
- Koľko kartičiek plánuješ

---

## 📝 Licencia

GPL v3 — pozri [LICENSE](LICENSE).

Tvoje kartičky, tvoje cvičenia, všetko, čo vytvoríš: je na tebe, akú licenciu zvolíš (CC-BY, GPL, verejná doména…). Samotné Slovingo je pod GPL v3.

---

**Otázky?** Otvor issue, alebo to jednoducho naklonuj a vyskúšaj. Najlepší README je ten, ktorý žiješ. 🚀
