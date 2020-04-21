# skiwi
Scheme compiler in c++

Introduction
------------

Skiwi is a just-in-time Scheme compiler for the x86-64 instruction set. The compiler
converts Scheme source code to assembly, which is then translated to machine code and 
executed immediately.

I was inspired/have mainly used the following sources when working on this compiler:

[An incremental approach to compiler construction, Abdulaziz ghuloum.](http://scheme2006.cs.uchicago.edu/11-ghuloum.pdf)

[Bones scheme compiler](http://www.call-with-current-continuation.org/bones/)

[Guile](https://www.gnu.org/software/guile/)

[Ken Silverman's evaldraw compiler](http://advsys.net/ken/download.htm)

Building the compiler
---------------------

The compiler has been tested on Windows 10 using Visual Studio 2017 and on Ubuntu 18.04.4
with gcc 7.5.0.

First you'll need to make sure that Intel's TBB library installed. 
On Windows you can download TBB's binaries from its website, and install them, preferably, in 
folder C:\Program Files\TBB. Another folder is also possible, but then you'll need to
adapt the CMakeLists.txt file and make it point to the correct location.
On Ubuntu you can simply run 
  sudo apt install libtbb-dev 
to install TBB.

Next a solution file / makefile can be generated with CMake. Use Visual Studio or make to build the code.

When running the skiwi repl (e.g. via the "s.exe" or "./s" executable program that you've just built) the compiler will still need to build some basic functionality. This functionality is written in scheme, and the code resides in the scm subfolder of the libskiwi folder. The compiler expects that the environment variable SKIWI_MODULE_PATH exists and points to this folder. The scm folder can be placed anywhere on your harddrive as long as SKIWI_MODULE_PATH points to it.

As soon as SKIWI_MODULE_PATH is correctly initialised you are ready to go.

Using the compiler as a stand-alone repl
----------------------------------------

A repl can be started by running the s program. 










