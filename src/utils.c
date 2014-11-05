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

#include "utils.h"

const char fatal[] = "[FATAL]";
const char debug[] = "[DEBUG]";
const char trace[] = "[TRACE]";

void print_debug(const char *format, ...) {
#if defined(DEBUG) || defined(TRACE)
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
#endif
}

void print_error(const char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}

void exit_error(const int err, const char *format, ...) {
	va_list args;
	va_start(args, format);
	print_error(format, args);
	va_end(args);
	exit(err);
}

git_time_t tm_to_utc(struct tm *tm) {
	git_time_t time;
	char *tz = getenv("TZ");
	setenv("TZ", "UTC", 1);
	time = mktime(tm);
	if (tz != NULL) {
		setenv("TZ", tz, 1);
	} else {
		unsetenv("TZ");
	}
	return time;
}
