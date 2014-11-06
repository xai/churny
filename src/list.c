/*
 * Copyright (C) 2014 Olaf Lessenich.  All rights reserved.
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
 */

#include "list.h"

node *head = NULL;
node *cur = NULL;
int size = 0;

node * list_add(char *value) {
	node *new = (node *) malloc(sizeof(node));

	if (new == NULL) {
		return NULL;
	}

	new->value = value;
	new->next = NULL;

	if (cur != NULL) {
		cur->next = new;
	}

	cur = new;

	if (head == NULL) {
		head = new;
	}

#if defined(DEBUG) || defined(TRACE)
	printf("Added %s to list\n", value);
#endif

	size = size + 1;

	return new;
}

bool list_contains(char *value) {
	node *ptr = head;

	while (ptr != NULL) {
		if (strcmp(value, ptr->value) == 0) {
			return true;
		}

		ptr = ptr->next;
	}

	return false;
}

int list_size() {
	return size;
}

void list_clear() {
	node *ptr = head;
	node *del;

	while (ptr != NULL) {
		del = ptr;
		ptr = ptr->next;
		free(del);
		del=NULL;
		size = size - 1;
	}

	head = NULL;
	cur = NULL;

	if (size != 0) {
		printf("Error: List has size %d after clear().\n",
		       size);
	}
}
