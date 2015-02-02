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
 * - on-demand checkout from a server
 * - take 2 directories as input
 * - take 2 tars as input
 *
 * TODO: Maintenance
 * - create unit tests
 * - check frees
 *
 */

#include "churny.h"

void static usage(const char *basename) {
	printf("Usage: %s [option]... [file]\n", basename);
	printf("  h\tPrints this message\n");
	printf("  c\tOnly count lines of code\n");
	printf("  m calculate churn separately for each month\n");
	printf("  y calculate churn separately for each year\n");
	printf("\n");
}

void static print_csv_header() {
	printf("%s\n", "Base Date;Last Date;Base Id; Last Id;Commits;"
			"Authors;Base LoC;Last LoC;Ratio;Added LoC;"
			"Removed LoC;Changed LoC;Relative Code Churn");
}

diffresult calculate_diff(git_repository *repo, const git_oid *prev,
		   const git_oid *cur, const char *extension) {
	const char id[] = "calculate_diff";

#if defined(DEBUG) || defined(TRACE)
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
	git_time_t prev_time;
	git_time_t cur_time;
	struct tm *tm;
	diffresult result;
	result.insertions = 0;
	result.deletions = 0;
	result.changes = 0;

	git_commit_lookup(&commit, repo, prev);
	prev_time = git_commit_time(commit);
	git_commit_tree(&prev_tree, commit);
	git_commit_lookup(&commit, repo, cur);
	cur_time = git_commit_time(commit);
	git_commit_tree(&cur_tree, commit);

	/* run diff */
	git_diff_tree_to_tree(&diff, repo, prev_tree, cur_tree, NULL);

	/* get stats */
	git_diff_stats *stats;
	git_buf b = GIT_BUF_INIT_CONST(NULL, 0);
	git_diff_stats_format_t format = 0;
	git_diff_get_stats(&stats, diff);
	format |= GIT_DIFF_STATS_NUMBER;
	git_diff_stats_to_buf(&b, stats, format, 80);

#ifdef TRACE
	if (b.ptr != NULL) {
		fputs(b.ptr, stdout);
	}
#endif

	if (strlen(extension) > 0) {
		/* look at each line and add changes
		 * if extension type matches */
		if (b.ptr == NULL) {
			return result;
		}

		char *line = strtok(strdup(b.ptr), "\n");
		unsigned long int cur_insertions = 0;
		unsigned long int cur_deletions = 0;
		int ret;
		char path[4096];

		while(line) {
			ret = sscanf(line, "%8lu%8lu%s",
				     &cur_insertions,
				     &cur_deletions, path);
			line  = strtok(NULL, "\n");
			if (ret == 3
			    && strlen(path) > strlen(extension)
			    && !strcmp(path + strlen(path)
				       - strlen(extension),
				       extension)) {
#ifdef TRACE
				printf("%lu insertions, "
				       "%lu deletions, "
				       "path: %s\n", cur_insertions,
				       cur_deletions, path);
#endif
				result.insertions = result.insertions
					+ cur_insertions;
				result.deletions = result.deletions
					+ cur_deletions;
			}
		}
	}  else {
		result.insertions = git_diff_stats_insertions(stats);
		result.deletions = git_diff_stats_deletions(stats);
	}

	/* cleanup */
	git_buf_free(&b);
	git_diff_stats_free(stats);
	git_diff_free(diff);
	git_tree_free(prev_tree);
	git_tree_free(cur_tree);
	git_commit_free(commit);

	result.changes = result.insertions + result.deletions;

#ifdef TRACE
	print_debug("%s %s - %d insertions + %d deletions "
		    "= %d changed lines\n", trace, id,
		    result.insertions, result.deletions,
		    result.changes);
#endif

#if defined(DEBUG) || defined(TRACE)
	int time_diff = (prev_time - cur_time) / (60 * 60 * 24);
	char s[2] = "";
	if (time_diff != 1)
		s[0] = 's';

	int time_string_length = strlen("2014-10-23 19:13") + 1;
	char prev_time_string[time_string_length];
	char cur_time_string[time_string_length];
	tm = localtime(&prev_time);
	strftime(prev_time_string, time_string_length, "%F %H:%M", tm);
	tm = gmtime(&cur_time);
	strftime(cur_time_string, time_string_length, "%F %H:%M", tm);
	print_debug("%s %s - diff(%s, %s) = %d changed lines "
		    "(Time range: %s - %s, %d day%s)\n", debug, id,
		    debug, id, cur_buf, prev_buf, result.changes,
		    cur_time_string, prev_time_string, time_diff, s);
#endif

	return result;
}

