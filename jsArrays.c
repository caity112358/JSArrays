#include <ffi.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct jsArrayWrapper *JsArray;

typedef bool (*IntComp)(int a, int b);
typedef int (*IntReduce)(int accum, int curr);
typedef int (*IntMap)(int curr);

typedef void (*SortFunc)(IntComp funcptr);

typedef int (*ReduceWrapper) (IntReduce funcptr, int start);
typedef int (*ReduceFunc)(IntReduce funcptr, int start, JsArray jsArray);

typedef JsArray (*MapWrapper)(IntMap funcptr);
typedef JsArray (*MapFunc)(IntMap funcptr, JsArray jsArray);

typedef void (*FreeFunc)();

#define FOR(i, a, x) for(int i = a; i < x; i++)

#define PAIR(t1, t2) typedef struct {\
	t1 value1;\
	t1 value2;\
} PAIR_##t1_##t2;

struct jsArrayWrapper {
	int* array;
	int len;
	int cap;
	SortFunc sort;		ffi_closure* sortClosure;	ffi_cif sortCif;
	ReduceFunc reduce;	ffi_closure* reduceClosure;	ffi_cif reduceCif;
	MapFunc map;		ffi_closure* mapClosure;	ffi_cif mapCif;
	FreeFunc free;
};

static ffi_type* sortArgTypes[] = { &ffi_type_pointer };
static ffi_type* reduceArgTypes[] = { &ffi_type_pointer, &ffi_type_sint };
static ffi_type* mapArgTypes[]    = { &ffi_type_pointer };

JsArray JsArrayNew();
void JsArrayAppend(JsArray jsArray, int num);
static void resize(JsArray jsArray);
void JsArrayFree(JsArray jsArray);
void JsArrayPrint(char *preamble, JsArray jsArray);

bool isGreater(int a, int b);
bool isLesser(int a, int b);

static void sort_handler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void sortingFunc(JsArray self, IntComp funcptr);

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

	printf("Array reduced to sum: %d\n", jsArray->reduce(toSum, 0, jsArray));
	printf("Array reduced to multiple: %d\n", jsArray->reduce(toMultiple, 1, jsArray));
	
	printf("\n");

	JsArrayFree(jsArray);
	
	return EXIT_SUCCESS;
}

static bool setupSort(JsArray self);

JsArray JsArrayNew() {
	JsArray self = malloc(sizeof(struct jsArrayWrapper));

	self->array = malloc(sizeof(int));
	self->len = 0;
	self->cap = 1;

	if (!setupSort(self)) 
		return NULL;
	
	self->reduce = reducingFunc;
	self->map = mappingFunc;
	//self->free = freeFunc;
	
	return self;
}

static bool setupSort(JsArray self) {
	void *code;
	self->sortClosure = ffi_closure_alloc(sizeof(ffi_closure), &code);
	if (!self->sortClosure) {
		free(self->array);
		free(self);
		return false;
	}

	if (ffi_prep_cif(&self->sortCif, FFI_DEFAULT_ABI, 1, &ffi_type_void, sortArgTypes) != FFI_OK ||
	    ffi_prep_closure_loc(self->sortClosure, &self->sortCif, sort_handler, self, code) != FFI_OK) {
		ffi_closure_free(self->sortClosure);
		free(self->array);
		free(self);
		return false;
	}

	self->sort = (SortFunc)code;
}

void JsArrayAppend(JsArray jsArray, int num) {		
	jsArray->array[jsArray->len++] = num;

	if (jsArray->len >= jsArray->cap) {
		resize(jsArray);
	}
}

static void resize(JsArray jsArray) {
	jsArray->cap *= 2;
	jsArray->array = realloc(jsArray->array, jsArray->cap * sizeof(int));
	
	if (jsArray->array == NULL) {
		fprintf(stderr, "Bad realloc");
		exit(1);
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

static void sort_handler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	IntComp cmp = *(IntComp*)args[0];
	sortingFunc(self, cmp);
}

static void sortingFunc(JsArray self, IntComp funcptr) {
	int* cArray = self->array;

	for (int i = 0; i < self->len - 1; i++) {
		int min = i;

		for (int j = i + 1; j < self->len; j++)
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
