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
| Bara de unelte | sub histogramă: Crop, Remove (Spot Removal), Transform, Masking | `src/libs/modulegroups.c` |
| Masking | Brush, Linear Gradient, Radial Gradient, Select Object (AI): creează o ajustare locală (Exposure cu mască) afișată sus în coloana Develop, cu unealta de desenat deja activă | `src/libs/modulegroups.c` |
| Color Grading | roți Shadows / Midtones / Highlights / Global (nuanță + intensitate), dublu-click = reset | `src/iop/colorbalancergb.c` |
| Copy… / Paste | jos în panoul stâng; Copy… deschide dialogul cu setările deja bifate | `src/libs/lr_copy_paste.c`, `src/gui/hist_dialog.c` |
| Previous / Reset | jos în panoul drept; Previous = setările pozei editate anterior, Reset = setări implicite (Ctrl+Z le aduce înapoi) | `src/libs/lr_previous_reset.c` |
| Valori pornire | Shadows, Highlights, Clarity, Dehaze pornesc de la 0, ca în Lightroom; Clarity e -100…+100 | `src/iop/shadhi.c`, `bilat.c`, `hazeremoval.c` |
| Setări | rotița mouse-ului derulează panourile; totul se poate opri din preferences (darkroom → „Lightroom Classic layout”, misc → „Lightroom-style sliders”) | `data/darktableconfig.xml.in`, `src/gui/gtk.c` |
| Treatment și White Balance | Treatment Color / Black & White; WB As Shot, Daylight, Cloudy, Shade, Tungsten, Fluorescent, Flash, Custom; Temp (scară logaritmică, pistă albastru→galben, pipetă) și Tint (scara Adobe, verde→magenta) pentru orice iluminant; dublu-click = As Shot. Totul comandă iluminantul CAT din color calibration | `src/iop/channelmixerrgb.c` |
| Valori ca în Lightroom | Exposure „0.00” la redarea implicită (+0.7 EV ascuns), Contrast (color balance rgb, 0 = neutru), Whites, Blacks (inversat, 0 la nivelul implicit), Dehaze, Vibrance, Saturation, Clarity: -100…+100 fără unități; zero se scrie „0” fără semn | `src/iop/exposure.c`, `colorbalancergb.c`, `hazeremoval.c`, `bilat.c`, `src/bauhaus/bauhaus.c` |
| Texture | modul nou de procesare: contrastul detaliilor medii (în EV, între zgomot și Clarity), -100 netezește, +100 accentuează | `src/iop/texture.c`, `src/common/iop_order.c` |
| Noise Reduction | Luminance / Detail / Color (0 / 50 / 25 la pornire) peste wavelets Y0U0V0; opțiunile darktable rămân pliate în „darktable controls” | `src/iop/denoiseprofile.c` |
| Sharpening | Amount 0…150 (40), Radius 0.5…3.0 (1.0), Masking | `src/iop/sharpen.c` |
| HSL / Color | slidere Red, Orange, Yellow, Green, Aqua, Blue, Purple, Magenta, vizibile implicit | `src/iop/colorequal.c` |
| Histogramă | RGB liniară, cu ISO, mm, f/ și timpul de expunere dedesubt | `src/libs/histogram.c` |
| Coloana dreaptă | fără taburi de grupuri și fără căutare (scurtătura „search modules” o aduce înapoi); antetul „Basic ▼” pliază panoul | `src/libs/modulegroups.c` |
| Import | „Import...” jos în stânga în Library (și Cmd+Shift+I din orice view): dialog „Import Photos” cu Copy / Add sus, miniaturi, Check All / Uncheck All / Check New, iar în dreapta „Apply During Import” (rating, metadate) și, la Copy, destinația și redenumirea | `src/libs/import.c` |
| Export | „Export...” lângă Import (și Cmd+Shift+E, inclusiv din Develop): dialog modal „Export N Files” cu presetările de export în stânga și toate setările de export în dreapta | `src/libs/export.c`, `src/libs/lib.h` |
| Încărcare și animații | la deschiderea unei poze apare imediat miniatura ei, cu „Loading...” jos; „working...” e o etichetă mică; Basic se pliază cu animație; butoanele histogramei apar doar la hover | `src/views/darkroom.c`, `src/control/control.c`, `src/libs/modulegroups.c`, `src/libs/histogram.c` |
| Panouri fără măști | panourile din coloana Develop nu mai arată rândul de măști darktable (ajustările locale din Masking îl păstrează) | `src/develop/blend_gui.c` |
| Develop | fără bara Filter / Sort deasupra pozei (Preferences: meniul darktable); sub poză doar clipping, soft proofing, guides, second window; stânga fără Color Picker, Virtual Copies, Keywording, Info, Masking, Export; fără Module Order | `src/views/darkroom.c`, `src/common/darktable.c` (pasul 2 al configurării, `plugins/darkroom/lightroom_ui_version`) |

## Ce nu are echivalent (deocamdată)

- Tone Curve parametrică (Highlights / Lights / Darks / Shadows): Tone Curve are curba pe puncte și canale R/G/B; varianta parametrică ar cere un modul nou de procesare.
- Generative Remove, Lens Blur, Panorama Merge, măști automate Sky / People. (HDR Merge există: Library → panoul Selected Images → „Create HDR”.)
- AI Denoise: există „Enhance (AI)” când build-ul are AI; altfel Noise Reduction clasic.

## Build de test pe Linux (pentru dezvoltare)

Dependențele se construiesc în `/opt/dtdeps` (headere din surse GitHub, librării din sistem + AppImage-ul darktable), apoi:

```
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=/opt/dtdeps -DUSE_LUA=OFF -DUSE_OPENCL=OFF ...
ninja -C build install
```
