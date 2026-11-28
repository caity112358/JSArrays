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
	checkMethod(shuffle);

	JsArrayFree(arr);
}
#undef checkMethod
#undef METHODFMT

validSizeTest(append, correctReturnVal) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);
	
	cr_expect(arr->append(1) == 1, "Incorrect return value on first append");
	cr_expect(arr->append(1) == 2, "Incorrect return value on second append");
	cr_expect(arr->append(1) == 3, "Incorrect return value on third append");
	JsArrayFree(arr);
}

validSizeTest(append, appendedElementAtCorrectIndex) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	arr->append(-128);
	cr_expect(arr->get(0) == -128, "Element appended to new %zu array is not retrievable", *size);

	for (int i = 0; i < 100; i++)
		arr->append(0);

	arr->append(127);
	cr_expect(arr->get(101) == 127, "Element appended to more filled %zu array is not retrievable", *size);

	JsArrayFree(arr);
}

validSizeTest(append, appendPreservesOrder) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	int64_t cArray[10];
	for (int i = 0; i < 10; i++)
		cArray[i] = (rand() % 255) - 128;

	for (int i = 0; i < 10; i++)
		arr->append(cArray[i]);
	for (int i = 0; i < 10; i++)
		cr_expect(arr->get(i) == cArray[i], "Order is not preserved");

	JsArrayFree(arr);
}

validSizeTest(append, valuesSurviveReallocation) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	arr->append(127);

	for (int i = 0; i < 64; i++)
		arr->append(i);

	cr_expect(arr->get(0) == 127, "Element at index 1 does not survive reallocation");

	JsArrayFree(arr);
}

validSizeTest(append, supportsMillionAppends) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	for (int i = 0; i < 1000000; i++) {
		cr_expect(arr->append(i) == i + 1, "Incorrect element append at position %d", i);
	}

	cr_expect(arr->length() == 1000000, "Array length is not 1000000");
	
	JsArrayFree(arr);
}

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
	arr->append(1);
	cr_expect(arr->length() == 1, "Length of %zu byte element array after adding one element is not 1", *size);
	JsArrayFree(arr);
}

validSizeTest(length, lengthBeforeAndAfterReallocation) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	for (int i = 0; i < 64; i++)
		arr->append(1);

	cr_expect(arr->length() == 64, "Length of %zu byte element array after adding 64 elements is not 64", *size);
	arr->append(1);
	cr_expect(arr->length() == 65, "Length of %zu byte element array after adding 65 elements is not 65", *size);
	
	JsArrayFree(arr);
}

validSizeTest(length, lengthOfLongArray) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	for (int i = 0; i < 1000000; i++)
		arr->append(1);

	cr_expect(arr->length() == 1000000, "Length of %zu byte element array after adding 1000000 elements is not 64", *size);
	JsArrayFree(arr);
}

validSizeTest(length, setDoesntChangeLength) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	for (int i = 0; i < 128; i++)
		arr->append(i);

	int len = arr->length();
	
	arr->set(25, -25);
	
	cr_expect(arr->length() == len, "Set method changed array length");
	
	JsArrayFree(arr);
}

validSizeTest(length, mapDoesntChangeLength) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	for (int i = 0; i < 128; i++)
		arr->append(i);

	int len = arr->length();
	arr->map(toNegative);
	cr_expect(arr->length() == len, "Map method changed array length");
	
	JsArrayFree(arr);
}

validSizeTest(length, reduceDoesntChangeLength) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	for (int i = 0; i < 128; i++)
		arr->append(1);

	int len = arr->length();
	arr->reduce(toSum, 0);
	cr_expect(arr->length() == len, "Reduce method changed array length");
	
	JsArrayFree(arr);
}

validSizeTest(length, sortDoesntChangeLength) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	for (int i = 0; i < 128; i++)
		arr->append((rand() % 255) - 128);

	int len = arr->length();
	arr->sort(increasing);
	cr_expect(arr->length() == len, "Sort method changed array length");
	
	JsArrayFree(arr);
}

validSizeTest(length, printDoesntChangeLength) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	for (int i = 0; i < 128; i++)
		arr->append(i);

	int len = arr->length();
	arr->print("","","\n");
	cr_expect(arr->length() == len, "Print method changed array length");
	
	JsArrayFree(arr);
}

validSizeTest(length, shuffleDoesntChangeLength) {
	JsArray arr = JsArrayNew(*size);
	cr_assert(arr);

	for (int i = 0; i < 128; i++)
		arr->append(i);

	int len = arr->length();
	//arr->shuffle();
	cr_expect(arr->length() == len, "Shuffle method changed array length");
	
	JsArrayFree(arr);
}

/*
Length of a clone, and of the result of a concat
*/



/*
get / set
Get the first, middle, and last element
Set then get returns the new value
Set doesn't clobber neighbours
Negative values come back sign-correct
*/ 

