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
 * TODO: Features
 * - filter file extensions (via regex and presets)
 * - on-demand checkout from a server
 * - take 2 directories as input
 * - take 2 tars as input
 *
 * TODO: Maintenance
 * - create unit tests
 * - check frees
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <git2.h>

static const char fatal[] = "[FATAL]";
static const char debug[] = "[DEBUG]";

static void print_debug(const char *format, ...) {
#ifdef DEBUG
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
#endif
}

static void print_error(const char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}

static void exit_error(const int err, const char *format, ...) {
	va_list args;
	va_start(args, format);
	print_error(format, args);
	va_end(args);
	exit(err);
}

void usage(const char *basename) {
	printf("Usage: %s [option]... [file]\n", basename);
	printf("  h\tPrints this message\n");
	printf("  n K\tConsider only K commits\n");
	printf("  t K\tConsider only commits within the last K years\n");
	printf("\n");
}

void print_csv_header(void) {
	printf("Commits;Initial LoC;HEAD LoC;Ratio;Changed LoC;Relative Code Churn;\n");
}

int calculate_loc(git_repository *repo, const git_oid *oid) {
	const char id[] = "calculate_loc";

	int loc = 0;
	git_commit *commit;
	git_object *gobject;

	git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
	opts.checkout_strategy = GIT_CHECKOUT_FORCE;

	if (oid != NULL) {
	chdir(git_repository_workdir(repo));

#ifdef DEBUG
		char buf[GIT_OID_HEXSZ + 1];
		git_oid_fmt(buf, oid);
		buf[GIT_OID_HEXSZ] = '\0';
		print_debug("%s %s - counting lines of commit %s: ", debug, id, buf);
#endif

		git_commit_lookup(&commit, repo, oid);
		git_object_lookup(&gobject, repo, oid, GIT_OBJ_COMMIT);
		git_checkout_tree(repo, gobject, &opts);
	} else {
		git_checkout_head(repo, &opts);
	}

	FILE *fp;

	//fp = popen("git ls-files 2>/dev/null | xargs cat 2>/dev/null | wc -l", "r");
	fp = popen("find . -type f -not -path './.git/*' -print | xargs cat 2>/dev/null | wc -l", "r");
	char prbuf[1024];
	memset(prbuf, 0, sizeof(prbuf));

	if (fp == NULL) {
		git_checkout_head(repo, NULL);
		perror("popen");
		exit(EXIT_FAILURE);
	}

	if (fgets(prbuf, sizeof(prbuf) - 1, fp) != NULL) {
		loc = atoi(prbuf);
	} else {
		exit_error(EXIT_FAILURE, "%s %s - Error while counting lines of code\n",
				fatal, id);
	}

	pclose(fp);
	git_object_free(gobject);
	git_commit_free(commit);

	if (oid != NULL) {
		git_checkout_head(repo, &opts);
	}

#ifdef DEBUG
	print_debug("%d\n", loc);
#endif

	return loc;
}

