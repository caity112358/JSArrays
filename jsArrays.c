#include "jsArrays.h"
#include <ffi.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef enum {
	BYTES8 = 64,
	BYTES4 = 32,
	BYTES2 = 16,
	BYTE1 = 8
} ElementSize;

typedef union {
	uint64_t bits64;
	uint32_t bits32[2];
	uint16_t bits16[4];
	uint8_t bits8[8];
} Element;

typedef enum {
	FAILURE,
	SUCCESS
} Result;

static Result setupMethod(JsArray self, int i);
static void assignMethod(JsArray self, void *code, int i);
static void resize(JsArray jsArray);
static void resizeTo(JsArray jsArray, int size);

static void sortHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void reduceHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void mapHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void concatHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void appendHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void printHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void lengthHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void cloneHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void someHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void everyHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void getHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void setHandler(ffi_cif* cif, void* ret, void** args, void* user_data);

static JsArray sortingFunc(JsArray self, IntComp funcptr);
static int reducingFunc(JsArray self, IntReduce funcptr, int start);
static JsArray mappingFunc(JsArray self, IntMap funcptr);
static JsArray concattingFunc(JsArray self, JsArray other);
static int appendingFunc(JsArray self, int item);
static void printingFunc(JsArray self, char *preamble, char *separator, char *end);
static JsArray cloningFunc(JsArray self);
static bool someFunc(JsArray self, IntTest test);
static bool everyFunc(JsArray self, IntTest test);
static void *gettingFunc(JsArray self, uint index);
static void settingFunc(JsArray self, uint index, void *item);

typedef void (*HandlerFunc)(ffi_cif* cif, void* ret, void** args, void* user_data);

struct jsArrayData {
	Element *arr;
	ElementSize elementSize;
	size_t len, cap;
	ffi_closure **closures;
	ffi_cif *cifs;
};

#define NUM_METHODS 12

typedef struct {
	int nargs;
	ffi_type *rtype;
	ffi_type **atypes;
	HandlerFunc handler;
} FuncData;

static ffi_type *sortArgTypes[] = { &ffi_type_pointer };
static ffi_type *reduceArgTypes[] = { &ffi_type_pointer, &ffi_type_sint };
static ffi_type *mapArgTypes[] = { &ffi_type_pointer };
static ffi_type *concatArgTypes[] = { &ffi_type_pointer };
static ffi_type *appendArgTypes[] = { &ffi_type_sint };
static ffi_type *printArgTypes[] = { &ffi_type_pointer, &ffi_type_pointer, &ffi_type_pointer };
static ffi_type *someArgTypes[] = { &ffi_type_pointer };
static ffi_type *everyArgTypes[] = { &ffi_type_pointer };
static ffi_type *getArgTypes[] = { &ffi_type_uint };
static ffi_type *setArgTypes[] = { &ffi_type_uint, &ffi_type_pointer };

static FuncData funcData[NUM_METHODS] = {
	{	.nargs = 1,	.rtype = &ffi_type_pointer,	.atypes = sortArgTypes,		.handler = sortHandler },
	{	.nargs = 2,	.rtype = &ffi_type_sint,	.atypes = reduceArgTypes,	.handler = reduceHandler },
	{	.nargs = 1,	.rtype = &ffi_type_pointer,	.atypes = mapArgTypes,		.handler = mapHandler },
	{	.nargs = 1,	.rtype = &ffi_type_pointer,	.atypes = concatArgTypes,	.handler = concatHandler },
	{	.nargs = 1,	.rtype = &ffi_type_sint,	.atypes = appendArgTypes,	.handler = appendHandler },
	{	.nargs = 3,	.rtype = &ffi_type_void,	.atypes = printArgTypes,	.handler = printHandler },
	{	.nargs = 0,	.rtype = &ffi_type_sint,	.atypes = NULL,			.handler = lengthHandler },
	{	.nargs = 0,	.rtype = &ffi_type_pointer,	.atypes = NULL,			.handler = cloneHandler },
	{	.nargs = 1,	.rtype = &ffi_type_uint8,	.atypes = someArgTypes, 	.handler = someHandler },
	{	.nargs = 1,	.rtype = &ffi_type_uint8,	.atypes = everyArgTypes, 	.handler = everyHandler },
	{	.nargs = 1	.rtype = &ffi_type_pointer,	.atypes = getArgTypes,		.handler = getHandler },
	{	.nargs = 2	.rtype = &ffi_type_void,	.atypes = setArgTypes,		.handler = setHandler }
};

