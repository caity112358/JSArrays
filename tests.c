#include "tests.h"

bool decreasing(int a, int b) {	return a > b; }
bool increasing(int a, int b) { return a < b; }

int toSum(int accum, int curr) { return accum + curr; }
int toMultiple(int accum, int curr) { return accum * curr; }

int toTimesThree(int curr) { return 3 * curr; }
int toSquare(int curr) { return curr * curr; }

TestInfo tests[NUM_TESTS] = {
	{ .test = test1, .name = "Test 1" }
};

bool test1(void) {
	return 1 != 1;
}
