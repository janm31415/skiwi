#|
dynamic wind/unwind from dynwind.scm by Aubrey Jaffer
|#

(define %dynamic-winds '())
(define dynamic-wind (lambda (before thunk after)
                             (before)
                             (set! %dynamic-winds (cons (cons before after) %dynamic-winds))
                             (let ((ans (thunk)))
                                (set! %dynamic-winds (cdr %dynamic-winds))
                                (after)
                                ans)))

(define %dynamic-unwind #f)

(define call-with-current-continuation
  (let ((oldcc %call/cc))
    (lambda (proc)
      (let ((winds %dynamic-winds))
        (oldcc
         (lambda (cont)
           (proc (lambda (c2)
                (%dynamic-unwind winds (fx- (length %dynamic-winds) (length winds)))
                (cont c2)))))))))

(set! %dynamic-unwind (lambda (to delta)
  (cond ((##eq? %dynamic-winds to))
  ((fx<? delta 0)
    (%dynamic-unwind (cdr to) (fx+ delta 1))
    ((car (car to)))
    (set! %dynamic-winds to))
  (else
    (let ((after (cdr (car %dynamic-winds))))
      (set! %dynamic-winds (cdr %dynamic-winds))
      (after)
      (%dynamic-unwind to (fx- delta 1)))))))

(define call/cc call-with-current-continuation)