//
// Created by augus on 9/22/2025.
//
#include <stdlib.h>
#include "siew/memory.h"

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