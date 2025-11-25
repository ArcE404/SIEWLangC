//
// Created by augus on 9/24/2025.
//

#include "siew/vm.h"

#include <stdarg.h>
#include <stdio.h>

#include "siew/debug.h"
#include "siew/compiler.h"

VM vm; // this is NOT a good idea. Thread safe left the room

static void resetStack() {
    vm.stackTop = vm.stack;
};

static void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm.ip - vm.chunk->code - 1;
    int line = vm.chunk->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
    resetStack();
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

static Value peek(int distance) {
    return vm.stackTop[-1 - distance];
}

static bool isFalsey(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++) // ip advance as soon of the byte is read. Allways the next byte to be used.
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()]) // the byte we read is the index

// This macro feels illegal. Sick.
// Now, something important is that the order of the pop() is relevant.
// When we declare a binary operation, and we compile it,
// the left (operant) will be before the right in the stack... duh...
// that means that we must first pop() the right to access the left in the stack... duh x2.
// And in that way we can evaluate our expression the way we agreed (left to right).
#define BINARY_OP(valueType, op) \
    do { \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
            runtimeError("Operands must be numbers."); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        double b = AS_NUMBER(pop()); \
        double a = AS_NUMBER(pop()); \
        push(valueType(a op b)); \
    } while (false) // This 'do while' is a trick to expand this block of code in almost everywhere, also allowing
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
            case OP_ADD:      BINARY_OP(NUMBER_VAL, +); break;
            case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE:   BINARY_OP(NUMBER_VAL, /); break;
            case OP_NOT:
                push(BOOL_VAL(isFalsey(pop())));
                break;
                // we can do a micro optimization here, just by negating the value directly
                // without poping and pushing the value, leaving the stack top alone
            case OP_NEGATE:
                // We should peek and not pop the result because the garbage collector
                // should be able to find the constants if a collection is trigger during
                // an operation
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OP_NIL: push(NIL_VAL); break;
            case OP_TRUE: push(BOOL_VAL(true)); break;
            case OP_FALSE: push(BOOL_VAL(false)); break;
            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }
            case OP_GREATER:  BINARY_OP(BOOL_VAL, >); break;
            case OP_LESS:     BINARY_OP(BOOL_VAL, <); break;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(const char* source) {
    Chunk chunk;
    initChunk(&chunk);

    if (!compile(source, &chunk)) {
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;

    InterpretResult result = run();

    freeChunk(&chunk);
    return result;
}