//
// Created by augus on 9/22/2025.
//
// TODO: THIS HEADER SHOULD BE PRIVATE INSIDE THE SRC/VM DIRECTORY, FOR NOW IS IN INCLUDE AS A TEMPORAL TESTING SOLUTION
#ifndef SIEWLANGC_DEBUG_H
#define SIEWLANGC_DEBUG_H

#include "chunk.h"

void disassembleChunk(Chunk * chunk, const char* name);
int disassembleInstruction(Chunk* chunk, int offset);

#endif //SIEWLANGC_DEBUG_H