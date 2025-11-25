//
// Created by augus on 9/23/2025.
//

#ifndef SIEWLANGC_VALUE_H
#define SIEWLANGC_VALUE_H
#include "common.h"

typedef enum {
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER
} ValueType;

// The order of this struct will match the order of how these bytes are going to be sort in memory
// firsts 4 bytes for type, then 4 bytes of padding (we don't control this) and then the largest amount of bytes
// that we hold in our VM types. In the case of double being the largest (for example), would be 8 bytes.
typedef struct {
    ValueType type;
    union  {
       bool boolean;
       double number;
    } as;
} Value;

// Fist we need to host the static value to treat it dynamically
#define BOOL_VAL(value)     ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL             ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value)   ((Value){VAL_NUMBER, {.number = value}})

//then, we need to read the dynamic value to make it static so C can understand it
#define AS_BOOL(value)    ((value).as.boolean)
#define AS_NUMBER(value)  ((value).as.number)

// Now, since we are accessing directly the union part of the memory, we must take are that we are asking
// and saving the right type, so we don't F things up.
#define IS_BOOL(value)    ((value).type == VAL_BOOL)
#define IS_NIL(value)     ((value).type == VAL_NIL)
#define IS_NUMBER(value)  ((value).type == VAL_NUMBER)

typedef struct {
    int capacity;
    int count;
    Value* values;
} ValueArray;

bool valuesEqual(Value a, Value b);
void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);
void printValue(Value value);

#endif //SIEWLANGC_VALUE_H