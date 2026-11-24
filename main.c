#include "main.h"
#include "jsArrays.h"

VALID_SIZE_TEST(newArray, nullChecks) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr != NULL, "Failed to initialise %zu byte element array", *size);
	JsArrayFree(arr);
}

Test(newArray, incorrectSize) {
	cr_expect(JsArrayNew(10) == NULL, "Incorrectly sized array is not NULL");
}

VALID_SIZE_TEST(newArray, methodChecks) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	cr_expect(arr->sort != NULL,	"Missing sort method on %zu byte element array", *size);
	cr_expect(arr->reduce != NULL,	"Missing reduce method on %zu byte element array", *size);
	cr_expect(arr->map != NULL,	"Missing map method on %zu byte element array", *size);
	cr_expect(arr->concat != NULL,	"Missing concat method on %zu byte element array", *size);
	cr_expect(arr->append != NULL,	"Missing append method on %zu byte element array", *size);
	cr_expect(arr->print != NULL,	"Missing print method on %zu byte element array", *size);
	cr_expect(arr->length != NULL,	"Missing length method on %zu byte element array", *size);
	cr_expect(arr->clone != NULL,	"Missing clone method on %zu byte element array", *size);
	cr_expect(arr->some != NULL,	"Missing some method on %zu byte element array", *size);
	cr_expect(arr->every != NULL,	"Missing every method on %zu byte element array", *size);
	cr_expect(arr->get != NULL,	"Missing get method on %zu byte element array", *size);
	cr_expect(arr->set != NULL,	"Missing set method on %zu byte element array", *size);

	JsArrayFree(arr);
}

VALID_SIZE_TEST(length, newArrayLength) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	cr_expect(arr->length() == 0, "Length of empty %zu byte element array is not zero", *size);
	JsArrayFree(arr);
}

VALID_SIZE_TEST(length, lengthBeforeAndAfterAdding) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	cr_expect(arr->length() == 0, "Length of empty array is not zero");
	arr->append(JsArrayItemise(1, *size));
	cr_expect(arr->length() == 1, "Length of %zu byte element array after adding one element is not 1", *size);
	JsArrayFree(arr);
}

VALID_SIZE_TEST(length, longLengthArray) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	for (int i = 0; i < 1000; i++)
		arr->append(JsArrayItemise(1, *size));

	cr_expect(arr->length() == 1000, "Length of %zu byte element array after adding 1000 elements is not 1000", *size);
	JsArrayFree(arr);
}