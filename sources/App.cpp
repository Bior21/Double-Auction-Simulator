/*
 * File: App.cpp
 * Author: Bior Madol Khom Bior
 *
 * This file implements the App class, which connects the player to the
 * market. Every frame it reads the mouse and keyboard, moves the market's
 * clock forward (unless the help is open), and draws the screen again.
 *
 * Keyboard: Left and Right change your price, Enter presses the first
 * button that can be clicked (Take, Ask, Accept, or Next day), and
 * H opens or closes the help.
 */
#include "App.h"
#include <ctime>
#include "Layout.h"
#include "Settings.h"
using namespace std;

/* Opens the window and starts a week with a seed from the clock, so every
 * week deals different secret numbers.
 */
App::App() : _market((unsigned int) time(nullptr)) {
    SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT);
    InitWindow(kWindowWidth, kWindowHeight, "Double Auction Simulator");
    SetTargetFPS(60);
    _fonts = loadFonts();
    _helpOpen = false;
}

/* Frees the fonts, then closes the window. */
App::~App() {
    unloadFonts(_fonts);
    CloseWindow();
}

/* The main loop: handle input, move time forward, then draw. */
void App::run() {
    while (!WindowShouldClose()) {
        _buttons = buttonsFor(_market, _helpOpen);
        handleInput();
        if (!_helpOpen) {
            _market.update(GetFrameTime());
        }

        _buttons = buttonsFor(_market, _helpOpen);
        BeginDrawing();
        drawScreen(_market, _fonts, _buttons, _helpOpen);
        EndDrawing();
    }
}

/* Clicks go to buttons. The keyboard changes your price or presses a button. */
void App::handleInput() {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse = GetMousePosition();
        for (const Button& button : _buttons) {
            if (button.enabled && CheckCollisionPointRec(mouse, button.rect)) {
                perform(button.action);
                return;
            }
        }
    }

    if (IsKeyPressed(KEY_H)) _helpOpen = !_helpOpen;
    if (_helpOpen) return;
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) _market.changeYourPrice(-1);
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) _market.changeYourPrice(1);

    /* Enter presses the first useful button: Take, Ask, Accept, Skip, or Next day. */
    if (IsKeyPressed(KEY_ENTER)) {
        for (const Button& button : _buttons) {
            bool useful = button.style == ButtonStyle::Green || button.style == ButtonStyle::Dark ||
                          button.action == Action::Ask || button.action == Action::Skip;
            if (button.enabled && useful) {
                perform(button.action);
                return;
            }
        }
    }
}

/* Does what a button says. */
void App::perform(Action action) {
    switch (action) {
        case Action::Take:      _market.take(); break;
        case Action::Counter:   _market.counter(); break;
        case Action::Pass:      _market.pass(); break;
        case Action::PriceDown: _market.changeYourPrice(-1); break;
        case Action::PriceUp:   _market.changeYourPrice(1); break;
        case Action::Ask:       _market.askAtYourPrice(); break;
        case Action::Accept:    _market.acceptBestOffer(); break;
        case Action::KeepCrate: _market.keepCrate(); break;
        case Action::Skip:      _market.skipToEndOfDay(); break;
        case Action::NextDay:   _market.nextDay(); break;
        case Action::NewWeek:   _market.newWeek(); break;
        case Action::OpenHelp:  _helpOpen = true; break;
        case Action::CloseHelp: _helpOpen = false; break;
    }
}
