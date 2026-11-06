/*
 * File: Drawing.cpp
 * Author: Bior Madol Khom Bior
 *
 * This file draws every screen of Double Auction Simulator. A day is drawn in four bands:
 * the stage at the top, where the seller names a price; the row of buyers
 * with their answers; your controls; and a strip with your card and your
 * place in the week. Between days, a summary screen takes over.
 *
 * Drawing only reads the market. It never changes it.
 */
#include "Drawing.h"
#include <cmath>
#include <string>
#include "Layout.h"
#include "People.h"
#include "Settings.h"
using namespace std;

/* The lines shown in the help panel. */
const vector<string> kHelpLines = {
    "Sellers come up one at a time and name a price for a crate.",
    "Buyers answer at the same moment: take the price, counter with a lower one, or pass.",
    "The seller takes the best answer, or asks again lower. After 3 rounds, they go home.",
    "Buyers profit by paying less than the crate is worth to them; sellers by selling above their cost.",
    "You buy on odd days and sell on even days. Most profit after 7 days wins.",
};

/* ---- Fonts and text ---- */

/* Loads one font file at a size, or raylib's font if the file is missing. */
static Font loadOneFont(const string& fileName, int size) {
    /* Plain letters and punctuation, plus the middle dot (·) and the arrow (→). */
    vector<int> characters;
    for (int code = 32; code < 127; code++) {
        characters.push_back(code);
    }
    characters.push_back(183);
    characters.push_back(8594);

    string path = string(GetApplicationDirectory()) + "fonts/" + fileName;
    Font font = LoadFontEx(path.c_str(), size, characters.data(), (int) characters.size());
    if (font.texture.id == 0) {
        font = GetFontDefault();
    }
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
    return font;
}

/* Loads all three fonts. */
Fonts loadFonts() {
    Fonts fonts;
    fonts.fun = loadOneFont("Baloo2-ExtraBold.ttf", 96);
    fonts.text = loadOneFont("Inter-Medium.ttf", 40);
    fonts.textBold = loadOneFont("Inter-Bold.ttf", 40);
    return fonts;
}

/* Frees all three fonts. */
void unloadFonts(Fonts& fonts) {
    UnloadFont(fonts.fun);
    UnloadFont(fonts.text);
    UnloadFont(fonts.textBold);
}

/* Returns how wide some text is at a size. */
static float textWidth(const Font& font, const string& text, float size) {
    return MeasureTextEx(font, text.c_str(), size, 0).x;
}

/* Draws text with its top left corner at (x, y). */
static void drawText(const Font& font, const string& text, float x, float y, float size, Color color) {
    DrawTextEx(font, text.c_str(), {x, y}, size, 0, color);
}

/* Draws text centered left to right on centerX. */
static void drawCentered(const Font& font, const string& text, float centerX, float y, float size, Color color) {
    drawText(font, text, centerX - textWidth(font, text, size) / 2, y, size, color);
}

/* Draws text centered both ways on a point. */
static void drawCenteredOn(const Font& font, const string& text, Vector2 center, float size, Color color) {
    Vector2 measured = MeasureTextEx(font, text.c_str(), size, 0);
    drawText(font, text, center.x - measured.x / 2, center.y - measured.y / 2, size, color);
}

/* Draws text whose right edge is at right. */
static void drawRightAligned(const Font& font, const string& text, float right, float y, float size, Color color) {
    drawText(font, text, right - textWidth(font, text, size), y, size, color);
}

/* Returns an amount of money like $64 or -$3. */
static string money(int amount) {
    return amount < 0 ? "-$" + to_string(-amount) : "$" + to_string(amount);
}

/* Returns 1st, 2nd, 3rd, 4th, and so on. */
static string ordinal(int number) {
    if (number == 1) return "1st";
    if (number == 2) return "2nd";
    if (number == 3) return "3rd";
    return to_string(number) + "th";
}

