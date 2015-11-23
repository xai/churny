# README #

### What is this repository for? ###

Just a small code churn analysis tool

### How do I get set up? ###

As a requirement, libgit2 has to be installed. There are packages for
many popular distributions, otherwise have a look at
http://libgit2.github.com.

Currently supported are versions 0.21 and 0.22.

This particular branch has an external dependency to jdime, a
structured diffing and merging tool that is not yet publicly available
in its current version. I am currently working really hard on
releasing jdime as an open source project. Such a release can be
expected about end of december.

If you have libgit2 and jdime installed, just run `make` to compile a
binary. Then run './churny -h' to print information on its usage.

Note: there is also a bash script in this repository that also
calculates code churn, but is no longer updated.

### Who do I talk to? ###

Olaf Lessenich (lessenic@fim.uni-passau.de)
