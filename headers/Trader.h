#pragma once

#include <string>
#include "Days.h"

/* How a bot trades. Stubborn bots hold out for a better price, fair bots
 * meet in the middle, and nervous bots give in quickly.
 */
enum class Personality { Stubborn, Fair, Nervous };

/* A buyer's answer to the price on offer. */
enum class AnswerKind { Waiting, Take, Counter, Pass };

/* One answer: its kind, and the price offered when it is a counter. */
struct Answer {
    AnswerKind kind = AnswerKind::Waiting;
    int price = 0;
};

/* One trader in the market: you or a bot.
 *
 * secret is today's private number: what a crate is worth to a buyer, or
 * what it cost a seller. done turns true once the trader has traded or
 * given up for the day.
 */
struct Trader {
    std::string name;
    int colorIndex = 0;
    Personality personality = Personality::Fair;
    bool isYou = false;

    Role role = Role::Away;
    int secret = 0;
    bool done = false;
    Answer answer;

    int profitToday = 0;
    int totalProfit = 0;
};

/* Returns a bot buyer's answer to an asking price in a given round
 * (1 to kRoundsPerSale). A bot never offers more than the crate is worth.
 */
Answer buyerAnswer(const Trader& buyer, int ask, int round);

/* Returns the first price a bot seller asks for. */
int openingAsk(const Trader& seller);

/* Returns a bot seller's next, lower price after nobody met its ask.
 * Stubborn sellers drop $1, fair sellers $2, and nervous sellers $3,
 * but never down to their cost.
 */
int loweredAsk(const Trader& seller, int ask);

/* Returns whether a bot seller accepts an offer in a given round. A bot
 * never sells at a loss, and it asks for less profit as rounds go by.
 */
bool acceptsOffer(const Trader& seller, int offer, int round);

/* Returns the profit a trader makes by trading at a price. */
int profitFrom(const Trader& trader, int price);
