#include "jsArrays.h"
#include <ffi.h>
#include <stdlib.h>

static Result setupMethod(JsArray self, int i);

static void resize(JsArray jsArray);

static void sortHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void reduceHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void mapHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void concatHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void appendHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void printHandler(ffi_cif* cif, void* ret, void** args, void* user_data);

static JsArray sortingFunc(JsArray self, IntComp funcptr);
static int reducingFunc(JsArray self, IntReduce funcptr, int start);
static JsArray mappingFunc(JsArray self, IntMap funcptr);
static JsArray concattingFunc(JsArray self, JsArray other);
static int appendingFunc(JsArray self, int item);
static void printingFunc(JsArray self, char *preamble);

typedef void (*HandlerFunc)(ffi_cif* cif, void* ret, void** args, void* user_data);

#define NUM_METHODS 6

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
static ffi_type *printArgTypes[] = { &ffi_type_pointer };

static FuncData funcData[NUM_METHODS] = {
	{ .nargs = 1, .rtype = &ffi_type_pointer, .atypes = sortArgTypes,   .handler = sortHandler },
	{ .nargs = 2, .rtype = &ffi_type_sint,    .atypes = reduceArgTypes, .handler = reduceHandler },
	{ .nargs = 1, .rtype = &ffi_type_pointer, .atypes = mapArgTypes,    .handler = mapHandler },
	{ .nargs = 1, .rtype = &ffi_type_pointer, .atypes = concatArgTypes, .handler = concatHandler },
	{ .nargs = 1, .rtype = &ffi_type_sint,    .atypes = appendArgTypes, .handler = appendHandler },
	{ .nargs = 1, .rtype = &ffi_type_void,    .atypes = printArgTypes,  .handler = printHandler },
};

JsArray JsArrayNew() {
	JsArray self = calloc(1, sizeof(struct jsArrayWrapper));
	if (!self) goto fail;

	self->array = calloc(1, sizeof(int));
	if (!self->array) goto fail;

	self->len = 0;
	self->cap = 1;

	self->closures = calloc(NUM_METHODS, sizeof(ffi_closure *));
	self->cifs = calloc(NUM_METHODS, sizeof(ffi_cif));
	if (!self->closures || !self->cifs) goto fail;

	for (int i = 0; i < NUM_METHODS; i++)
		if (setupMethod(self, i) == FAILURE)
			goto fail;

	return self;
fail:
	if (self) JsArrayFree(self);
	return NULL;
}

static Result setupMethod(JsArray self, int i) {
	void *code;
	self->closures[i] = ffi_closure_alloc(sizeof(ffi_closure), &code);
	if (!self->closures[i])
		return FAILURE;

	FuncData data = funcData[i];

	if (
		ffi_prep_cif(&self->cifs[i], FFI_DEFAULT_ABI, data.nargs, data.rtype, data.atypes) != FFI_OK
		|| ffi_prep_closure_loc(self->closures[i], &self->cifs[i], data.handler, self, code) != FFI_OK
	)
		return FAILURE;

	switch (i) {
		case 0: self->sort = code; break;
		case 1: self->reduce = code; break;
		case 2: self->map = code; break;
		case 3: self->concat = code; break;
		case 4: self->append = code; break;
		case 5: self->print = code; break;
	}
	return SUCCESS;
}

static void resize(JsArray self) {
	self->cap *= 2;
	self->array = realloc(self->array, self->cap * sizeof(int));

	if (self->array == NULL) {
		fprintf(stderr, "Bad realloc");
		exit(1);
	}
}

void JsArrayFree(JsArray self) {
	if (!self) return;

	if (self->array) free(self->array);

	if (self->closures)
		for (int i = 0; i < NUM_METHODS; i++)
			if(self->closures[i])
				ffi_closure_free(self->closures[i]);

	if (self->closures) free(self->closures);
	if (self->cifs) free(self->cifs);

	free(self);

	return;
}

static void sortHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	IntComp cmp = *(IntComp*)args[0];

	*(JsArray*) ret = sortingFunc(self, cmp);
}

static JsArray sortingFunc(JsArray self, IntComp funcptr) {
	int* cArray = self->array;

	for (int i = 0; i < self->len - 1; i++) {
		int min = i;

		for (int j = i + 1; j < self->len; j++)
			if (funcptr(cArray[j], cArray[min])) min = j;

		if (min != i) {
			int temp = cArray[min];
			cArray[min] = cArray[i];
			cArray[i] = temp;
		}
	}

	return self;
}

static void reduceHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	IntReduce reduce = *(IntReduce*) args[0];
	int start = *(int *) args[1];

	*(ffi_arg*) ret = reducingFunc(self, reduce, start);
}

static int reducingFunc(JsArray self, IntReduce funcptr, int start) {
	int* cArray = self->array;

	int accumulator = start;

	for (int i = 0; i < self->len; i++)
		accumulator = funcptr(accumulator, cArray[i]);

	return accumulator;
}

static void mapHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	IntMap map = *(IntMap *) args[0];

	*(JsArray*) ret = mappingFunc(self, map);
}

static JsArray mappingFunc(JsArray self, IntMap funcptr) {
	JsArray mapped = JsArrayNew();
	if (!mapped) return NULL;

	for (int i = 0; i < self->len; i++) {
		mapped->append(funcptr(self->array[i]));
	}

	return mapped;
}

static void concatHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	JsArray other = *(JsArray *) args[0];

	*(JsArray*) ret = concattingFunc(self, other);
}

static JsArray concattingFunc(JsArray self, JsArray other) {
	JsArray new = JsArrayNew();
	if (!new) return NULL;

	for (int i = 0; i < self->len + other->len; i++)
		new->append((i < self->len) ? self->array[i] : other->array[i - self->len]);

	return new;
}

static void appendHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	int item = *(int *) args[0];

	*(ffi_arg*) ret = appendingFunc(self, item);
}

static int appendingFunc(JsArray self, int item) {
	self->array[self->len++] = item;

	if (self->len >= self->cap) {
		resize(self);
	}

	return self->len;
}

static void printHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	char *preamble = *(char **) args[0];

	printingFunc(self, preamble);
}

static void printingFunc(JsArray self, char *preamble) {
	printf("%s", preamble);

	for (int i = 0; i < self->len; i++)
		printf(" %d", self->array[i]);

	printf("\n");
}
