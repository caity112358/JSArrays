#include "jsArrays.h"

#include <stdio.h>
#include <stdlib.h>

JsArray scanJsArray(void);
int readInt(void);
void errorOut(char *message);

bool decreasing(int a, int b) {	return a > b; }
bool increasing(int a, int b) { return a < b; }

int toSum(int accum, int curr) { return accum + curr; }
int toMultiple(int accum, int curr) { return accum * curr; }

int toTimesThree(int curr) { return 3 * curr; }
int toSquare(int curr) { return curr * curr; }

int main(void) {
	JsArray jsArray = scanJsArray();

	jsArray->print("og array: ", " ", "\n");
	printf("\n");

	JsArray clone = jsArray->clone()->sort(decreasing)->map(toSquare);
	clone->print("sorted and mapped clone: ", "~", "\n\n");

	jsArray->sort(increasing)->map(toTimesThree)->print("Sorted and multiplied og: ", ".", "\n\n");

	jsArray->concat(clone)->print("concattenated arrays: ", "-", "\n\n");
	
	JsArrayFree(clone);
	JsArrayFree(jsArray);

	return 0;
}

JsArray scanJsArray(void) {
	int arrayLength;
	printf("Enter array length: ");
	if (scanf("%d", &arrayLength) != 1)
		errorOut("Failed to scan");

	JsArray jsArray = JsArrayNew();

	printf("Enter %d integers: ", arrayLength);
	for (int i = 0; i < arrayLength; i++)
		jsArray->append(readInt());
	printf("\n");

	return jsArray;
}

int readInt(void) {
	int n;
	if (scanf("%d", &n) != 1)
		errorOut("Failed to scan");

	return n;
}

void errorOut(char *message) {
	fprintf(stderr, "%s\n", message);
	exit(1);
}
