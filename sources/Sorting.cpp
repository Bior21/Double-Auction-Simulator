/*
 * File: Sorting.cpp
 * Author: Bior Madol Khom Bior
 *
 * This file sorts things with insertion sort: take the next item and slide
 * it left past every bigger item until it sits in its place. It is used for
 * the week's standings and for the end-of-day "fair price", which is where
 * the sorted buyers' values (demand) meet the sorted sellers' costs (supply).
 */
#include "Sorting.h"
#include <algorithm>
using namespace std;

/* Insertion sort, smallest first. */
void insertionSort(vector<int>& numbers) {
    for (int next = 1; next < (int) numbers.size(); next++) {
        int item = numbers[next];
        int spot = next;
        while (spot > 0 && numbers[spot - 1] > item) {
            numbers[spot] = numbers[spot - 1];   // slide the bigger number right
            spot--;
        }
        numbers[spot] = item;
    }
}

/* Pairs the highest value with the lowest cost, the next highest with the
 * next lowest, and so on, for as long as the pair could trade at a profit.
 */
int marketPrice(vector<int> values, vector<int> costs) {
    insertionSort(values);
    insertionSort(costs);

    int price = 0;
    int pairs = (int) min(values.size(), costs.size());
    for (int pair = 0; pair < pairs; pair++) {
        int value = values[values.size() - 1 - pair];   // highest values first
        int cost = costs[pair];                          // lowest costs first
        if (value < cost) break;
        price = (value + cost) / 2;
    }
    return price;
}

/* Insertion sort on the traders' indices, by total profit, biggest first. */
vector<int> rankByProfit(const vector<Trader>& traders) {
    vector<int> order;
    for (int index = 0; index < (int) traders.size(); index++) {
        order.push_back(index);
    }
    for (int next = 1; next < (int) order.size(); next++) {
        int item = order[next];
        int spot = next;
        while (spot > 0 && traders[order[spot - 1]].totalProfit < traders[item].totalProfit) {
            order[spot] = order[spot - 1];
            spot--;
        }
        order[spot] = item;
    }
    return order;
}

/* * * * * Test Cases Below This Point * * * * */
#include "SimpleTest.h"

TEST("insertionSort puts numbers in order") {
    vector<int> numbers = {66, 52, 79, 52, 61};
    insertionSort(numbers);
    vector<int> expected = {52, 52, 61, 66, 79};
    EXPECT(numbers == expected);

    vector<int> empty;
    insertionSort(empty);
    EXPECT(empty.empty());
}

TEST("marketPrice finds where supply meets demand") {
    /* Pairs: 85 with 50, 75 with 58, 68 with 64, then 60 with 70 cannot trade.
     * The last pair that can trade is 68 and 64, so the price is 66.
     */
    EXPECT_EQUAL(marketPrice({60, 85, 68, 75}, {70, 50, 64, 58}), 66);
    EXPECT_EQUAL(marketPrice({40}, {50}), 0);
}

TEST("rankByProfit puts the richest trader first") {
    vector<Trader> traders(3);
    traders[0].totalProfit = 20;
    traders[1].totalProfit = 45;
    traders[2].totalProfit = 30;
    vector<int> expected = {1, 2, 0};
    EXPECT(rankByProfit(traders) == expected);
}
