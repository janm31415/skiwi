(import 'values)
(import 'dynamic-wind)

(define (call-with-output-file fname proc)
  (let ((in (open-output-file fname)))
    (call-with-values (lambda () (proc in))
      (lambda results
	(close-output-port in)
	(apply values results)))))

(define (call-with-input-file fname proc)
  (let ((in (open-input-file fname)))
    (call-with-values (lambda () (proc in))
      (lambda results
	(close-input-port in)
	(apply values results)))))

(define (with-input-from-file fname thunk)
  (call-with-input-file fname
    (lambda (in)
      (let ((old standard-input-port))
	(dynamic-wind
	    (lambda () (set! standard-input-port in))
	    thunk
	    (lambda () (set! standard-input-port old)))))))

(define (with-output-to-file fname thunk)
  (call-with-output-file fname
    (lambda (in)
      (let ((old standard-output-port))
	(dynamic-wind
	    (lambda () (set! standard-output-port in))
	    thunk
	    (lambda () (set! standard-output-port old)))))))