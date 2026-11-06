#pragma once

#include <string>
#include "raylib.h"

/* Everything a button can do when it is clicked. */
enum class Action {
    Take,
    Counter,
    Pass,
    PriceDown,
    PriceUp,
    Ask,
    Accept,
    KeepCrate,
    Skip,
    NextDay,
    NewWeek,
    OpenHelp,
    CloseHelp
};

/* What a button looks like. */
enum class ButtonStyle { Green, Blue, Plain, Dark, Round };

/* One button on the screen: its label, where it is, what it does, how it
 * looks, and whether it can be clicked right now.
 */
struct Button {
    std::string label;
    Rectangle rect;
    Action action;
    ButtonStyle style;
    bool enabled;
};
