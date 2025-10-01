//
// Created by augus on 9/23/2025.
//

#ifndef SIEWLANGC_VALUE_H
#define SIEWLANGC_VALUE_H
#include "common.h"

typedef double Value;

typedef struct {
    int capacity;
    int count;
    Value* values;
} ValueArray;

void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);
void printValue(Value value);

#endif //SIEWLANGC_VALUE_H