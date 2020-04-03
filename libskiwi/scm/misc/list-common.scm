(define (nthcdr n lst)
  (if (zero? n) lst (nthcdr (+ -1 n) (cdr lst))))

(define (butnthcdr k lst)
  (cond ((negative? k) lst) ;(slib:error "negative argument to butnthcdr" k)
					; SIMSYNCH FIFO8 uses negative k.
	((or (zero? k) (null? lst)) '())
	(else (let ((ans (list (car lst))))
		(do ((lst (cdr lst) (cdr lst))
		     (tail ans (cdr tail))
		     (k (+ -2 k) (+ -1 k)))
		    ((or (negative? k) (null? lst)) ans)
		  (set-cdr! tail (list (car lst))))))))
;@
(define (butlast lst n)
  (butnthcdr (- (length lst) n) lst))

;@
(define list*
  (letrec ((list*1 (lambda (obj)
		     (if (null? (cdr obj))
			 (car obj)
			 (cons (car obj) (list*1 (cdr obj)))))))
    (lambda (obj1 . obj2)
      (if (null? obj2)
	  obj1
	  (cons obj1 (list*1 obj2))))))

;@
(define (copy-list lst) (append lst '()))

;;;@ From: hugh@ear.mit.edu (Hugh Secker-Walker)
(define (nreverse rev-it)
;;; Reverse order of elements of LIST by mutating cdrs.
  (cond ((null? rev-it) rev-it)
	((not (list? rev-it))
	 (error 'nreverse "Not a list in arg1" rev-it))
	(else (do ((reved '() rev-it)
		   (rev-cdr (cdr rev-it) (cdr rev-cdr))
		   (rev-it rev-it rev-cdr))
		  ((begin (set-cdr! rev-it reved) (null? rev-cdr)) rev-it)))))

;@
(define (find-if pred? lst)
  (cond ((null? lst) #f)
	((pred? (car lst)) (car lst))
	(else (find-if pred? (cdr lst)))))
;@
(define (member-if pred? lst)
  (cond ((null? lst) #f)
	((pred? (car lst)) lst)
	(else (member-if pred? (cdr lst)))))

;@
(define (every pred lst . rest)
  (cond ((null? rest)
	 (let mapf ((lst lst))
	   (or (null? lst)
	       (and (pred (car lst)) (mapf (cdr lst))))))
	(else (let mapf ((lst lst) (rest rest))
		(or (null? lst)
		    (and (apply pred (car lst) (map car rest))
			 (mapf (cdr lst) (map cdr rest))))))))

;;;@ From: hugh@ear.mit.edu (Hugh Secker-Walker)
(define (make-list k . init)
  (set! init (if (pair? init) (car init)))
  (do ((k (+ -1 k) (+ -1 k))
       (result '() (cons init result)))
      ((negative? k) result)))