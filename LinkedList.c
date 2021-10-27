#include "LinkedList.h"

#include <stdlib.h>
#include <memory.h>

typedef struct Node {
	struct Node* Next;
	// Data contains next to Node.
} Node, *NodeRef;

struct LinkedList {
	size_t SizeOfElement;
	uint32_t TotalElements;
	NodeRef First, Last;
};

LinkedListRef CreateList(size_t sizeofElement) {
	LinkedListRef list = malloc(sizeof(LinkedList));

	if (list != NULL) {
		list->SizeOfElement = sizeofElement;
		list->TotalElements = 0;
		list->First = NULL;
		list->Last = NULL;
	}

	return list;
}

void DeinitIterator(void* args, void* e) {
	DeinitHandler handler = (DeinitHandler)args;
	handler(e);
}

void RemoveList(LinkedListRef list, DeinitHandler deinit) {
	NodeRef current = list->First;
	NodeRef next;
	while (current != NULL) {
		next = current->Next;

		if (deinit != NULL) {
			deinit(current + 1);
		}

		free(current);

		current = next;
	}

	free(list);
}

bool AddElement(LinkedListRef list, void* e) {
	NodeRef node = malloc(sizeof(Node) + list->SizeOfElement);

	memset(node, 0, sizeof(Node) + list->SizeOfElement);

	if (node == NULL) {
		return false;
	}

	if (list->First == NULL) {
		list->First = list->Last = node;
	}
	else {
		list->Last->Next = node;
		list->Last = node;
	}

	++list->TotalElements;

	memcpy(node + 1, e, list->SizeOfElement);

	return true;
}

void* GetElementAt(LinkedListRef list, uint32_t i) {
	if (i > list->TotalElements) {
		return NULL;
	}
	else {
		NodeRef node = list->First;
		int index = 0;
		while (index++ < i) {
			node = node->Next;
		}

		return (node + 1);
	}
}

bool RemoveElement(LinkedListRef list, uint32_t index, DeinitHandler deinit) {
	NodeRef node = list->First;
	NodeRef prev = NULL;
	
	if (index > list->TotalElements) {
		return false;
	}
	else if (index == 0) {
		list->First = node->Next;

		if (node == list->Last) { list->Last = NULL; }
	}
	else {
		for (int i = 0; i < index; i++) {
			prev = node;
			node = node->Next;
		}

		prev->Next = node->Next;
		if (node == list->Last) { list->Last = prev; }
	}

	if (deinit != NULL) { deinit(node + 1); }

	free(node);

	--list->TotalElements;

	return true;
}

void IterateList(LinkedListRef list, ListIterator it, void* args) {
	NodeRef node = list->First;
	uint32_t i = 0;

	while (node != NULL) {
		it(args, i, (node+1));

		node = node->Next;
		++i;
	}
}

uint32_t GetSize(LinkedListRef list) {
	return list->TotalElements;
}