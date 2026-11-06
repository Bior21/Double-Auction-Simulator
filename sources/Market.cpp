/*
 * File: Market.cpp
 * Author: Bior Madol Khom Bior
 *
 * This file implements the Market class, which runs the market week.
 *
 * Each day, some bots come to sell and some come to buy, and each gets a
 * secret number. Sellers come to the front one at a time and name a price.
 * Every buyer answers at the same moment: take it, counter, or pass. The
 * seller then takes the best answer or asks again lower, for up to three
 * rounds. Profit is the difference between a trader's secret number and
 * the price, and it adds up over the week.
 */
#include "Market.h"
#include <algorithm>
#include "Settings.h"
#include "Sorting.h"
using namespace std;

/* The bots' names, in the same order as kBotColors. */
const string kBotNames[kBotCount] = {
    "Amina", "Deng", "Rosa", "Kofi", "Lena", "Marco",
    "Nia", "Tomas", "Sam", "Ayo", "Mei", "Omar"
};

/* Creates the traders and starts day 1. */
Market::Market(unsigned int seed) : _random(seed) {
    makeTraders();
    newWeek();
}

/* You first, then the twelve bots with their names, colors, and personalities. */
void Market::makeTraders() {
    _traders.clear();
    Trader you;
    you.name = "You";
    you.isYou = true;
    _traders.push_back(you);

    const Personality kKinds[] = {Personality::Stubborn, Personality::Fair, Personality::Nervous};
    for (int bot = 0; bot < kBotCount; bot++) {
        Trader trader;
        trader.name = kBotNames[bot];
        trader.colorIndex = bot;
        trader.personality = kKinds[bot % 3];
        _traders.push_back(trader);
    }
}

/* Everyone back to $0 on day 1. */
void Market::newWeek() {
    for (Trader& trader : _traders) {
        trader.totalProfit = 0;
    }
    _day = 1;
    startDay();
}

/* Sets up one day of the market. */
void Market::startDay() {
    _settings = settingsForDay(_day);
    _deals.clear();
    _fairPrice = 0;

    for (Trader& trader : _traders) {
        trader.role = Role::Away;
        trader.done = false;
        trader.answer = Answer();
        trader.profitToday = 0;
    }

    /* Your card for the day. */
    Trader& you = _traders[0];
    you.role = _settings.yourRole;
    if (you.role == Role::Buyer) {
        you.secret = randomBetween(kYourLowestValue, kYourHighestValue);
    } else {
        you.secret = randomBetween(kYourLowestCost, kYourHighestCost);
    }

    /* Shuffle the bots, then fill the seller spots and the buyer spots. */
    vector<int> bots;
    for (int index = 1; index <= kBotCount; index++) {
        bots.push_back(index);
    }
    shuffle(bots.begin(), bots.end(), _random);

    int botSellers = _settings.sellers - (you.role == Role::Seller ? 1 : 0);
    int botBuyers = _settings.buyers - (you.role == Role::Buyer ? 1 : 0);
    _sellerLine.clear();
    for (int place = 0; place < botSellers + botBuyers; place++) {
        Trader& bot = _traders[bots[place]];
        if (place < botSellers) {
            bot.role = Role::Seller;
            bot.secret = randomBetween(kLowestCost, kHighestCost);
            _sellerLine.push_back(bots[place]);
        } else {
            bot.role = Role::Buyer;
            bot.secret = randomBetween(kLowestValue, kHighestValue);
        }
    }

    /* On a seller day, you come to the front in the middle of the line,
     * after watching a few sales first.
     */
    if (you.role == Role::Seller) {
        _sellerLine.insert(_sellerLine.begin() + _sellerLine.size() / 2, 0);
    }

    _saleIndex = -1;
    startNextSale();
}

/* Brings the next seller to the front. */
void Market::startNextSale() {
    _saleIndex++;
    if (_saleIndex >= (int) _sellerLine.size() || buyersStillLooking() == 0) {
        endDay();
        return;
    }

    int seller = _sellerLine[_saleIndex];
    _saleOver = false;
    _bestBuyer = -1;
    _phase = Phase::Asking;

    if (seller == 0) {
        /* Your turn: wait for you to name a price. */
        _sale = Sale(seller, 0);
        _yourAskPosted = false;
        _yourPrice = _traders[0].secret + 15;
        _sellerSays = "Name your price";
    } else {
        _sale = Sale(seller, openingAsk(_traders[seller]));
        _sellerSays = "One crate, $" + to_string(_sale.ask());
        startTimer(kAskSeconds);
    }
}

