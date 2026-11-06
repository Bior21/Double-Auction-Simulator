#pragma once

#include <vector>
#include "Trader.h"

/* Sorts numbers from smallest to largest with insertion sort. */
void insertionSort(std::vector<int>& numbers);

/* Returns the price where supply meets demand: the middle of the last
 * buyer and seller who could still trade with a profit, after lining up
 * buyers' values from high to low and sellers' costs from low to high.
 * Returns 0 if no buyer and seller can trade at all.
 */
int marketPrice(std::vector<int> values, std::vector<int> costs);

/* Returns the indices of the traders, richest first, using insertion sort. */
std::vector<int> rankByProfit(const std::vector<Trader>& traders);
