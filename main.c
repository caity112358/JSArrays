#include "tests.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
	for (int i = 0; i < NUM_TESTS; i++)
		if (!tests[i].test())
			printf("Test %d failed: %s\n", i + 1, tests[i].name);

	return 0;
}
