#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct jsArrayWrapper;
typedef struct jsArrayWrapper *JsArray;

typedef bool (*IntComp)(int a, int b);
typedef int (*IntReduce)(int accum, int curr);
typedef int (*IntMap)(int curr);

typedef void (*SortFunc)(IntComp funcptr, JsArray jsArray);
typedef int (*ReduceFunc)(IntReduce funcptr, int start, JsArray jsArray);
typedef JsArray (*MapFunc)(IntMap funcptr, JsArray jsArray);

struct jsArrayWrapper {
	int* array;
	int len;
	int cap;
	SortFunc sort;
	ReduceFunc reduce;
	MapFunc map;
};

JsArray JsArrayNew();
void JsArrayAppend(JsArray jsArray, int num);

bool isGreater(int a, int b);
bool isLesser(int a, int b);
void sortingFunc(IntComp funcptr, JsArray jsArray);

int toSum(int accum, int curr);
int toMultiple(int accum, int curr);
int reducingFunc(IntReduce funcptr, int start, JsArray jsArray);

int toTimesThree(int curr);
int toSquare(int curr);
JsArray mappingFunc(IntMap funcptr, JsArray jsArray);


typedef struct {
	int* array;
} jsArray;

int main(void) {
	int arrayLength;
	printf("Enter array length: ");
	scanf("%d", &arrayLength);

	JsArray jsArray = JsArrayNew();

	printf("Enter %d integers: ", jsArray->len);
	for (int i = 0; i < jsArray->len; i++) {
		int num;
		scanf("%d", &num);
		JsArrayAppend(jsArray, num);
	}
	printf("\n");

	printf("Array length: %d\n", jsArray->len);

	printf("Items in unsorted array:");
	for (int i = 0; i < jsArray->len; i++)
		printf(" %d", jsArray->array[i]);
	printf("\n\n");

	JsArray threeTimes = jsArray->map(toTimesThree, jsArray);
	printf("Items in three times mapped array:");
	for (int i = 0; i < threeTimes->len; i++)
		printf(" %d", threeTimes->array[i]);
	printf("\n");

	JsArray square = jsArray->map(toSquare, jsArray);
	printf("Items in square mapped array:");
	for (int i = 0; i < square->len; i++)
		printf(" %d", square->array[i]);
	printf("\n\n");

	jsArray->sort(isGreater, jsArray);

	printf("Items in isGreater sorted array:");
	for (int i = 0; i < jsArray->len; i++)
		printf(" %d", jsArray->array[i]);
	printf("\n");

	jsArray->sort(isLesser, jsArray);

	printf("Items in isLesser sorted array:");
	for (int i = 0; i < jsArray->len; i++)
		printf(" %d", jsArray->array[i]);
	printf("\n\n");

	threeTimes->sort(isGreater, threeTimes);

	printf("Items in three times isGreater sorted array:");
	for (int i = 0; i < threeTimes->len; i++)
		printf(" %d", threeTimes->array[i]);
	printf("\n");

	square->sort(isLesser, square);

	printf("Items in square isLesser sorted array:");
	for (int i = 0; i < square->len; i++)
		printf(" %d", square->array[i]);

	printf("\n\n");

	printf("Array reduced to sum: %d\n", jsArray->reduce(toSum, 0, jsArray));

	printf("Array reduced to multiple: %d", jsArray->reduce(toMultiple, 1, jsArray));
	
	printf("\n\n");

	return EXIT_SUCCESS;
}

JsArray JsArrayNew() {
	JsArray jsArray = malloc(sizeof(struct jsArrayWrapper));

	jsArray->array = malloc(8 * sizeof(int));
	jsArray->len = 0;
	jsArray->cap = 8;
	jsArray->sort = sortingFunc;
	jsArray->reduce = reducingFunc;
	jsArray->map = mappingFunc;

	return jsArray;
}

void JsArrayAppend(JsArray jsArray, int num) {
	if (jsArray->len == jsArray->cap) {
		jsArray->array = realloc(jsArray->array, jsArray->cap * 2);
	}
		
	jsArray->array[jsArray->len++] = num;
}

bool isGreater(int a, int b) {
	return a > b;
}

bool isLesser(int a, int b) {
	return a < b;
}

void sortingFunc(IntComp funcptr, JsArray jsArray) {
	int* cArray = jsArray->array;

	for (int i = 0; i < jsArray->len - 1; i++) {
		int min = i;

		for (int j = i + 1; j < jsArray->len; j++)
			if (funcptr(cArray[j], cArray[min])) min = j;

		if (min != i) {
			int temp = cArray[min];
			cArray[min] = cArray[i];
			cArray[i] = temp;
		}
	}
}

int toSum(int accum, int curr) {
	return accum + curr;
}

int toMultiple(int accum, int curr) {
	return accum * curr;
}

int reducingFunc(IntReduce funcptr, int start, JsArray jsArray) {
	int* cArray = jsArray->array;

	int accumulator = start;

	for (int i = 0; i < jsArray->len; i++)
		accumulator = funcptr(accumulator, cArray[i]);

	return accumulator;
}

int toTimesThree(int curr) {
	return 3 * curr;
}

int toSquare(int curr) {
	return curr * curr;
}

JsArray mappingFunc(IntMap funcptr, JsArray jsArray) {
	JsArray mapped = JsArrayNew();

	for (int i = 0; i < jsArray->len; i++) {
		JsArrayAppend(mapped, funcptr(jsArray->array[i]));
	}

	return mapped;
}
