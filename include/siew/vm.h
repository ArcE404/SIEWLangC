//
// Created by augus on 9/24/2025.
//

#ifndef SIEWLANGC_VM_H
#define SIEWLANGC_VM_H
#include "chunk.h"

#define STACK_MAX 256 // More than this and: nice stackoverflow. Nerd.

// this is a stack base virtual machine
typedef struct {
    Chunk *chunk;
    // Instruction pointer to the next byte to execute.
    // We keep it on the VM for easy access.
    // Inside run() it can also be copied to a local variable so the compiler keeps it in a CPU register.
    // This pointer changes constantly, so fewer memory trips = a slightly snappier interpreter
    // the reason why this is a pointer is that is faster to point in the middle of a list of bytes
    // the byte we want, that look up a list with an integer index.
    uint8_t* ip; // the name means Instruction Pointer.
    Value stack[STACK_MAX];
    Value* stackTop; // we point at the position past the top, that way we can say that point -> index 0 = empty
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

void initVM();
void freeVM();
InterpretResult interpret(const char* source);
void push(Value value);
Value pop();

#endif //SIEWLANGC_VM_H