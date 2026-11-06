#pragma once

#include <vector>
#include "Button.h"
#include "Market.h"
#include "raylib.h"

/* The fonts the game uses: Baloo 2 for big, friendly words and numbers,
 * and Inter for everything else.
 */
struct Fonts {
    Font fun;
    Font text;
    Font textBold;
};

/* Loads the fonts from the fonts folder next to the program. A missing
 * font file is replaced with raylib's built-in font.
 */
Fonts loadFonts();

/* Frees the memory used by the fonts. */
void unloadFonts(Fonts& fonts);

/* Draws the whole screen for the market's current state. */
void drawScreen(const Market& market, const Fonts& fonts, const std::vector<Button>& buttons, bool helpOpen);
