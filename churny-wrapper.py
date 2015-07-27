#!/usr/bin/env python3
import os
import sys
import subprocess
import multiprocessing
from configparser import ConfigParser

from github import Github
from pygit2 import clone_repository, discover_repository, Repository
from joblib import Parallel, delayed


def warning(*objs):
    print("WARNING: ", *objs, file=sys.stderr)


def run(cmd, output_path):
    output = open(output_path, "w")
    p = subprocess.Popen(cmd, shell=True, universal_newlines=True, stdout=output)
    p.wait()
    output.flush()
    output.close()


def analyze(url, api_token):
    url = url.strip().replace("http://www.github.com/", "")
    url = url.strip().replace("https://www.github.com/", "")
    g = Github(api_token)
    repo = g.get_repo(url)

    print("Analyzing: %s" % repo.clone_url)

    stats = open(os.path.join("github", repo.owner.login + "-" + repo.name), "w")
    stats.write("forks;branches;watchers;stars\n")
    stats.write("%d;%d;%d;%d\n"
                % (repo.forks_count, len(list(repo.get_branches())),
                   repo.watchers_count, repo.stargazers_count))
    stats.flush()
    stats.close()

    repo_path = os.path.join(os.getcwd(), "git", repo.owner.login, repo.name)

    # initialize repo
    if os.path.exists(repo_path):
        print("%s already cloned" % url)
        cloned_repo = Repository(discover_repository(repo_path))
    else:
        print("Cloning %s into %s" % (url, repo_path))
        cloned_repo = clone_repository(repo.clone_url, repo_path)

    # run churny
    run("churny " + repo_path, os.path.join("overall", repo.owner.login + "-" + repo.name))
    run("churny -m " + repo_path, os.path.join("monthly", repo.owner.login + "-" + repo.name))


def usage(path):
    print("Usage: %s [FILE]..." % path)
    print("\tFILE contains URLs to GitHub repositories, separated by newline\n")


if __name__ == "__main__":
    num_cores = multiprocessing.cpu_count()

    if len(sys.argv) <= 1:
        usage(sys.argv[0])
        exit(-1)

    # initialize github api token
    config_file = os.path.join(os.path.dirname(sys.argv[0]), "github.properties")
    if not os.path.exists(config_file):
        warning("File not found: %s" % config_file)
        warning("This file should have the following content:\n")
        print("[Github]", file=sys.stderr)
        print("token = REPLACE_WITH_YOUR_API_TOKEN", file=sys.stderr)
        exit(-1)

    config = ConfigParser()
    config.read(config_file)
    api_token = config.get("Github", "token")

    # initialize output directories
    for directory in ("github", "overall", "monthly"):
        if not os.path.exists(directory):
            os.mkdir(directory)

    for arg in sys.argv[1:]:
        urls = open(arg, 'r')
        Parallel(n_jobs=num_cores)(delayed(analyze)(url, api_token) for url in urls)