/* Returns a trader's color: blue for you, their own color for a bot. */
static Color colorOf(const Trader& trader) {
    return trader.isYou ? kYouBlue : kBotColors[trader.colorIndex];
}

/* Returns a color made more see-through, keeping colors that are already faint just as faint. */
static Color faded(Color color, float amount) {
    color.a = (unsigned char) (color.a * amount);
    return color;
}

/* Draws a white card with rounded corners and a thin border. */
static void drawCard(Rectangle card) {
    DrawRectangleRounded(card, 0.08f, 8, kWhite);
    DrawRectangleRoundedLinesEx(card, 0.08f, 8, 1.5f, kLine);
}

/* ---- The four bands of a market day ---- */

/* The top bar: title, day, twist, and a dot for each sale of the day. */
static void drawHeader(const Market& market, const Fonts& fonts) {
    DrawRectangle(0, 0, kWindowWidth, 60, kWhite);
    DrawLineEx({0, 60}, {(float) kWindowWidth, 60}, 2, kLine);
    drawText(fonts.fun, "Double Auction Simulator", 28, 10, 30, kInk);
    drawCentered(fonts.textBold, "Day " + to_string(market.day()) + " of " + to_string(kDayCount), 480, 12, 15, kInk);
    drawCentered(fonts.text, market.settings().twist, 480, 34, 12, kMuted);

    Phase phase = market.phase();
    if (phase == Phase::DayOver || phase == Phase::WeekOver) return;
    int count = market.saleCount();
    float firstDot = 886 - count * 20;
    drawRightAligned(fonts.text, "Sale", firstDot - 4, 23, 13, kMuted);
    for (int sale = 0; sale < count; sale++) {
        Vector2 dot = {firstDot + 10 + sale * 20, 31};
        if (sale < market.saleNumber() - 1) {
            DrawCircleV(dot, 6, kInk);
        } else if (sale == market.saleNumber() - 1) {
            DrawCircleV(dot, 7, kInk);
            DrawCircleV(dot, 5, kSun);
        } else {
            DrawCircleV(dot, 6, kLine);
        }
    }
}

/* The stage: the seller at the front, what they say, who is next, and today's deals. */
static void drawStage(const Market& market, const Fonts& fonts) {
    DrawRectangle(0, 62, kWindowWidth, 250, market.sale().seller() == 0 ? kYouSoft : kStage);
    DrawRectangleRounded({330, 268, 300, 20}, 1.0f, 8, kCounter);

    const Trader& seller = market.traders()[market.sale().seller()];
    Mood mood = Mood::Happy;
    if (market.phase() == Phase::Revealing && !seller.isYou) mood = Mood::Thinking;
    if (market.phase() == Phase::Result && !market.deals().empty() && market.deals().back().seller != seller.name) mood = Mood::Flat;
    drawPerson({480, 168}, 40, colorOf(seller), mood);
    drawCentered(fonts.textBold, seller.name, 480, 290, 15, kInk);
    drawText(fonts.text, "Round " + to_string(market.sale().round()) + " of " + to_string(kRoundsPerSale), 556, 292, 12, kMuted);

    Color border = seller.isYou ? kYouBlue : kLine;
    Color words = seller.isYou ? kYouBlue : kInk;
    if (market.phase() == Phase::Result && market.sellerSays().rfind("Sold", 0) == 0) {
        border = kGood;
        words = kGood;
    }
    drawBubble(fonts.fun, market.sellerSays(), 650, 84, 30, kWhite, border, words);

    /* The countdown while buyers answer. */
    if (market.phase() == Phase::Answering) {
        float fraction = market.timeGiven() > 0 ? market.timeLeft() / market.timeGiven() : 0;
        drawCountdown(fonts.textBold, {360, 120}, fraction, (int) ceilf(market.timeLeft()));
    }

    /* Who is waiting to sell. */
    drawText(fonts.textBold, "UP NEXT", 40, 84, 11, kMuted);
    vector<int> waiting = market.sellersStillWaiting();
    for (int place = 0; place < (int) waiting.size() && place < 5; place++) {
        drawPerson({56.0f + place * 44, 126}, 14, colorOf(market.traders()[waiting[place]]), Mood::Flat);
    }

    /* The last few deals of the day. */
    drawRightAligned(fonts.textBold, "SOLD TODAY", 920, 196, 11, kMuted);
    const vector<Deal>& deals = market.deals();
    int first = max(0, (int) deals.size() - 4);
    for (int index = first; index < (int) deals.size(); index++) {
        string line = deals[index].seller + " → " + deals[index].buyer + "  " + money(deals[index].price);
        drawRightAligned(fonts.textBold, line, 920, 216.0f + (index - first) * 20, 13, kInk);
    }
}

