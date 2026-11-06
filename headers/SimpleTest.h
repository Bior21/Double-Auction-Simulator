#pragma once

#include <iostream>
#include <string>
#include <vector>

/* A tiny test tool for this project.
 *
 * Write a test at the bottom of any .cpp file like this:
 *
 *     TEST("a new market starts on day 1") {
 *         Market market(7);
 *         EXPECT_EQUAL(market.day(), 1);
 *     }
 *
 * Every TEST adds itself to one list before main() starts. Running the
 * program with --test runs every test on that list and prints the results.
 */

/* One test: its name and the function that runs it. */
struct TestCase {
    std::string name;
    void (*run)();
};

/* Returns the list of every test in the program. */
inline std::vector<TestCase>& allTests() {
    static std::vector<TestCase> tests;
    return tests;
}

/* Returns the number of failed checks in the test that is running now. */
inline int& failedChecks() {
    static int count = 0;
    return count;
}

/* Adds a test to the list. TEST calls this for you. */
inline bool addTest(const std::string& name, void (*run)()) {
    allTests().push_back({name, run});
    return true;
}

/* Records one check. If it failed, prints what was expected and where. */
inline void checkThat(bool passed, const std::string& expression, const char* file, int line) {
    if (!passed) {
        failedChecks()++;
        std::cout << "    failed: " << expression << "  (" << file << ":" << line << ")" << std::endl;
    }
}

/* Runs every test, prints a line for each one, and returns 0 if all of
 * them passed or 1 if any failed.
 */
inline int runAllTests() {
    int testsFailed = 0;
    for (const TestCase& test : allTests()) {
        failedChecks() = 0;
        test.run();
        if (failedChecks() == 0) {
            std::cout << "  pass  " << test.name << std::endl;
        } else {
            std::cout << "  FAIL  " << test.name << std::endl;
            testsFailed++;
        }
    }
    int testsPassed = (int) allTests().size() - testsFailed;
    std::cout << std::endl << testsPassed << " of " << allTests().size() << " tests passed." << std::endl;
    return testsFailed == 0 ? 0 : 1;
}

/* These two macros glue a word and a line number together, so every test
 * function gets its own name, like testCase42.
 */
#define TEST_NAME_JOIN(word, number) word##number
#define TEST_NAME(word, number) TEST_NAME_JOIN(word, number)

/* Defines a test. The block of code after it is the body of the test. */
#define TEST(name)                                                                            \
    static void TEST_NAME(testCase, __LINE__)();                                              \
    [[maybe_unused]] static bool TEST_NAME(testAdded, __LINE__) =                              \
        addTest(name, TEST_NAME(testCase, __LINE__));                                         \
    static void TEST_NAME(testCase, __LINE__)()

/* Checks that a condition is true. */
#define EXPECT(condition) checkThat((condition), #condition, __FILE__, __LINE__)

/* Checks that two values are equal. */
#define EXPECT_EQUAL(actual, expected) \
    checkThat((actual) == (expected), #actual " == " #expected, __FILE__, __LINE__)
