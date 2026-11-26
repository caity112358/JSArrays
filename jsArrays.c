#include "jsArrays.h"
#include <ffi.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

typedef enum {
	BYTES8 = 8,
	BYTES4 = 4,
	BYTES2 = 2,
	BYTES1 = 1
} ElementSize;

typedef union {
	int64_t bits64;
	int32_t bits32[2];
	int16_t bits16[4];
	int8_t bits8[8];
} Element;

typedef enum {
	FAILURE,
	SUCCESS
} Result;

static bool validSize(size_t size);
static Result setupMethod(JsArray self, int i);
static void assignMethod(JsArray self, void *code, int i);
static void resize(JsArray jsArray);
static void resizeTo(JsArray jsArray, size_t size);

static void xerrorOut(char *message);

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

static JsArray sortingFunc(JsArray self, Comp funcptr);
static bool sortCompare(JsArray self, Comp funcptr, int64_t a, int64_t b);

static int reducingFunc(JsArray self, Reduce funcptr, int start);
static int reduceElement(JsArray self, Reduce funcptr, int accum, int64_t item);

static JsArray mappingFunc(JsArray self, Map funcptr);
static int64_t mapElement(JsArray self, Map funcptr, int64_t item);

static JsArray concattingFunc(JsArray self, JsArray other);

static int appendingFunc(JsArray self, int64_t item);

static void printingFunc(JsArray self, char *preamble, char *separator, char *end);
static void printElement(JsArray self, int64_t item);

static JsArray cloningFunc(JsArray self);

static bool someFunc(JsArray self, Test test);
static bool everyFunc(JsArray self, Test test);
static bool testElement(JsArray self, Test funcptr, int64_t item);

static int64_t gettingFunc(JsArray self, int index);
static void settingFunc(JsArray self, int index, int64_t item);

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
static ffi_type *appendArgTypes[] = { &ffi_type_sint64 };
static ffi_type *printArgTypes[] = { &ffi_type_pointer, &ffi_type_pointer, &ffi_type_pointer };
static ffi_type *someArgTypes[] = { &ffi_type_pointer };
static ffi_type *everyArgTypes[] = { &ffi_type_pointer };
static ffi_type *getArgTypes[] = { &ffi_type_sint };
static ffi_type *setArgTypes[] = { &ffi_type_sint, &ffi_type_sint64 };

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
	{	.nargs = 1,	.rtype = &ffi_type_sint64,	.atypes = getArgTypes,		.handler = getHandler },
	{	.nargs = 2,	.rtype = &ffi_type_void,	.atypes = setArgTypes,		.handler = setHandler }
};

JsArray JsArrayNew(size_t size) {
	if (!validSize(size)) return NULL;

	JsArray self = calloc(1, sizeof(struct jsArrayWrapper));
	if (!self) goto fail;

	self->data = calloc(1, sizeof(struct jsArrayData));
	if (!self->data) goto fail;

	self->data->arr = calloc(1, sizeof(Element));
	if (!self->data->arr) goto fail;
	self->data->len = 0;
	self->data->cap = 8 / size;
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

static bool validSize(size_t size) {
	return size == BYTES8 || size == BYTES4 || size == BYTES2 || size == BYTES1;
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
		case 0: self->sort	= (SortFunc)	code; break;
		case 1: self->reduce	= (ReduceFunc)	code; break;
		case 2: self->map	= (MapFunc)	code; break;
		case 3: self->concat	= (ConcatFunc)	code; break;
		case 4: self->append	= (AppendFunc)	code; break;
		case 5: self->print	= (PrintFunc)	code; break;
		case 6: self->length	= (LengthFunc)	code; break;
		case 7: self->clone	= (CloneFunc)	code; break;
		case 8: self->some	= (SomeFunc)	code; break;
		case 9: self->every	= (EveryFunc)	code; break;
		case 10: self->get	= (GetFunc)	code; break;
		case 11: self->set	= (SetFunc)	code; break;
		default: xerrorOut("Unassignable method");
	}
}

static void resizeTo(JsArray self, size_t size) {
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
		abort();
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

static void xerrorOut(char *message) {
	fprintf(stderr, "%s\n", message);
	abort();
}

static void sortHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	Comp cmp = *(Comp*)args[0];

	*(JsArray*) ret = sortingFunc(self, cmp);
}

static JsArray sortingFunc(JsArray self, Comp funcptr) {
	for (int i = 0; i < self->data->len - 1; i++) {
		int min = i;

		for (int j = i + 1; j < self->data->len; j++)
			if (sortCompare(self, funcptr, self->get(j), self->get(min))) min = j;

		if (min != i) {
			int64_t temp = self->get(min);
			self->set(min, self->get(i));
			self->set(i, temp);
		}
	}

	return self;
}

static bool sortCompare(JsArray self, Comp funcptr, int64_t a, int64_t b) {
	switch (self->data->elementSize) {
		case BYTES8: return funcptr.Comp64(a, b);
		case BYTES4: return funcptr.Comp32(a, b);
		case BYTES2: return funcptr.Comp16(a, b);
		case BYTES1: return funcptr.Comp8(a, b);
		default: xerrorOut("Failed comparison");
	}

	return NULL;
}