/* One buyer in the row, with their answer above their head. */
static void drawBuyer(const Market& market, const Fonts& fonts, int traderIndex, Vector2 head) {
    const Trader& buyer = market.traders()[traderIndex];
    Phase phase = market.phase();
    float radius = buyer.isYou ? 30 : 24;
    bool showAnswers = phase == Phase::Revealing || phase == Phase::Deciding || phase == Phase::Result;

    if (buyer.isYou) {
        DrawRectangleRounded({head.x - 76, 326, 152, 206}, 0.12f, 8, kYouSoft);
    }

    /* The buyer who just won this sale stays bright; buyers who finished earlier fade. */
    bool justBought = phase == Phase::Result && !market.deals().empty() && market.deals().back().buyer == buyer.name;
    Color color = colorOf(buyer);
    Mood mood = Mood::Happy;
    if (buyer.done && !justBought) {
        color = faded(color, 0.35f);
        mood = Mood::Flat;
    } else if (phase == Phase::Answering && !buyer.isYou) {
        mood = Mood::Thinking;
    } else if (showAnswers && buyer.answer.kind == AnswerKind::Pass) {
        mood = Mood::Flat;
    }
    drawPerson({head.x, buyer.isYou ? head.y - 6 : head.y}, radius, color, mood);
    drawCentered(buyer.isYou ? fonts.textBold : fonts.text, buyer.name, head.x, 508, buyer.isYou ? 16 : 14,
                 buyer.done && !justBought ? kMuted : kInk);

    /* What goes in the bubble above the head. */
    string words;
    Color border = kLine;
    Color textColor = kInk;
    if (buyer.done) {
        if (justBought) {
            words = "Bought!";
            border = kGood;
            textColor = kGood;
        }
    } else if (phase == Phase::Answering && !buyer.isYou) {
        words = "...";
    } else if ((phase == Phase::Answering && buyer.isYou) || showAnswers) {
        if (buyer.answer.kind == AnswerKind::Take) {
            words = "Take";
            border = kGood;
            textColor = kGood;
        } else if (buyer.answer.kind == AnswerKind::Counter) {
            words = money(buyer.answer.price);
        } else if (buyer.answer.kind == AnswerKind::Pass) {
            words = "Pass";
            textColor = kMuted;
        }
        if (buyer.isYou && buyer.answer.kind != AnswerKind::Take) border = kYouBlue;
        if (buyer.isYou && buyer.answer.kind == AnswerKind::Counter) textColor = kYouBlue;
        if (phase == Phase::Deciding && traderIndex == market.bestBuyer()) {
            border = kGood;
            textColor = kGood;
        }
    }
    if (!words.empty()) {
        drawBubble(fonts.fun, words, head.x, 340, 24, kWhite, border, textColor);
    }
}

/* The whole row of buyers. */
static void drawBuyers(const Market& market, const Fonts& fonts) {
    vector<int> row = buyerRow(market);
    for (int place = 0; place < (int) row.size(); place++) {
        drawBuyer(market, fonts, row[place], buyerSpot(place, (int) row.size()));
    }
}

