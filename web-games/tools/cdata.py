"""Lecture des fichiers C generes par les build_assets.py du firmware (src/game/gen/*.inc).

Les jeux web ne refont pas la conversion des images, des polices et des niveaux : ils lisent
exactement les memes donnees que le firmware. Ainsi, quand on regenere `gen/` cote ESP32, il
suffit de relancer `web-games/tools/export_*.py` pour que la version web suive.

Ce n'est pas un parseur C complet : il lit les declarations `const TYPE NOM[N] = { ... };`
telles que les ecrivent les generateurs (nombres, chaines, identifiants, accolades imbriquees).
"""
import base64
import re

TOKEN = re.compile(r'"(?:\\.|[^"\\])*"|-?\d+(?:\.\d+)?|&?[A-Za-z_][A-Za-z_0-9]*|[{},]')
DECL = re.compile(r'^(?:static\s+)?const\s+([A-Za-z_][\w\s\*]*?)\s+([A-Za-z_]\w*)\s*(\[[^\]]*\])?\s*(?:PROGMEM\s*)?=\s*\{',
                  re.M)
NUMS = re.compile(r'-?\d+')


def _match_brace(text, start):
    """Index de l'accolade fermante qui correspond a text[start] == '{' (chaines respectees)."""
    depth = 0
    i = start
    n = len(text)
    while i < n:
        c = text[i]
        if c == '"':
            i += 1
            while text[i] != '"':
                i += 2 if text[i] == '\\' else 1
        elif c == '{':
            depth += 1
        elif c == '}':
            depth -= 1
            if depth == 0:
                return i
        i += 1
    raise ValueError("accolade non fermee")


def _unquote(s):
    """Chaine C : \\n, \\t, \\" et les codes octaux (\\001 = le marqueur du nom du joueur)."""
    def esc(m):
        g = m.group(1)
        if g[0] in "01234567":
            return chr(int(g, 8))
        return {"n": "\n", "t": "\t"}.get(g, g)
    return re.sub(r'\\([0-7]{1,3}|.)', esc, s[1:-1])


def _parse_body(body):
    """'{...}' deja sans les accolades exterieures -> liste imbriquee (int, str, ident)."""
    stack = [[]]
    for tok in TOKEN.findall(body):
        if tok == '{':
            stack.append([])
        elif tok == '}':
            done = stack.pop()
            stack[-1].append(done)
        elif tok == ',':
            continue
        elif tok[0] == '"':
            stack[-1].append(_unquote(tok))
        elif re.fullmatch(r'-?\d+', tok):
            stack[-1].append(int(tok))
        elif re.fullmatch(r'-?\d+\.\d+', tok):
            stack[-1].append(float(tok))
        else:
            stack[-1].append(tok)   # identifiant (eventuellement precede de &)
    return stack[0]


def parse_inc(text):
    """-> {nom: (type, valeur)} ; valeur = liste (tableau ou structure initialisee)."""
    out = {}
    pos = 0
    while True:
        m = DECL.search(text, pos)
        if not m:
            break
        open_i = m.end() - 1
        close_i = _match_brace(text, open_i)
        body = text[open_i + 1:close_i]
        typ = " ".join(m.group(1).split())
        if re.fullmatch(r'[\s\d,\-]*', body):
            val = [int(x) for x in NUMS.findall(body)]    # tableau d'octets : voie rapide
        else:
            val = _parse_body(body)
        out[m.group(2)] = (typ, val)
        pos = close_i + 1
    return out


def packbits(data):
    """PackBits : 0..127 = copier n+1 octets, 129..255 = repeter l'octet suivant 257-n fois."""
    data = bytes(data)
    out = bytearray()
    i, n = 0, len(data)
    while i < n:
        run = 1
        while i + run < n and run < 128 and data[i + run] == data[i]:
            run += 1
        if run >= 3:
            out += bytes([257 - run, data[i]])
            i += run
            continue
        j = i
        while j < n and j - i < 128:
            if j + 2 < n and data[j] == data[j + 1] == data[j + 2]:
                break
            j += 1
        if j == i:
            j = i + 1
        out += bytes([j - i - 1]) + data[i:j]
        i = j
    return bytes(out)


def pack_b64(data):
    return base64.b64encode(packbits(data)).decode("ascii")


def unpackbits(blob):
    out = bytearray()
    i = 0
    while i < len(blob):
        n = blob[i]
        i += 1
        if n < 128:
            out += blob[i:i + n + 1]
            i += n + 1
        elif n > 128:
            out += bytes([blob[i]]) * (257 - n)
            i += 1
    return bytes(out)
