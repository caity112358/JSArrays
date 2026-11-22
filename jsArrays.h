#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct jsArrayWrapper *JsArray;

// Returns true if a comes before b
// Comp(a, b) -> bool. 
// Will be cast to one of the following
typedef void *Comp;
typedef bool (*Comp64)(int64_t a, int64_t b);
typedef bool (*Comp32)(int32_t a, int32_t b);
typedef bool (*Comp16)(int16_t a, int16_t b);
typedef bool (*Comp8)(int8_t a, int8_t b);


// Returns the value that becomes the accum arg
// on the next call. 
// Reduce(accum, curr) -> accum
// Will be cast to one of the following
typedef void *Reduce;
typedef int (*Reduce64)(int accum, int64_t item);
typedef int (*Reduce32)(int accum, int32_t item);
typedef int (*Reduce16)(int accum, int16_t item);
typedef int (*Reduce8)(int accum, int8_t item);

// Returns the current element mapped to another
// integer.
// Map(curr) -> new
// Will be cast to one of the following
typedef void *Map;
typedef int64_t (*Map64)(int64_t item);
typedef int32_t (*Map32)(int32_t item);
typedef int16_t (*Map16)(int16_t item);
typedef int8_t  (*Map8)(int8_t item);

// Returns true if the current element passes the test
// Test(curr) -> bool
// Will be cast to one of the following
typedef void *Test;
typedef bool (*Test64)(int64_t item);
typedef bool (*Test32)(int32_t item);
typedef bool (*Test16)(int16_t item);
typedef bool (*Test8)(int8_t item);

typedef JsArray (*SortFunc)(Comp funcptr);
typedef int (*ReduceFunc)(Reduce funcptr, int start);
typedef JsArray (*MapFunc)(Map funcptr);
typedef JsArray (*ConcatFunc)(JsArray other);
typedef int (*AppendFunc)(void *item);
typedef void (*PrintFunc)(char *preamble, char *separator, char *end);
typedef int (*LengthFunc)(void);
typedef JsArray (*CloneFunc)(void);
typedef bool (*SomeFunc)(Test test);
typedef bool (*EveryFunc)(Test test);
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
