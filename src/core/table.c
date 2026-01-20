//
// Created by augus on 1/1/2026.
//

#include "siew/table.h"

#include <string.h>

#include "siew/memory.h"

// This value should be chosen based on benchmarking to achieve an optimal
// hash table load factor.
#define TABLE_MAX_LOAD 0.75

void initTable(Table* table) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void freeTable(Table* table) {
    FREE_ARRAY(Entry, table->entries, table->capacity);
    initTable(table);
}


ObjString* tableFindString(Table* table, const char* chars, int length, uint32_t hash) {
    if (table->count == 0) return NULL;

    uint32_t index = hash % table->capacity;
    for (;;) {
        Entry* entry = &table->entries[index];
        if (entry->key == NULL) {
            // Stop if we find an empty non-tombstone entry.
            if (IS_NIL(entry->value)) return NULL;
        } else if (entry->key->length == length &&
            entry->key->hash == hash &&
            // This is the one place in the VM where we actually test strings for textual equality.
            // We do it here to deduplicate strings and then the rest of the VM can take for granted
            // that any two strings at different addresses in memory must have different contents.
            memcmp(entry->key->chars, chars, length) == 0) {
            // We found it.
            return entry->key;
            }
        index = (index + 1) % table->capacity;
    }
}

static Entry* findEntry(Entry* entries, int capacity, ObjString* key) {
    uint32_t index = key->hash % capacity;
    Entry* tombstone = NULL;
    for (;;) {
        Entry* entry = &entries[index];
        if (entry->key == NULL) {
            if (IS_NIL(entry->value)) {
                // Empty entry.
                // if it happens that we don't have the value, but we encounter a tombstone in the probing,
                // we should use the tombstone that is available
                return tombstone != NULL ? tombstone : entry;
            } else {
                // we found a tombstone
                if (tombstone == NULL) tombstone = entry;
            }
        } else if (entry->key == key) {
            // we found the key
            return entry;
        }

        index = (index + 1) % capacity;
    }
}

// In an open-addressed hash table (linear probing), entries cannot be removed
// by simply clearing the slot (setting it to NULL).
//
// Probing stops when a NULL entry is found, which signals that the key does
// not exist further in the probe sequence. If we removed an entry in the
// middle of a probe chain by setting it to NULL, we would break the chain and
// make subsequent entries unreachable.
//
// Example:
//   value1 -> value2 -> value3 -> NULL
//
// If value2 were removed by setting it to NULL:
//   value1 -> NULL -> value3 (unreachable) -> NULL
//
// To avoid breaking the probe sequence, we use a *tombstone*.
// A tombstone marks an entry as deleted while still allowing probing to
// continue through it.
//
// Example with tombstone:
//   value1 -> value2 (tombstone) -> value3 -> NULL
//
// Tombstones can later be reused by insertions, preserving correctness
// without prematurely terminating the probe sequence.
bool tableDelete(Table* table, ObjString* key) {
    if (table->count == 0) return false;

    Entry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    // we place a tombstone in the entry

    entry->key = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}


static void adjustCapacity(Table* table, int capacity) {
    // we are allocating memory, we are not growing the array, this means that the table->entries will still
    // be there around after this allocation
    Entry* entries = ALLOCATE(Entry, capacity);

    // here we create the new buckets for the new array with the new capacity
    // the count may change because we are counting tombstones in the setTable function
    // we don't count the tombstones here, we just ignore them.
    table->count = 0;
    for (int i = 0; i < capacity; i++) {
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
    }

    // now we need to re-insert the old entries into the new array so they will stay in the place they got before the
    // array got bigger
    for (int i = 0; i < table->capacity; i) {
        Entry* entry = &table->entries[i];
        // if the key is empty, we continue, this means that we are effectively ignoring tombstones, since
        // they have null key
        if (entry->key == NULL) continue;

        Entry* dest = findEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        // we just count those values that are not tombstones
        table->count++;
    }

    // we must release the memory of the old array
    FREE_ARRAY(Entry, table->entries, table->capacity);
    table->entries = entries;
    table->capacity = capacity;
}

void tableAddAll(Table* from, Table* to) {
    for (int i = 0; i < from->capacity; i++) {
        Entry* entry = &from->entries[i];
        if (entry->key != NULL) {
           tableSet(to, entry->key, entry->value);
        }
    }
}

bool tableGet(Table* table, ObjString* key, Value* value) {
    if (table->count == 0) return false;

    Entry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    *value = entry->value;
    return true;
}

bool tableSet(Table* table, ObjString* key, Value value) {
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        int capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }

    Entry* entry = findEntry(table->entries, table->capacity, key);
    bool isNewKey = entry->key == NULL;
    // if we had a new key but the value is not empty, that means that we are using a tombstone
    // we don't increment the counter in this case, we just continue.
    // This means that we are considering the tombstone as full entries.
    if (isNewKey && IS_NIL(entry->value)) table->count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}