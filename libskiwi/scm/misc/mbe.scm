;;;; "mbe.scm" "Macro by Example" (Eugene Kohlbecker, R4RS)
;;; From: Dorai Sitaram, dorai@cs.rice.edu, 1991, 1999
;
;Permission to copy this software, to modify it, to redistribute it,
;to distribute modified versions, and to use it for any purpose is
;granted, subject to the following restrictions and understandings.
;
;1.  Any copy made of this software must include this copyright notice
;in full.
;
;2.  I have made no warranty or representation that the operation of
;this software will be error-free, and I am under no obligation to
;provide any services, by way of maintenance, update, or otherwise.
;
;3.  In conjunction with products arising from the use of this
;material, there shall be no use of my name in any advertising,
;promotional, or sales literature without prior written consent in
;each case.

;;; revised Dec. 6, 1993 to R4RS syntax (if not semantics).
;;; revised Mar. 2 1994 for SLIB (agj @ alum.mit.edu).
;;; corrections, Apr. 24, 1997.
;;; corr., Jan. 30, 1999. (mflatt@cs.rice.edu, dorai@cs.rice.edu)

;;; A vanilla implementation of hygienic macro-by-example as described
;;; by Eugene Kohlbecker and in R4RS Appendix.  This file requires
;;; defmacro.

