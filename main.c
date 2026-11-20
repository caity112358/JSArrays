#include "jsArrays.h"
#include <limits.h>

int readInt(void);

bool decreasing(int a, int b) {	return a > b; }
bool increasing(int a, int b) { return a < b; }

int toSum(int accum, int curr) { return accum + curr; }
int toMultiple(int accum, int curr) { return accum * curr; }

int toTimesThree(int curr) { return 3 * curr; }
int toSquare(int curr) { return curr * curr; }

int main(void) {
	int arrayLength;
	printf("Enter array length: ");
	scanf("%d", &arrayLength);

	JsArray jsArray = JsArrayNew();

	printf("Enter %d integers: ", arrayLength);
	for (int i = 0; i < arrayLength; i++)
		jsArray->append(readInt());
	printf("\n");

	jsArray->print("og array:");
	printf("\n");
	
	JsArray clone = jsArray->clone()->sort(decreasing)->map(toSquare);
	clone->print("sorted and mapped clone");
	printf("\n");
	
	jsArray->sort(increasing)->map(toTimesThree)->print("Sorted and multiplied og:");
	printf("\n");
	
	jsArray->concat(clone)->print("concattenated arrays:"); // print should take a separator, maybe even a separator func?
	printf("\n");
	
	JsArrayFree(clone);
	JsArrayFree(jsArray);

	return 0;
}

int readInt(void) {
	int n;
	scanf("%d", &n);
	return n;
}
