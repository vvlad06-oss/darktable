#!/usr/bin/env python3
"""Generate po/en@lightroom.po: the capitalized English translation
(en@truecase) with Lightroom Classic names for views, panels and modules.

Usage: tools/lightroom/make_po.py  (run from the source root)
"""
import os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SRC_PO = os.path.join(ROOT, "po", "en@truecase.po")
DST_PO = os.path.join(ROOT, "po", "en@lightroom.po")

# (msgctxt, msgid) -> Lightroom name
LIGHTROOM = {
    # views
    (None, "lighttable"): "Library",
    (None, "darkroom"): "Develop",
    (None, "map"): "Map",
    ("view", "print"): "Print",
    (None, "slideshow"): "Slideshow",
    (None, "tethering"): "Tether",
    # processing modules = Develop panels
    (None, "rgb curve"): "Tone Curve",
    (None, "tone curve"): "Tone Curve (Legacy)",
    (None, "color equalizer"): "HSL / Color",
    (None, "color balance rgb"): "Color Grading",
    (None, "color balance"): "Color Balance (Legacy)",
    ("modulename", "sharpen"): "Sharpening",
    (None, "denoise (profiled)"): "Noise Reduction",
    (None, "lens correction"): "Lens Corrections",
    (None, "chromatic aberrations"): "Remove Chromatic Aberration",
    (None, "rotate and perspective"): "Transform",
    (None, "retouch"): "Spot Removal",
    (None, "spot removal"): "Spot Removal (Legacy)",
    (None, "vignetting"): "Post-Crop Vignetting",
    (None, "rgb primaries"): "Calibration",
    (None, "color calibration"): "White Balance",
    ("modulename", "white balance"): "Camera White Balance",
    (None, "local contrast"): "Clarity",
    (None, "haze removal"): "Dehaze",
    ("modulename", "texture"): "Texture",
    (None, "texture"): "Texture",
    (None, "highlight reconstruction"): "Highlight Recovery",
    (None, "shadows and highlights"): "Shadows & Highlights",
    (None, "rgb levels"): "Levels",
    (None, "levels"): "Levels (Legacy)",
    (None, "monochrome"): "Black & White",
    (None, "split-toning"): "Split Toning",
    (None, "graduated density"): "Graduated Filter",
    (None, "sigmoid"): "Profile (Sigmoid)",
    (None, "filmic rgb"): "Profile (Filmic)",
    (None, "AgX"): "Profile (AgX)",
    # side panels
    (None, "navigation"): "Navigator",
    (None, "history"): "History",
    (None, "history stack"): "Copy & Sync Settings",
    (None, "duplicate manager"): "Virtual Copies",
    (None, "tagging"): "Keywording",
    (None, "metadata editor"): "Metadata",
    (None, "image information"): "Info",
    (None, "mask manager"): "Masking",
    (None, "styles"): "Presets",
    (None, "collection filters"): "Library Filter",
    (None, "scopes"): "Histogram",
    (None, "neural restore"): "Enhance (AI)",
    # module groups
    ("modulegroup", "Develop"): "Develop",
}

# all processing module and panel names get Title Case
TITLE = [
    (None, n) for n in """basic adjustments|bloom|blurs|framing|contrast equalizer|base curve
censorize|channel mixer|old local contrast|crop and rotate|contrast brightness saturation
color look up table|color contrast|color correction|color harmonizer|input color profile
colorize|color mapping|output color profile|color reconstruction|color transfer|color zones
crop|defringe|demosaic|diffuse or sharpen|dither or posterize|enlarge canvas|legacy equalizer
exposure|filmic|global tonemap|grain|highpass|hot pixels|invert|liquify|lowlight vision
lowpass|LUT 3D|negadoctor|astrophoto denoise|overexposed|composite|unbreak input profile
external raster masks|raw denoise|raw overexposed|fill light|soften|tone equalizer|velvia
vibrance|watermark|zone system|surface blur|tone mapping|orientation
background jobs|camera settings|collections|color picker|export|geotagging|actions on selection
import|module order|live view|find location|locations|map settings|print settings
recently used collections|selection|session|snapshots|timeline""".replace("\n", "|").split("|")
] + [("modulename", n) for n in ("scale into final size", "display encoding",
                                  "raw black/white point", "rotate pixels", "scale pixels")]

SMALL = {"and", "or", "of", "to", "in", "into", "for", "the", "a", "on"}


def title_case(text):
    words = text.split(" ")
    out = []
    for i, w in enumerate(words):
        if not w:
            out.append(w); continue
        if w.isupper() or any(c.isupper() for c in w[1:]):
            out.append(w)               # RGB, LUT, AgX...
        elif i > 0 and w.lower() in SMALL:
            out.append(w.lower())
        else:
            out.append(w[0].upper() + w[1:])
    return " ".join(out)


def unquote(s):
    s = s.strip()
    assert s.startswith('"') and s.endswith('"'), s
    return bytes(s[1:-1], "utf-8").decode("unicode_escape").encode("latin-1").decode("utf-8")


def quote(s):
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n").replace("\t", "\\t") + '"'


def entries(lines):
    """split a .po file in blocks separated by blank lines"""
    block = []
    for line in lines:
        if line.strip() == "":
            if block:
                yield block
            block = []
        else:
            block.append(line)
    if block:
        yield block


def parse_block(block):
    ctx = msgid = None
    cur = None
    fields = {}
    for line in block:
        if line.startswith("#"):
            continue
        m = re.match(r'(msgctxt|msgid_plural|msgid|msgstr(?:\[\d+\])?)\s+(".*")\s*$', line)
        if m:
            cur = m.group(1)
            fields[cur] = unquote(m.group(2))
        elif line.startswith('"') and cur:
            fields[cur] += unquote(line)
    return fields


def main():
    lines = open(SRC_PO, encoding="utf-8").read().split("\n")
    wanted = dict(LIGHTROOM)
    out = []
    done = set()
    for block in entries(lines):
        f = parse_block(block)
        key = (f.get("msgctxt"), f.get("msgid"))
        new = None
        if key in wanted:
            new = wanted[key]
        elif key in TITLE:
            new = title_case(f.get("msgstr") or f.get("msgid"))
        if new is not None and "msgid_plural" not in f:
            done.add(key)
            block = [l for l in block if l.startswith("#")]
            if key[0] is not None:
                block.append("msgctxt " + quote(key[0]))
            block.append("msgid " + quote(key[1]))
            block.append("msgstr " + quote(new))
        elif "msgid" in f and f["msgid"] == "":
            block = [l.replace("Language: en", "Language: en@lightroom") for l in block]
        out.append("\n".join(block))
    # names that were not in the catalog yet
    for key, new in list(wanted.items()):
        if key in done:
            continue
        b = []
        if key[0] is not None:
            b.append("msgctxt " + quote(key[0]))
        b.append("msgid " + quote(key[1]))
        b.append("msgstr " + quote(new))
        out.append("\n".join(b))
    header = ("# Lightroom Classic flavoured English for darktable, generated by\n"
              "# tools/lightroom/make_po.py from en@truecase.po - do not edit.\n")
    open(DST_PO, "w", encoding="utf-8").write(header + "\n\n".join(out) + "\n")
    missing = [k for k in TITLE if k not in done]
    print(f"wrote {DST_PO}: {len(done)} names replaced")
    if missing:
        print("not found:", missing)


if __name__ == "__main__":
    main()