int calculate_diff(git_repository *repo, const git_oid *prev,
		const git_oid *cur) {
	const char id[] = "calculate_diff";

#ifdef DEBUG
	char prev_buf[GIT_OID_HEXSZ + 1];
	char cur_buf[GIT_OID_HEXSZ + 1];
	git_oid_fmt(prev_buf, prev);
	prev_buf[GIT_OID_HEXSZ] = '\0';
	git_oid_fmt(cur_buf, cur);
	cur_buf[GIT_OID_HEXSZ] = '\0';
#endif

	git_commit *commit;
	git_tree *prev_tree;
	git_tree *cur_tree;
	git_diff *diff;
	time_t prev_time;
	time_t cur_time;
	struct tm *tm;
	
	git_commit_lookup(&commit, repo, prev);
	prev_time = git_commit_time(commit);
	git_commit_tree(&prev_tree, commit);
	git_commit_lookup(&commit, repo, cur);
	cur_time = git_commit_time(commit);
	git_commit_tree(&cur_tree, commit);

	// run diff
	git_diff_tree_to_tree(&diff, repo, prev_tree, cur_tree, NULL);

	// get stats
	git_diff_stats *stats;
	git_buf b = GIT_BUF_INIT_CONST(NULL, 0);
	git_diff_stats_format_t format = 0;
	git_diff_get_stats(&stats, diff);
	format |= GIT_DIFF_STATS_NUMBER;
	git_diff_stats_to_buf(&b, stats, format, 80);

#ifdef TRACE
	fputs(b.ptr, stdout);
#endif

	int insertions = git_diff_stats_insertions(stats);
	int deletions = git_diff_stats_deletions(stats);

	//cleanup
	git_buf_free(&b);
	git_diff_stats_free(stats);
	git_diff_free(diff);
	git_tree_free(prev_tree);
	git_tree_free(cur_tree);
	git_commit_free(commit);

	int churn = insertions + deletions;

#ifdef DEBUG
	int time_diff = (prev_time - cur_time) / (60 * 60 * 24);
	char s[2] = "";
	if (time_diff != 1)
		s[0] = 's';
		
	int time_string_length = strlen("2014-10-23 19:13") + 1;
	char prev_time_string[time_string_length];
	char cur_time_string[time_string_length];
	tm = localtime(&prev_time);
	strftime(prev_time_string, time_string_length, "%F %H:%M", tm);
	tm = localtime(&cur_time);
	strftime(cur_time_string, time_string_length, "%F %H:%M", tm);
     	print_debug("%s %s - diff(%s, %s) = %d changed lines (Time range: %s - %s, %d day%s)\n", debug, id,
		    cur_buf, prev_buf, churn, cur_time_string, prev_time_string, time_diff, s);
#endif

	return churn;
}

unsigned long int calculate_code_churn(git_repository *repo, int max_commits, time_t min_time) {
	const char id[] = "calculate_code_churn";
#ifdef DEBUG
	print_debug("%s %s - %s\n", debug, id, git_repository_workdir(repo));
#endif

	// walk over revisions and sum up code churn
	git_oid prev_oid;
	git_oid cur_oid;
	git_oid head;
	git_revwalk *walk = NULL;
	git_commit *commit;
	time_t commit_time;
	int num_commits = 0;
	unsigned long int total_changed_lines = 0;

	git_reference_name_to_id(&head, repo, "HEAD");
	git_revwalk_new(&walk, repo);
	git_revwalk_sorting(walk, GIT_SORT_TIME);
	git_revwalk_push(walk, &head);

	while (!git_revwalk_next(&cur_oid, walk)) {
	  
		if (min_time != 0) {
			git_commit_lookup(&commit, repo, &cur_oid);
			commit_time = git_commit_time(commit);
			if (commit_time < min_time) {
#ifdef DEBUG
				struct tm *tm;
				int time_string_length = strlen("2014-10-23 19:13") + 1;
				char commit_time_string[time_string_length];
				tm = localtime(&commit_time);
				strftime(commit_time_string, time_string_length, "%F %H:%M", tm);
				print_debug("Commit is not in specified time window: %s\n", commit_time_string);
#endif
				break;
			}
		}

		if (max_commits != 0 && num_commits >= max_commits) {
#ifdef DEBUG
			print_debug("Maximum specified amount of commits found: %d\n", max_commits);
#endif
			break;
		}
	
		num_commits = num_commits + 1;

		if (num_commits > 2) {
			total_changed_lines = total_changed_lines
					+ calculate_diff(repo, &prev_oid, &cur_oid);
		}

		prev_oid = cur_oid;
	}

	git_commit_free(commit);

#ifdef DEBUG
	char s[2] = "";
	if (num_commits != 1)
		s[0] = 's';
	print_debug("%s %s - %d commits found\n", debug, id, num_commits);
#endif

	if (num_commits == 0) {
		return 0;
	}
	
	// count lines of code
	int first_loc = calculate_loc(repo, &prev_oid);
	int head_loc = calculate_loc(repo, &head);
	double ratio = (double) first_loc / (double) head_loc;

	// compute relative code churn
	double churn = head_loc;
	churn = (double) total_changed_lines / (double) head_loc;

	// print results
	print_csv_header();
	printf("%d;%d;%d;%.2f;%lu;%.2f;\n", num_commits, first_loc, head_loc,
	       ratio, total_changed_lines, churn);

	// cleanup
	git_revwalk_free(walk);

	return churn;
}