void print_results(git_repository *repo, const git_oid *first,
		   const git_oid *last, const int num_commits,
		   const diffresult diff, int number_authors,
		   const char *extension) {
	if (num_commits > 1) {
		git_commit *first_commit;
		git_commit *last_commit;
		git_time_t first_commit_time;
		git_time_t last_commit_time;
		int time_string_length = strlen("2014-10-23") + 1;
		char first_time_string[time_string_length];
		char last_time_string[time_string_length];
		char first_buf[GIT_OID_HEXSZ + 1];
		char last_buf[GIT_OID_HEXSZ + 1];
		git_oid_fmt(first_buf, first);
		first_buf[GIT_OID_HEXSZ] = '\0';
		git_oid_fmt(last_buf, last);
		last_buf[GIT_OID_HEXSZ] = '\0';
		struct tm *tm;

		git_commit_lookup(&first_commit, repo, first);
		git_commit_lookup(&last_commit, repo, last);
		first_commit_time = git_commit_time(first_commit);
		last_commit_time = git_commit_time(last_commit);

		tm = gmtime(&first_commit_time);
		strftime(first_time_string, time_string_length,
			 "%F %H:%M", tm);
		tm = gmtime(&last_commit_time);
		strftime(last_time_string, time_string_length,
			 "%F %H:%M", tm);

		/* count lines of code */
		int first_loc = calculate_loc(repo, first, extension);
		int last_loc = calculate_loc(repo, last, extension);
		double ratio = first_loc == 0 ?
			last_loc == 0 ?
			0.0 : 1.0 : (double) last_loc
			/ (double) first_loc;

		/* compute relative code churn */
		double churn = last_loc;
		churn = last_loc == 0 ?
			0 : (double) diff.changes / (double) last_loc;

		/* print results */
		char results[1024 + 1];
		sprintf(results, "%s;%s;%s;%s;%d;%d;%d;%d;%.2f;%lu;"
			"%lu;%lu;%.2f\n", first_time_string,
			last_time_string, first_buf, last_buf,
			num_commits, number_authors, first_loc,
			last_loc, ratio, diff.insertions,
			diff.deletions, diff.changes, churn);

		printf("%s", results);

		/* cleanup */
		git_commit_free(first_commit);
		git_commit_free(last_commit);
	}
}

