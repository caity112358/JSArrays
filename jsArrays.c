#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct jsArrayWrapper;
typedef struct jsArrayWrapper *JsArray;

typedef bool (*IntComp)(int a, int b);
typedef int (*IntReduce)(int accum, int curr);
typedef int (*IntMap)(int curr);

typedef void (*SortWrapper)(IntComp functptr);
typedef void (*SortFunc)(IntComp funcptr, JsArray jsArray);

typedef int (*ReduceWrapper) (IntReduce funcptr, int start);
typedef int (*ReduceFunc)(IntReduce funcptr, int start, JsArray jsArray);

typedef JsArray (*MapWrapper)(IntMap funcptr);
typedef JsArray (*MapFunc)(IntMap funcptr, JsArray jsArray);

#define FOR(i, x) for(int i = 0; i < x; i++)

#define PAIR(t1, t2) typedef struct {\
	t1 value1;\
	t1 value2;\
} PAIR_##t1_##t2;

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
void JsArrayFree(JsArray jsArray);
void JsArrayPrint(char *preamble, JsArray jsArray);

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

	JsArray threeTimes = jsArray->map(toTimesThree, jsArray);
	JsArrayPrint("Items in three times mapped array:", threeTimes);

	JsArray square = jsArray->map(toSquare, jsArray);
	JsArrayPrint("Items in square mapped array:", square);

	printf("\n");

	jsArray->sort(isGreater, jsArray);

	JsArrayPrint("Items in isGreater sorted array:", jsArray);

	jsArray->sort(isLesser, jsArray);

	JsArrayPrint("Items in isLesser sorted array:", jsArray);
	
	printf("\n");

	threeTimes->sort(isGreater, threeTimes);

	JsArrayPrint("Items in three times isGreater sorted array:", threeTimes);
	JsArrayFree(threeTimes);

	square->sort(isLesser, square);

	JsArrayPrint("Items in square isLesser sorted array:", square);
	JsArrayFree(square);
	
	printf("\n");

	printf("Array reduced to sum: %d\n", jsArray->reduce(toSum, 0, jsArray));
	printf("Array reduced to multiple: %d\n", jsArray->reduce(toMultiple, 1, jsArray));
	
	printf("\n");

	JsArrayFree(jsArray);
	
	return EXIT_SUCCESS;
}

JsArray JsArrayNew() {
	JsArray jsArray = malloc(sizeof(struct jsArrayWrapper));

	jsArray->array = malloc(sizeof(int));
	jsArray->len = 0;
	jsArray->cap = 1;
	jsArray->sort = sortingFunc;
	jsArray->reduce = reducingFunc;
	jsArray->map = mappingFunc;

	return jsArray;
}

void JsArrayAppend(JsArray jsArray, int num) {		
	jsArray->array[jsArray->len++] = num;

	if (jsArray->len >= jsArray->cap) {		
		jsArray->cap *= 2;
		jsArray->array = realloc(jsArray->array, jsArray->cap * sizeof(int));
		
		if (jsArray->array == NULL) {
			fprintf(stderr, "Bad realloc");
			exit(1);
		}
	}
}

void JsArrayFree(JsArray jsArray) {
	free(jsArray->array);
	free(jsArray);

	return;
}

void JsArrayPrint(char *preamble, JsArray jsArray) {
	printf("%s", preamble);
	
	for (int i = 0; i < jsArray->len; i++)
		printf(" %d", jsArray->array[i]);

	printf("\n");
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
