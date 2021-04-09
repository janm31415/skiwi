;;; SUMFP -- Compute sum of integers from 0 to 10000 using floating point
(load "run-benchmark.scm")

  
  (define (run n)
    (let loop ((i n) (sum 0.))
      (if (< i 0.)
          sum
          (loop (- i 1.) (+ i sum)))))
   
  (define (main . args)
    (run-benchmark
      "sumfp"
      sumfp-iters
      (lambda (result) (equal? result 50005000.))
      (lambda (n) (lambda () (run n)))
      10000.))