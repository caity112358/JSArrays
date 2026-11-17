#include "jsArrays.h"

bool isGreater(int a, int b);
bool isLesser(int a, int b);

int toSum(int accum, int curr);
int toMultiple(int accum, int curr);

int toTimesThree(int curr);
int toSquare(int curr);

int main(void) {
	int arrayLength;
	printf("Enter array length: ");
	scanf("%d", &arrayLength);

	JsArray jsArray = JsArrayNew();

	printf("Enter %d integers: ", arrayLength);
	for (int i = 0; i < arrayLength; i++) {
		int num;
		scanf("%d", &num);
		JsArrayAppend(jsArray, num);
	}
	printf("\n");

	printf("Array length: %d\n", jsArray->len);

	JsArrayPrint("Items in unsorted array:", jsArray);

	printf("\n");

	JsArray threeTimes = jsArray->map(toTimesThree);
	JsArrayPrint("Items in three times mapped array:", threeTimes);

	JsArray square = jsArray->map(toSquare);
	JsArrayPrint("Items in square mapped array:", square);

	printf("\n");

	jsArray->sort(isGreater);

	JsArrayPrint("Items in isGreater sorted array:", jsArray);

	jsArray->sort(isLesser);

	JsArrayPrint("Items in isLesser sorted array:", jsArray);

	printf("\n");

	threeTimes->sort(isGreater);

	JsArrayPrint("Items in three times isGreater sorted array:", threeTimes);
	JsArrayFree(threeTimes);

	square->sort(isLesser);

	JsArrayPrint("Items in square isLesser sorted array:", square);
	JsArrayFree(square);

	printf("\n");

	printf("Array reduced to sum: %d\n", jsArray->reduce(toSum, 0));
	printf("Array reduced to multiple: %d\n", jsArray->reduce(toMultiple, 1));

	printf("\n");

	JsArrayFree(jsArray);

	return EXIT_SUCCESS;
}

bool isGreater(int a, int b) {
	return a > b;
}

bool isLesser(int a, int b) {
	return a < b;
}

int toSum(int accum, int curr) {
	return accum + curr;
}

int toMultiple(int accum, int curr) {
	return accum * curr;
}

int toTimesThree(int curr) {
	return 3 * curr;
}

int toSquare(int curr) {
	return curr * curr;
}