/* Every bot buyer answers right away, but nobody sees the answers until the reveal. */
void Market::startAnswering() {
    _phase = Phase::Answering;
    for (int index = 0; index < (int) _traders.size(); index++) {
        Trader& trader = _traders[index];
        if (trader.role != Role::Buyer || trader.done) continue;
        if (trader.isYou) {
            trader.answer = Answer();
            _yourPrice = min(_sale.ask() - 4, trader.secret - 5);   // start below what it's worth to you
        } else {
            trader.answer = buyerAnswer(trader, _sale.ask(), _sale.round());
        }
    }
    startTimer(canAnswer() ? _settings.answerSeconds : kWatchAnswerSeconds);
}

/* Shows everyone's answers. */
void Market::revealAnswers() {
    _phase = Phase::Revealing;
    if (_sale.seller() != 0) _sellerSays = "Hmm...";
    startTimer(kRevealSeconds);
}

/* Decides what happens after the answers are shown. */
void Market::decide() {
    int buyer = _sale.bestBuyer(_traders, _random);
    bool someoneTook = buyer >= 0 && _traders[buyer].answer.kind == AnswerKind::Take;

    /* Taking the price always makes the deal, whoever the seller is. */
    if (someoneTook) {
        makeDeal(buyer, _sale.ask());
        return;
    }

    /* When you are the seller, you choose what happens next. */
    if (_sale.seller() == 0) {
        if (buyer < 0 && _sale.isLastRound()) {
            sellerLeaves();
            return;
        }
        _bestBuyer = buyer;
        _yourPrice = _sale.ask() - 2;
        _phase = Phase::Deciding;
        _sellerSays = buyer >= 0 ? "Best offer: $" + to_string(_traders[buyer].answer.price) : "No offers";
        return;
    }

    /* A bot seller takes a good enough counter, or asks again lower. */
    const Trader& seller = _traders[_sale.seller()];
    if (buyer >= 0 && acceptsOffer(seller, _traders[buyer].answer.price, _sale.round())) {
        makeDeal(buyer, _traders[buyer].answer.price);
    } else if (_sale.isLastRound()) {
        sellerLeaves();
    } else {
        _sale.askAgain(loweredAsk(seller, _sale.ask()));
        _sellerSays = "Too low. $" + to_string(_sale.ask()) + "?";
        _phase = Phase::Asking;
        startTimer(kAskSeconds);
    }
}

/* Both traders get their profit, and both are done for the day. */
void Market::makeDeal(int buyer, int price) {
    Trader& seller = _traders[_sale.seller()];
    Trader& winner = _traders[buyer];
    seller.profitToday += profitFrom(seller, price);
    winner.profitToday += profitFrom(winner, price);
    seller.totalProfit += profitFrom(seller, price);
    winner.totalProfit += profitFrom(winner, price);
    seller.done = true;
    winner.done = true;

    _deals.push_back({seller.name, winner.name, price});
    _sellerSays = "Sold to " + string(winner.isYou ? "you" : winner.name) + ", $" + to_string(price);
    _saleOver = true;
    _phase = Phase::Result;
    startTimer(kResultSeconds);
}

/* The seller goes home with the crate. */
void Market::sellerLeaves() {
    _traders[_sale.seller()].done = true;
    _sellerSays = "No deal. I'll keep it.";
    _saleOver = true;
    _phase = Phase::Result;
    startTimer(kResultSeconds);
}

/* Ends the day and works out where supply met demand today. */
void Market::endDay() {
    _phase = Phase::DayOver;
    vector<int> values;
    vector<int> costs;
    for (const Trader& trader : _traders) {
        if (trader.role == Role::Buyer) values.push_back(trader.secret);
        if (trader.role == Role::Seller) costs.push_back(trader.secret);
    }
    _fairPrice = marketPrice(values, costs);
}

/* Moves the current step forward when its time runs out. */
void Market::update(float seconds) {
    if (_phase == Phase::DayOver || _phase == Phase::WeekOver || waitingForYou()) return;

    _timer -= seconds;
    if (_timer > 0) return;

    if (_phase == Phase::Asking) {
        startAnswering();
    } else if (_phase == Phase::Answering) {
        if (canAnswer()) _traders[0].answer.kind = AnswerKind::Pass;   // out of time
        revealAnswers();
    } else if (_phase == Phase::Revealing) {
        decide();
    } else if (_phase == Phase::Result && _saleOver) {
        startNextSale();
    }
}

/* You can answer while you are a buyer still looking and the answer step is open. */
bool Market::canAnswer() const {
    const Trader& you = _traders[0];
    return _phase == Phase::Answering && you.role == Role::Buyer && !you.done &&
           you.answer.kind == AnswerKind::Waiting;
}