diffresult calculate_interval_code_churn(git_repository *repo,
		const interval interval,
		const char *extension) {
	const char id[] = "calculate_interval_code_churn";
#if defined(DEBUG) || defined(TRACE)
	print_debug("%s %s - %s\n", debug, id,
		    git_repository_workdir(repo));
#endif

	/* walk over revisions and sum up code churn */
	git_oid prev_oid;
	git_oid cur_oid;
	git_oid head;
	git_revwalk *walk = NULL;
	git_commit *commit;
	setenv( "TC", "CEST", 1 );
	git_time_t commit_time;
	const git_signature *signature;
	git_oid last_commit;
	git_time_t last_commit_time = 0;
	int time_string_length = strlen("2014-10-23 00:00") + 1;
	int num_commits = 0;
	diffresult total_diff;
	diffresult diff;
	total_diff.insertions = 0;
	total_diff.deletions = 0;
	total_diff.changes = 0;
	diff.insertions = 0;
	diff.deletions = 0;
	diff.changes = 0;
	struct tm *tm;
	struct tm tm_min_time;
	char from_time_string[time_string_length];
	char to_time_string[time_string_length];
	git_time_t min_time = time(NULL);

	tm = gmtime(&min_time);
	tm_min_time = *tm;

	/* set min_time to beginning of month or year */
	switch (interval) {
	case YEAR:
		tm_min_time.tm_mon = 0;
		/* fallthrough to also set to beginning of month */
	case MONTH:
		tm_min_time.tm_mday = 1;
	}

	/* set to 0:00:00 */
	tm_min_time.tm_sec = 0;
	tm_min_time.tm_min = 0;
	tm_min_time.tm_hour = 0;

	/* we have no information on daylight saving time*/
	tm_min_time.tm_isdst = -1;

	min_time = tm_to_utc(&tm_min_time);

#if defined(DEBUG) || defined(TRACE)
	strftime(from_time_string, time_string_length, "%F %H:%M",
		 &tm_min_time);
	print_debug("%s %s - Analyzing until %s (%lu)\n", debug, id,
		    from_time_string, min_time);
#endif

	git_reference_name_to_id(&head, repo, "HEAD");
	git_revwalk_new(&walk, repo);
	git_revwalk_sorting(walk, GIT_SORT_TIME);
	git_revwalk_push(walk, &head);

	/* iterates over all commits starting with the latest one */
	while (!git_revwalk_next(&cur_oid, walk)) {

		git_commit_lookup(&commit, repo, &cur_oid);
		commit_time = git_commit_time(commit);

		if (last_commit_time == 0) {
			last_commit_time = commit_time;
			last_commit = cur_oid;
		}

#if defined(DEBUG) || defined(TRACE)
		char commit_time_string[time_string_length];
		tm = gmtime(&commit_time);
		strftime(commit_time_string, time_string_length,
			 "%F %H:%M", tm);
		print_debug("%s %s - Commit found: %s (%lu)\n", debug,
			    id, commit_time_string, commit_time);
#endif

		if (num_commits >= 2) {
			diffresult cur_diff
				= calculate_diff(repo, &cur_oid,
						 &prev_oid,
						 extension);
			diff.insertions = diff.insertions
				+ cur_diff.insertions;
			diff.deletions = diff.deletions
				+ cur_diff.deletions;
			diff.changes = diff.changes
				+ cur_diff.changes;
			total_diff.insertions = total_diff.insertions
				+ cur_diff.insertions;
			total_diff.deletions = total_diff.deletions
				+ cur_diff.deletions;
			total_diff.changes = total_diff.changes
				+ cur_diff.changes;
		}

		/* if the commit is not in the time interval,
		 * calculate churn and continue */
		if (commit_time < min_time) {
#if defined(DEBUG) || defined(TRACE)
			print_debug("%s %s - Commit is not in "
				    "specified time window: %s\n",
				    debug, id, commit_time_string);
#endif
			/* print results, reset counters
			 *  and continue */
			print_results(repo, &cur_oid, &last_commit,
				      num_commits, diff, list_size(),
				      extension);

			/* reset counters */
			if (num_commits > 1) {
				last_commit = cur_oid;
				last_commit_time = commit_time;
				diff.insertions = 0;
				diff.deletions = 0;
				diff.changes = 0;
				num_commits = 0;
				list_clear();
			}

			while (commit_time < min_time) {
				switch (interval) {
				case MONTH:
					tm_min_time.tm_mon
						= tm_min_time.tm_mon
						- 1;
					if (tm_min_time.tm_mon < 0) {
						tm_min_time.tm_mon
							= 11;
					} else {
						break;
					}
				case YEAR:
					tm_min_time.tm_year
						= tm_min_time.tm_year
						- 1;
				}

				tm_min_time.tm_hour = 0;

				/* we have no information on
				 * daylight saving time */
				tm_min_time.tm_isdst = -1;
				min_time = tm_to_utc(&tm_min_time);
			}
#if defined(DEBUG) || defined(TRACE)
			strftime(from_time_string,
				 time_string_length, "%F %H:%M",
				 &tm_min_time);
			print_debug("%s %s - Analyzing until %s\n",
				    debug, id, from_time_string);
#endif
		}

		signature = git_commit_author(commit);
		if (!list_contains(signature->name)) {
			list_add(signature->name);
		}

		num_commits = num_commits + 1;
		prev_oid = cur_oid;
	}

	print_results(repo, &cur_oid, &last_commit, num_commits,
		      diff, list_size(), extension);


#if defined(DEBUG) || defined(TRACE)
	char s[2] = "";
	if (num_commits != 1) {
		s[0] = 's';
	}
	print_debug("%s %s - %d commits found\n", debug, id,
		    num_commits);
	print_debug("%%s %s - lu total lines of changed code\n",
		    debug, id, total_diff.changes);
#endif

	/* cleanup */
	git_commit_free(commit);
	git_revwalk_free(walk);
	list_clear();

	return total_diff;
}

