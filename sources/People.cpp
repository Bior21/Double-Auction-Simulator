/*
 * File: People.cpp
 * Author: Bior Madol Khom Bior
 *
 * This file draws the market's characters and what they say. Every
 * character is made of circles: a round body, a round head, two dot eyes,
 * and a mouth whose shape shows the mood. Speech bubbles are rounded boxes
 * with a small triangle tail.
 */
#include "People.h"
#include "Settings.h"
using namespace std;

/* Draws a character, sized by the radius of its head. */
void drawPerson(Vector2 head, float radius, Color color, Mood mood) {
    /* Body, then head, then a soft highlight on the head. */
    DrawEllipse((int) head.x, (int) (head.y + radius * 1.35f), radius * 1.05f, radius * 0.85f, color);
    DrawCircleV(head, radius, color);
    DrawCircleV(head, radius * 0.82f, Fade(kWhite, 0.18f));

    /* Eyes. */
    float eyeY = head.y - radius * 0.08f;
    DrawCircleV({head.x - radius * 0.3f, eyeY}, radius * 0.12f, kInk);
    DrawCircleV({head.x + radius * 0.3f, eyeY}, radius * 0.12f, kInk);

    /* Mouth: a curve up for happy, a line for flat, a curve down for grumpy. */
    float mouthY = head.y + radius * 0.32f;
    float half = radius * 0.32f;
    float thick = radius * 0.09f < 2 ? 2 : radius * 0.09f;
    if (mood == Mood::Happy) {
        DrawSplineSegmentBezierQuadratic({head.x - half, mouthY}, {head.x, mouthY + radius * 0.3f},
                                         {head.x + half, mouthY}, thick, kInk);
    } else if (mood == Mood::Grumpy) {
        DrawSplineSegmentBezierQuadratic({head.x - half, mouthY + radius * 0.18f}, {head.x, mouthY - radius * 0.08f},
                                         {head.x + half, mouthY + radius * 0.18f}, thick, kInk);
    } else if (mood == Mood::Flat) {
        DrawLineEx({head.x - half * 0.8f, mouthY + 3}, {head.x + half * 0.8f, mouthY + 3}, thick, kInk);
    } else {
        DrawCircleV({head.x, mouthY + 4}, radius * 0.1f, kInk);   // a small "o" while thinking
    }
}

/* Draws a bubble that fits its text, with a tail pointing down. */
void drawBubble(const Font& font, const string& text, float centerX, float top, float size,
                Color fill, Color border, Color textColor) {
    Vector2 measured = MeasureTextEx(font, text.c_str(), size, 0);
    float width = measured.x + size * 1.4f;
    if (width < size * 2.8f) width = size * 2.8f;   // short words still get a round bubble
    float height = size + 22;
    Rectangle box = {centerX - width / 2, top, width, height};

    /* The tail is drawn first so the box covers its top edge. */
    Vector2 left = {centerX - 8, top + height - 2};
    Vector2 tip = {centerX, top + height + 10};
    Vector2 right = {centerX + 8, top + height - 2};
    DrawTriangle(left, tip, right, border);
    DrawTriangle({left.x + 2.5f, left.y}, {tip.x, tip.y - 3.5f}, {right.x - 2.5f, right.y}, fill);

    DrawRectangleRounded(box, 1.0f, 12, fill);
    DrawRectangleRoundedLinesEx(box, 1.0f, 12, 2, border);
    DrawRectangle((int) (centerX - 6), (int) (top + height - 3), 12, 3, fill);
    DrawTextEx(font, text.c_str(), {centerX - measured.x / 2, top + (height - measured.y) / 2}, size, 0, textColor);
}

/* Draws a ring that shrinks clockwise, with the seconds left inside. */
void drawCountdown(const Font& font, Vector2 center, float fraction, int secondsLeft) {
    DrawRing(center, 15, 20, 0, 360, 48, kLine);
    DrawRing(center, 15, 20, -90, -90 + 360 * fraction, 48, kInk);
    string text = to_string(secondsLeft) + "s";
    Vector2 measured = MeasureTextEx(font, text.c_str(), 15, 0);
    DrawTextEx(font, text.c_str(), {center.x - measured.x / 2, center.y - measured.y / 2}, 15, 0, kInk);
}
