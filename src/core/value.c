//
// Created by augus on 9/23/2025.
//

#include "siew/value.h"
#include <stdio.h>
#include <string.h>

#include "siew/memory.h"
#include "siew/object.h"

void initValueArray(ValueArray* array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void writeValueArray(ValueArray* array, Value value) {
    if (array->capacity < array->count + 1) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(Value, array->values, oldCapacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(ValueArray* array) {
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}

bool valuesEqual(Value a, Value b) {
    if (a.type != b.type) return false;

    // we cannot use the function memcmp() because of the union that we use in the value struct.
    // C gives no guarantee that the unused memory of the union and the padding between the union and other memory
    // are going to be the same between two Value structs
    switch (a.type) {
        case VAL_BOOL: return AS_BOOL(a) == AS_BOOL(b);
        case VAL_NIL: return true;
        case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
            // now that we’ve interned all the strings, we can take advantage of it in the bytecode interpreter
        case VAL_OBJ: return AS_OBJ(a) == AS_OBJ(b);
        default: return false; // Unreachable
    }
}

void printValue(Value value) {
    switch (value.type) {
        case VAL_BOOL:
            printf(AS_BOOL(value) ? "true" : "false");
            break;
        case VAL_NIL: printf("nil"); break;
        case VAL_NUMBER: printf("%g", AS_NUMBER(value)); break;
        case VAL_OBJ: printObject(value); break;
    }
}