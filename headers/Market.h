#pragma once

#include <random>
#include <string>
#include <vector>
#include "Days.h"
#include "Sale.h"
#include "Trader.h"

/* The step a sale is on. A sale goes Asking → Answering → Revealing, then
 * either ends in a Result or goes back to Asking at a lower price.
 * Deciding is only used when you are the seller and must choose.
 */
enum class Phase { Asking, Answering, Revealing, Deciding, Result, DayOver, WeekOver };

/* One finished deal: who sold, who bought, and the price. */
struct Deal {
    std::string seller;
    std::string buyer;
    int price;
};

/* Type that runs the market week: it deals each day's secret numbers,
 * brings sellers to the front one at a time, collects every buyer's
 * answer, makes deals, and keeps everyone's profit.
 *
 * You are always trader 0. The other traders are bots.
 */
class Market {
public:
    /* Creates a market on day 1. The seed decides every secret number,
     * so the same seed always plays the same week.
     */
    Market(unsigned int seed);

    /* Moves time forward. Each step of a sale lasts a set number of
     * seconds, except when the market is waiting for you to choose.
     */
    void update(float seconds);

    /* Your moves as a buyer: take the price, counter at yourPrice(), or pass. */
    bool canAnswer() const;
    void take();
    void counter();
    void pass();

    /* Your moves as a seller: name your price, then accept the best offer,
     * ask again at yourPrice(), or keep your crate.
     */
    bool isYourTurnToAsk() const;
    bool isYourTurnToDecide() const;
    void askAtYourPrice();
    void acceptBestOffer();
    void keepCrate();

    /* The price you are choosing with the − and + buttons. */
    int yourPrice() const;
    void changeYourPrice(int amount);

    /* Returns whether you have traded or given up for today. */
    bool youAreDone() const;

    /* Plays the rest of the day at once. Only works once you are done. */
    void skipToEndOfDay();

    /* After a day: starts the next day, or ends the week after day 7. */
    void nextDay();

    /* Starts a whole new week with everyone back at $0. */
    void newWeek();

    /* Information for the screen and the tests. */
    Phase phase() const;
    int day() const;
    const DaySettings& settings() const;
    const std::vector<Trader>& traders() const;
    const Trader& you() const;
    const Sale& sale() const;
    int saleNumber() const;
    int saleCount() const;
    std::vector<int> sellersStillWaiting() const;
    const std::vector<Deal>& deals() const;
    std::string sellerSays() const;
    int bestBuyer() const;
    float timeLeft() const;
    float timeGiven() const;
    int fairPrice() const;
    std::vector<int> standings() const;
    int yourRank() const;

private:
    std::mt19937 _random;
    std::vector<Trader> _traders;
    int _day;
    DaySettings _settings;
    Phase _phase;

    /* The sellers in the order they come to the front, and whose turn it is. */
    std::vector<int> _sellerLine;
    int _saleIndex;
    Sale _sale;
    bool _saleOver;
    bool _yourAskPosted;
    int _bestBuyer;

    float _timer;
    float _timeGiven;
    int _yourPrice;
    std::string _sellerSays;
    std::vector<Deal> _deals;
    int _fairPrice;

    /* Creates you and the twelve bots. */
    void makeTraders();

    /* Picks today's buyers and sellers, deals secret numbers, and lines up the sellers. */
    void startDay();

    /* Brings the next seller to the front, or ends the day when none are left. */
    void startNextSale();

    /* Opens the answering step and lets every bot buyer choose its answer. */
    void startAnswering();

    /* Shows everyone's answers. */
    void revealAnswers();

    /* After the reveal: makes a deal, lowers the price, or waits for you. */
    void decide();

    /* Makes a deal between the seller and a buyer at a price. */
    void makeDeal(int buyer, int price);

    /* Ends the sale without a deal. */
    void sellerLeaves();

    /* Ends the day and works out the fair price. */
    void endDay();

    /* Sets a timer for the current step. */
    void startTimer(float seconds);

    /* Returns whether the market is paused until you choose something. */
    bool waitingForYou() const;

    /* Returns how many buyers are still in the market today. */
    int buyersStillLooking() const;

    /* Returns a whole number from low to high, inclusive. */
    int randomBetween(int low, int high);
};