diffresult calculate_code_churn(git_repository *repo,
				const char *extension) {
	const char id[] = "calculate_code_churn";
#if defined(DEBUG) || defined(TRACE)
	print_debug("%s %s - %s\n", debug, id,
		    git_repository_workdir(repo));
#endif

	/* walk over revisions and sum up code churn */
	git_oid prev_oid;
	git_oid cur_oid;
	git_oid head;
	git_revwalk *walk = NULL;
	git_commit *commit;
	const git_signature *signature;
	git_time_t commit_time;
	git_time_t first_commit_time = 0;
	git_oid first_commit;
	git_oid last_commit;
	git_time_t last_commit_time = 0;
	int time_string_length = strlen("2014-10-23 00:00") + 1;
	int num_commits = 0;
	diffresult total_diff;
	total_diff.insertions = 0;
	total_diff.deletions = 0;
	total_diff.changes = 0;
	struct tm *tm;
	char from_time_string[time_string_length];
	char to_time_string[time_string_length];

	git_reference_name_to_id(&head, repo, "HEAD");
	git_revwalk_new(&walk, repo);
	git_revwalk_sorting(walk, GIT_SORT_TIME);
	git_revwalk_push(walk, &head);

	/* iterates over all commits starting with the latest one */
	while (!git_revwalk_next(&cur_oid, walk)) {

		git_commit_lookup(&commit, repo, &cur_oid);
		commit_time = git_commit_time(commit);

		if (last_commit_time == 0) {
			last_commit_time = commit_time;
			last_commit = cur_oid;
		}

#if defined(DEBUG) || defined(TRACE)
		char commit_time_string[time_string_length];
		tm = gmtime(&commit_time);
		strftime(commit_time_string, time_string_length,
			 "%F %H:%M", tm);
		print_debug("%s %s - Commit found: %s\n", debug, id,
			    commit_time_string);
#endif

		first_commit_time = commit_time;
		first_commit = cur_oid;

		signature = git_commit_author(commit);
		if (!list_contains(signature->name)) {
			list_add(signature->name);
		}

		num_commits = num_commits + 1;

		if (num_commits >= 2) {
			diffresult cur_diff
				= calculate_diff(repo, &prev_oid,
						 &cur_oid, extension);
			total_diff.insertions = total_diff.insertions
				+ cur_diff.insertions;
			total_diff.deletions = total_diff.deletions
				+ cur_diff.deletions;
			total_diff.changes = total_diff.changes
				+ cur_diff.changes;
		}

		prev_oid = cur_oid;
	}


#if defined(DEBUG) || defined(TRACE)
	char s[2] = "";
	if (num_commits != 1) {
		s[0] = 's';
	}
	print_debug("%s %s - %d commits found\n", debug, id,
		    num_commits);
#endif

	/* print results */
	print_results(repo, &first_commit, &last_commit, num_commits,
		      total_diff, list_size(), extension);

	/* cleanup */
	list_clear();
	git_commit_free(commit);
	git_revwalk_free(walk);

	return total_diff;
}