/* Takes the price. The bots have already answered, so the reveal starts now. */
void Market::take() {
    if (!canAnswer()) return;
    _traders[0].answer = {AnswerKind::Take, _sale.ask()};
    revealAnswers();
}

/* Counters at your chosen price. */
void Market::counter() {
    if (!canAnswer()) return;
    _traders[0].answer = {AnswerKind::Counter, _yourPrice};
    revealAnswers();
}

/* Lets this price go by. */
void Market::pass() {
    if (!canAnswer()) return;
    _traders[0].answer = {AnswerKind::Pass, 0};
    revealAnswers();
}

/* It is your turn to ask when you are the seller at the front and have not asked yet. */
bool Market::isYourTurnToAsk() const {
    return _phase == Phase::Asking && _sale.seller() == 0 && !_yourAskPosted;
}

/* It is your turn to decide after buyers answered your price without taking it. */
bool Market::isYourTurnToDecide() const {
    return _phase == Phase::Deciding;
}

/* Names your price, either to start your sale or to ask again lower. */
void Market::askAtYourPrice() {
    if (isYourTurnToAsk()) {
        _sale = Sale(0, _yourPrice);
    } else if (isYourTurnToDecide() && !_sale.isLastRound()) {
        _sale.askAgain(_yourPrice);
    } else {
        return;
    }
    _yourAskPosted = true;
    _sellerSays = "One crate, $" + to_string(_sale.ask());
    _phase = Phase::Asking;
    startTimer(kAskSeconds);
}

/* Sells to the best counter offer. */
void Market::acceptBestOffer() {
    if (isYourTurnToDecide() && _bestBuyer >= 0) {
        makeDeal(_bestBuyer, _traders[_bestBuyer].answer.price);
    }
}

/* Takes your crate home without selling. */
void Market::keepCrate() {
    if (isYourTurnToDecide()) sellerLeaves();
}

/* Returns the price you are choosing. */
int Market::yourPrice() const {
    return _yourPrice;
}

/* Raises or lowers your price, staying within the allowed range. */
void Market::changeYourPrice(int amount) {
    _yourPrice = clamp(_yourPrice + amount, kLowestPrice, kHighestPrice);
}

/* You are done once you have traded, or your crate went home with you. */
bool Market::youAreDone() const {
    return _traders[0].done;
}

/* Runs every remaining step at once. Each update moves one step forward. */
void Market::skipToEndOfDay() {
    if (!youAreDone()) return;
    for (int step = 0; step < 10000 && _phase != Phase::DayOver; step++) {
        update(100);
    }
}

/* Starts the next day, or ends the week. */
void Market::nextDay() {
    if (_phase != Phase::DayOver) return;
    if (_day == kDayCount) {
        _phase = Phase::WeekOver;
    } else {
        _day++;
        startDay();
    }
}

/* Starts the countdown for the current step. */
void Market::startTimer(float seconds) {
    _timer = seconds;
    _timeGiven = seconds;
}

/* The market pauses while you name your price or decide on offers. */
bool Market::waitingForYou() const {
    return isYourTurnToAsk() || isYourTurnToDecide();
}

/* Counts buyers who have not bought anything yet today. */
int Market::buyersStillLooking() const {
    int count = 0;
    for (const Trader& trader : _traders) {
        if (trader.role == Role::Buyer && !trader.done) count++;
    }
    return count;
}

/* Returns a random whole number from low to high. */
int Market::randomBetween(int low, int high) {
    uniform_int_distribution<int> pick(low, high);
    return pick(_random);
}

/* Returns the current step. */
Phase Market::phase() const {
    return _phase;
}

/* Returns the day, from 1 to kDayCount. */
int Market::day() const {
    return _day;
}

/* Returns today's settings. */
const DaySettings& Market::settings() const {
    return _settings;
}

/* Returns every trader, you first. */
const vector<Trader>& Market::traders() const {
    return _traders;
}

/* Returns you. */
const Trader& Market::you() const {
    return _traders[0];
}

/* Returns the sale at the front of the market. */
const Sale& Market::sale() const {
    return _sale;
}

/* Returns which sale of the day this is, starting at 1. */
int Market::saleNumber() const {
    return _saleIndex + 1;
}

/* Returns how many sellers came today. */
int Market::saleCount() const {
    return (int) _sellerLine.size();
}

/* Returns the sellers who have not come to the front yet. */
vector<int> Market::sellersStillWaiting() const {
    vector<int> waiting;
    for (int place = _saleIndex + 1; place < (int) _sellerLine.size(); place++) {
        waiting.push_back(_sellerLine[place]);
    }
    return waiting;
}

/* Returns today's deals, oldest first. */
const vector<Deal>& Market::deals() const {
    return _deals;
}

