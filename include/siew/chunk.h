//
// Created by augus on 9/22/2025.
//

#ifndef SIEWLANGC_CHUNK_H
#define SIEWLANGC_CHUNK_H

#include "common.h"
#include "value.h"

typedef enum {
    OP_CONSTANT, // TODO: THIS CONSTANT ONLY HAS ONE BYTE (255) OPERANT. THAT'S TOO LITTLE. IMPLEMENT A 24-BIT ONE.
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    // TODO: Implement "not equal", "greater-equal" and "less-equal" as standalone operations.
    // These operators cannot be desugared using logical negations of other comparisons.
    //
    // Relying on equivalences such as a <= b  being implemented as !(a > b) breaks IEEE 754 rules.
    // Under IEEE 754, any comparison involving a NaN operand returns false. This means:
    //
    //   NaN <= b   → false
    //   NaN > b    → false
    //
    // If we implement <= as the negation of >, we incorrectly get:
    //
    //   NaN <= b   → !(false) → true   // WRONG
    //
    // To remain compliant, each operator must be implemented independently instead of derived
    // from the result of another comparison.
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_RETURN
} OpCode;

typedef struct{
    int count;
    int capacity;
    uint8_t* code;
    int* lines; // TODO: THIS IS A WASTE OF MEMORY, FIND A BETTER SOLUTION
    ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
int addConstant(Chunk* chunk, Value value);

#endif //SIEWLANGC_CHUNK_H