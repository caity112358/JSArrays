#include "jsArrays.h"
#include <limits.h>

int readInt(void);

bool isGreater(int a, int b);
bool isLesser(int a, int b);

int toSum(int accum, int curr);
int toMultiple(int accum, int curr);

int toTimesThree(int curr);
int toSquare(int curr);

int oldmain(void);

int main(void) {
	return oldmain();
}

int readInt(void) {
	int n;
	scanf("%d", &n);
	return n;
}

int newMain(void) {
	int arrayLength;
	printf("Enter array length: ");
	scanf("%d", &arrayLength);

	JsArray jsArray = JsArrayNew();

	printf("Enter %d integers: ", arrayLength);
	for (int i = 0; i < arrayLength; i++) 
		jsArray->append(readInt());

	printf("\n");

	JsArray sortedMapped = jsArray->sort(isGreater)->map(toSquare);
	sortedMapped->print("Sorted and squared array:");

	printf("\n");
	
	// Leaks memory
	jsArray->sort(isLesser)->map(toTimesThree)->print("Sorted and multiplied array:"); 

	JsArrayFree(jsArray);
	JsArrayFree(sortedMapped);
	
	return 0;
}

int oldmain(void) {
	int arrayLength;
	printf("Enter array length: ");
	scanf("%d", &arrayLength);

	JsArray jsArray = JsArrayNew();

	printf("Enter %d integers: ", arrayLength);
	for (int i = 0; i < arrayLength; i++)
		jsArray->append(readInt());
	
	printf("\n");

	printf("Array length: %d\n", jsArray->len);

	jsArray->print("Items in unsorted array:");

	printf("\n");

	JsArray threeTimes = jsArray->map(toTimesThree);
	threeTimes->print("Items in three times mapped array:");

	JsArray square = jsArray->map(toSquare);
	square->print("Items in square mapped array:");

	printf("\n");

	jsArray->sort(isGreater);

	jsArray->print("Items in isGreater sorted array:");

	jsArray->sort(isLesser);

	jsArray->print("Items in isLesser sorted array:");

	printf("\n");

	threeTimes->sort(isGreater);

	threeTimes->print("Items in three times isGreater sorted array:");
	JsArrayFree(threeTimes);

	square->sort(isLesser);

	square->print("Items in square isLesser sorted array:");
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
