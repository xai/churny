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

#ifndef LIST_H_ /* Include guard */
#define LIST_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct node_struct {
    char* value;
    struct node_struct* next;
};

typedef struct node_struct node;

node* list_add(char* value);

bool list_contains(char* value);

int list_size();

void list_clear();

#endif