JsArray JsArrayNew(size_t size) {
	JsArray self = calloc(1, sizeof(struct jsArrayWrapper));
	if (!self) goto fail;

	self->data = calloc(1, sizeof(struct jsArrayData));
	if (!self->data) goto fail;
	
	self->data->arr = calloc(1, size);
	if (!self->data->arr) goto fail;
	self->data->len = 0;
	self->data->cap = 1;
	self->data->elementSize = size;

	self->data->closures = calloc(NUM_METHODS, sizeof(ffi_closure *));
	self->data->cifs = calloc(NUM_METHODS, sizeof(ffi_cif));
	if (!self->data->closures || !self->data->cifs) goto fail;

	for (int i = 0; i < NUM_METHODS; i++)
		if (setupMethod(self, i) == FAILURE)
			goto fail;

	return self;
fail:
	if (self) JsArrayFree(self);
	return NULL;
}

static Result setupMethod(JsArray self, int i) {
	void *code = NULL;
	self->data->closures[i] = ffi_closure_alloc(sizeof(ffi_closure), &code);
	if (!self->data->closures[i])
		return FAILURE;

	FuncData data = funcData[i];

	if (
		ffi_prep_cif(&self->data->cifs[i], FFI_DEFAULT_ABI, data.nargs, data.rtype, data.atypes) != FFI_OK
		|| ffi_prep_closure_loc(self->data->closures[i], &self->data->cifs[i], data.handler, self, code) != FFI_OK
	)
		return FAILURE;

	assignMethod(self, code, i);
	
	return SUCCESS;
}

static void assignMethod(JsArray self, void *code, int i) {
	switch (i) {
		case 0: self->sort = (SortFunc) code; break;
		case 1: self->reduce = (ReduceFunc) code; break;
		case 2: self->map = (MapFunc) code; break;
		case 3: self->concat = (ConcatFunc) code; break;
		case 4: self->append = (AppendFunc) code; break;
		case 5: self->print = (PrintFunc) code; break;
		case 6: self->length = (LengthFunc) code; break;
		case 7: self->clone = (CloneFunc) code; break;
		case 8: self->some = (SomeFunc) code; break;
		case 9: self->every = (EveryFunc) code; break;
		case 10: self->get = (GetFunc) code; break;
		case 11: self->set = (GetFunc) code; break;
		default: fprintf(stderr, "Unassignable method"); exit(1); break;
	}
}

static void resizeTo(JsArray self, int size) {
	if (self->data->cap >= size) return;
	
	while (self->data->cap < size) {
		self->data->cap *= 2;
	}

	resize(self);
}

static void resize(JsArray self) {	
	self->data->arr = realloc(self->data->arr, self->data->cap * self->data->elementSize);

	if (self->data->arr == NULL) {
		fprintf(stderr, "Bad realloc");
		exit(1);
	}
}

void JsArrayFree(JsArray self) {
	if (!self) return;

	if (!self->data) goto free_end;
	if (self->data->arr) free(self->data->arr);
	if (self->data->closures)
		for (int i = 0; i < NUM_METHODS; i++)
			if(self->data->closures[i])
				ffi_closure_free(self->data->closures[i]);

	if (self->data->closures) free(self->data->closures);
	if (self->data->cifs) free(self->data->cifs);
	free(self->data);

free_end:
	free(self);
	return;
}

static void sortHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	IntComp cmp = *(IntComp*)args[0];

	*(JsArray*) ret = sortingFunc(self, cmp);
}

static JsArray sortingFunc(JsArray self, IntComp funcptr) {
	for (int i = 0; i < self->length() - 1; i++) {
		int min = i;

		for (int j = i + 1; j < self->length(); j++)
			if (funcptr(self->get(j), self->get(min))) min = j;

		if (min != i) {
			int temp = self->get(min);
			self->set(min, self->get(i));
			self->set(i, temp);
		}
	}

	return self;
}

