;;; SUM1 -- One of the Kernighan and Van Wyk benchmarks.

(load "run-benchmark.scm")
  
  (define inport #f)
  
  (define (sumport port sum-so-far)
    (let ((x (read port)))
      (if (eof-object? x)
          sum-so-far
          (sumport port (+ x sum-so-far)))))
  
  (define (sum port)
    (sumport port 0.0))
  
  (define (go)
    (set! inport (open-input-file "rn100"))
    (let ((result (sum inport)))
      (close-input-port inport)
      result))
  
  (define (main . args)
    (run-benchmark
     "sum1"
     sum1-iters
     (lambda (result) 
       (display result)
       (newline)
       (and (>= result 15794.974999999)
            (<= result 15794.975000001)))
     (lambda () (lambda () (go)))))

