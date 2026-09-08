## Sep 2026

Migrated from per-weight/style static `.ttf` files to a single variable font file per family, now that JUCE 9 supports variable fonts. Each family's `static/` subfolder has been removed; the `OFL.txt`/`README.txt` license files remain.

- `Fira_Code/FiraCode-VariableFont_wght.ttf` — `wght` axis only.
- `IBM_Plex_Sans/IBMPlexSans-VariableFont_wdth,wght.ttf` — `wdth` and `wght` axes.

Both files are compiled into the `FontData` binary data blob (see `CMakeLists.txt`). `_10PinLookAndFeel::updateFonts()` (`src/view/lookAndFeel.cpp`) loads each variable typeface once, then calls `Typeface::cloneWithVariableSettings()` to pick the specific weight/width instance used for labels and values — that's the place to change if a new weight/width is needed, no new font file required.

## Apr 2026 (superseded)

We're storing all of the static `.ttf` files for the font families used in the project, even if we don't actually make use of those styles or weights. When JUCE finishes adding support for the VariableFont versions of these typefaces, we'll remove these static versions and replace them with the single variable files instead (and update the code to match). 

Take care to add any styles/weights needed by adding that `ttf` file into the binary data blob `FontData` that we're using to hold all these assets inside the executable. 