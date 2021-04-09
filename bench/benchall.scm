(import 'srfi-6)
(import 'srfi-28)
(import 'values)
(import 'dynamic-wind)
(import 'io)
(import 'eval)

(define new-benchmarks
  '(conform))

(define all-benchmarks
   '(ack array1 boyer browse cat compiler cpstak ctak dderiv deriv 
     destruc diviter dynamic earley fft fib fibc fibfp formattest 
     fpsum gcbench graphs lattice maze mazefun mbrot nboyer 
     nqueens ntakl paraffins parsing perm9 peval pnpoly primes 
     puzzle quicksort ray sboyer scheme simplex slatex string 
     sum sum1 sumfp sumloop tail tak takl trav1 trav2 triangl)
)

(define failing
  '(matrix wc)
)

(define main '())

(define (run-a-single-bench filename)
  (load filename)
  (main)
)

(define (run-named-benchmark name)
  (display  name)
  (newline)
  (let ((filename (string-append (symbol->string name) ".scm")))
    (run-a-single-bench filename)
  )
)

(for-each run-named-benchmark new-benchmarks)

