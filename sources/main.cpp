/*
 * File: main.cpp
 * Author: Bior Madol Khom Bior
 *
 * This file starts Double Auction Simulator. Run the program on its own to play, or run
 * it with --test to run every test in the project instead:
 *
 *     ./double_auction
 *     ./double_auction --test
 */
#include <string>
#include "App.h"
#include "SimpleTest.h"
using namespace std;

/* Plays the game, or runs the tests when given --test. */
int main(int argc, char* argv[]) {
    if (argc > 1 && string(argv[1]) == "--test") {
        return runAllTests();
    }

    App app;
    app.run();
    return 0;
}
