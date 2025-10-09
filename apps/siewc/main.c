#include <stdio.h>
#include <stdlib.h>

#include "siew/common.h"
#include "siew/chunk.h"
#include "siew/debug.h"
#include "siew/vm.h"

static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb"); // we open the file, the pointer is at the beginning

    fseek(file, 0L, SEEK_END); // we move the pointer to the end

    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    size_t fileSize = ftell(file); // then we ask how many bytes exist from the end to the beginning
    rewind(file); // we move the pointer again to the beginning

    // + 1 to for the null byte at the end
    char* buffer = (char*)malloc(fileSize + 1); // we save the bytes in a buffer (and we tell that those are chars)

    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    // and we read the file in one go and store it in the buffer
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);

    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }

    // we add the null byte at the end
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

static void runFile(const char* path) {
    char* source = readFile(path);
    InterpretResult result = interpret(source);
    free(source);

    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

// TODO: THIS CAN BE BETTER, HANDLE MULTIPLE LINES, WITH NOT HARDCODED LINE LENGTH LIMIT

static void repl() {
    char line[1024];
    for (;;) {
        printf("siew> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        interpret(line);
    }
}

int main(int argc, char *argv[]) {
    initVM();

    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        fprintf(stderr, "Usage: siew [path]\n");
        exit(64);
    }

    freeVM();
    return 0;
}
