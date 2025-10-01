//
// Created by augus on 9/22/2025.
//

#ifndef SIEWLANGC_MEMORY_H
#define SIEWLANGC_MEMORY_H

#include "siew/common.h"
/*
 * this is why the allocation of new memory in the array
 * is consider to be O(1) and not O(n). Because we are
 * growing the capacity by a multiple of its actual capacity.
 */
#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type*)reallocate(pointer, sizeof(type) * (oldCount), \
        sizeof(type) * (newCount))

#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)

void* reallocate(void* pointer, size_t oldSize, size_t newSize);

#endif //SIEWLANGC_MEMORY_H