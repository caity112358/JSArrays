#ifndef JSARRAYS_H
#define JSARRAYS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct jsArrayWrapper *JsArray;

// Returns true if a comes before b
// Comp(a, b) -> bool.
typedef union {
	bool (*Comp64)(int64_t a, int64_t b);
	bool (*Comp32)(int32_t a, int32_t b);
	bool (*Comp16)(int16_t a, int16_t b);
	bool (*Comp8)(int8_t a, int8_t b);
} Comp;

// Returns the value that becomes the accum arg
// on the next call.
// Reduce(accum, curr) -> new accum
typedef union {
	int (*Reduce64)(int accum, int64_t item);
	int (*Reduce32)(int accum, int32_t item);
	int (*Reduce16)(int accum, int16_t item);
	int (*Reduce8)(int accum, int8_t item);
} Reduce;

// Returns the current element mapped to another
// integer.
// Map(curr) -> new
typedef union {
	int64_t (*Map64)(int64_t item);
	int32_t (*Map32)(int32_t item);
	int16_t (*Map16)(int16_t item);
	int8_t  (*Map8)(int8_t item);
} Map;

// Returns true if the current element passes the test
// Test(curr) -> bool
typedef union {
	bool (*Test64)(int64_t item);
	bool (*Test32)(int32_t item);
	bool (*Test16)(int16_t item);
	bool (*Test8)(int8_t item);
} Test;

typedef JsArray (*SortFunc)(Comp funcptr);
typedef int (*ReduceFunc)(Reduce funcptr, int start);
typedef JsArray (*MapFunc)(Map funcptr);
typedef JsArray (*ConcatFunc)(JsArray other);
typedef int (*AppendFunc)(int64_t item);
typedef void (*PrintFunc)(char *preamble, char *separator, char *end);
typedef int (*LengthFunc)(void);
typedef JsArray (*CloneFunc)(void);
typedef bool (*SomeFunc)(Test test);
typedef bool (*EveryFunc)(Test test);
typedef int64_t (*GetFunc)(int index);
typedef void (*SetFunc)(int index, int64_t item);
typedef void (*ShuffleFunc)(void);

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

	// Shuffles the array
	ShuffleFunc shuffle;
	
	// Pointer to extra data defined in the .c file, essentiall private vars
	JsArrayData data;
};

JsArray JsArrayNew(size_t size);
void JsArrayFree(JsArray jsArray);

#endif