#include "jsArrays.h"
#include <ffi.h>
#include <stdlib.h>

static Result setupSort(JsArray self);
static Result setupMap(JsArray self);
static Result setupReduce(JsArray self);
static Result setupConcat(JsArray self);
static Result setupAppend(JsArray self);
static Result setupPrint(JsArray self);

static void resize(JsArray jsArray);
static void sort_handler(ffi_cif* cif, void* ret, void** args, void* user_data);
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

static ffi_type* sortArgTypes[] = { &ffi_type_pointer };
static ffi_type* reduceArgTypes[] = { &ffi_type_pointer, &ffi_type_sint };
static ffi_type* mapArgTypes[]    = { &ffi_type_pointer };
static ffi_type* concatArgTypes[] = { &ffi_type_pointer };
static ffi_type* appendArgTypes[] = { &ffi_type_sint };
static ffi_type* printArgTypes[] = { &ffi_type_pointer };

JsArray JsArrayNew() {
	JsArray self = calloc(1, sizeof(struct jsArrayWrapper));
	if (!self) goto fail;
	
	self->array = malloc(sizeof(int));
	if (!self->array) goto fail;
	
	self->len = 0;
	self->cap = 1;

	if (setupSort(self) == FAILURE) goto fail;
	if (setupReduce(self) == FAILURE) goto fail;
	if (setupMap(self) == FAILURE) goto fail;
	if (setupConcat(self) == FAILURE) goto fail;
	if (setupAppend(self) == FAILURE) goto fail;
	if (setupPrint(self) == FAILURE) goto fail;
	
	return self;
	
fail:
	if (self) JsArrayFree(self);
	return NULL;
}

static Result setupSort(JsArray self) {
	void *code;
	self->sortClosure = ffi_closure_alloc(sizeof(ffi_closure), &code);
	if (!self->sortClosure)
		return FAILURE;

	if (
		ffi_prep_cif(&self->sortCif, FFI_DEFAULT_ABI, 1, &ffi_type_pointer, sortArgTypes) != FFI_OK 
		||ffi_prep_closure_loc(self->sortClosure, &self->sortCif, sort_handler, self, code) != FFI_OK
	)
		return FAILURE;

	self->sort = (SortFunc) code;
	return SUCCESS;
}

static Result setupReduce(JsArray self) {
	void *code;
	self->reduceClosure = ffi_closure_alloc(sizeof(ffi_closure), &code);
	if (!self->reduceClosure)
		return FAILURE;

	if (
		ffi_prep_cif(&self->reduceCif, FFI_DEFAULT_ABI, 2, &ffi_type_sint, reduceArgTypes) != FFI_OK
		|| ffi_prep_closure_loc(self->reduceClosure, &self->reduceCif, reduceHandler, self, code) != FFI_OK
	) 
		return FAILURE;

	self->reduce = (ReduceFunc) code;
	return SUCCESS;
}

static Result setupMap(JsArray self) {
	void *code;
	self->mapClosure = ffi_closure_alloc(sizeof(ffi_closure), &code);
	if (!self->mapClosure)
		return FAILURE;

	if (
		ffi_prep_cif(&self->mapCif, FFI_DEFAULT_ABI, 1, &ffi_type_pointer, mapArgTypes) != FFI_OK
		|| ffi_prep_closure_loc(self->mapClosure, &self->mapCif, mapHandler, self, code) != FFI_OK
	) 
		return FAILURE;

	self->map = (MapFunc) code;
	return SUCCESS;
}

static Result setupConcat(JsArray self) {
	void *code;

	self->concatClosure = ffi_closure_alloc(sizeof(ffi_closure), &code);
	if (!self->concatClosure)
		return FAILURE;

	if (
		ffi_prep_cif(&self->concatCif, FFI_DEFAULT_ABI, 1, &ffi_type_pointer, concatArgTypes) != FFI_OK
		|| ffi_prep_closure_loc(self->concatClosure, &self->concatCif, concatHandler, self, code) != FFI_OK	
	)
		return FAILURE;
	
	self->concat = (ConcatFunc) code;
	return SUCCESS;
}

static Result setupAppend(JsArray self) {
	void *code;

	self->appendClosure = ffi_closure_alloc(sizeof(ffi_closure), &code);
	if (!self->appendClosure) return FAILURE;

	if (
		ffi_prep_cif(&self->appendCif, FFI_DEFAULT_ABI, 1, &ffi_type_sint, appendArgTypes) != FFI_OK
		|| ffi_prep_closure_loc(self->appendClosure, &self->appendCif, appendHandler, self, code) != FFI_OK
	)
		return FAILURE;
	
	self->append = (AppendFunc) code;
	return SUCCESS;
}

static Result setupPrint(JsArray self) {
	void *code; 

	self->printClosure = ffi_closure_alloc(sizeof(ffi_closure), &code);
	if (!self->printClosure) return FAILURE;

	if (
		ffi_prep_cif(&self->printCif, FFI_DEFAULT_ABI, 1, &ffi_type_void, printArgTypes) != FFI_OK
		|| ffi_prep_closure_loc(self->printClosure, &self->printCif, printHandler, self, code) != FFI_OK
	) {
		return FAILURE;
	}

	self->print = (PrintFunc) code;
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
	assert(self);
	
	if (self->array) free(self->array);
	if (self->sortClosure) ffi_closure_free(self->sortClosure);
	if (self->mapClosure) ffi_closure_free(self->mapClosure);
	if (self->reduceClosure)ffi_closure_free(self->reduceClosure);
	if (self->concatClosure) ffi_closure_free(self->concatClosure);
	if (self->appendClosure) ffi_closure_free(self->appendClosure);
	if (self->printClosure) ffi_closure_free(self->printClosure);
	
	free(self);

	return;
}

static void sort_handler(ffi_cif* cif, void* ret, void** args, void* user_data) {
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