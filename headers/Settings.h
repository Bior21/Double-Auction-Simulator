#pragma once

#include "raylib.h"

/* Every number and color the game uses, kept in one place so they are
 * easy to find and change.
 */

/* Window size, in pixels. */
const int kWindowWidth  = 960;
const int kWindowHeight = 720;

/* The week. */
const int kDayCount      = 7;
const int kRoundsPerSale = 3;     // a seller asks at most three times
const int kBotCount      = 12;    // traders other than you

/* Secret numbers are drawn from these ranges each day. */
const int kLowestValue = 55;      // what a crate is worth to a buyer
const int kHighestValue = 85;
const int kLowestCost = 45;       // what a crate cost a seller
const int kHighestCost = 75;
const int kYourLowestValue = 66;  // your own card stays in the middle of the market
const int kYourHighestValue = 78;
const int kYourLowestCost = 52;
const int kYourHighestCost = 62;

/* Prices you can choose with the − and + buttons. */
const int kLowestPrice = 30;
const int kHighestPrice = 110;

/* How long each step of a sale stays on screen, in seconds. */
const float kAskSeconds          = 1.1f;   // the seller names a price
const float kWatchAnswerSeconds  = 1.0f;   // bots answering while you only watch
const float kRevealSeconds       = 1.5f;   // everyone's answers are shown
const float kResultSeconds       = 1.5f;   // "Sold to Amina, $64"

/* Screen colors. */
const Color kInk      = { 28,  36,  48, 255};
const Color kMuted    = {118, 128, 141, 255};
const Color kLine     = {232, 235, 239, 255};
const Color kWhite    = {255, 255, 255, 255};
const Color kPaper    = {251, 250, 247, 255};
const Color kStage    = {243, 236, 224, 255};
const Color kCounter  = {227, 214, 193, 255};
const Color kYouBlue  = { 47, 111, 237, 255};
const Color kYouSoft  = {234, 241, 255, 255};
const Color kGood     = { 22, 163,  74, 255};
const Color kGoodSoft = {233, 248, 238, 255};
const Color kBad      = {224,  83,  61, 255};
const Color kSun      = {255, 201,  60, 255};
const Color kShade    = { 20,  26,  34, 170};

/* One color for each bot, in the same order as their names in Market.cpp. */
const Color kBotColors[kBotCount] = {
    {232, 106, 146, 255}, { 91, 179, 169, 255}, {239, 125,  87, 255}, {155, 126, 222, 255},
    {242, 165,  65, 255}, {108, 155, 210, 255}, {140, 194, 101, 255}, {201, 162,  39, 255},
    {181, 138, 214, 255}, { 77, 166, 120, 255}, {222, 120, 120, 255}, {110, 132, 160, 255},
};
