[English version](README.md) [French version](README_fr.md)

# Slovingo

Uč sa jazyk bez aplikácie, bez účtu, bez servera. Len textové kartičky a Python skript, ktorý vygeneruje mini-web v HTML.

Priamo dostupné na **[www.lslinux.org/slovingo](https://www.lslinux.org/slovingo)**.

## 🎯 Prečo Slovingo

Najznámejšie aplikácie nemajú kurz slovenčiny. Ling bol vyskúšaný, ale nenadchol (a po prvom kurze je platený). Preto vznikol tento projekt: chcel som si slová počas čítania aj vypočuť a vytvárať si vlastné kartičky.

Žiadny účet, žiadne predplatné, žiadny server, ktorý môže zajtra zmiznúť. Len textové súbory v Markdowne + malý Python skript, ktorý z nich spraví mini-web na offline prezeranie.

![Some screenshots](img/screenshots.png)

- Tvoj pokrok sa ukladá lokálne (nič sa nikam neposiela)
- Kartička zobrazuje slovnú zásobu, gramatiku, zvuk — všetko klikateľné a vysloviteľné
- Klik na vetu → preklad + rozklad slovo po slove + prehratie zvuku
- Testy, vety s medzerami, počúvanie — generované automaticky z kartičiek
- Po nainštalovaní ako PWA funguje aj naďalej offline, aj keby lslinux.org jedného dňa zanikol

## ⚠️ Obsah generovaný AI

Kartičky boli napísané s pomocou AI.

- **Slovenčina pre frankofónov**: priebežne kontrolovaná a opravovaná, keďže kurz sám absolvujem.
- **Francúzština pre Slovákov**: pre mňa oveľa ťažšie poriadne skontrolovať (nie je k dispozícii rodený Slovák na kontrolu). Ak nájdeš chybu, PR alebo issue sú vítané 🙏

## 🚀 Ako sa učiť

- **[Slovenčina pre frankofónov](https://www.lslinux.org/slovingo/sk-fr/)** — *Ahoj Slovenčina!*
- **[Francúzština pre Slovákov](https://www.lslinux.org/slovingo/fr-sk/)** — *Dis bonjour!*
- **Bretónčina pre frankofónov** — začiatočný kurz, zatiaľ bez syntézy reči

Klikni a ide sa. Žiadna registrácia.

## 📚 Ako vytvoriť kurz v inom jazyku

Slovingo je generický engine: jeden jazyk = jeden súbor `lang.json` + kartičky v Markdowne.

Kartička je bežný Markdown plus pár syntaktických trikov (formát **SMD**):

```markdown
# Základná slovná zásoba

| Môj jazyk | Slovenčina |
|-----------|-----------|
| …         | Dobrý deň |
| …         | Ďakujem   |

! Ukážková veta
> Prirodzený preklad.
> Slovo1 = preklad slova 1
+ Gramatická poznámka, ak treba.
```

Žiadny Python netreba meniť: všetko, čo závisí od jazyka (hlas, farby, názov), je v `lang.json`. Celý formát je v [Format-SMD.txt](docs/Format-SMD.txt).

```bash
git clone https://github.com/…/slovingo.git
cd slovingo
cp -r langs/sk-fr langs/môj-jazyk
python3 src/publish.py --lang-dir langs/môj-jazyk
```

## 🛠️ Lokálna inštalácia

```bash
git clone https://github.com/…/slovingo.git
cd slovingo
python3 src/publish.py --lang-dir langs/sk-fr
# Otvor langs/sk-fr/dist/index.html v prehliadači
```

Python 3.8+, žiadne externé závislosti, funguje všade (Linux/macOS/Windows). Detaily v [INSTALL.md](INSTALL.md).

## 🤝 Prispievanie

Príspevky sú vítané, hlavne kontrola a opravy francúzskeho kurzu!

- **Oprava/pridanie kartičky**: fork → uprav v `langs/<kód>/fiches/` → otestuj cez `python3 src/publish.py --lang-dir langs/<kód>` → PR
- **Nahlásenie chyby**: otvor issue (čo nefunguje + prehliadač/OS)
- **Návrh jazyka**: otvor issue (jazyk, ISO kód, prečo inde chýba)

## 📝 Licencia

GitHub repozitár — engine, kartičky aj cvičenia — je pod GPL v3, pozri [LICENSE](LICENSE). Mimo repozitára si každý môže zvoliť pre svoje vlastné kartičky ľubovoľnú licenciu (CC-BY, verejná doména…).

---

**Otázky?** Otvor issue, alebo to jednoducho naklonuj a vyskúšaj. 🚀
