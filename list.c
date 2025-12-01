#include "list.h"

#include <stdlib.h>

struct node* list_push_back(void* e, list** l) {
	struct node* node = *l;

	if (!node) {
		*l = malloc(sizeof(list));
		(*l)->data = e;
		(*l)->next = NULL;

		return *l;
	}

	while (node->next)
		node = node->next;

	node->next = malloc(sizeof(list));
	if (node->next) {
		node->next->data = e;
		node->next->next = NULL;
	}

	return node->next;
}

void free_list(list** l) {
	struct node* node = *l;
	struct node* prev = NULL;

	while (node) {
		if (prev) {
			free(prev->data);
			free(prev);
		}

		prev = node;
		node = node->next;
	}

	if (prev) {
		free(prev->data);
		free(prev);
	}

	*l = NULL;
}

void list_remove(struct node* it, list** l) {
	struct node* node = *l;
	struct node* prev = NULL;

	if (!it)
		return;

	while (node) {
		if (node == it) {
			if (*l == it)
				*l = node->next;

			if (prev)
				prev->next = node->next;

			free(node->data);
			free(node);
			return;
		}

		prev = node;
		node = node->next;
	}
}

void* get_value(struct node* it) {
	return it->data;
}
