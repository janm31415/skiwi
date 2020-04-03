(define (last-pair l) (if (pair? (cdr l)) (last-pair (cdr l)) l))

;@
(define append!
  (lambda args
    (cond ((null? args) '())
	  ((null? (cdr args)) (car args))
	  ((null? (car args)) (apply append! (cdr args)))
	  (else
	   (set-cdr! (last-pair (car args))
		     (apply append! (cdr args)))
	   (car args)))))