int main(int argc, char **argv) {
	const char id[] = "main";

#ifdef DEBUG
	// print program call
	print_debug("%s %s - program call ", debug, id);
	int i;
	for (i = 0; i < argc; i++) {
		print_debug("%s", argv[i]);
		if (i < argc - 1) {
			print_debug(" ");
		}
	}
	print_debug("\n");
#endif

	// parse arguments
	int c;
	time_t min_time = 0;
	int max_commits = 0;

	while ((c = getopt(argc, argv, "hn:t:")) != -1) {
		switch (c) {
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'n':
#ifdef DEBUG
			print_debug("Only the last %d commits are considered.\n",
				    atoi(optarg));
#endif
			// this is actually inaccurate
			max_commits = atoi(optarg);
			break;
		case 't':
#ifdef DEBUG
			print_debug("Only commits within the last %d years are considered.\n",
				    atoi(optarg));
#endif
			// this is actually inaccurate
			min_time = time(NULL) - atoi(optarg) * 365 * 24 * 60 * 60;
#ifdef DEBUG
			print_debug("Only commits after date %d are considered\n", min_time);
#endif
			break;
		default:
			printf ("?? getopt returned character code 0%o ??\n", c);
			//abort();
		}
	}

	char *path = NULL;
	git_repository *repo = NULL;

#ifdef DEBUG
	print_debug("argc = %d\noptind = %d\n", argc, optind);
#endif

	switch (argc - optind) {
	case 0:
		// at least one argument is required
		fprintf(stderr, "%s %s - At least one argument is required!\n", fatal,
				id);
		usage(argv[0]);
		return EXIT_FAILURE;
	case 1:
		// a git repository is expected
		path = argv[optind];

#ifdef DEBUG
		print_debug("%s %s - path = \"%s\"\n", debug, id, path);
#endif
		// make sure path exists and is a directory
		struct stat s;
		int err = stat(path, &s);
		if (err != -1 && S_ISDIR(s.st_mode)) {

#ifdef DEBUG
			print_debug("%s %s - Directory exists: %s\n", debug, id, path);
#endif

			// make sure that there is a .git directory in there
			char *gitmetadir;
			if ((gitmetadir = malloc(strlen(path) + 6)) != NULL) {
				strcat(gitmetadir, path);
				strcat(gitmetadir, "/.git");
				err = stat(gitmetadir, &s);
				if (err != -1 && S_ISDIR(s.st_mode)) {
					// seems to be a git repository
					free(gitmetadir);

#ifdef DEBUG
					print_debug("%s %s - Is a git repository: %s\n", debug, id,
							path);
#endif

					// initialize repo
					git_threads_init();
					if (git_repository_open(&repo, path) == 0) {

#ifdef DEBUG
						print_debug("%s %s - Initialized repository: %s\n",
								debug, id, path);
#endif

					} else {
						exit_error(EXIT_FAILURE,
								"%s %s - Could not open repository: %s\n",
								fatal, id, path);
					}
				} else {
					exit_error(EXIT_FAILURE,
							"%s %s - Is not a git repository: %s\n", fatal, id,
							path);
				}
			} else {
				exit_error(EXIT_FAILURE, "%s %s - malloc failed!\n", fatal, id);
			}
		} else if (err == -1) {
			perror("stat");
			exit(err);
		} else {
			exit_error(EXIT_FAILURE, "%s %s - Is not a directory: %s\n", fatal,
					id, path);
		}
		break;
	case 2:
		// either two tars or two directories are expected
#ifdef DEBUG
		print_debug("Expecting two tars or two directories\n");
#endif
		break;
	default:
		fprintf(stderr, "%s %s - Wrong amount of arguments: %d!\n", fatal,
			id, argc - optind);
		usage(argv[0]);
		return EXIT_FAILURE;		
	}

	if (repo != NULL) {
		// run the actual analysis
		calculate_code_churn(repo, max_commits, min_time);

		// cleanup
		git_repository_free(repo);
	}

	git_threads_shutdown();
	return EXIT_SUCCESS;
}
