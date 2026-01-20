//
// Created by augus on 12/29/2025.
//

#ifndef SIEWLANGC_TABLE_H
#define SIEWLANGC_TABLE_H
#include "value.h"
#include "object.h"

typedef struct {
    ObjString* key;
    Value value;
} Entry;

typedef struct {
    // the ratio of capacity (the allocated size) and the count (the current count of key-pair values entries)
    // are the load factor of this hash table
    int count;
    int capacity;
    Entry* entries;
} Table;

void initTable(Table* table);
void freeTable(Table* table);
bool tableSet(Table* table, ObjString* key, Value value);
void tableAddAll(Table* from, Table* to);
ObjString* tableFindString(Table* table, const char* chars, int length, uint32_t hash);
bool tableGet(Table* table, ObjString* key, Value* value);
bool tableDelete(Table* table, ObjString* key);

#endif //SIEWLANGC_TABLE_H