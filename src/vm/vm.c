//
// Created by augus on 9/24/2025.
//

#include "siew/vm.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "siew/debug.h"
#include "siew/compiler.h"
#include "siew/memory.h"
#include "siew/object.h"

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
    vm.objects = NULL;
}

void freeVM() {
    freeObjects();
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

static void concatenate() {
    /* Memory management at its peak.
     * Suppose we have:
     *   a = "hello ";
     *   b = "world";
     *
     * What we’re doing here is allocating a new chunk of memory large enough to
     * hold both strings together (length(a) + length(b)).
     *
     * At first, that new memory block is empty. We copy the bytes of `a` into it:
     *   hello _ _ _ _ _ _
     *
     * Then we move the pointer to the end of `a` (the space in this case) and start
     * copying the bytes of `b`, producing:
     *   hello world_
     *
     * The final byte we write is always the null terminator. Even though our
     * ObjString tracks length explicitly and doesn’t technically need it, spending
     * this single byte keeps our strings compatible with the C std library.
     *
     * Sick.
     */
    ObjString* b = AS_STRING(pop());
    ObjString* a = AS_STRING(pop());

    int length = a->length + b->length;

    char* chars = ALLOCATE(char, length+ 1);

    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);

    chars[length] = '\0';

    // We use takeString instead of copyString because this concatenation isn’t a
    // string literal baked into the source code. This char array is something we
    // built dynamically, and it already lives on the heap.
    //
    // That means the resulting SIEW string object can safely take ownership of
    // this memory. If we used copyString here, not only would it be redundant, but
    // this function would also become responsible for freeing the temporary buffer
    // we just allocated. Totally unnecessary.
    //
    // Instead, we simply hand over the freshly concatenated buffer to the object.
    // takeString claims ownership of the chars we pass to it.
    // Very important detail to remember.
    ObjString* result = takeString(chars, length);
    push(OBJ_VAL(result));
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
            case OP_ADD: {
                // TODO: do that a number and a string can be concatenated
                if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                    concatenate();
                }else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a + b));
                }else {
                    runtimeError(
                        "Operands must be numbers or strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
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