/* The box around the − and + buttons, with your price in the middle. */
static void drawPriceBox(const Market& market, const Fonts& fonts, bool active) {
    float fade = active ? 1.0f : 0.35f;
    Rectangle box = {290, 562, 400, 64};
    DrawRectangleRounded(box, 0.3f, 8, kWhite);
    DrawRectangleRoundedLinesEx(box, 0.3f, 8, 2, faded(kYouBlue, fade));
    drawCenteredOn(fonts.fun, money(market.yourPrice()), {400, 596}, 48, faded(kInk, fade));
}

/* Your part of the screen: the price box and the words next to your buttons. */
static void drawControls(const Market& market, const Fonts& fonts) {
    DrawLineEx({0, 540}, {(float) kWindowWidth, 540}, 2, kLine);
    const Trader& you = market.you();

    if (you.done) {
        /* Your result for today. */
        if (you.profitToday != 0 || (!market.deals().empty() && (market.deals().back().buyer == "You" ||
                                                                 market.deals().back().seller == "You"))) {
            drawCentered(fonts.fun, "+" + money(you.profitToday), 260, 548, 60, kGood);
            drawCentered(fonts.text, "today's profit", 260, 616, 14, kMuted);
        } else {
            drawCentered(fonts.fun, "No deal today", 260, 562, 40, kMuted);
        }
        return;
    }

    if (you.role == Role::Buyer) {
        drawPriceBox(market, fonts, market.canAnswer());
    } else if (market.isYourTurnToAsk()) {
        drawText(fonts.textBold, "Name your price", 40, 574, 20, kInk);
        drawText(fonts.text, "Your crate cost " + money(you.secret), 40, 602, 14, kMuted);
        drawPriceBox(market, fonts, true);
    } else if (market.isYourTurnToDecide()) {
        int best = market.bestBuyer();
        if (best >= 0) {
            const Trader& buyer = market.traders()[best];
            drawText(fonts.textBold, buyer.name + " offers " + money(buyer.answer.price), 40, 574, 20, kInk);
            int profit = buyer.answer.price - you.secret;
            drawText(fonts.textBold, "Profit " + money(profit), 40, 602, 15, profit >= 0 ? kGood : kBad);
        } else {
            drawText(fonts.textBold, "No offers", 40, 574, 20, kInk);
            drawText(fonts.text, "Ask again, or keep it", 40, 602, 14, kMuted);
        }
        drawPriceBox(market, fonts, !market.sale().isLastRound());
    } else {
        /* A seller day, waiting for your turn at the front. */
        int wait = 0;
        vector<int> waiting = market.sellersStillWaiting();
        for (int place = 0; place < (int) waiting.size(); place++) {
            if (waiting[place] == 0) wait = place + 1;
        }
        string line = wait == 1 ? "You're up after this sale" : "You're up in " + to_string(wait) + " sales";
        if (market.sale().seller() == 0) line = "Buyers are answering...";
        drawCentered(fonts.fun, line, 480, 552, 36, kInk);
        drawCentered(fonts.text, "Watch the prices", 480, 606, 14, kMuted);
    }
}

/* The bottom strip: your card and your place this week. */
static void drawYourStrip(const Market& market, const Fonts& fonts) {
    DrawRectangle(0, 652, kWindowWidth, 68, kPaper);
    DrawLineEx({0, 652}, {(float) kWindowWidth, 652}, 2, kLine);
    const Trader& you = market.you();
    string label = you.role == Role::Buyer ? "Your crate is worth " : "Your crate cost ";
    drawText(fonts.text, label, 28, 676, 17, kMuted);
    drawText(fonts.textBold, money(you.secret), 28 + textWidth(fonts.text, label, 17), 674, 21, kInk);
    string place = ordinal(market.yourRank()) + " this week  ·  " + money(you.totalProfit);
    drawRightAligned(fonts.text, place, 932, 676, 17, kMuted);
}

/* ---- Between days ---- */

