#pragma once

#include <random>
#include <vector>
#include "Trader.h"

/* Type that keeps track of one seller's turn at the front of the market:
 * who is selling, the price they are asking, and which round it is.
 * The buyers' answers are kept on the traders themselves.
 */
class Sale {
public:
    /* Creates a sale for the trader at index seller, starting at a price. */
    Sale(int seller = -1, int ask = 0);

    /* Returns the index of the seller in the list of traders. */
    int seller() const;

    /* Returns the price being asked right now. */
    int ask() const;

    /* Returns the round, from 1 to kRoundsPerSale. */
    int round() const;

    /* Returns whether this is the last round of the sale. */
    bool isLastRound() const;

    /* Starts the next round at a new price. */
    void askAgain(int newAsk);

    /* Returns the buyer with the best answer, or -1 if everyone passed.
     * Anyone who took the price beats every counter; when several took it,
     * one of them is picked at random. Otherwise the highest counter wins.
     */
    int bestBuyer(const std::vector<Trader>& traders, std::mt19937& random) const;

private:
    int _seller;
    int _ask;
    int _round;
};