(define (mbe:butnthcdr k lst)
  (cond ((negative? k) lst) ;(slib:error "negative argument to mbe:butnthcdr" k)
					; SIMSYNCH FIFO8 uses negative k.
	((or (zero? k) (null? lst)) '())
	(else (let ((ans (list (car lst))))
		(do ((lst (cdr lst) (cdr lst))
		     (tail ans (cdr tail))
		     (k (+ -2 k) (+ -1 k)))
		    ((or (negative? k) (null? lst)) ans)
		  (set-cdr! tail (list (car lst))))))))		 
		  
(define (mbe:butlast lst n)
  (mbe:butnthcdr (- (length lst) n) lst))
  
(define (mbe:some pred lst . rest)
  (cond ((null? rest)
	 (let mapf ((lst lst))
	   (and (not (null? lst))
		(or (pred (car lst)) (mapf (cdr lst))))))
	(else (let mapf ((lst lst) (rest rest))
		(and (not (null? lst))
		     (or (apply pred (car lst) (map car rest))
			 (mapf (cdr lst) (map cdr rest))))))))
			 
(define (mbe:every pred lst . rest)
  (cond ((null? rest)
	 (let mapf ((lst lst))
	   (or (null? lst)
	       (and (pred (car lst)) (mapf (cdr lst))))))
	(else (let mapf ((lst lst) (rest rest))
		(or (null? lst)
		    (and (apply pred (car lst) (map car rest))
			 (mapf (cdr lst) (map cdr rest))))))))
			 
(define (mbe:nreverse rev-it)
;;; Reverse order of elements of LIST by mutating cdrs.
  (cond ((null? rev-it) rev-it)
	((not (list? rev-it))
	 (error "mbe:nreverse: Not a list in arg1" rev-it))
	(else (do ((reved '() rev-it)
		   (rev-cdr (cdr rev-it) (cdr rev-cdr))
		   (rev-it rev-it rev-cdr))
		  ((begin (set-cdr! rev-it reved) (null? rev-cdr)) rev-it)))))
		  
(define (mbe:last-pair l) (if (pair? (cdr l)) (mbe:last-pair (cdr l)) l))	 

(define mbe:nconc  
      (lambda args
	(cond ((null? args) '())
	      ((null? (cdr args)) (car args))
	      ((null? (car args)) (apply mbe:nconc (cdr args)))
	      (else
	       (set-cdr! (mbe:last-pair (car args))
			 (apply mbe:nconc (cdr args)))
	       (car args)))))

(define *mbe:gentemp-counter* -1)

(define mbe:gentemp
  (lambda ()
    (set! *mbe:gentemp-counter* (+ *mbe:gentemp-counter* 1))
    (string->symbol (string-append "mbe:GenTemp%" 
				   (number->string *mbe:gentemp-counter*)))))

(define mbe:rassq
  (lambda (k al)
    (let loop ((al al))
      (if (null? al) #f
	(let ((c (car al)))
	  (if (eq? (cdr c) k) c
	    (loop (cdr al))))))))

(define mbe:tag
  (lambda (e kk al)
    (cond ((pair? e)
	    (let* ((a-te-al (mbe:tag (car e) kk al))
		    (d-te-al (mbe:tag (cdr e) kk (cdr a-te-al))))
	      (cons (cons (car a-te-al) (car d-te-al))
		(cdr d-te-al))))
      ((vector? e)
	(list->vector
	  (mbe:tag (vector->list e) kk al)))
      ((symbol? e)
	(cond ((eq? e '...) (cons '... al))
	  ((memq e kk) (cons e al))
	  ((mbe:rassq e al) =>
	    (lambda (c)
	      (cons (car c) al)))
	  (else
	    (let ((te (mbe:gentemp)))
	      (cons te (cons (cons te e) al))))))
      (else (cons e al)))))

;;untagging

(define mbe:untag
  (lambda (e al tmps)
    (if (pair? e)
      (let ((a (mbe:untag (car e) al tmps)))
	(if (list? e)
	  (case a
	    ((quote) (mbe:untag-no-tags e al))
	    ((quasiquote) (list a (mbe:untag-quasiquote (cadr e) al tmps)))
	    ((if begin)
	      `(,a ,@(map (lambda (e1)
			    (mbe:untag e1 al tmps)) (cdr e))))
	    ((set! define)
	      `(,a ,(mbe:untag-vanilla (cadr e) al tmps)
		 ,@(map (lambda (e1)
			  (mbe:untag e1 al tmps)) (cddr e))))
	    ((lambda) (mbe:untag-lambda (cadr e) (cddr e) al tmps))
	    ((letrec) (mbe:untag-letrec (cadr e) (cddr e) al tmps))
	    ((let)
	      (let ((e2 (cadr e)))
		(if (symbol? e2)
		  (mbe:untag-named-let e2 (caddr e) (cdddr e) al tmps)
		  (mbe:untag-let e2 (cddr e) al tmps))))
	    ((let*) (mbe:untag-let* (cadr e) (cddr e) al tmps))
	    ((do) (mbe:untag-do (cadr e) (caddr e) (cdddr e) al tmps))
	    ((case)
	      `(case ,(mbe:untag-vanilla (cadr e) al tmps)
		 ,@(map
		     (lambda (c)
		       `(,(mbe:untag-vanilla (car c) al tmps)
			  ,@(mbe:untag-list (cdr c) al tmps)))
		     (cddr e))))
	    ((cond)
	      `(cond ,@(map
			 (lambda (c)
			   (mbe:untag-list c al tmps))
			 (cdr e))))
	    (else (cons a (mbe:untag-list (cdr e) al tmps))))
	  (cons a (mbe:untag-list* (cdr e) al tmps))))
      (mbe:untag-vanilla e al tmps))))

(define mbe:untag-list
  (lambda (ee al tmps)
    (map (lambda (e)
	   (mbe:untag e al tmps)) ee)))

(define mbe:untag-list*
  (lambda (ee al tmps)
    (let loop ((ee ee))
      (if (pair? ee)
	(cons (mbe:untag (car ee) al tmps)
	  (loop (cdr ee)))
	(mbe:untag ee al tmps)))))

(define mbe:untag-no-tags
  (lambda (e al)
    (cond ((pair? e)
	    (cons (mbe:untag-no-tags (car e) al)
	      (mbe:untag-no-tags (cdr e) al)))
      ((vector? e)
	(list->vector
	  (mbe:untag-no-tags (vector->list e) al)))
      ((not (symbol? e)) e)
      ((assq e al) => cdr)
      (else e))))

(define mbe:untag-quasiquote
  (lambda (form al tmps)
    (let qq ((x form) (level 0))
      (cond
       ((pair? x)
	(let ((first (qq (car x) level)))
	  (cond
	   ((and (eq? first 'unquote) (list? x))
	    (let ((rest (cdr x)))
	      (if (or (not (pair? rest))
		      (not (null? (cdr rest))))
		  (error 'unquote 'takes-exactly-one-expression)
		  (if (zero? level)
		      (list 'unquote (mbe:untag (car rest) al tmps))
		      (cons first (qq rest (sub1 level)))))))
	   ((and (eq? first 'quasiquote) (list? x))
	    (cons 'quasiquote (qq (cdr x) (add1 level))))
	   ((and (eq? first 'unquote-splicing) (list? x))
	    (error 'unquote-splicing 'invalid-context-within-quasiquote))
	   ((pair? first)
	    (let ((car-first (qq (car first) level)))
	      (if (and (eq? car-first 'unquote-splicing)
		       (list? first))
		  (let ((rest (cdr first)))
		    (if (or (not (pair? rest))
			    (not (null? (cdr rest))))
			(error 'unquote-splicing
				    'takes-exactly-one-expression)
			(list (list 'unquote-splicing
				    (if (zero? level)
					(mbe:untag (cadr rest) al tmps)
					(qq (cadr rest) (sub1 level)))
				    (qq (cdr x) level)))))
		  (cons (cons car-first
			      (qq (cdr first) level))
			(qq (cdr x) level)))))
	   (else
	    (cons first (qq (cdr x) level))))))
       ((vector? x)
	(list->vector
	 (qq (vector->list x) level)))
       (else (mbe:untag-no-tags x al))))))

(define mbe:untag-lambda
  (lambda (bvv body al tmps)
    (let ((tmps2 (mbe:nconc (mbe:flatten bvv) tmps)))
      `(lambda ,bvv
	 ,@(mbe:untag-list body al tmps2)))))

(define mbe:untag-letrec
  (lambda (varvals body al tmps)
    (let ((tmps (mbe:nconc (map car varvals) tmps)))
      `(letrec
	 ,(map
	    (lambda (varval)
	      `(,(car varval)
		 ,(mbe:untag (cadr varval) al tmps)))
	    varvals)
	 ,@(mbe:untag-list body al tmps)))))

(define mbe:untag-let
  (lambda (varvals body al tmps)
    (let ((tmps2 (mbe:nconc (map car varvals) tmps)))
      `(let
	 ,(map
	     (lambda (varval)
	       `(,(car varval)
		  ,(mbe:untag (cadr varval) al tmps)))
	     varvals)
	 ,@(mbe:untag-list body al tmps2)))))

