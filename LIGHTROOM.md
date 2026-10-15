# darktable — ediția „Lightroom Classic”

Branch `lightroom-ui`, pornit din darktable **5.6.2**. Interfața arată și se comportă ca Lightroom Classic; procesarea imaginii rămâne a lui darktable.

## Instalare pe Mac (Apple Silicon, macOS 15+)

1. Deschide pagina **Releases** a acestui repository și ia ultimul `lightroom-build-N` → `darktable-lightroom-ui-arm64.dmg`.
2. Trage darktable în Applications. Aplicația nu e semnată de Apple, așa că la prima pornire: click dreapta pe darktable → **Open** → **Open**.
3. La prima pornire, configurația existentă trece o singură dată pe tema și layout-ul Lightroom. Scurtăturile vechi sunt păstrate în `~/.config/darktable/shortcutsrc.before-lightroom`. Limba „Lightroom English” se activează după o repornire.

Fiecare push pe `lightroom-ui` construiește automat un .dmg nou (workflow `.github/workflows/lightroom-macos.yml`).

## Ce e făcut

| Zona | Ce s-a schimbat | Fișiere |
|---|---|---|
| Slidere | etichetă \| pistă \| valoare pe un rând, coloane aliniate, pistă subțire; click pe valoare = scrii valoarea, drag pe etichetă = reglaj fin | `src/bauhaus/bauhaus.c` |
| Panoul Basic | WB, Temp / Tone: Exposure, Contrast, Highlights, Shadows, Whites, Blacks / Presence: Clarity, Dehaze, Vibrance, Saturation | `src/libs/modulegroups.c` (preset „Lightroom Classic”) |
| Coloana Develop | sub Basic: Tone Curve, HSL / Color, Color Grading, Sharpening, Noise Reduction, Lens Corrections, Remove CA, Transform, Crop, Spot Removal, Post-Crop Vignetting, Grain, Calibration | `src/libs/modulegroups.c` |
| Antete panouri | nume aliniat la dreapta + triunghi ◀/▼, butoane doar la hover | `src/develop/imageop.c` |
| Temă | `darktable-lightroom` (griuri mate), implicită | `data/themes/darktable-lightroom.css` |
| Nume | Library / Develop, Tone Curve, Color Grading, Virtual Copies, Keywording, Masking… | `po/en@lightroom.po` (generat de `tools/lightroom/make_po.py`) |
| Scurtături | G, E, C, N, D, R, Q, Shift+W, X, 6–9, J, Z, F, \\, Ctrl+', Ctrl+Shift+E/I/R | `data/shortcutsrc.lightroom` |
| Setări | rotița mouse-ului derulează panourile; totul se poate opri din preferences (darkroom → „Lightroom Classic layout”, misc → „Lightroom-style sliders”) | `data/darktableconfig.xml.in`, `src/gui/gtk.c` |

## Ce urmează

- bara de unelte sub histogramă (Crop, Spot Removal, Masking)
- roți de culoare în Color Grading
- Tone Curve parametrică (Highlights / Lights / Darks / Shadows)
- butoane Copy… / Paste și Previous / Reset

## Build de test pe Linux (pentru dezvoltare)

Dependențele se construiesc în `/opt/dtdeps` (headere din surse GitHub, librării din sistem + AppImage-ul darktable), apoi:

```
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=/opt/dtdeps -DUSE_LUA=OFF -DUSE_OPENCL=OFF ...
ninja -C build install
```
