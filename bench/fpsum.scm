;;; FPSUM - Compute sum of integers from 0 to 1e6 using floating point

(load "run-benchmark.scm")

  (define (run)
    (let loop ((i 1e6) (n 0.))
      (if (< i 0.)
        n
        (loop (- i 1.) (+ i n)))))
   
  (define (main . args)
    (run-benchmark
      "fpsum"
      fpsum-iters
      (lambda (result) (equal? result 500000500000.)) 
      (lambda () run)))
