#pragma once
#include "jsArrays.h"
#include <stdbool.h>

#define NUM_TESTS 1

typedef bool (*TestFunc)(void);
typedef struct {
	TestFunc test;
	char *name;
} TestInfo;

extern TestInfo tests[NUM_TESTS];

bool test1(void);