static void reduceHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	IntReduce reduce = *(IntReduce*) args[0];
	int start = *(int *) args[1];

	*(ffi_sarg*) ret = reducingFunc(self, reduce, start);
}

static int reducingFunc(JsArray self, IntReduce funcptr, int start) {
	int* cArray = self->data->arr;

	int accumulator = start;

	for (int i = 0; i < self->length(); i++)
		accumulator = funcptr(accumulator, cArray[i]);

	return accumulator;
}

static void mapHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	IntMap map = *(IntMap *) args[0];

	*(JsArray*) ret = mappingFunc(self, map);
}

static JsArray mappingFunc(JsArray self, IntMap funcptr) {
	for (int i = 0; i < self->length(); i++) {
		self->set(i, funcptr(self->get(i)));
	}

	return self;
}

static void concatHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	JsArray other = *(JsArray *) args[0];

	*(JsArray*) ret = concattingFunc(self, other);
}

static JsArray concattingFunc(JsArray self, JsArray other) {
	resizeTo(self, self->data->len + other->data->len);

	int len = other->length();
	
	for (int i = 0; i < len; i++)
		self->append(other->get(i));

	return self;
}

static void appendHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	int item = *(int *) args[0];

	*(ffi_sarg*) ret = appendingFunc(self, item);
}

static int appendingFunc(JsArray self, int item) {
	self->set(self->data->len++, item);

	if (self->length() >= self->data->cap) {
		resizeTo(self, self->data->cap * 2);
	}

	return self->length();
}

static void printHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	char *preamble = *(char **) args[0];
	char *separator = *(char **) args[1];
	char *end = *(char **) args[2];
	
	printingFunc(self, preamble, separator, end);
}

static void printingFunc(JsArray self, char *preamble, char *separator, char *end) {
	printf("%s", preamble);

	for (int i = 0; i < self->length(); i++) {
		if (i != 0) printf("%s", separator);
		printf("%d", self->get(i));
	}

	printf("%s", end);
}

static void lengthHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;

	*(ffi_sarg*) ret = self->data->len;
}

static void cloneHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;

	*(JsArray*) ret = cloningFunc(self);
}

static JsArray cloningFunc(JsArray self) {
	JsArray new = JsArrayNew(self->data->elementSize);
	if (new == NULL) return NULL;
	
	return new->concat(self);
}

static void someHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	IntTest test = *(IntTest *) args[0];

	*(ffi_arg*) ret =  someFunc(self, test);
}

static bool someFunc(JsArray self, IntTest test) {
	for (int i = 0; i < self->length(); i++) {
		if (test(self->get(i))) return true;
	}

	return false;
}

static void everyHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	IntTest test = *(IntTest *) args[0];

	*(ffi_arg*) ret = everyFunc(self, test);
}

static bool everyFunc(JsArray self, IntTest test) {
	for (int i = 0; i < self->length(); i++) {
		if (!test(self->get(i))) return false;
	}

	return true;
}

static void getHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	uint index = *(uint *) args[0];

	*(void **) ret = gettingFunc(self, index);
}

static void *gettingFunc(JsArray self, uint index) {
	switch (self->data->elementSize) {
		case BYTES8: return (void*)(uintptr_t) self->data->arr[index].bits64;
		case BYTES4: return (void*)(uintptr_t) self->data->arr[index / 2].bits32[index % 2];
		case BYTES2: return (void*)(uintptr_t) self->data->arr[index / 4].bits16[index % 4];
		case BYTE1: return (void*)(uintptr_t) self->data->arr[index / 8].bits8[index % 8];
	}
}

static void setHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	uint index = *(uint *) args[0];
	void *item = *(void **) args[1];
	
	*(void **) ret = gettingFunc(self, index);
}

static void settingFunc(JsArray self, uint index, void *item) {
	switch (self->data->elementSize) {
		case BYTES8: self->data->arr[index].bits64 = (uint64_t)(uintptr_t) item; break;
		case BYTES4: self->data->arr[index / 2].bits32[index % 2] = (uint32_t)(uintptr_t) item; break;
		case BYTES2: self->data->arr[index / 4].bits16[index % 4] = (uint16_t)(uintptr_t) item; break;
		case BYTE1: self->data->arr[index / 8].bits8[index % 8] = (uint8_t)(uintptr_t) item; break;
	}
}