(define mbe:untag-named-let
  (lambda (lname varvals body al tmps)
    (let ((tmps2 (cons lname (mbe:nconc (map car varvals) tmps))))
      `(let ,lname
	 ,(map
	     (lambda (varval)
	       `(,(car varval)
		  ,(mbe:untag (cadr varval) al tmps)))
	     varvals)
	 ,@(mbe:untag-list body al tmps2)))))

(define mbe:untag-let*
  (lambda (varvals body al tmps)
    (let ((tmps2 (mbe:nconc (mbe:nreverse (map car varvals)) tmps)))
      `(let*
	 ,(let loop ((varvals varvals)
		      (i (length varvals)))
	    (if (null? varvals) '()
	      (let ((varval (car varvals)))
		(cons `(,(car varval)
			 ,(mbe:untag (cadr varval)
			    al (list-tail tmps2 i)))
		  (loop (cdr varvals) (- i 1))))))
	 ,@(mbe:untag-list body al tmps2)))))

(define mbe:untag-do
  (lambda (varinistps exit-test body al tmps)
    (let ((tmps2 (mbe:nconc (map car varinistps) tmps)))
      `(do
	 ,(map
	    (lambda (varinistp)
	      (let ((var (car varinistp)))
		`(,var ,@(mbe:untag-list (cdr varinistp) al
			   (cons var tmps)))))
	    varinistps)
	 ,(mbe:untag-list exit-test al tmps2)
	 ,@(mbe:untag-list body al tmps2)))))

