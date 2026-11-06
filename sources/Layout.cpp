/*
 * File: Layout.cpp
 * Author: Bior Madol Khom Bior
 *
 * This file decides where things go on the screen: where each buyer stands
 * and which buttons to show. The buttons depend on what you can do right
 * now: answer a price, name your own price, decide on offers, skip ahead,
 * or move to the next day.
 */
#include "Layout.h"
#include <algorithm>
#include <string>
#include "Settings.h"
using namespace std;

/* The bots buying today, with you added in the middle on buyer days. */
vector<int> buyerRow(const Market& market) {
    vector<int> row;
    const vector<Trader>& traders = market.traders();
    for (int index = 1; index < (int) traders.size(); index++) {
        if (traders[index].role == Role::Buyer) row.push_back(index);
    }
    if (market.you().role == Role::Buyer) {
        row.insert(row.begin() + row.size() / 2, 0);
    }
    return row;
}

/* Spreads the row across the screen, closer together when it is crowded. */
Vector2 buyerSpot(int index, int count) {
    float spacing = min(180.0f, 880.0f / count);
    float x = kWindowWidth / 2.0f + (index - (count - 1) / 2.0f) * spacing;
    return {x, 446};
}

/* Returns one button. */
static Button makeButton(const string& label, Rectangle rect, Action action, ButtonStyle style, bool enabled) {
    Button button;
    button.label = label;
    button.rect = rect;
    button.action = action;
    button.style = style;
    button.enabled = enabled;
    return button;
}

/* Adds the − and + buttons and the button that sends your price. */
static void addPricePicker(vector<Button>& buttons, const string& sendLabel, Action send, bool enabled) {
    buttons.push_back(makeButton("-", {308, 576, 36, 36}, Action::PriceDown, ButtonStyle::Round, enabled));
    buttons.push_back(makeButton("+", {456, 576, 36, 36}, Action::PriceUp, ButtonStyle::Round, enabled));
    buttons.push_back(makeButton(sendLabel, {508, 572, 168, 44}, send, ButtonStyle::Blue, enabled));
}

/* Picks the buttons for whatever is on the screen. */
vector<Button> buttonsFor(const Market& market, bool helpOpen) {
    vector<Button> buttons;
    if (helpOpen) {
        buttons.push_back(makeButton("Got it", {390, 466, 180, 52}, Action::CloseHelp, ButtonStyle::Dark, true));
        return buttons;
    }
    buttons.push_back(makeButton("?", {904, 14, 34, 34}, Action::OpenHelp, ButtonStyle::Round, true));

    Phase phase = market.phase();
    if (phase == Phase::DayOver) {
        bool lastDay = market.day() == kDayCount;
        buttons.push_back(makeButton(lastDay ? "See final standings" : "Next day", {360, 620, 240, 56},
                                     Action::NextDay, ButtonStyle::Dark, true));
        return buttons;
    }
    if (phase == Phase::WeekOver) {
        buttons.push_back(makeButton("Play again", {390, 620, 180, 56}, Action::NewWeek, ButtonStyle::Dark, true));
        return buttons;
    }

    /* Once you have traded, you can skip the rest of the day. */
    if (market.youAreDone()) {
        buttons.push_back(makeButton("Skip to end of day", {560, 566, 300, 56}, Action::Skip, ButtonStyle::Plain, true));
        return buttons;
    }

    if (market.you().role == Role::Buyer) {
        bool open = market.canAnswer();
        buttons.push_back(makeButton("Take $" + to_string(market.sale().ask()), {40, 562, 220, 64},
                                     Action::Take, ButtonStyle::Green, open));
        addPricePicker(buttons, "Counter", Action::Counter, open);
        buttons.push_back(makeButton("Pass", {720, 562, 200, 64}, Action::Pass, ButtonStyle::Plain, open));
    } else if (market.isYourTurnToAsk()) {
        addPricePicker(buttons, "Ask", Action::Ask, true);
    } else if (market.isYourTurnToDecide()) {
        bool roundsLeft = !market.sale().isLastRound();
        addPricePicker(buttons, "Ask again", Action::Ask, roundsLeft);
        int best = market.bestBuyer();
        if (best >= 0) {
            int offer = market.traders()[best].answer.price;
            buttons.push_back(makeButton("Accept $" + to_string(offer), {720, 562, 200, 64},
                                         Action::Accept, ButtonStyle::Green, true));
        } else {
            buttons.push_back(makeButton("Keep crate", {720, 562, 200, 64}, Action::KeepCrate, ButtonStyle::Plain, true));
        }
    }
    return buttons;
}

/* * * * * Test Cases Below This Point * * * * */
#include "SimpleTest.h"

TEST("you stand in the middle of the buyer row") {
    Market market(4);
    vector<int> row = buyerRow(market);
    EXPECT_EQUAL((int) row.size(), market.settings().buyers);
    EXPECT_EQUAL(row[row.size() / 2], 0);
}

TEST("every buyer stands inside the window") {
    for (int count = 1; count <= 7; count++) {
        EXPECT(buyerSpot(0, count).x > 40);
        EXPECT(buyerSpot(count - 1, count).x < kWindowWidth - 40);
    }
}

TEST("a buyer gets Take, Counter, and Pass") {
    Market market(4);
    while (!market.canAnswer()) market.update(100);
    vector<Button> buttons = buttonsFor(market, false);
    bool hasTake = false, hasCounter = false, hasPass = false;
    for (const Button& button : buttons) {
        if (button.action == Action::Take) hasTake = button.enabled;
        if (button.action == Action::Counter) hasCounter = button.enabled;
        if (button.action == Action::Pass) hasPass = button.enabled;
    }
    EXPECT(hasTake && hasCounter && hasPass);
}

TEST("the help panel shows only its own button") {
    Market market(4);
    vector<Button> buttons = buttonsFor(market, true);
    EXPECT_EQUAL(buttons.size(), 1u);
    EXPECT(buttons[0].action == Action::CloseHelp);
}
