#pragma once

#include <vector>
#include "Button.h"
#include "Drawing.h"
#include "Market.h"

/* Type that runs Double Auction Simulator in a window. It opens the window, turns mouse
 * clicks and key presses into market moves, moves the market's clock
 * forward, and redraws the screen sixty times a second.
 */
class App {
public:
    /* Opens the window, loads the fonts, and starts a new week. */
    App();

    /* Frees the fonts and closes the window. */
    ~App();

    /* Runs the game until the player closes the window. */
    void run();

private:
    Market _market;
    Fonts _fonts;
    std::vector<Button> _buttons;
    bool _helpOpen;

    /* Reacts to the mouse and keyboard for one frame. */
    void handleInput();

    /* Does whatever a button's action says. */
    void perform(Action action);
};
