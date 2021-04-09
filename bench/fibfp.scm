;;; FIBFP -- Computes fib(35) using floating point

(load "run-benchmark.scm")
  
  (define (fibfp n)
    (if (< n 2.)
      n
      (+ (fibfp (- n 1.))
           (fibfp (- n 2.)))))
  
  (define (main . args)
    (run-benchmark
      "fibfp"
      fibfp-iters
      (lambda (result) (equal? result 9227465.))
      (lambda (n) (lambda () (fibfp n)))
      35.))

