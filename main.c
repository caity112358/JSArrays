#include "main.h"
#include "jsArrays.h"

validSizeTest(newArray, nullChecks) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr != NULL, "Failed to initialise %zu byte element array", *size);
	JsArrayFree(arr);
}

Test(newArray, incorrectSize) {
	cr_expect(JsArrayNew(10) == NULL, "Incorrectly sized array is not NULL");
}

#define METHODFMT "Missing %s method on %zu byte element array"
validSizeTest(newArray, methodChecks) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	cr_expect(arr->sort	!= NULL,	METHODFMT, "sort", *size);
	cr_expect(arr->reduce	!= NULL,	METHODFMT, "reduce", *size);
	cr_expect(arr->map	!= NULL,	METHODFMT, "map", *size);
	cr_expect(arr->concat	!= NULL,	METHODFMT, "concat", *size);
	cr_expect(arr->append	!= NULL,	METHODFMT, "append", *size);
	cr_expect(arr->print	!= NULL,	METHODFMT, "print", *size);
	cr_expect(arr->length	!= NULL,	METHODFMT, "length", *size);
	cr_expect(arr->clone	!= NULL,	METHODFMT, "clone", *size);
	cr_expect(arr->some	!= NULL,	METHODFMT, "some", *size);
	cr_expect(arr->every	!= NULL,	METHODFMT, "every", *size);
	cr_expect(arr->get	!= NULL,	METHODFMT, "get", *size);
	cr_expect(arr->set	!= NULL,	METHODFMT, "set", *size);

	JsArrayFree(arr);
}
#undef METHODFMT

validSizeTest(length, newArrayLength) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	cr_expect(arr->length() == 0, "Length of empty %zu byte element array is not zero", *size);
	JsArrayFree(arr);
}

validSizeTest(length, lengthBeforeAndAfterAdding) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	cr_expect(arr->length() == 0, "Length of empty array is not zero");
	arr->append(JsArrayItemise(1, *size));
	cr_expect(arr->length() == 1, "Length of %zu byte element array after adding one element is not 1", *size);
	JsArrayFree(arr);
}

validSizeTest(length, longLengthArray) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	for (int i = 0; i < 1000; i++)
		arr->append(JsArrayItemise(1, *size));

	cr_expect(arr->length() == 1000, "Length of %zu byte element array after adding 1000 elements is not 1000", *size);
	JsArrayFree(arr);
}