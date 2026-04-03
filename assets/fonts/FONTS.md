## Apr 2026

We're storing all of the static `.ttf` files for the font families used in the project, even if we don't actually make use of those styles or weights. When JUCE finishes adding support for the VariableFont versions of these typefaces, we'll remove these static versions and replace them with the single variable files instead (and update the code to match). 

Take care to add any styles/weights needed by adding that `ttf` file into the binary data blob `FontData` that we're using to hold all these assets inside the executable. 