(define mbe:untag-vanilla
  (lambda (e al tmps)
    (cond ((pair? e)
	    (cons (mbe:untag-vanilla (car e) al tmps)
	      (mbe:untag-vanilla (cdr e) al tmps)))
      ((vector? e)
	(list->vector
	  (mbe:untag-vanilla (vector->list e) al tmps)))
      ((not (symbol? e)) e)
      ((memq e tmps) e)
      ((assq e al) => cdr)
      (else e))))

(define mbe:flatten
  (lambda (e)
    (let loop ((e e) (r '()))
      (cond ((pair? e) (loop (car e)
			     (loop (cdr e) r)))
	    ((null? e) r)
	    (else (cons e r))))))

;;;; End of hygiene filter.


;;; finds the leftmost index of list l where mbe:something equal to x
;;; occurs
(define mbe:position
  (lambda (x l)
    (let loop ((l l) (i 0))
      (cond ((not (pair? l)) #f)
	    ((equal? (car l) x) i)
	    (else (loop (cdr l) (+ i 1)))))))

;;; (mbe:append-map f l) == (apply append (map f l))

(define mbe:append-map
  (lambda (f l)
    (let loop ((l l))
      (if (null? l) '()
	  (append (f (car l)) (loop (cdr l)))))))

;;; tests if expression e matches pattern p where k is the list of
;;; keywords
(define mbe:matches-pattern?
  (lambda (p e k)
    (cond ((mbe:ellipsis? p)
	   (and (or (null? e) (pair? e))
		(let* ((p-head (car p))
		       (p-tail (cddr p))
		       (e-head=e-tail (mbe:split-at-ellipsis e p-tail)))
		  (and e-head=e-tail
		       (let ((e-head (car e-head=e-tail))
			     (e-tail (cdr e-head=e-tail)))
			 (and (mbe:every
			       (lambda (x) (mbe:matches-pattern? p-head x k))
			       e-head)
			      (mbe:matches-pattern? p-tail e-tail k)))))))
	  ((pair? p)
	   (and (pair? e)
		(mbe:matches-pattern? (car p) (car e) k)
		(mbe:matches-pattern? (cdr p) (cdr e) k)))
	  ((symbol? p) (if (memq p k) (eq? p e) #t))
	  (else (equal? p e)))))

;;; gets the bindings of pattern variables of pattern p for
;;; expression e;
;;; k is the list of keywords
(define mbe:get-bindings
  (lambda (p e k)
    (cond ((mbe:ellipsis? p)
	   (let* ((p-head (car p))
		  (p-tail (cddr p))
		  (e-head=e-tail (mbe:split-at-ellipsis e p-tail))
		  (e-head (car e-head=e-tail))
		  (e-tail (cdr e-head=e-tail)))
	     (cons (cons (mbe:get-ellipsis-nestings p-head k)
		     (map (lambda (x) (mbe:get-bindings p-head x k))
			  e-head))
	       (mbe:get-bindings p-tail e-tail k))))
	  ((pair? p)
	   (append (mbe:get-bindings (car p) (car e) k)
	     (mbe:get-bindings (cdr p) (cdr e) k)))
	  ((symbol? p)
	   (if (memq p k) '() (list (cons p e))))
	  (else '()))))

;;; expands pattern p using environment r;
;;; k is the list of keywords
(define mbe:expand-pattern
  (lambda (p r k)
    (cond ((mbe:ellipsis? p)
	   (append (let* ((p-head (car p))
			  (nestings (mbe:get-ellipsis-nestings p-head k))
			  (rr (mbe:ellipsis-sub-envs nestings r)))
		     (map (lambda (r1)
			    (mbe:expand-pattern p-head (append r1 r) k))
			  rr))
	     (mbe:expand-pattern (cddr p) r k)))
	  ((pair? p)
	   (cons (mbe:expand-pattern (car p) r k)
	     (mbe:expand-pattern (cdr p) r k)))
	  ((symbol? p)
	   (if (memq p k) p
	     (let ((x (assq p r)))
	       (if x (cdr x) p))))
	  (else p))))

;;; returns a list that nests a pattern variable as deeply as it
;;; is ellipsed
(define mbe:get-ellipsis-nestings
  (lambda (p k)
    (let sub ((p p))
      (cond ((mbe:ellipsis? p) (cons (sub (car p)) (sub (cddr p))))
	    ((pair? p) (append (sub (car p)) (sub (cdr p))))
	    ((symbol? p) (if (memq p k) '() (list p)))
	    (else '())))))

;;; finds the subenvironments in r corresponding to the ellipsed
;;; variables in nestings

(define mbe:ellipsis-sub-envs
  (lambda (nestings r)
    (let ((sub-envs-list
	   (let loop ((r r) (sub-envs-list '()))
	     (if (null? r) (mbe:nreverse sub-envs-list)
		 (let ((c (car r)))
		   (loop (cdr r)
			 (if (mbe:contained-in? nestings (car c))
			     (cons (cdr c) sub-envs-list)
			     sub-envs-list)))))))
      (case (length sub-envs-list)
	((0) #f)
	((1) (car sub-envs-list))
	(else
	 (let loop ((sub-envs-list sub-envs-list) (final-sub-envs '()))
	   (if (mbe:some null? sub-envs-list) (mbe:nreverse final-sub-envs)
	       (loop (map cdr sub-envs-list)
		     (cons (mbe:append-map car sub-envs-list)
			   final-sub-envs)))))))))

;;; checks if nestings v and y have an intersection
(define mbe:contained-in?
  (lambda (v y)
    (if (or (symbol? v) (symbol? y)) (eq? v y)
	(mbe:some (lambda (v_i)
			(mbe:some (lambda (y_j)
					(mbe:contained-in? v_i y_j))
				      y))
		      v))))

;;; split expression e so that its second half matches with
;;; pattern p-tail
(define mbe:split-at-ellipsis
  (lambda (e p-tail)
    (if (null? p-tail) (cons e '())
      (let ((i (mbe:position (car p-tail) e)))
	(if i (cons (mbe:butlast e (- (length e) i))
		    (list-tail e i))
	    (error 'mbe:split-at-ellipsis 'bad-arg))))))

;;; tests if x is an ellipsing pattern, i.e., of the form
;;; (blah ... . blah2)
(define mbe:ellipsis?
  (lambda (x)
    (and (pair? x) (pair? (cdr x)) (eq? (cadr x) '...))))
;@

;@ define-syntax
(define-macro (define-syntax macro-name syn-rules)
  (if (or (not (pair? syn-rules))
	  (not (eq? (car syn-rules) 'syntax-rules)))
      (error 'define-syntax 'not-an-r4rs-high-level-macro
		  macro-name syn-rules)
      (let ((keywords (cons macro-name (cadr syn-rules)))
	    (clauses (cddr syn-rules)))
	`(define-macro (,macro-name . macro-arg)
	   (let ((macro-arg (cons ',macro-name macro-arg))
		 (keywords ',keywords))
	     (cond ,@(map
		      (lambda (clause)
			(let ((in-pattern (car clause))
			      (out-pattern (cadr clause)))
			  `((mbe:matches-pattern? ',in-pattern macro-arg
						  keywords)
			    (let ((tagged-out-pattern+alist
				   (mbe:tag
				    ',out-pattern
				    (mbe:nconc (mbe:flatten ',in-pattern)
					   keywords) '())))
			      (mbe:untag
			       (mbe:expand-pattern
				(car tagged-out-pattern+alist)
				(mbe:get-bindings ',in-pattern macro-arg
						  keywords)
				keywords)
			       (cdr tagged-out-pattern+alist)
			       '())))))
		      clauses)
		   (else (error ',macro-name 'no-matching-clause
				     ',clauses))))))))