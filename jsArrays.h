#include <ffi.h>

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct jsArrayWrapper *JsArray;

typedef bool (*IntComp)(int a, int b);
typedef int (*IntReduce)(int accum, int curr);
typedef int (*IntMap)(int curr);

typedef JsArray (*SortFunc)(IntComp funcptr);
typedef int (*ReduceFunc)(IntReduce funcptr, int start);
typedef JsArray (*MapFunc)(IntMap funcptr);

typedef enum {
	FAILURE,
	SUCCESS
} Result;

struct jsArrayWrapper {
	int* array;
	int len;
	int cap;
	SortFunc sort;		ffi_closure* sortClosure;	ffi_cif sortCif;
	ReduceFunc reduce;	ffi_closure* reduceClosure;	ffi_cif reduceCif;
	MapFunc map;		ffi_closure* mapClosure;	ffi_cif mapCif;
};

JsArray JsArrayNew();
static Result setupSort(JsArray self);
static Result setupMap(JsArray self);
static Result setupReduce(JsArray self);
 
void JsArrayAppend(JsArray jsArray, int num);
static void resize(JsArray jsArray);

void JsArrayFree(JsArray jsArray);

void JsArrayPrint(char *preamble, JsArray jsArray);

static void sort_handler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void sortingFunc(JsArray self, IntComp funcptr);

static void reduceHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
int reducingFunc(JsArray self, IntReduce funcptr, int start);

static void mapHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
JsArray mappingFunc(JsArray self, IntMap funcptr);