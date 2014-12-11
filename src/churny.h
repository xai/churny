#ifndef CHURNY_H_   /* Include guard */
#define CHURNY_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <git2.h>
#include "utils.h"
#include "loc.h"
#include "list.h"

typedef int interval;
#define YEAR 1
#define MONTH 2

static void usage(const char *basename);
static void print_csv_header();
diffresult calculate_diff(git_repository *repo, const git_oid *prev,
		const git_oid *cur, const char *extension);
void print_results(git_repository *repo, const git_oid *first,
		const git_oid *last, const int num_commits,
		const diffresult diff, int number_authors,
		const char *extension);
diffresult calculate_interval_code_churn(git_repository *repo,
		const interval interval,
		const char *extension);
diffresult calculate_code_churn(git_repository *repo,
		const char *extension);
int main(int argc, char **argv);

#endif
