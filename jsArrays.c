#include <assert.h>
#include <ffi.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct jsArrayWrapper *JsArray;

typedef bool (*IntComp)(int a, int b);
typedef int (*IntReduce)(int accum, int curr);
typedef int (*IntMap)(int curr);

typedef JsArray (*SortFunc)(IntComp funcptr);
typedef int (*ReduceFunc) (IntReduce funcptr, int start);
typedef JsArray (*MapFunc)(IntMap funcptr);

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
static void reduceHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
int reducingFunc(JsArray self, IntReduce funcptr, int start);

int toTimesThree(int curr);
int toSquare(int curr);
static void mapHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
JsArray mappingFunc(JsArray self, IntMap funcptr);

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

static bool setupSort(JsArray self);
static bool setupMap(JsArray self);
static bool setupReduce(JsArray self);
static bool setupFree(JsArray self);

#define FAILURE false
#define SUCCESS true

JsArray JsArrayNew() {
	JsArray self = malloc(sizeof(struct jsArrayWrapper));
	if (!self) goto fail;
	
	self->array = malloc(sizeof(int));
	if (!self->array) goto fail;
	
	self->len = 0;
	self->cap = 1;

	self->sortClosure = NULL;
	self->mapClosure = NULL;
	self->reduceClosure = NULL;

	if (setupSort(self) == FAILURE) goto fail;
	if (setupReduce(self) == FAILURE) goto fail;
	if (setupMap(self) == FAILURE) goto fail;

	return self;
	
fail:
	if (self) JsArrayFree(self);
	return NULL;
}

static bool setupSort(JsArray self) {
	void *code;
	self->sortClosure = ffi_closure_alloc(sizeof(ffi_closure), &code);
	if (!self->sortClosure)
		return FAILURE;

	if (
		ffi_prep_cif(&self->sortCif, FFI_DEFAULT_ABI, 1, &ffi_type_void, sortArgTypes) != FFI_OK 
		||ffi_prep_closure_loc(self->sortClosure, &self->sortCif, sort_handler, self, code) != FFI_OK
	)
		return FAILURE;

	self->sort = (SortFunc) code;
	return SUCCESS;
}

static bool setupReduce(JsArray self) {
	void *code;
	self->reduceClosure = ffi_closure_alloc(sizeof(ffi_closure), &code);
	if (!self->reduceClosure)
		return FAILURE;

	if (
		ffi_prep_cif(&self->reduceCif, FFI_DEFAULT_ABI, 2, &ffi_type_sint, reduceArgTypes) != FFI_OK
		|| ffi_prep_closure_loc(self->reduceClosure, &self->reduceCif, reduceHandler, self, code) != FFI_OK
	) 
		return FAILURE;

	self->reduce = (ReduceFunc) code;
	return SUCCESS;
}

static bool setupMap(JsArray self) {
	void *code;
	self->mapClosure = ffi_closure_alloc(sizeof(ffi_closure), &code);
	if (!self->mapClosure)
		return FAILURE;

	if (
		ffi_prep_cif(&self->mapCif, FFI_DEFAULT_ABI, 1, &ffi_type_pointer, mapArgTypes) != FFI_OK
		|| ffi_prep_closure_loc(self->mapClosure, &self->mapCif, mapHandler, self, code) != FFI_OK
	) 
		return FAILURE;

	self->map = (MapFunc) code;
	return SUCCESS;
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

void JsArrayFree(JsArray self) {
	assert(self);
	
	if (self->array) free(self->array);
	if (self->sortClosure) ffi_closure_free(self->sortClosure);
	if (self->mapClosure) ffi_closure_free(self->mapClosure);
	if (self->reduceClosure)ffi_closure_free(self->reduceClosure);
	
	free(self);

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
	return sortingFunc(self, cmp);
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

static void reduceHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	IntReduce reduce = *(IntReduce*)args[0];
	int start = *(int *) args[1];
	
	*(ffi_arg*)ret = reducingFunc(self, reduce, start);
}

int reducingFunc(JsArray self, IntReduce funcptr, int start) {
	int* cArray = self->array;

	int accumulator = start;

	for (int i = 0; i < self->len; i++)
		accumulator = funcptr(accumulator, cArray[i]);

	return accumulator;
}

int toTimesThree(int curr) {
	return 3 * curr;
}

int toSquare(int curr) {
	return curr * curr;
}

static void mapHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	IntMap map = *(IntMap *) args[0];
	
	*(JsArray*)ret = mappingFunc(self, map);
}

JsArray mappingFunc(JsArray self, IntMap funcptr) {
	JsArray mapped = JsArrayNew();

	for (int i = 0; i < self->len; i++) {
		JsArrayAppend(mapped, funcptr(self->array[i]));
	}

	return mapped;
}
