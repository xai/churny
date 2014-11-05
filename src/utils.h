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

#ifndef UTILS_H_   /* Include guard */
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <git2.h>

extern const char fatal[];
extern const char debug[];
extern const char trace[];

typedef struct {
	unsigned long int insertions;
	unsigned long int deletions;
	unsigned long int changes;
} diffresult;

void print_debug(const char *format, ...);
void print_error(const char *format, ...);
void exit_error(const int err, const char *format, ...);
git_time_t tm_to_utc(struct tm *tm);

#endif
