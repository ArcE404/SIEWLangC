//
// Created by augus on 9/24/2025.
//

#include "siew/vm.h"

#include <stdio.h>

#include "siew/debug.h"

VM vm; // this is NOT a good idea. Thread safe left the room

static void resetStack() {
    vm.stackTop = vm.stack;
}

void initVM() {
    resetStack();
}

void freeVM() {
}

void push(Value value) {
    // this is saving the value at the top of the stack
    // remember that we are pointing to the next available space in the stack-array
    *vm.stackTop = value;

    // now we increment the pointer to the next empty slot of the stack-array
    // the next time this function will be executed we will be saving somthing here
    // with the above instruction
    vm.stackTop++;
}

Value pop() {
    // we return the pointer to the most recent used slot in the stack-array
    vm.stackTop--;

    // we return the value. We don't need to delete the value, moving the stack top pointer down is enough
    // to say that the value is no longer in use
    return *vm.stackTop;
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++) // ip advance as soon of the byte is read. Allways the next byte to be used.
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()]) // the byte we read is the index
// this macro feels illegal. Sick.

// now, something important is that the order of the pop() is relevant.
// When we declare a binary operation, and we compile it,
// the left (operant) will be before the right in the stack... duh...
// that means that we must first pop() the right to access the left in the stack... duh x2.
// And in that way we can evaluate our expression the way we agreed (left to right).
#define BINARY_OP(op) \
    do { \
        double b = pop(); \
        double a = pop(); \
        push(a op b); \
    } while (false) // This 'do while' is trick to expand this block of code in almost everywhere, also allowing
    // places with a ';' at the end


    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        for (Value* slot = vm.stack; slot < vm.stackTop; slot++)
        {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");
        disassembleInstruction(vm.chunk, (int) (vm.ip - vm.chunk->code));
#endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_RETURN: {
                printValue(pop());
                printf("\n");
                return INTERPRET_OK;
            }
            case OP_ADD:      BINARY_OP(+); break;
            case OP_SUBTRACT: BINARY_OP(-); break;
            case OP_MULTIPLY: BINARY_OP(*); break;
            case OP_DIVIDE:   BINARY_OP(/); break;
            case OP_NEGATE: push(-pop()); break;
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(Chunk *chunk) {
    vm.chunk = chunk;

    // Instruction pointer to the next byte to execute.
    // We keep it on the VM for easy access.
    // Inside run() it can also be copied to a local variable so the compiler keeps it in a CPU register.
    // This pointer changes constantly, so fewer memory trips = a slightly snappier interpreter
    vm.ip = vm.chunk->code;
    return run();
}