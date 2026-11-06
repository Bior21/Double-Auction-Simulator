#pragma once

#include <vector>
#include "Button.h"
#include "Market.h"
#include "raylib.h"

/* Returns the traders who are buying today, in the order they stand in the
 * row. On buyer days you stand in the middle.
 */
std::vector<int> buyerRow(const Market& market);

/* Returns where the head of the buyer at place index stands, when count
 * buyers share the row.
 */
Vector2 buyerSpot(int index, int count);

/* Returns the buttons that belong on the screen right now. */
std::vector<Button> buttonsFor(const Market& market, bool helpOpen);