/* A short list of standings, with you highlighted. showToday adds what each trader made today. */
static void drawStandings(const Market& market, const Fonts& fonts, Rectangle card, int rows, bool showToday) {
    drawCard(card);
    drawText(fonts.textBold, "This week", card.x + 24, card.y + 22, 19, kInk);
    vector<int> order = market.standings();
    for (int place = 0; place < rows && place < (int) order.size(); place++) {
        const Trader& trader = market.traders()[order[place]];
        float y = card.y + 64 + place * 34;
        if (trader.isYou) DrawRectangleRounded({card.x + 12, y - 6, card.width - 24, 30}, 0.4f, 8, kYouSoft);
        drawText(fonts.text, to_string(place + 1) + ".", card.x + 28, y, 15, kMuted);
        drawText(trader.isYou ? fonts.textBold : fonts.text, trader.name, card.x + 58, y, 16, kInk);
        if (showToday && trader.profitToday > 0) {
            drawRightAligned(fonts.textBold, "+" + money(trader.profitToday), card.x + card.width - 100, y + 1, 14, kGood);
        }
        drawRightAligned(fonts.textBold, money(trader.totalProfit), card.x + card.width - 28, y, 16, kInk);
    }
    if (market.yourRank() > rows) {
        float y = card.y + 64 + rows * 34;
        drawText(fonts.textBold, to_string(market.yourRank()) + ".  You", card.x + 28, y, 15, kYouBlue);
        drawRightAligned(fonts.textBold, money(market.you().totalProfit), card.x + card.width - 28, y, 16, kYouBlue);
    }
}

/* The summary after each day. */
static void drawDayOver(const Market& market, const Fonts& fonts) {
    DrawRectangle(0, 62, kWindowWidth, kWindowHeight - 62, kPaper);
    drawCentered(fonts.fun, "Day " + to_string(market.day()) + " is over", 480, 76, 58, kInk);

    /* Your deal. */
    Rectangle card = {40, 170, 420, 300};
    drawCard(card);
    drawText(fonts.textBold, "Your day", card.x + 24, card.y + 22, 19, kInk);
    const Trader& you = market.you();
    string story = "You didn't trade today.";
    for (const Deal& deal : market.deals()) {
        if (deal.buyer == "You") story = "You bought from " + deal.seller + " for " + money(deal.price) + ".";
        if (deal.seller == "You") story = "You sold to " + deal.buyer + " for " + money(deal.price) + ".";
    }
    drawText(fonts.text, story, card.x + 24, card.y + 58, 16, kMuted);
    drawCentered(fonts.fun, (you.profitToday > 0 ? "+" : "") + money(you.profitToday), card.x + card.width / 2,
                 card.y + 96, 96, you.profitToday > 0 ? kGood : kMuted);
    drawCentered(fonts.text, "Fair price today: about " + money(market.fairPrice()), card.x + card.width / 2,
                 card.y + 246, 16, kMuted);

    drawStandings(market, fonts, {500, 170, 420, 300}, 6, true);

    /* What happens tomorrow. */
    Rectangle next = {40, 494, 880, 96};
    drawCard(next);
    if (market.day() < kDayCount) {
        DaySettings tomorrow = settingsForDay(market.day() + 1);
        string role = tomorrow.yourRole == Role::Buyer ? "You'll be buying." : "You'll be selling.";
        drawText(fonts.textBold, "Tomorrow: " + tomorrow.twist, next.x + 24, next.y + 24, 19, kInk);
        drawText(fonts.text, role, next.x + 24, next.y + 56, 15, kMuted);
    } else {
        drawText(fonts.textBold, "That was the last day of the week.", next.x + 24, next.y + 24, 19, kInk);
        drawText(fonts.text, "See who made the most profit.", next.x + 24, next.y + 56, 15, kMuted);
    }
}