/* Returns what the seller at the front is saying. */
string Market::sellerSays() const {
    return _sellerSays;
}

/* Returns the buyer with the best offer while you decide, or -1. */
int Market::bestBuyer() const {
    return _bestBuyer;
}

/* Returns the seconds left in the current step. */
float Market::timeLeft() const {
    return max(0.0f, _timer);
}

/* Returns how many seconds the current step started with. */
float Market::timeGiven() const {
    return _timeGiven;
}

/* Returns today's fair price, worked out when the day ends. */
int Market::fairPrice() const {
    return _fairPrice;
}

/* Returns every trader's index, richest first. */
vector<int> Market::standings() const {
    return rankByProfit(_traders);
}

/* Returns your place in the standings, starting at 1. */
int Market::yourRank() const {
    vector<int> order = standings();
    for (int place = 0; place < (int) order.size(); place++) {
        if (order[place] == 0) return place + 1;
    }
    return (int) order.size();
}

/* * * * * Test Cases Below This Point * * * * */
#include "SimpleTest.h"

/* Plays like someone who never trades: pass every price and keep the crate. */
static void playWithoutTrading(Market& market) {
    if (market.canAnswer()) market.pass();
    if (market.isYourTurnToAsk()) market.askAtYourPrice();
    if (market.isYourTurnToDecide()) market.keepCrate();
    market.update(100);
}

TEST("a new market starts on day 1 with you as a buyer") {
    Market market(7);
    EXPECT_EQUAL(market.day(), 1);
    EXPECT(market.you().role == Role::Buyer);
    EXPECT(market.phase() == Phase::Asking);
    EXPECT_EQUAL(market.saleCount(), 5);
}

TEST("each day has the right number of buyers and sellers") {
    Market market(3);
    for (int day = 1; day <= kDayCount; day++) {
        int buyers = 0;
        int sellers = 0;
        for (const Trader& trader : market.traders()) {
            if (trader.role == Role::Buyer) buyers++;
            if (trader.role == Role::Seller) sellers++;
        }
        EXPECT_EQUAL(buyers, market.settings().buyers);
        EXPECT_EQUAL(sellers, market.settings().sellers);

        for (int step = 0; step < 10000 && market.phase() != Phase::DayOver; step++) {
            playWithoutTrading(market);
        }
        market.nextDay();
    }
    EXPECT(market.phase() == Phase::WeekOver);
}

TEST("bots never trade at a loss over a whole week") {
    for (unsigned int seed = 0; seed < 20; seed++) {
        Market market(seed);
        while (market.phase() != Phase::WeekOver) {
            for (int step = 0; step < 10000 && market.phase() != Phase::DayOver; step++) {
                playWithoutTrading(market);
            }
            for (const Trader& trader : market.traders()) {
                EXPECT(trader.profitToday >= 0);
            }
            market.nextDay();
        }
    }
}

TEST("taking the price earns your value minus the price") {
    Market market(11);
    while (!market.canAnswer()) market.update(100);
    int price = market.sale().ask();
    market.take();
    while (market.phase() != Phase::Result) market.update(100);

    /* Another buyer may also have taken it and won the coin toss. */
    if (market.you().done) {
        EXPECT_EQUAL(market.you().profitToday, market.you().secret - price);
        EXPECT_EQUAL(market.deals().back().buyer, "You");
    }
}

TEST("the market waits for you to name your price on a seller day") {
    Market market(5);
    while (market.day() == 1) {
        playWithoutTrading(market);
        if (market.phase() == Phase::DayOver) market.nextDay();
    }
    while (!market.isYourTurnToAsk()) market.update(100);
    market.update(100);
    EXPECT(market.isYourTurnToAsk());

    /* Ask far too much: nobody takes it, so you get to decide. */
    while (market.yourPrice() < kHighestPrice) market.changeYourPrice(10);
    market.askAtYourPrice();
    while (market.phase() != Phase::Deciding && market.phase() != Phase::Result) market.update(100);
    EXPECT(market.phase() == Phase::Deciding);
}

TEST("skipToEndOfDay only works once you are done") {
    Market market(2);
    market.skipToEndOfDay();
    EXPECT(market.phase() != Phase::DayOver);
}

TEST("standings put the richest trader first") {
    Market market(9);
    for (int step = 0; step < 10000 && market.phase() != Phase::DayOver; step++) {
        playWithoutTrading(market);
    }
    vector<int> order = market.standings();
    for (int place = 0; place + 1 < (int) order.size(); place++) {
        EXPECT(market.traders()[order[place]].totalProfit >= market.traders()[order[place + 1]].totalProfit);
    }
    EXPECT(market.fairPrice() > 0);
}
