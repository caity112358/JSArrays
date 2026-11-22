#include "jsArrays.h"
#include <criterion/criterion.h>
#include <criterion/parameterized.h>

#define VALID_SIZE_PARAMS \
	return cr_make_param_array(size_t, validSizes, sizeof(validSizes) / sizeof(validSizes[0]))

static size_t validSizes[] = {
	sizeof(int64_t),
	sizeof(int32_t),
	sizeof(int16_t),
	sizeof(int8_t)
};

ParameterizedTestParameters(newArray, nullChecks)  { VALID_SIZE_PARAMS; }

ParameterizedTest(size_t *size, newArray, nullChecks) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr != NULL, "Failed to initialise %zu byte element array", *size);
	JsArrayFree(arr);
}

Test(newArray, incorrectSizes) {
	cr_expect(JsArrayNew(10) == NULL, "Incorrectly sized array is not NULL");
}

ParameterizedTestParameters(newArray, methodChecks) { VALID_SIZE_PARAMS; }

ParameterizedTest(size_t *size, newArray, methodChecks) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	cr_expect(arr->sort != NULL, "Missing sort method on %zu byte element array", *size);
	cr_expect(arr->reduce != NULL, "Missing reduce method on %zu byte element array", *size);
	cr_expect(arr->map != NULL, "Missing map method on %zu byte element array", *size);
	cr_expect(arr->concat != NULL, "Missing concat method on %zu byte element array", *size);
	cr_expect(arr->append != NULL, "Missing append method on %zu byte element array", *size);
	cr_expect(arr->print != NULL, "Missing print method on %zu byte element array", *size);
	cr_expect(arr->length != NULL, "Missing length method on %zu byte element array", *size);
	cr_expect(arr->clone != NULL, "Missing clone method on %zu byte element array", *size);
	cr_expect(arr->some != NULL, "Missing some method on %zu byte element array", *size);
	cr_expect(arr->every != NULL, "Missing every method on %zu byte element array", *size);
	cr_expect(arr->get != NULL, "Missing get method on %zu byte element array", *size);
	cr_expect(arr->set != NULL, "Missing set method on %zu byte element array", *size);

	JsArrayFree(arr);
}

ParameterizedTestParameters(length, newArrayLength) { VALID_SIZE_PARAMS; }

ParameterizedTest(size_t *size, length, newArrayLength) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	cr_expect(arr->length() == 0, "Length of empty %zu byte element array is not zero", *size);
	JsArrayFree(arr);
}
