//
// Created by augus on 9/22/2025.
//

#ifndef SIEWLANGC_CHUNK_H
#define SIEWLANGC_CHUNK_H

#include "common.h"
#include "value.h"

typedef enum {
    OP_CONSTANT, // TODO: THIS CONSTANT ONLY HAS ONE BYTE (255) OPERANT. THAT'S TOO LITTLE. IMPLEMENT A 24-BIT ONE.
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NEGATE,
    OP_RETURN
} OpCode;

typedef struct{
    int count;
    int capacity;
    uint8_t* code; // this is a dynamic array in C, interesting...
    int* lines; // TODO: THIS IS A WASTE OF MEMORY, FIND A BETTER SOLUTION
    ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
int addConstant(Chunk* chunk, Value value);

#endif //SIEWLANGC_CHUNK_H