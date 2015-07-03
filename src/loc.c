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

#include "loc.h"

int calculate_loc(
    git_repository* repo, const git_oid* oid, const char* extension) {
    const char id[] = "calculate_loc";

    int loc = 0;
    git_commit* commit;
    git_object* gobject;

    git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
    opts.checkout_strategy = GIT_CHECKOUT_FORCE;

    chdir(git_repository_workdir(repo));

#if defined(DEBUG) || defined(TRACE)
    char buf[GIT_OID_HEXSZ + 1];
    git_oid_fmt(buf, oid);
    buf[GIT_OID_HEXSZ] = '\0';
    print_debug("%s %s - counting lines of commit %s: ", debug, id, buf);
#endif

    git_commit_lookup(&commit, repo, oid);
    git_object_lookup(&gobject, repo, oid, GIT_OBJ_COMMIT);
    git_checkout_tree(repo, gobject, &opts);

    loc = calculate_loc_dir(NULL, extension);

    git_object_free(gobject);
    git_commit_free(commit);

    /* checkout HEAD again */
    git_checkout_head(repo, &opts);

#if defined(DEBUG) || defined(TRACE)
    print_debug("%d\n", loc);
#endif

    if (loc < 0) {
        exit_error(EXIT_FAILURE, "%s %s - Error while "
                                 "counting lines of code\n",
            fatal, id);
    }
    return loc;
}

int calculate_loc_dir(const char* path, const char* extension) {
    const char id[] = "calculate_loc_dir";

    int loc = -1;

    if (path != NULL) {
        chdir(path);
    }

    char find_name[] = "-name \\*";
    char find_options[strlen(extension) + strlen(find_name) + 1];
    if (strlen(extension) > 0) {
        strcpy(find_options, find_name);
        strcat(find_options, extension);
    } else {
        strcpy(find_options, "");
    }

    char cmd[200 + strlen(find_options)];
    sprintf(cmd, "find . -type f %s -not -path './.git/*' -print "
                 "| xargs file --mime-encoding 2>/dev/null "
                 "| egrep -i ': .*ascii' "
                 "| cut -d: -f1 "
                 "| xargs cat 2>/dev/null "
                 "| egrep -v '^[[:space:]]*$' "
                 "| wc -l",
        find_options);

#ifdef DEBUG
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    print_debug("Current directory: %s\n", cwd);
    print_debug("Running external command: %s\n", cmd);
#endif

    FILE* fp;
    fp = popen(cmd, "r");
    char prbuf[1024];
    memset(prbuf, 0, sizeof(prbuf));

    if (fp == NULL) {
        perror("popen");
        return loc;
    }

    if (fgets(prbuf, sizeof(prbuf) - 1, fp) != NULL) {
        loc = atoi(prbuf);
    } else {
        exit_error(EXIT_FAILURE, "%s %s - Error while "
                                 "counting lines of code\n",
            fatal, id);
    }

    pclose(fp);

#ifdef DEBUG
    if (loc == 1) {
        sprintf(cmd, "find . -type f %s -not -path './.git/*'"
                     " -print | xargs file --mime-encoding "
                     "| egrep -i ': .*ascii' | cut -d: -f1",
            find_options);
        fp = popen(cmd, "r");
        memset(prbuf, 0, sizeof(prbuf));

        if (fp == NULL) {
            perror("popen");
            return -1;
        }

        while (fgets(prbuf, sizeof(prbuf) - 1, fp) != NULL) {
            printf("%s", prbuf);
        }

        pclose(fp);
    }
#endif
    return loc;
}
