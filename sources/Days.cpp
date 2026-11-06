/*
 * File: Days.cpp
 * Author: Bior Madol Khom Bior
 *
 * This file sets up the seven days of the market week. You buy on odd days
 * and sell on even days. Each day after the first two changes the market a
 * little: more buyers, more sellers, fewer sellers, less time to think, or
 * a crowd on both sides.
 */
#include "Days.h"
#include "Settings.h"
using namespace std;

/* Returns the settings for one day of the week. */
DaySettings settingsForDay(int day) {
    if (day < 1) day = 1;
    if (day > kDayCount) day = kDayCount;

    /* Twist, your role, sellers, buyers, seconds to answer. */
    const DaySettings kWeek[kDayCount] = {
        {"Calm market",               Role::Buyer,  5, 5, 5},
        {"Calm market",               Role::Seller, 5, 5, 5},
        {"Rainy: more buyers",        Role::Buyer,  5, 7, 5},
        {"Glut: more sellers",        Role::Seller, 7, 5, 5},
        {"Shortage: fewer sellers",   Role::Buyer,  3, 5, 5},
        {"Rush hour: 3 seconds",      Role::Seller, 5, 5, 3},
        {"Busy market",               Role::Buyer,  6, 7, 5},
    };
    return kWeek[day - 1];
}

/* * * * * Test Cases Below This Point * * * * */
#include "SimpleTest.h"

TEST("you buy on odd days and sell on even days") {
    for (int day = 1; day <= kDayCount; day++) {
        Role expected = (day % 2 == 1) ? Role::Buyer : Role::Seller;
        EXPECT(settingsForDay(day).yourRole == expected);
    }
}

TEST("rain adds buyers, a glut adds sellers, and a shortage removes sellers") {
    EXPECT(settingsForDay(3).buyers > settingsForDay(1).buyers);
    EXPECT(settingsForDay(4).sellers > settingsForDay(2).sellers);
    EXPECT(settingsForDay(5).sellers < settingsForDay(1).sellers);
}

TEST("rush hour gives 3 seconds to answer") {
    EXPECT_EQUAL(settingsForDay(6).answerSeconds, 3.0f);
    EXPECT_EQUAL(settingsForDay(1).answerSeconds, 5.0f);
}

TEST("every day has enough bots to fill the market") {
    for (int day = 1; day <= kDayCount; day++) {
        DaySettings settings = settingsForDay(day);
        EXPECT(settings.sellers + settings.buyers - 1 <= kBotCount);
    }
}
