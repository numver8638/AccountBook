#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct LinkedList LinkedList, * LinkedListRef;

typedef void (*DeinitHandler)(void* e);
typedef void (*ListIterator)(void* args, uint32_t i, void* e);

LinkedListRef CreateList(size_t sizeofElement);
void RemoveList(LinkedListRef list, DeinitHandler deinit);

bool AddElement(LinkedListRef list, void* e);
void* GetElementAt(LinkedListRef list, uint32_t i);
bool RemoveElement(LinkedListRef list, uint32_t i, DeinitHandler deinit);
void IterateList(LinkedListRef list, ListIterator it, void* args);
uint32_t GetSize(LinkedListRef list);