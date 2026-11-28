#include <criterion/criterion.h>
#include <criterion/parameterized.h>
#include <stdint.h>

#define validSizeTest(suite_name, test_name) \
ParameterizedTestParameters(suite_name, test_name) { \
	return cr_make_param_array(size_t, validSizes, sizeof(validSizes) / sizeof(validSizes[0])); \
} \
ParameterizedTest(size_t *size, suite_name, test_name)

size_t validSizes[4] = {
	sizeof(int64_t),
	sizeof(int32_t),
	sizeof(int16_t),
	sizeof(int8_t)
};

bool increasing(int64_t a, int64_t b) {return a < b; }

int64_t toSum(int64_t accum, int64_t curr) { return accum + curr; }

int64_t toNegative(int64_t curr) { return -curr; }