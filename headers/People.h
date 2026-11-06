#pragma once

#include <string>
#include "raylib.h"

/* How a character's face looks. */
enum class Mood { Happy, Flat, Grumpy, Thinking };

/* Draws a round character: a body, a head, two eyes, and a mouth that
 * shows its mood. radius is the size of the head.
 */
void drawPerson(Vector2 head, float radius, Color color, Mood mood);

/* Draws a speech bubble centered on centerX with its top at top, and a
 * small tail pointing down at whoever is speaking.
 */
void drawBubble(const Font& font, const std::string& text, float centerX, float top, float size,
                Color fill, Color border, Color textColor);

/* Draws a countdown ring that empties as time runs out. fraction goes
 * from 1 (full) to 0 (empty).
 */
void drawCountdown(const Font& font, Vector2 center, float fraction, int secondsLeft);
