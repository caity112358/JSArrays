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
typedef JsArray (*ConcatFunc)(JsArray other);
typedef int (*AppendFunc)(int item);
typedef void (*PrintFunc)(char *preamble);

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
	ConcatFunc concat;	ffi_closure* concatClosure; 	ffi_cif concatCif;
	AppendFunc append;	ffi_closure* appendClosure;	ffi_cif appendCif;
	PrintFunc print;	ffi_closure* printClosure;	ffi_cif printCif;
};

JsArray JsArrayNew();
void JsArrayFree(JsArray jsArray);