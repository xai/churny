/*
 * Copyright (C) 2014 Olaf Lessenich
 * Copyright (C) 2014-2015 University of Passau, Germany
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License v2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 021110-1307, USA.
 *
 * Contributors:
 *     Olaf Lessenich <lessenic@fim.uni-passau.de>
 */

#include "list.h"

List* list_create() {
    List* list = (List*)malloc(sizeof(List));
    list->first = NULL;
    list->last = NULL;
    list->size = 0;
    return list;
}

void list_add(List* list, void* value) {
    Node* new = (Node*)malloc(sizeof(Node));

    if (new == NULL) {
        return;
    }

    new->value = value;
	new->prev = list->last;
    new->next = NULL;

    if (list->last == NULL) {
        list->first = new;
        list->last = new;
    } else {
        list->last->next = new;
        list->last = new;
    }

    list->size = list->size + 1;
}

bool list_contains(
    List* list, void* value, int (*cmp)(void const*, void const*)) {
    Node* ptr = list->first;

    while (ptr != NULL) {
        if (cmp(value, ptr->value) == 0) {
            return true;
        }

        ptr = ptr->next;
    }

    return false;
}

int list_size(List* list) { return list->size; }

void list_clear(List* list) {
    Node* ptr = list->first;
    Node* del;

    while (ptr != NULL) {
        del = ptr;
        ptr = ptr->next;
        free(del);
        del = NULL;
        list->size = list->size - 1;
    }

    list->first = NULL;
    list->last = NULL;
}

Node* List_get(List* list, int pos) {
	if (pos >= list->size) {
		return NULL;
	}

    Node* ptr = list->first;

	for (int i = 0; i < pos; i++) {
		ptr = ptr->next;
	}

	return ptr;
}

void list_destroy(List* list) {
    list_clear(list);
    free(list);
}

int string_compare(void const* item1, void const* item2) {
    return strcmp((char const*)item1, (char const*)item2);
}
