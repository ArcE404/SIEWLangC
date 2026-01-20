//
// Created by augus on 11/25/2025.
//

#ifndef SIEWLANGC_OBJECT_H
#define SIEWLANGC_OBJECT_H

#include "common.h"
#include "value.h"

#define OBJ_TYPE(value)     (AS_OBJ(value)->type)

/*
 * One important detail about C macros is that their parameters are not evaluated once.
 * Instead, the macro simply performs a literal text substitution: every place the
 * parameter appears inside the macro body, the argument expression is expanded again.
 *
 * This becomes a serious problem when the argument has side effects. For example,
 * passing something like POP() into a macro that uses the parameter twice will call
 * POP() twice, removing two values from the stack instead of one.
 *
 * In the case of the isObjType() logic, we need to access the value only once.
 * Writing the whole check as a macro would cause multiple evaluations of the 'value'
 * parameter, which breaks the VM whenever the caller passes an expression that
 * manipulates the stack or has other side effects.
 *
 * We avoid this by using the function, we evaluate value just once this way.
 */
#define IS_STRING(value)       isObjType(value, OBJ_STRING)

#define AS_STRING(value)       ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)      (((ObjString*)AS_OBJ(value))->chars)

typedef enum {
    OBJ_STRING,
} ObjType;

struct Obj {
    ObjType type;
    Obj* next;
};

/*
 * This is a handmade inheritance. Another thing that feels illegal.
 * Since C has a spec that guarantees there is no padding at the beginning of a struct,
 * and also that the fields appear in memory in the exact order they are declared,
 * the layout of a struct is predictable. In other words, what we write in a struct
 * definition is the same order the bytes will have in memory.
 *
 * Because the first field of ObjString is an Obj, the bytes of an ObjString begin
 * exactly with a valid Obj header. This allows us to safely treat a pointer to an
 * ObjString as if it were a pointer to an Obj: both, at the beginning, has the same
 * memory layout.
 *
 * For this reason, when we store an ObjString inside a Value, we store it as an Obj*.
 * C does not validate the actual dynamic type of the pointer,
 * As a mater of fact, C doesn't care at us at all and trust we are competent
 * enough to respect its specs, therefore it simply trusts that the cast makes sense.
 * And since the memory really does start with a full Obj struct, the interpretation is correct.
 *
 * Later, when we want to “downcast” the Obj* back to its real type (ObjString*),
 * we manually cast the pointer. Again, no runtime checks occur, but the cast is safe
 * because we know the original object was allocated as an ObjString, and its layout
 * extends the Obj header.
 */

struct ObjString {
    Obj obj;
    int length;
    char* chars;
    uint32_t hash;
};

ObjString* takeString(char* chars, int length);

ObjString* copyString(const char* chars, int length);
void printObject(Value value);

static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif //SIEWLANGC_OBJECT_H