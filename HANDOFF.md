# Predare: darktable „Lightroom Classic” — stare și ce mai e de făcut

Proiect: darktable 5.6.2 modificat ca să arate și să se comporte ca Lightroom Classic pe Mac (Apple Silicon, macOS 15+). Procesarea rămâne a lui darktable. Repo: `github.com/vvlad06-oss/darktable`.

## Ramuri

- `lightroom-ui`: ramura principală; fiecare push construiește pe CI un DMG publicat ca prerelease `lightroom-build-N` (`.github/workflows/lightroom-macos.yml`, ~21 min). Ultimul: **build-8** (commit `b0d19b7598`).
- `lightroom-ui-wip`: lucrul de după build-8 (Before/After, Tone Curve Region, Auto, Profile, migrarea scurtăturilor). **Testat de mână pe 19 oct. 2026** pe o copie a configurației din build-6: `\` Before/After (și cu Profile + curbă + expunere), Auto (un singur Cmd+Z), Profile (listă, aplicare, Undo), Region (Shadows / Highlights, Undo), Cmd+Shift+E (Export, „Add...”), Cmd+Shift+I (Import, Copy ↔ Add). Reparat la test: pe macOS, scurtătura veche `cmd+shift+i = copy & import` din `shortcutsrc` lua Cmd+Shift+I de la Import...; pasul 3 al migrării o șterge acum. Pasul următor: merge în `lightroom-ui` → build-9.

## Ce e făcut (vezi și LIGHTROOM.md)

- Basic ca în Lightroom: Treatment, Profile (presetările sigmoid), WB (As Shot / Daylight / … / Custom), Temp (scară log, pipetă), Tint (scara Adobe), Exposure 0.00 la redarea implicită, Contrast / Highlights / Shadows / Whites / Blacks, Texture (modul nou), Clarity, Dehaze, Vibrance, Saturation; buton **Auto** pe rândul Tone; antetul „Basic ▼” se pliază cu animație.
- Noise Reduction (Luminance / Detail / Color peste wavelets Y0U0V0), Sharpening (Amount / Radius / Masking), HSL cu nume Lightroom, Tone Curve cu **Region** (Highlights / Lights / Darks / Shadows).
- Histogramă RGB cu EXIF dedesubt; fără taburi de grupuri / căutare; fără bara Filter/Sort în Develop; bara de sub poză redusă; panoul stâng doar Navigator / Snapshots / History; fără Module Order; fără rândul de măști pe panourile Develop.
- Import / Export ca în Lightroom: butoane jos în Library, Cmd+Shift+I / Cmd+Shift+E din orice view; dialog „Import Photos” (Copy / Add, miniaturi, Apply During Import); dialog „Export N Files” (presetări + Add..., setările darktable).
- Încărcare: miniatura pozei + „Loading...”; „working...” mic; **Before / After** pe tasta `\` (eticheta „Before”).
- Migrări o singură dată, cheia `plugins/darkroom/lightroom_ui_version` (acum 3): panouri ascunse, histogramă, lățimi panouri, curățarea scurtăturilor vechi din `shortcutsrc`.
- Packaging: `gtk.immodules` nu mai trimite la GTK-ul din Homebrew (altfel aplicația se poate bloca pe Mac-uri cu Homebrew).

## Ce mai e de făcut

1. Merge `lightroom-ui-wip` în `lightroom-ui` și push → build-9.
2. De verificat vizual: eticheta „Loading...” (cu o singură poză în cache se încarcă prea repede ca să se vadă); „Custom” la Profile după o schimbare manuală în modulul sigmoid. Detaliu: dialogul de import urcă puțin pe ecran la fiecare comutare Copy ↔ Add.
3. Crash GTK la pornire văzut de 2 ori, doar cu ecranul blocat / imediat după deblocare (`gtk_css_node_validate`). Cu ecranul deblocat, 6 porniri la rând fără probleme. De urmărit.
4. Neimplementat încă: animația panourilor laterale (Tab) — ar forța reprocesarea pozei la fiecare cadru; Profile pentru workflow-urile filmic / AgX (acum doar sigmoid); presetări Lightroom built-in pentru export.

## Cum se compilează și se testează local

- Dependențe deja instalate pe Mac: Command Line Tools, Homebrew (`/opt/homebrew`), `.ci/Brewfile`, `gh` (logat ca vvlad06-oss).
- Build: `./build.sh --install --build-generator Ninja --build-type Release --prefix "$PWD/build/macosx" -- -DUSE_GRAPHICSMAGICK=OFF -DUSE_IMAGEMAGICK=ON -DBUILD_TESTING=OFF`
- Pachet `.app`: `packaging/macosx/3_make_hb_darktable_package.sh` — **calea repo-ului nu poate conține spații** (scriptul upstream se rupe).
- Pentru teste, copiază configurația (`~/.config/darktable` → alt folder, `--configdir`) și dă bundle-ului de test alt id (`org.darktable.lrtest`), ca să nu atingi biblioteca și aplicația instalată.
- Click-urile trimise în fundal nu ajung bine la GTK pe macOS (folosește poziția reală a cursorului); tastatura merge.
