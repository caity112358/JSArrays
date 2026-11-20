#include "jsArrays.h"
#include <ffi.h>
#include <stdbool.h>
#include <stdlib.h>

typedef enum {
	FAILURE,
	SUCCESS
} Result;

static Result setupMethod(JsArray self, int i);
static void assignMethod(JsArray self, void *code, int i);
static void resize(JsArray jsArray);

static void sortHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void reduceHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void mapHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void concatHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void appendHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void printHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void lengthHandler(ffi_cif* cif, void* ret, void** args, void* user_data);
static void cloneHandler(ffi_cif* cif, void* ret, void** args, void* user_data);

static JsArray sortingFunc(JsArray self, IntComp funcptr);
static int reducingFunc(JsArray self, IntReduce funcptr, int start);
static JsArray mappingFunc(JsArray self, IntMap funcptr);
static JsArray concattingFunc(JsArray self, JsArray other);
static int appendingFunc(JsArray self, int item);
static void printingFunc(JsArray self, char *preamble);
static JsArray cloningFunc(JsArray self);

typedef void (*HandlerFunc)(ffi_cif* cif, void* ret, void** args, void* user_data);

struct jsArrayData {
	int len;
	int cap;
	ffi_closure **closures;
	ffi_cif *cifs;
};

#define NUM_METHODS 8

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
static ffi_type *lengthArgTypes[] = {};
static ffi_type *cloneArgTypes[] = {};

static FuncData funcData[NUM_METHODS] = {
	{	.nargs = 1,	.rtype = &ffi_type_pointer,	.atypes = sortArgTypes,		.handler = sortHandler },
	{	.nargs = 2,	.rtype = &ffi_type_sint,	.atypes = reduceArgTypes,	.handler = reduceHandler },
	{	.nargs = 1,	.rtype = &ffi_type_pointer,	.atypes = mapArgTypes,		.handler = mapHandler },
	{	.nargs = 1,	.rtype = &ffi_type_pointer,	.atypes = concatArgTypes,	.handler = concatHandler },
	{	.nargs = 1,	.rtype = &ffi_type_sint,	.atypes = appendArgTypes,	.handler = appendHandler },
	{	.nargs = 1,	.rtype = &ffi_type_void,	.atypes = printArgTypes,	.handler = printHandler },
	{	.nargs = 0,	.rtype = &ffi_type_sint,	.atypes = lengthArgTypes,	.handler = lengthHandler },
	{	.nargs = 0, .rtype = &ffi_type_pointer,	.atypes = cloneArgTypes,	.handler = cloneHandler }
};

JsArray JsArrayNew(void) {
	JsArray self = calloc(1, sizeof(struct jsArrayWrapper));
	if (!self) goto fail;

	self->array = calloc(1, sizeof(int));
	if (!self->array) goto fail;

	self->data = calloc(1, sizeof(struct jsArrayData));
	if (!self->data) goto fail;
	
	self->data->len = 0;
	self->data->cap = 1;

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
	void *code;
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
		default: fprintf(stderr, "Unassignable method"); exit(1); break;
	}
}

static void resize(JsArray self) {
	self->data->cap *= 2;
	self->array = realloc(self->array, self->data->cap * sizeof(int));

	if (self->array == NULL) {
		fprintf(stderr, "Bad realloc");
		exit(1);
	}
}

void JsArrayFree(JsArray self) {
	if (!self) return;

	if (self->array) free(self->array);

	if (!self->data) {
		free(self);
		return;
	}
	
	if (self->data->closures)
		for (int i = 0; i < NUM_METHODS; i++)
			if(self->data->closures[i])
				ffi_closure_free(self->data->closures[i]);

	if (self->data->closures) free(self->data->closures);
	if (self->data->cifs) free(self->data->cifs);

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

	for (int i = 0; i < self->length() - 1; i++) {
		int min = i;

		for (int j = i + 1; j < self->length(); j++)
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
		self->array[i] = funcptr(self->array[i]);
	}

	return self;
}

static void concatHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	JsArray other = *(JsArray *) args[0];

	*(JsArray*) ret = concattingFunc(self, other);
}

static JsArray concattingFunc(JsArray self, JsArray other) {
	for (int i = 0; i < other->length(); i++)
		self->append(other->array[i]);

	return self;
}

static void appendHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	int item = *(int *) args[0];

	*(ffi_arg*) ret = appendingFunc(self, item);
}

static int appendingFunc(JsArray self, int item) {
	self->array[self->data->len++] = item;

	if (self->length() >= self->data->cap) {
		resize(self);
	}

	return self->length();
}

static void printHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;
	char *preamble = *(char **) args[0];

	printingFunc(self, preamble);
}

static void printingFunc(JsArray self, char *preamble) {
	printf("%s", preamble);

	for (int i = 0; i < self->length(); i++)
		printf(" %d", self->array[i]);

	printf("\n");
}

static void lengthHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;

	*(ffi_arg*) ret = self->data->len;
}

static void cloneHandler(ffi_cif* cif, void* ret, void** args, void* user_data) {
	JsArray self = user_data;

	*(JsArray*) ret = cloningFunc(self);
}

static JsArray cloningFunc(JsArray self) {
	JsArray new = JsArrayNew();

	for (int i = 0; i < self->length(); i++) {
		new->append(self->array[i]);
	}

	return new;
}