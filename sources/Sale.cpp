/*
 * File: Sale.cpp
 * Author: Bior Madol Khom Bior
 *
 * This file implements the Sale class. A sale lasts up to three rounds. In
 * each round the seller names a price, every buyer answers at the same
 * moment, and the sale picks the best answer: someone who took the price,
 * or else the highest counter.
 */
#include "Sale.h"
#include "Settings.h"
using namespace std;

/* Starts a sale in round 1. */
Sale::Sale(int seller, int ask) {
    _seller = seller;
    _ask = ask;
    _round = 1;
}

/* Returns the seller's index. */
int Sale::seller() const {
    return _seller;
}

/* Returns the current asking price. */
int Sale::ask() const {
    return _ask;
}

/* Returns the current round. */
int Sale::round() const {
    return _round;
}

/* Returns whether no more rounds are left after this one. */
bool Sale::isLastRound() const {
    return _round >= kRoundsPerSale;
}

/* Moves to the next round at a new price. */
void Sale::askAgain(int newAsk) {
    _ask = newAsk;
    _round++;
}

/* Looks at every buyer still in the market and finds the best answer. */
int Sale::bestBuyer(const vector<Trader>& traders, mt19937& random) const {
    vector<int> takers;
    int bestCounter = -1;
    for (int index = 0; index < (int) traders.size(); index++) {
        const Trader& trader = traders[index];
        if (trader.role != Role::Buyer || trader.done) continue;

        if (trader.answer.kind == AnswerKind::Take) {
            takers.push_back(index);
        } else if (trader.answer.kind == AnswerKind::Counter) {
            if (bestCounter < 0 || trader.answer.price > traders[bestCounter].answer.price) {
                bestCounter = index;
            }
        }
    }

    /* Several buyers took the price at the same moment, so pick one fairly. */
    if (!takers.empty()) {
        uniform_int_distribution<int> pick(0, (int) takers.size() - 1);
        return takers[pick(random)];
    }
    return bestCounter;
}

/* * * * * Test Cases Below This Point * * * * */
#include "SimpleTest.h"

/* Returns three buyers who gave the three answers. */
static vector<Trader> buyersWhoAnswered(Answer first, Answer second, Answer third) {
    vector<Trader> traders(3);
    Answer answers[] = {first, second, third};
    for (int index = 0; index < 3; index++) {
        traders[index].role = Role::Buyer;
        traders[index].answer = answers[index];
    }
    return traders;
}

TEST("the highest counter wins when nobody takes the price") {
    mt19937 random(1);
    vector<Trader> traders = buyersWhoAnswered({AnswerKind::Counter, 62}, {AnswerKind::Pass, 0},
                                               {AnswerKind::Counter, 64});
    Sale sale(0, 68);
    EXPECT_EQUAL(sale.bestBuyer(traders, random), 2);
}

TEST("taking the price beats any counter") {
    mt19937 random(1);
    vector<Trader> traders = buyersWhoAnswered({AnswerKind::Counter, 67}, {AnswerKind::Take, 68},
                                               {AnswerKind::Pass, 0});
    Sale sale(0, 68);
    EXPECT_EQUAL(sale.bestBuyer(traders, random), 1);
}

TEST("nobody wins when everyone passes, and buyers who are done are skipped") {
    mt19937 random(1);
    vector<Trader> traders = buyersWhoAnswered({AnswerKind::Pass, 0}, {AnswerKind::Pass, 0},
                                               {AnswerKind::Take, 68});
    traders[2].done = true;
    Sale sale(0, 68);
    EXPECT_EQUAL(sale.bestBuyer(traders, random), -1);
}

TEST("a sale lasts three rounds") {
    Sale sale(0, 70);
    EXPECT(!sale.isLastRound());
    sale.askAgain(68);
    sale.askAgain(66);
    EXPECT_EQUAL(sale.round(), 3);
    EXPECT_EQUAL(sale.ask(), 66);
    EXPECT(sale.isLastRound());
}
