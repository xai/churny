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

#include "jdime.h"
#include "utils.h"

char* print_jdime_csv_header(void) {
    /* this hardcoded thing is ugly. */
    /* it needs to be replaced by a call to jdime */
    char* csv_header = (char*)malloc(1024 * sizeof(char));
    strcpy(csv_header, "classes;matched classes;changed classes;"
                       "added classes;removed classes;conflicting classes;"
                       "classlevelnodes;matched classlevelnodes;"
                       "changed classlevelnodes;added classlevelnodes;"
                       "removed classlevelnodes;conflicting classlevelnodes;"
                       "methods;matched methods;changed methods;"
                       "added methods;removed methods;conflicting methods;"
                       "methodlevelnodes;matched methodlevelnodes;"
                       "changed methodlevelnodes;added methodlevelnodes;"
                       "removed methodlevelnodes;conflicting "
                       "methodlevelnodes;nodes;matched nodes;changed nodes;"
                       "added nodes;removed nodes;conflicting nodes;"
                       "toplevelnodes;matched toplevelnodes;"
                       "changed toplevelnodes;added toplevelnodes;"
                       "removed toplevelnodes;conflicting toplevelnodes");
    return csv_header;
}

char* run_jdime_diff(
    git_repository* repo, const git_oid* prev, const git_oid* cur) {
    const char id[] = "run_jdime_diff";

    /* get commit hashes */
    char prev_buf[GIT_OID_HEXSZ + 1];
    char cur_buf[GIT_OID_HEXSZ + 1];
    git_oid_fmt(prev_buf, prev);
    prev_buf[GIT_OID_HEXSZ] = '\0';
    git_oid_fmt(cur_buf, cur);
    cur_buf[GIT_OID_HEXSZ] = '\0';
    git_commit* commit;
    git_object* gobject;
    git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
    opts.checkout_strategy = GIT_CHECKOUT_FORCE;
    char tempdir[] = "/tmp/churny.XXXXXX";
    const char* repodir = git_repository_workdir(repo);

    chdir(git_repository_workdir(repo));

    /* create a temporary directory */
    if (mkdtemp(tempdir) == NULL) {
        perror("tempdir: error: Could not create tmp "
               "directory");
        exit(EXIT_FAILURE);
    }

#ifdef TRACE
    printf("Temporary directory created: %s\n", tempdir);
#endif

    if (chdir(tempdir) == -1) {
        perror("tempdir: error: ");
        exit(EXIT_FAILURE);
    }

    char leftdir[strlen(tempdir) + 6];
    char rightdir[strlen(tempdir) + 6];
    mode_t mode = 0700;
    sprintf(leftdir, "%s/left", tempdir);
    sprintf(rightdir, "%s/right", tempdir);
    mkdir(leftdir, mode);
    mkdir(rightdir, mode);

    /* checkout left revision and sync to tempdir/left */
    git_commit_lookup(&commit, repo, prev);
    git_object_lookup(&gobject, repo, prev, GIT_OBJ_COMMIT);
    git_checkout_tree(repo, gobject, &opts);

    char rsync_cmd[18 + strlen(repodir) + strlen(tempdir) + 1];
    sprintf(rsync_cmd, "rsync -aH %s %s", repodir, leftdir);
#ifdef TRACE
    printf("Running external command: %s\n", rsync_cmd);
#endif
    if (system(rsync_cmd) != 0) {
        perror("Error while running rsync: ");
    }

    /* checkout right revision and sync to tempdir/right */
    git_commit_lookup(&commit, repo, cur);
    git_object_lookup(&gobject, repo, cur, GIT_OBJ_COMMIT);
    git_checkout_tree(repo, gobject, &opts);

    sprintf(rsync_cmd, "rsync -aH %s %s", repodir, rightdir);
#ifdef TRACE
    printf("Running external command: %s\n", rsync_cmd);
#endif
    if (system(rsync_cmd) != 0) {
        perror("Error while running rsync: ");
    }

    char jdime_cmd[70 + strlen(leftdir) + strlen(rightdir) + 1];
    sprintf(jdime_cmd, "jdime -debug warn -r -mode structured "
                       "-diffonly -consecutive -stats %s %s",
        leftdir, rightdir);
#ifdef TRACE
    printf("Running external command: %s\n", jdime_cmd);
#endif

    FILE* fp;
    fp = popen(jdime_cmd, "r");
    /* char prbuf[4096]; */
    size_t bufsize = 4096;
    char* prbuf = (char*)malloc(sizeof(char) * bufsize);
    memset(prbuf, 0, bufsize);

    if (fp == NULL) {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    while (fgets(prbuf, bufsize - 1, fp) != NULL) {
        /* ignore header */
        if (i > 0) {
#ifdef DEBUG
            printf("%s", prbuf);
#endif
        }
        i++;
    }

    pclose(fp);

    /* clean up: delete temporary directory */
    char rm_cmd[7 + strlen(tempdir) + 1];
    sprintf(rm_cmd, "rm -rf %s", tempdir);
#ifdef TRACE
    printf("%s\n", rm_cmd);
#endif
    system(rm_cmd);

    git_object_free(gobject);
    git_commit_free(commit);

    /* checkout HEAD again */
    git_checkout_head(repo, &opts);

    return prbuf;
}