/* The final standings after day 7. */
static void drawWeekOver(const Market& market, const Fonts& fonts) {
    DrawRectangle(0, 62, kWindowWidth, kWindowHeight - 62, kPaper);
    drawCentered(fonts.fun, "The week is over", 480, 72, 58, kInk);
    int total = (int) market.traders().size();
    string place = "You finished " + ordinal(market.yourRank()) + " of " + to_string(total);
    drawCentered(fonts.fun, place, 480, 136, 36, market.yourRank() == 1 ? kGood : kYouBlue);
    drawStandings(market, fonts, {240, 196, 480, 400}, 9, false);
}

/* ---- Buttons and help ---- */

/* Draws one button in its style, faded when it cannot be clicked. */
static void drawButton(const Button& button, const Fonts& fonts) {
    float fade = button.enabled ? 1.0f : 0.35f;
    Rectangle rect = button.rect;

    if (button.style == ButtonStyle::Round) {
        Vector2 center = {rect.x + rect.width / 2, rect.y + rect.height / 2};
        DrawCircleV(center, rect.width / 2, faded(kYouSoft, fade));
        drawCenteredOn(fonts.fun, button.label, {center.x, center.y + 2}, 34, faded(kYouBlue, fade));
        return;
    }

    Color fill = kWhite;
    Color border = kLine;
    Color textColor = kInk;
    if (button.style == ButtonStyle::Green) fill = border = kGood;
    if (button.style == ButtonStyle::Blue) fill = border = kYouBlue;
    if (button.style == ButtonStyle::Dark) fill = border = kInk;
    if (button.style != ButtonStyle::Plain) textColor = kWhite;

    DrawRectangleRounded(rect, 0.3f, 8, faded(fill, fade));
    DrawRectangleRoundedLinesEx(rect, 0.3f, 8, 2, faded(border, fade));
    drawCenteredOn(fonts.textBold, button.label, {rect.x + rect.width / 2, rect.y + rect.height / 2}, 19,
                   faded(textColor, fade));
}

/* Splits text into lines that each fit inside width. */
static vector<string> wrapText(const Font& font, const string& text, float size, float width) {
    vector<string> lines;
    string line;
    string word;
    for (int index = 0; index <= (int) text.size(); index++) {
        if (index < (int) text.size() && text[index] != ' ') {
            word += text[index];
            continue;
        }
        string longer = line.empty() ? word : line + " " + word;
        if (!line.empty() && textWidth(font, longer, size) > width) {
            lines.push_back(line);
            line = word;
        } else {
            line = longer;
        }
        word.clear();
    }
    if (!line.empty()) lines.push_back(line);
    return lines;
}

/* The help panel over a darkened screen. */
static void drawHelp(const Fonts& fonts) {
    DrawRectangle(0, 0, kWindowWidth, kWindowHeight, kShade);
    Rectangle panel = {160, 140, 640, 400};
    DrawRectangleRounded(panel, 0.06f, 8, kWhite);
    drawCentered(fonts.fun, "How to play", 480, 150, 50, kInk);
    float y = 228;
    for (const string& line : kHelpLines) {
        DrawCircleV({200, y + 11}, 4, kYouBlue);
        for (const string& part : wrapText(fonts.text, line, 17, 560)) {
            drawText(fonts.text, part, 216, y, 17, kInk);
            y += 25;
        }
        y += 10;
    }
}

/* Draws the whole screen. */
void drawScreen(const Market& market, const Fonts& fonts, const vector<Button>& buttons, bool helpOpen) {
    ClearBackground(kWhite);
    drawHeader(market, fonts);

    if (market.phase() == Phase::DayOver) {
        drawDayOver(market, fonts);
    } else if (market.phase() == Phase::WeekOver) {
        drawWeekOver(market, fonts);
    } else {
        drawStage(market, fonts);
        drawBuyers(market, fonts);
        drawControls(market, fonts);
        drawYourStrip(market, fonts);
    }

    if (helpOpen) {
        for (const Button& button : buttonsFor(market, false)) {
            drawButton(button, fonts);
        }
        drawHelp(fonts);
    }
    for (const Button& button : buttons) {
        drawButton(button, fonts);
    }
}
