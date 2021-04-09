
Below is the output of the benchmarks.
I ran these on an Intel Core i7 - 10850H CPU @ 2.70Ghz laptop.

These results can be replicated by running the command line call

 > s.exe benchall.scm
  
where 's.exe' is the skiwi compiler that is built via CMake as explained
on the main page.

    skiwi> compiling primitives library...
    skiwi> compiling string-to-symbol...
    skiwi> compiling apply...
    skiwi> compiling call/cc library...
    skiwi> compiling r5rs library...
    skiwi> compiling modules...
    skiwi> compiling packages.scm...
    skiwi> compiling benchall.scm...
    skiwi> compiling srfi6.scm...
    skiwi> compiling srfi28.scm...
    skiwi> compiling values.scm...
    skiwi> compiling dynamic-wind.scm...
    skiwi> compiling input-output.scm...
    skiwi> compiling eval.scm...
    ack
    skiwi> compiling ack.scm...
    skiwi> compiling run-benchmark.scm...
    running "ack" (20)
    Ran for 3.5240000000000000213 seconds
    Compiled and ran for 3.5440000000000000391 seconds
    array1
    skiwi> compiling array1.scm...
    skiwi> compiling run-benchmark.scm...
    running "array1" (2)
    Ran for 1.6779999999999999361 seconds
    Compiled and ran for 1.7030000000000000693 seconds
    boyer
    skiwi> compiling boyer.scm...
    skiwi> compiling run-benchmark.scm...
    running "boyer" (50)
    Ran for 5.3369999999999997442 seconds
    Compiled and ran for 8.1089999999999999858 seconds
    browse
    skiwi> compiling browse.scm...
    skiwi> compiling run-benchmark.scm...
    running "browse" (600)
    Ran for 3.9319999999999999396 seconds
    Compiled and ran for 4.0099999999999997868 seconds
    cat
    skiwi> compiling cat.scm...
    skiwi> compiling run-benchmark.scm...
    running "cat" (12)
    Ran for 3.9660000000000001918 seconds
    Compiled and ran for 3.9900000000000002132 seconds
    compiler
    skiwi> compiling compiler.scm...
    skiwi> compiling run-benchmark.scm...
    running "compiler" (500)
    Ran for 5.1020000000000003126 seconds
    Compiled and ran for 43.139000000000002899 seconds
    conform
    skiwi> compiling conform.scm...
    skiwi> compiling run-benchmark.scm...
    running "conform" (70)
    Ran for 4.4889999999999998792 seconds
    Compiled and ran for 4.6630000000000002558 seconds
    cpstak
    skiwi> compiling cpstak.scm...
    skiwi> compiling run-benchmark.scm...
    running "cpstak" (1700)
    Ran for 3.5729999999999999538 seconds
    Compiled and ran for 3.597999999999999865 seconds
    ctak
    skiwi> compiling ctak.scm...
    skiwi> compiling run-benchmark.scm...
    running "ctak" (160)
    Ran for 1.0929999999999999716 seconds
    Compiled and ran for 1.1209999999999999964 seconds
    dderiv
    skiwi> compiling dderiv.scm...
    skiwi> compiling run-benchmark.scm...
    running "dderiv" (3000000)
    Ran for 5.0170000000000003482 seconds
    Compiled and ran for 5.0750000000000001776 seconds
    deriv
    skiwi> compiling deriv.scm...
    skiwi> compiling run-benchmark.scm...
    running "deriv" (4000000)
    Ran for 4.3380000000000000782 seconds
    Compiled and ran for 4.3659999999999996589 seconds
    destruc
    skiwi> compiling destruc.scm...
    skiwi> compiling run-benchmark.scm...
    running "destruc" (800)
    Ran for 3.1070000000000002061 seconds
    Compiled and ran for 3.1450000000000000178 seconds
    diviter
    skiwi> compiling diviter.scm...
    skiwi> compiling run-benchmark.scm...
    running "diviter" (1200000)
    Ran for 4.4039999999999999147 seconds
    Compiled and ran for 4.4359999999999999432 seconds
    dynamic
    skiwi> compiling dynamic.scm...
    skiwi> compiling run-benchmark.scm...
    skiwi> compiling dynamic.src.scm...
    running "dynamic" (70)
    Ran for 2.4919999999999999929 seconds
    Compiled and ran for 2.9609999999999998543 seconds
    earley
    skiwi> compiling earley.scm...
    skiwi> compiling run-benchmark.scm...
    running "earley" (400)
    Ran for 5.5190000000000001279 seconds
    Compiled and ran for 5.6479999999999996874 seconds
    fft
    skiwi> compiling fft.scm...
    skiwi> compiling run-benchmark.scm...
    running "fft" (4000)
    Ran for 2.3969999999999997975 seconds
    Compiled and ran for 2.4329999999999998295 seconds
    fib
    skiwi> compiling fib.scm...
    skiwi> compiling run-benchmark.scm...
    running "fib" (6)
    Ran for 3.3780000000000001137 seconds
    Compiled and ran for 3.402000000000000135 seconds
    fibc
    skiwi> compiling fibc.scm...
    skiwi> compiling run-benchmark.scm...
    running "fibc" (900)
    Ran for 1.7070000000000000728 seconds
    Compiled and ran for 1.7359999999999999876 seconds
    fibfp
    skiwi> compiling fibfp.scm...
    skiwi> compiling run-benchmark.scm...
    running "fibfp" (2)
    Ran for 1.3990000000000000213 seconds
    Compiled and ran for 1.4250000000000000444 seconds
    formattest
    skiwi> compiling formattest.scm...
    running "fibfp" (2)
    Ran for 1.4550000000000000711 seconds
    Compiled and ran for 1.4679999999999999716 seconds
    fpsum
    skiwi> compiling fpsum.scm...
    skiwi> compiling run-benchmark.scm...
    running "fpsum" (60)
    Ran for 1.1999999999999999556 seconds
    Compiled and ran for 1.2290000000000000924 seconds
    gcbench
    skiwi> compiling gcbench.scm...
    skiwi> compiling run-benchmark.scm...
    The garbage collector should touch about 32 megabytes of heap storage.
    The use of more or less memory will skew the results.
    running "GCBench18" (2)
    Garbage Collector Test
     Stretching memory with a binary tree of depth 18
     Total memory available= ???????? bytes  Free memory= ???????? bytes
    GCBench: Main
     Creating a long-lived binary tree of depth 16
     Creating a long-lived array of 524284 inexact reals
     Total memory available= ???????? bytes  Free memory= ???????? bytes
    Creating 33824 trees of depth 4
    GCBench: Top down construction
    GCBench: Bottom up construction
    Creating 8256 trees of depth 6
    GCBench: Top down construction
    GCBench: Bottom up construction
    Creating 2052 trees of depth 8
    GCBench: Top down construction
    GCBench: Bottom up construction
    Creating 512 trees of depth 10
    GCBench: Top down construction
    GCBench: Bottom up construction
    Creating 128 trees of depth 12
    GCBench: Top down construction
    GCBench: Bottom up construction
    Creating 32 trees of depth 14
    GCBench: Top down construction
    GCBench: Bottom up construction
    Creating 8 trees of depth 16
    GCBench: Top down construction
    GCBench: Bottom up construction
     Total memory available= ???????? bytes  Free memory= ???????? bytes
    Garbage Collector Test
     Stretching memory with a binary tree of depth 18
     Total memory available= ???????? bytes  Free memory= ???????? bytes
    GCBench: Main
     Creating a long-lived binary tree of depth 16
     Creating a long-lived array of 524284 inexact reals
     Total memory available= ???????? bytes  Free memory= ???????? bytes
    Creating 33824 trees of depth 4
    GCBench: Top down construction
    GCBench: Bottom up construction
    Creating 8256 trees of depth 6
    GCBench: Top down construction
    GCBench: Bottom up construction
    Creating 2052 trees of depth 8
    GCBench: Top down construction
    GCBench: Bottom up construction
    Creating 512 trees of depth 10
    GCBench: Top down construction
    GCBench: Bottom up construction
    Creating 128 trees of depth 12
    GCBench: Top down construction
    GCBench: Bottom up construction
    Creating 32 trees of depth 14
    GCBench: Top down construction
    GCBench: Bottom up construction
    Creating 8 trees of depth 16
    GCBench: Top down construction
    GCBench: Bottom up construction
     Total memory available= ???????? bytes  Free memory= ???????? bytes
    Ran for 2.7789999999999999147 seconds
    Compiled and ran for 2.8349999999999999645 seconds
    graphs
    skiwi> compiling graphs.scm...
    skiwi> compiling run-benchmark.scm...
    running "graphs" (500)
    Ran for 3.8929999999999997939 seconds
    Compiled and ran for 3.9649999999999998579 seconds
    lattice
    skiwi> compiling lattice.scm...
    skiwi> compiling run-benchmark.scm...
    running "lattice" (2)
    Ran for 11.089000000000000412 seconds
    Compiled and ran for 11.141000000000000014 seconds
    maze
    skiwi> compiling maze.scm...
    skiwi> compiling run-benchmark.scm...
    running "maze" (4000)
    Ran for 3.3359999999999998543 seconds
    Compiled and ran for 4.4470000000000000639 seconds
    mazefun
    skiwi> compiling mazefun.scm...
    skiwi> compiling run-benchmark.scm...
    running "mazefun" (2500)
    Ran for 4.2800000000000002487 seconds
    Compiled and ran for 4.5140000000000002345 seconds
    mbrot
    skiwi> compiling mbrot.scm...
    skiwi> compiling run-benchmark.scm...
    running "mbrot" (120)
    Ran for 1.6279999999999998916 seconds
    Compiled and ran for 1.6599999999999999201 seconds
    nboyer
    skiwi> compiling nboyer.scm...
    skiwi> compiling run-benchmark.scm...
    running "nboyer0" (150)
    Ran for 5.625 seconds
    Compiled and ran for 8.4649999999999998579 seconds
    nqueens
    skiwi> compiling nqueens.scm...
    skiwi> compiling run-benchmark.scm...
    running "nqueens" (4000)
    Ran for 6.1470000000000002416 seconds
    Compiled and ran for 6.17900000000000027 seconds
    ntakl
    skiwi> compiling ntakl.scm...
    skiwi> compiling run-benchmark.scm...
    running "ntakl" (600)
    Ran for 5.2320000000000002061 seconds
    Compiled and ran for 5.2619999999999995666 seconds
    paraffins
    skiwi> compiling paraffins.scm...
    skiwi> compiling run-benchmark.scm...
    running "paraffins" (1800)
    Ran for 2.9190000000000000391 seconds
    Compiled and ran for 2.9670000000000000817 seconds
    parsing
    skiwi> compiling parsing.scm...
    skiwi> compiling run-benchmark.scm...
    running "parsing:parsing-data.scm:360" (360)
    Ran for 3.7879999999999998117 seconds
    Compiled and ran for 4.0039999999999995595 seconds
    perm9
    skiwi> compiling perm9.scm...
    skiwi> compiling run-benchmark.scm...
    running "perm9" (12)
    Ran for 1.5229999999999999094 seconds
    Compiled and ran for 1.6490000000000000213 seconds
    peval
    skiwi> compiling peval.scm...
    skiwi> compiling run-benchmark.scm...
    running "peval" (400)
    Ran for 5.3410000000000001918 seconds
    Compiled and ran for 5.6399999999999996803 seconds
    pnpoly
    skiwi> compiling pnpoly.scm...
    skiwi> compiling run-benchmark.scm...
    running "pnpoly" (140000)
    Ran for 2.5219999999999997975 seconds
    Compiled and ran for 5.0080000000000000071 seconds
    primes
    skiwi> compiling primes.scm...
    skiwi> compiling run-benchmark.scm...
    running "primes" (180000)
    Ran for 7.8099999999999996092 seconds
    Compiled and ran for 7.8399999999999998579 seconds
    puzzle
    skiwi> compiling puzzle.scm...
    skiwi> compiling run-benchmark.scm...
    running "puzzle" (180)
    Ran for 3.8919999999999999041 seconds
    Compiled and ran for 3.9470000000000000639 seconds
    quicksort
    skiwi> compiling quicksort.scm...
    skiwi> compiling run-benchmark.scm...
    running "quicksort30" (60)
    Ran for 2.8340000000000000746 seconds
    Compiled and ran for 2.8719999999999998863 seconds
    ray
    skiwi> compiling ray.scm...
    skiwi> compiling run-benchmark.scm...
    running "ray" (5)
    Ran for 2.6709999999999998188 seconds
    Compiled and ran for 2.7419999999999999929 seconds
    sboyer
    skiwi> compiling sboyer.scm...
    skiwi> compiling run-benchmark.scm...
    running "sboyer0" (200)
    Ran for 8.6690000000000004832 seconds
    Compiled and ran for 8.7750000000000003553 seconds
    scheme
    skiwi> compiling scheme.scm...
    skiwi> compiling run-benchmark.scm...
    running "scheme" (40000)
    Ran for 4.0359999999999995879 seconds
    Compiled and ran for 4.3289999999999997371 seconds
    simplex
    skiwi> compiling simplex.scm...
    skiwi> compiling run-benchmark.scm...
    running "simplex" (160000)
    Ran for 6.7889999999999997016 seconds
    Compiled and ran for 7.8540000000000000924 seconds
    slatex
    skiwi> compiling slatex.scm...
    skiwi> compiling run-benchmark.scm...
    running "slatex" (30)
    Ran for 2.8919999999999999041 seconds
    Compiled and ran for 3.4839999999999999858 seconds
    string
    skiwi> compiling string.scm...
    skiwi> compiling run-benchmark.scm...
    running "string" (4)
    Ran for 0.099000000000000004663 seconds
    Compiled and ran for 0.12600000000000000089 seconds
    sum
    skiwi> compiling sum.scm...
    skiwi> compiling run-benchmark.scm...
    running "sum" (30000)
    Ran for 4.5410000000000003695 seconds
    Compiled and ran for 4.5659999999999998366 seconds
    sum1
    skiwi> compiling sum1.scm...
    skiwi> compiling run-benchmark.scm...
    running "sum1" (5)
    Ran for 1.0800000000000000711 seconds
    15794.975000000120417
    Compiled and ran for 1.1109999999999999876 seconds
    sumfp
    skiwi> compiling sumfp.scm...
    skiwi> compiling run-benchmark.scm...
    running "sumfp" (8000)
    Ran for 1.6910000000000000586 seconds
    Compiled and ran for 1.7159999999999999698 seconds
    sumloop
    skiwi> compiling sumloop.scm...
    skiwi> compiling run-benchmark.scm...
    running "sumloop" (2)
    Ran for 3.6299999999999998934 seconds
    Compiled and ran for 3.6560000000000001386 seconds
    tail
    skiwi> compiling tail.scm...
    skiwi> compiling run-benchmark.scm...
    running "tail" (4)
    Ran for 1.3420000000000000817 seconds
    Compiled and ran for 1.3709999999999999964 seconds
    tak
    skiwi> compiling tak.scm...
    skiwi> compiling run-benchmark.scm...
    running "tak" (3000)
    Ran for 4.8589999999999999858 seconds
    Compiled and ran for 4.8869999999999995666 seconds
    takl
    skiwi> compiling takl.scm...
    skiwi> compiling run-benchmark.scm...
    running "takl" (500)
    Ran for 4.8860000000000001208 seconds
    Compiled and ran for 4.9150000000000000355 seconds
    trav1
    skiwi> compiling trav1.scm...
    skiwi> compiling run-benchmark.scm...
    running "trav1" (150)
    Ran for 3.7559999999999997833 seconds
    Compiled and ran for 3.8149999999999999467 seconds
    trav2
    skiwi> compiling trav2.scm...
    skiwi> compiling run-benchmark.scm...
    running "trav2" (40)
    Ran for 6.3419999999999996376 seconds
    Compiled and ran for 6.4279999999999999361 seconds
    triangl
    skiwi> compiling triangl.scm...
    skiwi> compiling run-benchmark.scm...
    running "triangl" (12)
    Ran for 3.3410000000000001918 seconds
    Compiled and ran for 3.3809999999999997833 seconds
    #undefined
    
    Welcome to Skiwi
    Type ,? for help.
    skiwi>