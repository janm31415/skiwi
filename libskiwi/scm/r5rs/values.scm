#|
from https://www.scheme.com/tspl3/control.html#./control:s53
|#
(define values #f)
(define call-with-values #f)

(let ((magic (##cons 'multiple 'values)))
  (define magic?
    (lambda (x)
      (and (##pair? x) (##eq? (##car x) magic)))) 

 (set! values
    (lambda args
      (if (and (not (##null? args)) (##null? (cdr args)))
          (car args)
          (##cons magic args)))) 

 (set! call-with-values
    (lambda (producer consumer)
      (let ((x (producer)))
        (if (magic? x)
            (apply consumer (cdr x))
            (consumer x))))))