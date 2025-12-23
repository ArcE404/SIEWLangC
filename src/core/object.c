//
// Created by augus on 11/25/2025.
//

#include <stdio.h>
#include <string.h>

#include "siew/memory.h"
#include "siew/object.h"
#include "siew/value.h"
#include "siew/vm.h"


// this macro is to avoid casting to the desire type every time we call the function
// we just add the type and the object type and call it a day
#define ALLOCATE_OBJ(type, objectType) \
(type*)allocateObject(sizeof(type), objectType)

// "parent function" allocate the "base class" bytes and also receive the bytes of the
// child type, allocates all. The pointer is return to the "child function"
// where the yet unused bytes are waiting to be initialized
static Obj* allocateObject(size_t size, ObjType type) {
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;

    object->next = vm.objects;
    vm.objects = object;

    return object;
}

static ObjString* allocateString(char* chars, int length) {
    // we can think of this function as the constructor of a OOP language
    // first we create the base class obj, then we initialize the child class (ObjString)
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    return string;
}

ObjString* copyString(const char* chars, int length) {
    char* heapChars = ALLOCATE(char, length + 1); // + 1 to add the terminator byte
    memcpy(heapChars, chars, length);

    // We receive the raw source lexeme for the string literal, which may not be
    // null-terminated (lets remember that this is just pointing a range of characters inside the monolithic
    // source string). Since ObjString stores its own length explicitly, we could
    // avoid adding a terminator altogether. However, paying the cost of one extra
    // byte is a great trade-off: it lets us interoperate smoothly with standard C
    // library functions that expect null-terminated strings. So we manually append
    // the '\0' here before creating the ObjString.
    heapChars[length] = '\0';
    return allocateString(heapChars, length);
}

ObjString* takeString(char* chars, int length) {
    return allocateString(chars, length);
}

void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
    }
}