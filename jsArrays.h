#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct jsArrayWrapper *JsArray;

// Returns true if a comes before b
typedef bool (*IntComp)(int a, int b);

// Returns the value that becomes the accum arg
// on the next call
typedef int (*IntReduce)(int accum, int curr);

// Returns the current element mapped to another
// integer
typedef int (*IntMap)(int curr);

// Returns true if the current element passes the test
typedef bool (*IntTest)(int curr);

typedef JsArray (*SortFunc)(IntComp funcptr);
typedef int (*ReduceFunc)(IntReduce funcptr, int start);
typedef JsArray (*MapFunc)(IntMap funcptr);
typedef JsArray (*ConcatFunc)(JsArray other);
typedef int (*AppendFunc)(int item);
typedef void (*PrintFunc)(char *preamble, char *separator, char *end);
typedef int (*LengthFunc)(void);
typedef JsArray (*CloneFunc)(void);
typedef bool (*SomeFunc)(IntTest test);
typedef bool (*EveryFunc)(IntTest test);
typedef void *(*GetFunc)(uint index);
typedef void (*SetFunc)(uint index, void *item);

typedef struct jsArrayData *JsArrayData;

struct jsArrayWrapper {
	// Sorts the array in place
	SortFunc sort;

	// Reduces the array to a single signed int
	ReduceFunc reduce;

	// Maps the array in place using the given IntMap
	MapFunc map;

	// Concatenates a second array to the end of this array
	ConcatFunc concat;

	// Appends a single element to the end of this array
	AppendFunc append;

	// Prints the array with the given preamble, separator, and ending
	PrintFunc print;

	// Returns the "private" length variable
	LengthFunc length;

	// Returns a copy of this JsArray 
	CloneFunc clone;

	// Returns true if the given test passes on any element
	SomeFunc some;

	// Returns true if the given test passes on every element
	EveryFunc every;

	GetFunc get;
	SetFunc set;
	
	// Pointer to extra data defined in the .c file, essentiall private vars
	JsArrayData data;
};

JsArray JsArrayNew(size_t size);
void JsArrayFree(JsArray jsArray);
