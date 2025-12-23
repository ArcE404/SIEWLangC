//
// Created by augus on 9/22/2025.
//
#include <stdlib.h>
#include "siew/memory.h"

#include "siew/object.h"
#include "siew/value.h"
#include "siew/vm.h"

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    // if the new size we want to allocate is 0 that means that
    // we need to free space, we don't need it anymore.
    if (newSize == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, newSize);

    // if there is no memory, realloc returns null,
    // we exit in that case
    if (result == NULL) {
        exit(1);
    }

    return result;
}

static void freeObject(Obj* object) {
    switch (object->type) {
        case OBJ_STRING:
            ObjString* string = (ObjString*)object;
            FREE_ARRAY(string->chars, string->chars, string->length);
            FREE(ObjString, object);
            break;
    }
}

void freeObjects() {
    Obj* object = vm.objects;

    while (object != NULL) {
        Obj* next = object->next;
        freeObject(object);
        object = next;
    }
}