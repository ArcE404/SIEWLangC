#include <stdio.h>
#include "siew/common.h"
#include "siew/chunk.h"
#include "siew/debug.h"
#include "siew/vm.h"

int main(int argc, char *argv[]) {
    initVM();

    Chunk chunk;
    initChunk(&chunk);

    int constant = addConstant(&chunk, 1.4);
    writeChunk(&chunk, OP_CONSTANT, 123);
    // first the byte of the Operation
    // then the byte of the operant, in this case what we are writing is the index
    // since addConstant returns the index were the value is saved
    writeChunk(&chunk, constant, 123);
    writeChunk(&chunk, OP_RETURN, 123);

    //disassembleChunk(&chunk, "test chunk");
    interpret(&chunk);

    freeVM();
    freeChunk(&chunk);
    return 0;
}