int main(int argc, char **argv) {
	const char id[] = "main";

#if defined(DEBUG) || defined(TRACE)
	/* print program call */
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

	/* parse arguments */
	int c;
	interval interval = 0;
	bool count_only = false;
	char extension[255] = "";

	while ((c = getopt(argc, argv, "chjl:my")) != -1) {
		switch (c) {
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'c':
			count_only = true;
			break;
		case 'l':
			strcpy(extension, optarg);
			break;
		case 'm':
			interval = MONTH;
			break;
		case 'y':
			interval = YEAR;
			break;
		default:
			printf("?? getopt returned character code "
			       "0%o ??\n", c);
		}
	}

	char *path = NULL;
	git_repository *repo = NULL;

#if defined(DEBUG) || defined(TRACE)
	print_debug("argc = %d\noptind = %d\n", argc, optind);
#endif

	switch (argc - optind) {
	case 0:
		/* at least one argument is required */
		fprintf(stderr, "%s %s - At least one argument is "
			"required!\n", fatal, id);
		usage(argv[0]);
		return EXIT_FAILURE;
	case 1:
		/* a git repository is expected */
		path = argv[optind];

#if defined(DEBUG) || defined(TRACE)
		print_debug("%s %s - path = \"%s\"\n", debug, id,
			    path);
#endif
		/* make sure path exists and is a directory */
		struct stat s;
		int err = stat(path, &s);
		if (err != -1 && S_ISDIR(s.st_mode)) {

#if defined(DEBUG) || defined(TRACE)
			print_debug("%s %s - Directory exists: %s\n",
				    debug, id, path);
#endif

			/* make sure that there is a .git directory
			 * in there */
			char gitmetadir[strlen(path) + 6];
			strcpy(gitmetadir, path);
			strcat(gitmetadir, "/.git");
			err = stat(gitmetadir, &s);
			if (err != -1 && S_ISDIR(s.st_mode)) {
				/* seems to be a git repository */
#if defined(DEBUG) || defined(TRACE)
				print_debug("%s %s - Is a git "
					    "repository: %s\n",
					    debug, id, path);
#endif

				/* initialize repo */
				git_libgit2_init();
				if (git_repository_open(&repo, path)
				    == 0) {

#if defined(DEBUG) || defined(TRACE)
					print_debug("%s %s - "
						    "Initialized "
						    "repository: "
						    "%s\n",
						    debug, id, path);
#endif

				} else {
					exit_error(EXIT_FAILURE,
						   "%s %s - Could "
						   "not open "
						   "repository: %s\n",
						   fatal, id, path);
				}
			} else {
				exit_error(EXIT_FAILURE,
					   "%s %s - Is not a git "
					   "repository: %s\n", fatal,
					   id, path);
			}
		} else if (err == -1) {
			perror("stat");
			exit(err);
		} else {
			exit_error(EXIT_FAILURE, "%s %s - Is not a "
				   "directory: %s\n", fatal, id,
				   path);
		}
		break;
	case 2:
		/* either two tars or two directories are expected */
#if defined(DEBUG) || defined(TRACE)
		print_debug("%s %s - Expecting two tars or two "
			    "directories\n", debug, id);
#endif
		break;
	default:
		fprintf(stderr, "%s %s - Wrong amount of arguments: "
			"%d!\n", fatal, id, argc - optind);
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	if (repo != NULL) {
		/* run the actual analysis */
		if (count_only) {
			/* only count LOC, print result and exit */
			printf("%d\n", calculate_loc_dir(git_repository_workdir(repo),
						extension));
		} else if (interval > 0) {
			print_csv_header();
			calculate_interval_code_churn(repo, interval,
					extension);
		} else {
			print_csv_header();
			calculate_code_churn(repo, extension);
		}

		/* cleanup */
		git_repository_free(repo);
	}

	git_libgit2_shutdown();
	return EXIT_SUCCESS;
}
