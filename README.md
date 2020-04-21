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
![](images/s_repl.png)
Simply type your scheme code here and get immediate feedback.
Any scheme code you type is compiled to machine code and run. All compiled expressions reside in virtual memory until you close the application. 

A very basic module system is implemented that allows you to import additional functionality. Essentially it is a stripped version of the module system of [Chibi scheme](https://github.com/ashinn/chibi-scheme). For the implementation, see modules.scm in subfolder libskiwi/scm/core.
The module system allows to import additional functionality, e.g.:

    skiwi> (import 'csv)       ;; functionality for reading and writing comma separated value files
  
    skiwi> (import 'srfi-1)    ;; load srfi 1 functionality.
  
    skiwi> (import 'test-r4rs) ;; run unit tests for r4rs functionality (written by [Aubrey Jaffer](http://people.csail.mit.edu/jaffer/))
  
    ...
  
See libskiwi/scm/packages.scm for the currently defined modules. You can always add your own modules here.

Integration with slib
---------------------
I've been working to integrate skiwi with [slib](http://people.csail.mit.edu/jaffer/SLIB). There are still issues probably but some slib functionality can be used. First you'll have to install slib. Unpack the slib distribution to your folder of liking and make an environment variable SCHEME_LIBRARY_PATH that points to this folder. Then, start skiwi and type (import 'slib). You should now be able to use the slib functionality.

Integrate skiwi as scripting language in your c/c++ program
-----------------------------------------------------------
In this section I'll explain how skiwi can be integrated in your c or c++ program by looking at the Game of Life example code that is in the repository.
Skiwi is designed as a library that you can include in your own project. You only have to include the libskiwi.h header file and you're good to go.
I assume you are familiar with Conway's Game of Life, if not, take a look at the [Wikipedia page](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life). Essentially it is a cellular automaton with the following two rules:
-  If a cell is on and has either two or three neighbors that are on in the current generation, it stays on; otherwise, the cell turns off.
 - If a cell is off and has exactly three “on” neighbors in the current generation, it turns on; otherwise, it stays off. 
 
When you run the "life" project from the repository, you'll see that the skiwi repl is available in the console window, and one additional window is opened, representing the current state of the Game of Life grid. 

![](images/life.png)

In the console window, type

    skiwi> (run)
to see the next generations. Similarly type

    skiwi> (stop)
to stop computing and visualizing new generations. For an overview of all the extra "Game of Life" related functionality, type

    skiwi> ,external






