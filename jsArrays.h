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
typedef int (*LengthFunc)(void);

typedef struct jsArrayData *JsArrayData;

struct jsArrayWrapper {
	int* array;
	SortFunc sort;
	ReduceFunc reduce;
	MapFunc map;
	ConcatFunc concat;
	AppendFunc append;
	PrintFunc print;
	LengthFunc length;
	JsArrayData data;
};

JsArray JsArrayNew();
void JsArrayFree(JsArray jsArray);
