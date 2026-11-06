#pragma once

#include <string>

/* What a trader does today: buy, sell, or stay home. */
enum class Role { Buyer, Seller, Away };

/* How one day of the week is set up: its twist, your role, how many
 * sellers and buyers come to the market (you included), and how many
 * seconds buyers get to answer each price.
 */
struct DaySettings {
    std::string twist;
    Role yourRole;
    int sellers;
    int buyers;
    float answerSeconds;
};

/* Returns the settings for a day, from 1 to kDayCount. Days outside that
 * range use the nearest real day.
 */
DaySettings settingsForDay(int day);
