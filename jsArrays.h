#ifndef JSARRAYS_H
#define JSARRAYS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct jsArrayWrapper *JsArray;

// Returns true if a comes before b
// Comp(a, b) -> bool.
typedef bool (*Comp)(int64_t a, int64_t b);

// Returns the value that becomes the accum arg
// on the next call.
// Reduce(accum, curr) -> new accum
typedef int64_t (*Reduce)(int64_t accum, int64_t item);

// Returns the current element mapped to another
// integer.
// Map(curr) -> new
typedef int64_t (*Map)(int64_t item);

// Returns true if the current element passes the test
// Test(curr) -> bool
typedef bool (*Test)(int64_t item);

typedef JsArray (*SortFunc)(Comp funcptr);
typedef int64_t (*ReduceFunc)(Reduce funcptr, int64_t start);
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

#endif