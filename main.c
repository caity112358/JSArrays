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
#define checkMethod(methodName) cr_expect(arr->methodName != NULL, METHODFMT, "methodName", *size)
validSizeTest(newArray, methodChecks) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);
	
	checkMethod(sort);
	checkMethod(reduce);
	checkMethod(map);
	checkMethod(concat);
	checkMethod(append);
	checkMethod(print);
	checkMethod(length);
	checkMethod(clone);
	checkMethod(some);
	checkMethod(every);
	checkMethod(get);
	checkMethod(set);

	JsArrayFree(arr);
}
#undef checkMethod
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

validSizeTest(length, lengthBeforeAndAfterReallocation) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	for (int i = 0; i < 64; i++)
		arr->append(JsArrayItemise(1, *size));

	cr_expect(arr->length() == 64, "Length of %zu byte element array after adding 64 elements is not 64", *size);
	arr->append(JsArrayItemise(1, *size));
	cr_expect(arr->length() == 65, "Length of %zu byte element array after adding 65 elements is not 65", *size);
	
	JsArrayFree(arr);
}

validSizeTest(length, lengthOfLongArray) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	for (int i = 0; i < 1000000; i++)
		arr->append(JsArrayItemise(1, *size));

	cr_expect(arr->length() == 1000000, "Length of %zu byte element array after adding 1000000 elements is not 64", *size);
	JsArrayFree(arr);
}