static void reduceHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	Reduce reduce = *(Reduce*) args[0];
	int start = *(int *) args[1];

	*(ffi_sarg*) ret = reducingFunc(self, reduce, start);
}

static int reducingFunc(JsArray self, Reduce funcptr, int start) {
	int accumulator = start;

	for (int i = 0; i < self->data->len; i++)
		accumulator = reduceElement(self, funcptr, accumulator, self->get(i));

	return accumulator;
}

static int reduceElement(JsArray self, Reduce funcptr, int accum, int64_t item) {
	switch (self->data->elementSize) {
		case BYTES8: return funcptr.Reduce64(accum, item);
		case BYTES4: return funcptr.Reduce32(accum, item);
		case BYTES2: return funcptr.Reduce16(accum, item);
		case BYTES1: return funcptr.Reduce8(accum, item);
		default: xerrorOut("Failed reduction");
	}

	return 0;
}

static void mapHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	Map map = *(Map *) args[0];

	*(JsArray*) ret = mappingFunc(self, map);
}

static JsArray mappingFunc(JsArray self, Map funcptr) {
	for (int i = 0; i < self->data->len; i++) {
		self->set(i, mapElement(self, funcptr, self->get(i)));
	}

	return self;
}

static int64_t mapElement(JsArray self, Map funcptr, int64_t item) {
	switch (self->data->elementSize) {
		case BYTES8: return funcptr.Map64(item); break;
		case BYTES4: return funcptr.Map32(item); break;
		case BYTES2: return funcptr.Map16(item); break;
		case BYTES1: return funcptr.Map8(item); break;
		default: xerrorOut("Failed map");
	}

	return 0;
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
	int64_t item = *(int64_t *) args[0];

	*(ffi_sarg*) ret = appendingFunc(self, item);
}

static int appendingFunc(JsArray self, int64_t item) {
	if (self->data->len >= self->data->cap) {
		resizeTo(self, self->data->cap * 2);
	}

	self->set(self->data->len++, item);

	return self->data->len;
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

	for (int i = 0; i < self->data->len; i++) {
		if (i != 0) printf("%s", separator);
		printElement(self, self->get(i));
	}

	printf("%s", end);
}

static void printElement(JsArray self, int64_t item) {
	switch (self->data->elementSize) {
		case BYTES8: printf("%" PRId64, item); break;
		case BYTES4: printf("%" PRId32, (int32_t) item); break;
		case BYTES2: printf("%" PRId16, (int16_t) item); break;
		case BYTES1: printf("%" PRId8, (int8_t) item); break;
		default: xerrorOut("Failed print");
	}
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
	Test test = *(Test *) args[0];

	*(ffi_arg*) ret =  someFunc(self, test);
}

static bool someFunc(JsArray self, Test test) {
	for (int i = 0; i < self->data->len; i++) {
		if (testElement(self, test, self->get(i))) return true;
	}

	return false;
}

static void everyHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	Test test = *(Test *) args[0];

	*(ffi_arg*) ret = everyFunc(self, test);
}

static bool everyFunc(JsArray self, Test test) {
	for (int i = 0; i < self->data->len; i++) {
		if (!testElement(self, test, self->get(i))) return false;
	}

	return true;
}

static bool testElement(JsArray self, Test funcptr, int64_t item) {
	switch (self->data->elementSize) {
		case BYTES8: return funcptr.Test64(item);
		case BYTES4: return funcptr.Test32(item);
		case BYTES2: return funcptr.Test16(item);
		case BYTES1: return funcptr.Test8(item);
		default: xerrorOut("Failed test");
	}

	return NULL;
}

static void getHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	int index = *(int *) args[0];

	*(int64_t *) ret = gettingFunc(self, index);
}

static int64_t gettingFunc(JsArray self, int index) {
	if (abs(index) >= self->data->len) return (int64_t)(int64_t) 0;

	switch (self->data->elementSize) {
		case BYTES8: return self->data->arr[index].bits64; break;
		case BYTES4: return self->data->arr[index / 2].bits32[index % 2]; break;
		case BYTES2: return self->data->arr[index / 4].bits16[index % 4]; break;
		case BYTES1: return self->data->arr[index / 8].bits8[index % 8]; break;
		default: xerrorOut("Failed get");
	}

	return 0;
}

static void setHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	int index = *(int *) args[0];
	int64_t item = *(int64_t *) args[1];

	settingFunc(self, index, item);
}

static void settingFunc(JsArray self, int index, int64_t item) {
	if (abs(index) >= self->data->len) return;

	switch (self->data->elementSize) {
		case BYTES8: self->data->arr[index].bits64 = item; break;
		case BYTES4: self->data->arr[index / 2].bits32[index % 2] = item; break;
		case BYTES2: self->data->arr[index / 4].bits16[index % 4] = item; break;
		case BYTES1: self->data->arr[index / 8].bits8[index % 8] = item; break;
		default: xerrorOut("Failed set");
	}
}
