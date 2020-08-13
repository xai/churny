# README #

### What is this repository for? ###

Just a small code churn analysis tool

### How do I get set up? ###

As a requirement, libgit2 has to be installed. There are packages for
many popular distributions, otherwise have a look at
http://libgit2.github.com.  
Also, you need cmake.

If you have cmake and libgit2 installed, compile the project as follows:
```
git clone https://github.com/xai/churny
mkdir build
cd build
cmake ..
make
```

Now you can run `./churny -h` to print information on its usage.

Note: there is also a bash script in this repository that also
calculates code churn, but is no longer updated.

### Who do I talk to? ###

Olaf Lessenich (xai@linux.com)
