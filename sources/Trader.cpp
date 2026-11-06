/*
 * File: Trader.cpp
 * Author: Bior Madol Khom Bior
 *
 * This file decides what the bots do. Every decision starts from the bot's
 * secret number and its personality. A buyer looks at how much profit a
 * price would leave it; a seller looks at how much profit an offer would
 * give it. Both want less profit as the rounds go by, so deals get easier
 * to make the longer a sale goes on.
 */
#include "Trader.h"
#include "Settings.h"
using namespace std;

/* Returns the profit a bot buyer wants to keep before it simply takes a price. */
static int profitBuyerKeeps(Personality personality, int round) {
    int wanted = 5;
    if (personality == Personality::Stubborn) wanted = 8;
    if (personality == Personality::Nervous) wanted = 2;
    wanted -= (round - 1) * 2;
    return wanted < 0 ? 0 : wanted;
}

/* Returns how far below its value a bot buyer counters. */
static int counterBelowValue(Personality personality, int round) {
    int gap = 9;
    if (personality == Personality::Stubborn) gap = 16;
    if (personality == Personality::Nervous) gap = 5;
    gap -= (round - 1) * 3;
    return gap < 1 ? 1 : gap;
}

/* Returns the profit a bot seller wants before it accepts an offer. */
static int profitSellerWants(Personality personality, int round) {
    int wanted = 7;
    if (personality == Personality::Stubborn) wanted = 12;
    if (personality == Personality::Nervous) wanted = 4;
    wanted -= (round - 1) * 3;
    return wanted < 1 ? 1 : wanted;
}

/* Takes a good price, counters a high one, and passes on a price far too high. */
Answer buyerAnswer(const Trader& buyer, int ask, int round) {
    Answer answer;
    if (ask <= buyer.secret - profitBuyerKeeps(buyer.personality, round)) {
        answer.kind = AnswerKind::Take;
        answer.price = ask;
        return answer;
    }

    int counter = buyer.secret - counterBelowValue(buyer.personality, round);
    if (ask - buyer.secret > 20 || counter < kLowestPrice) {
        answer.kind = AnswerKind::Pass;
        return answer;
    }
    answer.kind = AnswerKind::Counter;
    answer.price = counter;
    return answer;
}

/* Starts high: the more stubborn the seller, the higher the first price. */
int openingAsk(const Trader& seller) {
    int markup = 17;
    if (seller.personality == Personality::Stubborn) markup = 24;
    if (seller.personality == Personality::Nervous) markup = 11;
    return seller.secret + markup;
}

/* Drops the price a little, by personality, but stays above cost. */
int loweredAsk(const Trader& seller, int ask) {
    int drop = 2;
    if (seller.personality == Personality::Stubborn) drop = 1;
    if (seller.personality == Personality::Nervous) drop = 3;
    int lowered = ask - drop;
    return lowered <= seller.secret ? seller.secret + 1 : lowered;
}

/* Accepts an offer that leaves enough profit for this round. */
bool acceptsOffer(const Trader& seller, int offer, int round) {
    return offer >= seller.secret + profitSellerWants(seller.personality, round);
}

/* Buyers profit by paying less than their value; sellers by charging more than their cost. */
int profitFrom(const Trader& trader, int price) {
    if (trader.role == Role::Buyer) return trader.secret - price;
    return price - trader.secret;
}

/* * * * * Test Cases Below This Point * * * * */
#include "SimpleTest.h"

/* Returns a trader with a role, a secret number, and a personality. */
static Trader makeTrader(Role role, int secret, Personality personality) {
    Trader trader;
    trader.role = role;
    trader.secret = secret;
    trader.personality = personality;
    return trader;
}

TEST("a bot buyer takes a cheap price, counters a high one, and passes on a silly one") {
    Trader buyer = makeTrader(Role::Buyer, 70, Personality::Fair);
    EXPECT(buyerAnswer(buyer, 60, 1).kind == AnswerKind::Take);

    Answer counter = buyerAnswer(buyer, 72, 1);
    EXPECT(counter.kind == AnswerKind::Counter);
    EXPECT(counter.price < 72);

    EXPECT(buyerAnswer(buyer, 95, 1).kind == AnswerKind::Pass);
}

TEST("a bot buyer never offers more than the crate is worth to it") {
    Personality kinds[] = {Personality::Stubborn, Personality::Fair, Personality::Nervous};
    for (Personality personality : kinds) {
        Trader buyer = makeTrader(Role::Buyer, 64, personality);
        for (int ask = 40; ask <= 100; ask++) {
            for (int round = 1; round <= kRoundsPerSale; round++) {
                Answer answer = buyerAnswer(buyer, ask, round);
                if (answer.kind != AnswerKind::Pass) EXPECT(answer.price <= 64);
            }
        }
    }
}

TEST("sellers drop $1, $2, or $3 by personality, but never to their cost") {
    EXPECT_EQUAL(loweredAsk(makeTrader(Role::Seller, 50, Personality::Stubborn), 70), 69);
    EXPECT_EQUAL(loweredAsk(makeTrader(Role::Seller, 50, Personality::Fair), 70), 68);
    EXPECT_EQUAL(loweredAsk(makeTrader(Role::Seller, 50, Personality::Nervous), 70), 67);
    EXPECT_EQUAL(loweredAsk(makeTrader(Role::Seller, 50, Personality::Nervous), 52), 51);
}

TEST("a bot seller never accepts a loss and asks for less as rounds go by") {
    Trader seller = makeTrader(Role::Seller, 55, Personality::Fair);
    EXPECT(!acceptsOffer(seller, 54, 3));
    EXPECT(!acceptsOffer(seller, 59, 1));
    EXPECT(acceptsOffer(seller, 59, 2));
    EXPECT(openingAsk(seller) > 55);
}

TEST("profit is value minus price for buyers and price minus cost for sellers") {
    EXPECT_EQUAL(profitFrom(makeTrader(Role::Buyer, 70, Personality::Fair), 64), 6);
    EXPECT_EQUAL(profitFrom(makeTrader(Role::Seller, 55, Personality::Fair), 64), 9);
}
