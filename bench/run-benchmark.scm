(import 'srfi-6)
(import 'srfi-28)
(import 'values)
(import 'dynamic-wind)
(import 'io)
(import 'eval)

; Gabriel benchmarks
(define boyer-iters        50)
(define browse-iters      600)
(define cpstak-iters     1700)
(define ctak-iters        160)
(define dderiv-iters  3000000)
(define deriv-iters   4000000)
(define destruc-iters     800)
(define diviter-iters 1200000)
(define divrec-iters  1200000)
(define puzzle-iters      180)
(define tak-iters        3000)
(define takl-iters        500)
(define trav1-iters       150)
(define trav2-iters        40)
(define triangl-iters      12)

; Kernighan and Van Wyk benchmarks
(define ack-iters           20)
(define array1-iters        2)
(define cat-iters           12)
(define string-iters        4)
(define sum1-iters          5)
(define sumloop-iters       2)
(define tail-iters          4)
(define wc-iters           15)

; C benchmarks
(define fft-iters        4000)
(define fib-iters           6)
(define fibfp-iters         2)
(define mbrot-iters       120)
(define nucleic-iters      12)
(define pnpoly-iters   140000)
(define sum-iters       30000)
(define sumfp-iters      8000)
(define tfib-iters         20)

; Other benchmarks
(define conform-iters      70)
(define dynamic-iters      70)
(define earley-iters      400)
(define fibc-iters        900)
(define graphs-iters      500)
(define lattice-iters       2)
(define matrix-iters      600)
(define maze-iters       4000)
(define mazefun-iters    2500)
(define nqueens-iters    4000)
(define ntakl-iters       600)
(define paraffins-iters  1800)
(define peval-iters       400)
(define pi-iters            3)
(define primes-iters   180000)
(define ray-iters           5)
(define scheme-iters    40000)
(define simplex-iters  160000)
(define slatex-iters       30)
(define perm9-iters        12)
(define nboyer-iters      150)
(define sboyer-iters      200)
(define gcbench-iters       2)
(define compiler-iters    500)

; New benchmarks
(define parsing-iters    360)
(define gcold-iters      600)

(define quicksort-iters   60)
(define fpsum-iters       60)
(define nbody-iters        1)
(define bibfreq-iters      2)
		
(define fast-run #f)

;(define fatal-error (lambda args
;	(error 'fatal-error (apply (lambda (x) (format "~s" x)) args))))
 
(define fatal-error (lambda args
	(error 'fatal-error args)))

(define run-bench (lambda (count run)
   (let loop ((count (- count 1)) (run run))
     ;(display (format "loop ~s~%" count))
     (cond
	   ((eq? count 0) (run))
	   (else (run) (loop (- count 1) run))
	 )
   )
  ))

(define run-benchmark (lambda (name count ok? run-maker . args)
  (display (format "running ~s (~s)~%" name count))
  (let ((run (apply run-maker args)))
       (let ((result (if fast-run (run) (run-bench count run))))
	        (if (ok? result)
	        (begin
	          (display result)
            (newline)
			      result
			    )
			    (begin (display "I got ")(display result)(display "\n")(error "wrong result")))
	   ))))
		