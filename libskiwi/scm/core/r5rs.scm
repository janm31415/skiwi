
(define exact? fixnum?)

(define inexact? flonum?)

(define number? (lambda (x) (or (##fixnum? x) (##flonum? x))))

(define real? number?)

(define complex? number?)

(define rational? (lambda (x) 
                  (and (number? x)
                    (or (##fixnum? x) 
                        (not (##eq? 2047 (##ieee754-exponent x))) ; inf or nan
                  ))))

(define negative? (lambda (x) (< x 0)))
(define positive? (lambda (x) (> x 0)))

(define even? (lambda (x) (zero? (if (##fixnum? x) (##bitwise-and x 1) (/ x 2) )  )))
(define odd? (lambda (x) (not (even? x))))

(define modulo (lambda(x y)
                  (let([z(remainder x y)])
                    (if (negative? y)
                        (if (positive? z) (+ z y) z)
                        (if (negative? z) (+ z y) z)))))

(define abs (lambda (x) (if (negative? x) (- x) x)))

(define inexact->exact (lambda (x)
                               (if (##flonum? x)
                                   (##flonum->fixnum x)
                                   x)))

(define exact->inexact (lambda (x)
                               (if (##fixnum? x)
                                   (##fixnum->flonum x)
                                   x)))

(define finite? (lambda (x)
                        (or (##fixnum? x)
                            (not (##eq? 2047 (ieee754-exponent x)))
                         )))

(define nan? (lambda (x)
                     (and (not (##fixnum? x))
                          (##eq? 2047 (ieee754-exponent x))
                          (not (##eq? 0 (ieee754-mantissa x)))
                      )))

(define inf? (lambda (x)
                     (and (not (##fixnum? x))
                          (##eq? 2047 (ieee754-exponent x))
                          (##eq? 0 (ieee754-mantissa x))
                      )))

(define integer? (lambda (x)
                         (or (##fixnum? x)
                             (and  (##flonum? x)
                                   (let ([e (ieee754-exponent x)]
                                         [m (ieee754-mantissa x)])
                                        (cond 
                                            [(##eq? 2047 e) #f]
                                            [(##eq? 0 e) (##eq? 0 m)]
                                            [(fx>=? e 1075) #t]
                                            [(fx<? e 1023) #f]
                                            [else (##eq? 0 (arithmetic-shift m (fx- e 1012)))]
                                         ))))))

(define expt (lambda (x y)
                     (if (##eq? y 0)
                         1
                         (cond [(and (##flonum? y) (##fl=? y 0.0)) 1.0]
                               [(and (##fixnum? x) (##fixnum? y) (positive? y) (##fx<? y 256)) (fixnum-expt x y)]
                               [(negative? y) (/ 1.0 (flonum-expt x (- y)))]
                               [else (flonum-expt x y)]
                         ))))

(define exp (lambda (x)
                    (let ([e 2.7182818284590452353602874])
                         (expt e x)
                    )))

(define sqrt ieee754-sqrt)
(define sin ieee754-sin)
(define cos ieee754-cos)
(define tan ieee754-tan)
(define asin ieee754-asin)
(define acos ieee754-acos)
(define log ieee754-log)
(define round (lambda (n)
                      (if (##flonum? n)
                          (##ieee754-flround n)
                          n
                          
                      )))
(define atan (lambda (x . args)
                     (let ([l (length args)])
                          (if (##eq? l 0)
                              (ieee754-atan1 x)
                              (let* ([pi (##ieee754-pi)]
                                     [pi/2 (##fl/ pi 2.0)])
                                    (let ([Y (exact->inexact x)]
                                          [X (car args)])                                          
                                         (cond [(= X 0) (if (> Y 0) pi/2 (- pi/2))]
                                               [(> X 0) (ieee754-atan1 (/ Y X))]
                                               [(< Y 0) (- (ieee754-atan1 (/ Y X)) pi)]
                                               [else (+ (ieee754-atan1 (/ Y X)) pi)]
                                          )))))))

(define ceiling (lambda (n)
                        (if (##fixnum? n)
                            n
                            (let ([t (ieee754-truncate n)])
                                 (if (< t n)
                                     (fx+ t 1)
                                     t
                                 )))))

(define floor   (lambda (n)
                        (if (##fixnum? n)
                            n
                            (let ([t (ieee754-truncate n)])
                                 (if (> t n)                                                                          
                                     (fx- t 1)
                                     t
                                 )))))

(define truncate ieee754-truncate)


(define gcd1 (lambda (x y)
                     (let loop ([x x][y y])
                               (if (zero? y)
                                   (abs x)
                                   (loop y (remainder x y))
                                ))))

(define gcd (lambda ( . args)
                    (if (##null? args)
                        0
                        (let loop ([args args] [f #t])
                                  (let ([head (car args)] [next (cdr args)])
                                       (if (##null? next)
                                           (abs head)
                                           (let ([n2 (car next)])
                                                (loop (##cons (gcd1 head n2) (cdr next)) #f)
                                           )))))))

(define lcm1 (lambda (x y)
                     (quotient (* x y) (gcd1 x y))))

(define lcm (lambda ( . args)
                    (if (##null? args)
                        1
                        (let loop ([args args] [f #t])
                                  (let ([head (car args)] [next (cdr args)])
                                       (if (##null? next)
                                           (abs head)
                                           (let ([n2 (car next)])
                                                (loop (##cons (lcm1 head n2) (cdr next)) #f)
                                           )))))))



(define char->integer char->fixnum)
(define integer->char fixnum->char)
(define %char-downcase (lambda (c)
                               (let ([n (char->fixnum c)])
                                    (if (or (fx<? n 65) (fx>? n 90))
                                        n
                                        (bitwise-or 32 n)
                                     ))))
(define char-downcase (lambda (c) (fixnum->char (%char-downcase c))))

(define %char-upcase (lambda (c)
                             (let ([n (char->fixnum c)])
                                  (if (or (fx<? n 97) (fx>? n 122))
                                      n
                                      (bitwise-and 223 n)
                                   ))))
(define char-upcase (lambda (c) (fixnum->char (%char-upcase c))))

(define char-ci=? (lambda (x y) (##eq? (%char-downcase x) (%char-downcase y))))
(define char-ci<? (lambda (x y) (< (%char-downcase x) (%char-downcase y))))
(define char-ci>? (lambda (x y) (> (%char-downcase x) (%char-downcase y))))
(define char-ci<=? (lambda (x y) (<= (%char-downcase x) (%char-downcase y))))
(define char-ci>=? (lambda (x y) (>= (%char-downcase x) (%char-downcase y))))

(define char-alphabetic? (lambda (c)
                         (let ([n (char->fixnum c)])
                              (cond [(fx<? n 65) #f]
                                    [(fx>? n 122) #f]
                                    [(fx>? n 96) #t]
                                    [(fx<? n 91) #t]
                                    [else #f]
                               ))))

(define char-numeric?    (lambda (c)
                         (let ([n (char->fixnum c)])
                              (cond [(fx<? n 48) #f]
                                    [(fx>? n 57) #f]
                                    [else #t]
                               ))))

(define char-lower-case?    (lambda (c)
                         (let ([n (char->fixnum c)])
                              (cond [(fx<? n 97) #f]
                                    [(fx>? n 122) #f]
                                    [else #t]
                               ))))

(define char-upper-case?    (lambda (c)
                         (let ([n (char->fixnum c)])
                              (cond [(fx<? n 65) #f]
                                    [(fx>? n 90) #f]
                                    [else #t]
                               ))))

(define char-whitespace? (lambda (c)
                                 (let ([n (char->fixnum c)])
                                      (or (##eq? n 32) (##eq? n 9) (##eq? n 12) (##eq? n 10) (##eq? n 13))
                                 )))
(define char->string (lambda (c)
                             (let ([s (make-string 1 #\\000)])
                                  (string-set! s 0 c) 
                                  s)))

(define number->string (lambda (num . base)
                               (let* ([l (length base)] 
                                      [b (if (##eq? l 0) 10 (car base))])  
                                      (define (string-has-e s)
                                        (if (%eqv? s "")
                                            #f
                                            (if (char=? (string-ref s 0) #\e)
                                                #t
                                                (string-has-e (substring s 1 (string-length s)))
                                             )))                                
                                      (cond [(nan? num) "+nan.0"]
                                            [(finite? num) (let ([str (num2str num b)])(if (and (##flonum? num) (integer? num) (not (string-has-e str))) (string-append str ".0") str))]
                                            [(negative? num) "-inf.0"]
                                            [else "+inf.0"]
                                      ))))

(define string->number (lambda (str . base)
                               (let* ([l (length base)] 
                                      [b (if (##eq? l 0) 10 (car base))]
                                      [len (string-length str)])  
                                      (cond ((##eq? len 0) #f)
                                            ((or (%eqv? str "+nan.0")
                                                 (%eqv? str "-nan.0"))
                                             (/ 0 0))
                                            ((%eqv? str "+inf.0") (/ 1 0))
                                            ((%eqv? str "-inf.0") (/ -1 0))
                                            (else (str2num str b)))
                                      )))

(define string=? %eqv?)
(define string<? (lambda (x y)
                 (let* ([xlen (string-length x)]
                        [ylen (string-length y)]
                        [len (if (fx<? xlen ylen) xlen ylen)]
                        [r (compare-strings x y len)])
                        (if (##eq? r 0) (fx<? xlen ylen) (##eq? r -1))
                  )))
(define string>? (lambda (x y)
                 (let* ([xlen (string-length x)]
                        [ylen (string-length y)]
                        [len (if (fx<? xlen ylen) xlen ylen)]
                        [r (compare-strings x y len)])
                        (if (##eq? r 0) (fx>? xlen ylen) (##eq? r 1))
                  )))
(define string<=? (lambda (x y)
                  (let* ([xlen (string-length x)]
                         [ylen (string-length y)]
                         [len (if (fx<? xlen ylen) xlen ylen)]
                         [r (compare-strings x y len)])
                         (if (##eq? r 0) (fx<=? xlen ylen) (fx<=? r 0))
                   )))
(define string>=? (lambda (x y)
                  (let* ([xlen (string-length x)]
                         [ylen (string-length y)]
                         [len (if (fx<? xlen ylen) xlen ylen)]
                         [r (compare-strings x y len)])
                         (if (##eq? r 0) (fx>=? xlen ylen) (fx>=? r 0))
                   )))

(define string-ci=? (lambda (x y)
                  (let ([xlen (string-length x)]
                        [ylen (string-length y)])
                        (and (##eq? xlen ylen) (##eq? (compare-strings-ci x y xlen) 0))
                  )))
(define string-ci<? (lambda (x y)
                 (let* ([xlen (string-length x)]
                        [ylen (string-length y)]
                        [len (if (fx<? xlen ylen) xlen ylen)]
                        [r (compare-strings-ci x y len)])
                        (if (##eq? r 0) (fx<? xlen ylen) (##eq? r -1))
                  )))
(define string-ci>? (lambda (x y)
                 (let* ([xlen (string-length x)]
                        [ylen (string-length y)]
                        [len (if (fx<? xlen ylen) xlen ylen)]
                        [r (compare-strings-ci x y len)])
                        (if (##eq? r 0) (fx>? xlen ylen) (##eq? r 1))
                  )))
(define string-ci<=? (lambda (x y)
                  (let* ([xlen (string-length x)]
                         [ylen (string-length y)]
                         [len (if (fx<? xlen ylen) xlen ylen)]
                         [r (compare-strings-ci x y len)])
                         (if (##eq? r 0) (fx<=? xlen ylen) (fx<=? r 0))
                   )))
(define string-ci>=? (lambda (x y)
                  (let* ([xlen (string-length x)]
                         [ylen (string-length y)]
                         [len (if (fx<? xlen ylen) xlen ylen)]
                         [r (compare-strings-ci x y len)])
                         (if (##eq? r 0) (fx>=? xlen ylen) (fx>=? r 0))
                   )))

(define string->list (lambda (str)
                             (let ([n (string-length str)])
                                  (let loop ([i (fx- n 1)] [lst '()])
                                            (if (fx<? i 0)
                                                lst
                                                (loop (fx- i 1) (##cons (string-ref str i) lst))
                                            )))))
(define list->string (lambda (lst)
                             (let* ([n (length lst)] [s (make-string n #\\000)])
                                   (let loop ([i 0][lst lst])
                                             (if (fx>=? i n)
                                                 s
                                                 (begin (string-set! s i (car lst)) (loop (fx+ i 1) (cdr lst)))
                                             )))))

(define string-append (lambda ( . slst)
                              (if (##null? slst)
                                  ""
                                  (if (##null? (##cdr slst))
                                      (##car slst)
                                      (let ([s (##car slst)])
                                           (let loop ([slst slst])
                                                (if (##null? (cdr slst))
                                                    s
                                                    (begin
                                                       (set! s (string-append1 s (car (cdr slst))))
                                                       (loop (cdr slst))
                                                    )))                              
                                      )
                                  ))))


(define vector->list (lambda (str)
                             (let ([n (vector-length str)])
                                  (let loop ([i (fx- n 1)] [lst '()])
                                            (if (fx<? i 0)
                                                lst
                                                (loop (fx- i 1) (##cons (vector-ref str i) lst))
                                            )))))
(define list->vector (lambda (lst)
                             (let* ([n (length lst)] [s (make-vector n)])
                                   (let loop ([i 0][lst lst])
                                             (if (fx>=? i n)
                                                 s
                                                 (begin (vector-set! s i (car lst)) (loop (fx+ i 1) (cdr lst)))
                                             )))))

(define caar (lambda (x) (car (car x))))
(define cadr (lambda (x) (car (cdr x))))
(define cdar (lambda (x) (cdr (car x))))
(define cddr (lambda (x) (cdr (cdr x))))
(define caaar (lambda (x) (car (car (car x)))))
(define caadr (lambda (x) (car (car (cdr x)))))
(define cadar (lambda (x) (car (cdr (car x)))))
(define caddr (lambda (x) (car (cdr (cdr x)))))
(define cdaar (lambda (x) (cdr (car (car x)))))
(define cdadr (lambda (x) (cdr (car (cdr x)))))
(define cddar (lambda (x) (cdr (cdr (car x)))))
(define cdddr (lambda (x) (cdr (cdr (cdr x)))))
(define caaaar (lambda (x) (car (car (car (car x))))))
(define caaadr (lambda (x) (car (car (car (cdr x))))))
(define caadar (lambda (x) (car (car (cdr (car x))))))
(define caaddr (lambda (x) (car (car (cdr (cdr x))))))
(define cadaar (lambda (x) (car (cdr (car (car x))))))
(define cadadr (lambda (x) (car (cdr (car (cdr x))))))
(define caddar (lambda (x) (car (cdr (cdr (car x))))))
(define cadddr (lambda (x) (car (cdr (cdr (cdr x))))))
(define cdaaar (lambda (x) (cdr (car (car (car x))))))
(define cdaadr (lambda (x) (cdr (car (car (cdr x))))))
(define cdadar (lambda (x) (cdr (car (cdr (car x))))))
(define cdaddr (lambda (x) (cdr (car (cdr (cdr x))))))
(define cddaar (lambda (x) (cdr (cdr (car (car x))))))
(define cddadr (lambda (x) (cdr (cdr (car (cdr x))))))
(define cdddar (lambda (x) (cdr (cdr (cdr (car x))))))
(define cddddr (lambda (x) (cdr (cdr (cdr (cdr x))))))

(define list? (lambda (x)
                      (let loop ([fast x] [slow x])
                                (or (##null? fast)
                                    (and (##pair? fast)
                                         (let ([fast (##cdr fast)])
                                              (or (##null? fast)
                                                  (and (##pair? fast)
                                                       (let ([fast (##cdr fast)][slow (cdr slow)])
                                                            (and (##not (##eq? fast slow))
                                                            (loop fast slow)))))))))))

(define append (lambda ( . lsts)
                       (if (##null? lsts)
                           '()
                           (let loop ([lsts lsts])
                                     (if (##null? (##cdr lsts))
                                         (##car lsts)
                                         (let copy ([node (##car lsts)])
                                                   (if (##pair? node)
                                                       (##cons (##car node) (copy (##cdr node)))
                                                       (loop (##cdr lsts)) 
                                                    )))))))

(define list-tail (lambda (x k)
                          (if (fx=? k 0)
                              x
                              (list-tail (cdr x) (fx- k 1))
                          )))
(define list-ref (lambda (lst i)
                   (car (list-tail lst i))
                   ))

(define reverse (lambda (lst)
                        (let loop ([lst lst] [rest '()])
                                  (if (##pair? lst)
                                      (loop (##cdr lst) (##cons (##car lst) rest))
                                      rest
                                  ))))

(define (list-set! list k val)
    (if (zero? k)
        (set-car! list val)
        (list-set! (cdr list) (- k 1) val)))

(define (list-set lst idx val)
  (if (null? lst)
    lst
    (cons
      (if (zero? idx)
        val
        (car lst))
      (list-set (cdr lst) (- idx 1) val))))

(define map (lambda (proc lst1 . lsts)
                    (if (##null? lsts)
                        (let loop ([lst lst1]) ; fast case
                                  (if (##null? lst)
                                      '()
                                      (##cons (proc (car lst)) (loop (cdr lst)))
                                   ))
                        (let loop ([lsts (##cons lst1 lsts)])
                                  (let ([hds (let loop2 ([lsts lsts])
                                                        (if (##null? lsts)
                                                            '()
                                                            (let ([x (car lsts)])
                                                                 (and (not (##null? x))
                                                                      (let ([r (loop2 (cdr lsts))])
                                                                           (and r (##cons (car x) r))
                                                                      )))))])
                                        (if hds
                                            (##cons (apply proc hds)
                                                  (loop (let loop3 ([lsts lsts])
                                                                   (if (##null? lsts)
                                                                       '()
                                                                       (##cons (cdar lsts) (loop3 (cdr lsts)))
                                                                    ))))
                                             '()
                                         ))))))

(define (curry func . args) ; not r5rs standard, but used by csv reader
    (lambda x (apply func (append args x))))


(define for-each (lambda (proc lst1 . lsts)
                         (if (##null? lsts)
                             (let loop ([lst lst1]) ; fast case
                                  (unless (##null? lst)
                                          (proc (car lst))
                                          (loop (cdr lst))))
                             (let loop ([lsts (##cons lst1 lsts)])
                                       (let ([hds (let loop2 ([lsts lsts])
                                                              (if (##null? lsts)
                                                                  '()
                                                                  (let ([x (car lsts)])
                                                                       (and (not (##null? x))
                                                                            (let ([r (loop2 (cdr lsts))])
                                                                                 (and r (##cons (car x) r)))))))])  
                                            (when hds (apply proc hds)
                                                      (loop (let loop3 ([lsts lsts])
                                                                       (if (##null? lsts)
                                                                           '()
                                                                           (##cons (cdar lsts) (loop3 (cdr lsts)))
                                                                        )))))))))

(define force (lambda (p)
                      (if (promise? p)
                          ((%slot-ref p 0))
                          p
                      )))

(define make-promise (lambda  (proc)
                              (let ([result-ready? #f]
                                        [result #f])
                                        (%make-promise (lambda ()
                                                          (if result-ready?
                                                              result
                                                              (let ([x (proc)])
                                                                  (if result-ready?
                                                                      result
                                                                      (begin (set! result-ready? #t)
                                                                              (set! result x)
                                                                              result
                                                                      )))))))))

(define (%list->number xs neg)
  (define (iter xs r)
    (if (##null? xs)
        r
        (iter (cdr xs)
              (+ ((if neg - +)
                  0
                  (- (char->fixnum (car xs)) (char->fixnum #\0)))
                 (* 10 r)))))
  (iter xs 0))

(define (%read-token port)
   (let ((first-char (read-char port)))
     (cond  ((eof-object? first-char)
             first-char)
            ((%char-whitespace? first-char)
             (%read-token port))
            ((or (##eq? first-char #\( ) (##eq? first-char #\[ ))
             %left-paren-token)
            ((or (##eq? first-char #\)) (##eq? first-char #\] ))
             %right-paren-token)
            ((and (##eq? first-char #\.) (%char-whitespace? (peek-char port)))
             %dot-token)
            ((##eq? first-char #\-)
             (let ((next-char (peek-char port)))
               (if (char-numeric? next-char)
                   (%read-number #t (read-char port) port)
                   (%read-identifier first-char port))))
            ((char-alphabetic? first-char)
             (%read-identifier first-char port))
            ((char-numeric? first-char)
             (%read-number #f first-char port))
            ((##eq? #\" first-char)
             (%read-string first-char port))
            ((##eq? #\' first-char)
                        (list 'quote (read port)))
            ((##eq? first-char #\#)
             (%read-character first-char port))
            ((##eq? first-char #\;)
             (%read-comment first-char port)
             (%read-token port))
            (else
             ;(error '%read-token "illegal lexical syntax" first-char)
             (%read-identifier first-char port)
             ))))

(define (%char-whitespace? char)
  (or (##eq? char #\space)
      (##eq? char #\newline)
      (##eq? char #\tab)
      (##eq? char #\return)))

(define (%char-special? char)
  (or (##eq? char #\!)
      (##eq? char #\$)
      (##eq? char #\%)
      (##eq? char #\&)
      (##eq? char #\*)
      (##eq? char #\/)
      (##eq? char #\:)
      (##eq? char #\<)
      (##eq? char #\>)
      (##eq? char #\=)
      (##eq? char #\?)
      (##eq? char #\^)
      (##eq? char #\_)
      (##eq? char #\~)))

(define %left-paren-token (list '*%left-paren-token*))
(define %right-paren-token (list '*%right-paren-token*))
(define %dot-token (list '*%dot-token*))
(define (%token-leftpar? thing)
  (##eq? thing %left-paren-token))
(define (%token-rightpar? thing)
  (##eq? thing %right-paren-token))
(define (%token-dot? thing)
  (##eq? thing %dot-token))

(define (%read-string chr port)
  (define (helper list-so-far)
    (let ((next-char (read-char port)))
      (if (##eq? #\\ next-char)
          (helper (##cons (read-char port) list-so-far))
          (if (##eq? #\" next-char)
              (list->string (reverse list-so-far))
              (helper (##cons next-char list-so-far))))))
  (helper '()))

(define (%read-character chr port)
  (let ((next-char (read-char port)))
    (if (##eq? next-char #\\)
        (let ((first-char (read-char port)))
          (let ((s (%read-identifier first-char port)))
            (cond ((##eq? s 'space) #\space)
                  ((##eq? s 'newline) #\newline)
                  ((##eq? s 'tab) #\tab)
                  ((##eq? s 'return) #\return)
                  (else first-char))))
        (if (##eq? next-char #\t)
            #t
            (if (##eq? next-char #\f)
                #f
                (if (##eq? next-char #\()
                    (list->vector (%read-list '() port))
                    (error '%read-character "expected bool, char or vector")))))))

(define (%read-identifier chr port)
  (define (%read-identifier-helper list-so-far)
    (let ((next-char (peek-char port)))
     (if (and  (char? next-char)
               (or (char-alphabetic? next-char)
                   (char-numeric? next-char)
                   (char=? next-char #\-)
                   (%char-special? next-char)
                   ))
          (%read-identifier-helper (##cons (read-char port) list-so-far))
          (reverse list-so-far))))
  (string->symbol (list->string (%read-identifier-helper (list chr)))))
#|
(define (%read-number neg chr port)
  (define (%read-number-helper list-so-far)
    (let ((next-char (peek-char port)))
      (if (and (char? next-char) (char-numeric? next-char))
          (%read-number-helper (##cons (read-char port) list-so-far))
          (reverse list-so-far))))
  (%list->number (%read-number-helper (list chr)) neg))
|#
#|
(define (%read-number neg chr port)
  (define (%read-number-helper list-so-far)
    (let ((next-char (peek-char port)))
      (if (and (char? next-char) (char-numeric? next-char))
          (%read-number-helper (##cons (read-char port) list-so-far))
          (reverse list-so-far))))		  
  (let* ((i (%list->number (%read-number-helper (list chr)) neg))  (next-char (peek-char port)))
       (if (##eq? next-char #\.)
	       (begin
		     (read-char port)
		     (let ((dec (%read-number-helper (list))))
			 ((if neg - +) i (/ (%list->number dec #f) (expt 10 (length dec)) ))
		     ))
		   i)))
|#

(define (%read-number neg chr port)
  (define (%read-number-helper list-so-far dot expo)
    (let ((next-char (peek-char port)))
      (if (and (char? next-char) (char-numeric? next-char))
          (%read-number-helper (##cons (read-char port) list-so-far) dot expo)
          (if (and (##eq? next-char #\.) (##not dot) (##not expo))
              (%read-number-helper (##cons (read-char port) list-so-far) #t expo)
              (if (and (##eq? next-char #\e) (##not expo))
                  (let* ((lst (##cons (read-char port) list-so-far))
                         (next-char (peek-char port)))
                     (if (or (##eq? next-char #\+) (##eq? next-char #\-))
                         (%read-number-helper (##cons (read-char port) lst) dot #t)
                         (%read-number-helper lst dot #t)
                     )
                  )   
                  (reverse list-so-far)
              )
           )
       )
     )
   )
  ((if neg - +) (string->number (list->string (%read-number-helper (list chr) #f #f))))
)


(define (%read-comment chr port)
  (define (slurp-line port)
    (let ((next-char (read-char port)))
      (if (or (##eq? next-char #\newline) (eof-object? next-char))
          #t
          (slurp-line port))))
  ;(let ((next-char (read-char port)))
  ;  (if (##eq? next-char chr)
  ;      (slurp-line port)
  ;      (error '%read-comment "expected a comment"))))
  (slurp-line port))

(define (%read port)
   (let ((next-token (%read-token port)))
      (cond ((%token-leftpar? next-token)
             (%read-list '() port))
            (else
             next-token))))

(define read (lambda (. args)
                     (let ([l (length args)])
                        (if (##eq? l 0)
                            (%read standard-input-port)
                            (%read (car args))
                        ))))

(define (%read-list list-so-far port)
  (let ((token (%read-token port)))
    (cond ((%token-rightpar? token)
           (reverse list-so-far))
          ((%token-leftpar? token)
           (%read-list (##cons (%read-list '() port) list-so-far) port))
          ((%token-dot? token)
           (let* ((rest (%read port))
                  (r (%read-token port)))         
             (if (%token-rightpar? r)
                 (append (reverse list-so-far) rest)
                 (error '%read-list "unexpected dotted list" rest r))))
          (else
           (%read-list (##cons token list-so-far) port)))))


(define standard-input-port (make-port #t "stdin" 0 (make-string 4096) 4096 4096))
(define standard-output-port (make-port #f "stdout" 1 (make-string 4096) 0 4096))
(define standard-error-port (make-port #f "stderr" 2 (make-string 4096) 0 4096))

(define current-output-port (lambda (. args)
                                    (let ([l (length args)])
                                      (if (##eq? l 0)
                                          standard-output-port
                                          (set! standard-output-port (car args))
                                      ))))

(define current-input-port (lambda (. args)
                                   (let ([l (length args)])
                                     (if (##eq? l 0)
                                         standard-input-port
                                         (set! standard-input-port (car args))
                                     ))))

(define current-error-port (lambda (. args)
                                   (let ([l (length args)])
                                     (if (##eq? l 0)
                                         standard-error-port
                                         (set! standard-error-port (car args))
                                     ))))

(define open-output-file (lambda (n)  
                                 (let ([fh (open-file n #f)])
                                      (make-port #f n fh (make-string 4096) 0 4096)
                                 )))

(define open-input-file (lambda (n)
                                (let ([fh (open-file n #t)])
                                     (make-port #t n fh (make-string 4096) 4096 4096)
                                )))

(define close-input-port (lambda (n)
                                 (%slot-set! n 3 "") (%slot-set! n 4 0) ; empty buffer so that it can be cleared by the gc
                                 (if (fx>=? (%slot-ref n 2) 0)
                                     (let ([res (close-file (%slot-ref n 2))]) (%slot-set! n 2 -1) res)
                                     -1
                                 )
                         ))

(define %write-char-helper (lambda (ch p)
                                   (when (##eq? (%slot-ref p 2) -2)                              ; basic string port (has id -2)
                                       (let ([req-len (fxadd1 (%slot-ref p 4))]
                                             [avail-len (%slot-ref p 5)])
                                         (when (fx>? req-len avail-len)                        ; enlarge available string buffer
                                               (let ([new_s (string-append (%slot-ref p 3) (make-string 256))]) 
                                                       (%slot-set! p 3 new_s)
                                                       (%slot-set! p 5 (fx+ (%slot-ref p 5) 256))
                                               ))
                                       ))                                       
                                       (%write-char ch p)                                       
                                   ))

(define %write-string-helper (lambda (s p)
                                   (when (##eq? (%slot-ref p 2) -2)                               ; basic string port (has id -2)
                                       (let ([req-len (fx+ (string-length s) (%slot-ref p 4))]
                                             [avail-len (%slot-ref p 5)])
                                         (when (fx>? req-len avail-len)                         ; enlarge available string buffer
                                               (let ([new_s (string-append (%slot-ref p 3) (make-string 256))]) 
                                                       (%slot-set! p 3 new_s)
                                                       (%slot-set! p 5 (fx+ (%slot-ref p 5) 256))
                                               ))
                                       ))                                       
                                       (%write-string s p)                                       
                                   ))

(define newline (lambda (. args)
                        (let ([l (length args)])
                          (if (##eq? l 0)
                              (begin (%write-char-helper #\newline standard-output-port) (flush-output-port standard-output-port))
                              (begin (%write-char-helper #\newline (car args)) (flush-output-port (car args)))
                          ))))

(define write-char (lambda (c . args)
                           (let ([l (length args)])
                             (if (##eq? l 0)
                                 (%write-char-helper c standard-output-port)
                                 (%write-char-helper c (car args))
                             ))))

(define write-string (lambda (c . args)
                           (let ([l (length args)])
                             (if (##eq? l 0)
                                 (%write-string-helper c standard-output-port)
                                 (%write-string-helper c (car args))
                             ))))

(define read-char (lambda (. args)
                          (let ([l (length args)])
                             (if (##eq? l 0)
                                 (%read-char standard-input-port)
                                 (%read-char (car args))
                             ))))

(define peek-char (lambda (. args)
                          (let ([l (length args)])
                             (if (##eq? l 0)
                                 (%peek-char standard-output-port)
                                 (%peek-char (car args))
                             ))))

(define flush-output-port (lambda (. args)
                          (let ([l (length args)])
                             (if (##eq? l 0)
                                 (%flush-output-port standard-output-port)
                                 (%flush-output-port (car args))
                             ))))

(define close-output-port (lambda (n)
                                  (if (fx>=? (%slot-ref n 2) 0)
                                      (begin (flush-output-port n) (let ([res (close-file (%slot-ref n 2))]) (%slot-set! n 2 -1) res))
                                      -1
                                  )))

(define error (lambda (reason . args)  ; conform srfi-23
                        (display "Error: ")
                        (display reason)
                        (for-each (lambda (arg) 
                                    (display " ")
		                                (write arg))
		                      args)
                        (newline)
                        (flush-output-port)
                        (%error)
               ))                        

(define (%output-to-port x rd port)
  (define (escape-symbol? str)
     (let ((len (string-length str)))
       (let loop ((i 0))
          (if (fx>=? i len)
              #f
              (let ((c (string-ref str i)))
                (or (char=? c #\|) (char=? c #\\) (char=? c #\() (char=? c #\))
                    (char=? c #\;) (char=? c #\#) (char=? c #\") (char=? c #\[)
                    (char=? c #\]) (char=? c #\{) (char=? c #\}) 
                    (fx<=? (char->integer c) 32)
                    (loop (fx+ i 1))
                ))))))
  (let show ([x x])
    (cond
      [(vector? x) 
          (let ((len (vector-length x)))
            (write-char #\# port)
            (show (vector->list x))
          )]
      [(##pair? x)
         (write-char #\( port)
         (show (car x))
         (let loop ((x (cdr x)))
           (cond [(##null? x) (write-char #\) port)]
                 [(##pair? x) (write-char #\space port) (show (car x)) (loop (cdr x))]
                 [else (write-char #\space port) (write-char #\. port) (write-char #\space port) (show x) (write-char #\) port)]
           ))
         ]
      [(string? x)
         (cond [rd (write-char #\" port)
                   (let ([len (string-length x)])
                     (do ((i 0 (fx+ i 1)))
                         ((fx>=? i len) (write-char #\" port))
                         (let* ([c (string-ref x i)]
                                [n (char->integer c)])
                                (cond [(or (char=? c #\\) (char=? c #\")) (write-char #\\ port) (write-char c port)]
                                      [(char=? c #\newline) (write-char #\\ port) (write-char #\n port)]
                                      [(fx<? n 32) (write-char #\x port) (write-string (number->string n 16) port) (write-char #\; port)]
                                      [else (write-char c port)]
                                ))))
                  ]
               [else (write-string x port)]
         )
         ]
      [(symbol? x)
         (let ([str (symbol->string x)])
#|
              (cond [(and rd (escape-symbol? str) #f)
                       (write-char #\| port)
                       (let* ([str str]
                              [len (string-length str)])
                             (do ((i 0 (fx+ i 1)))
                                 ((fx>=? i len) (write-char #\| port))
                                 (let* ([c (string-ref str i)]
                                        [n (char->integer c)])
                                        (cond
                                           [(or (char=? c #\\) (char=? c #\|)) (write-char #\\ port) (write-char c port)]
                                           [(fx<? n 32) (write-char #\x port) (write-string (number->string n 16) port) (write-char #\; port)]
                                           [else (write-char c port)]
                                        )))
                       )
                    ]
                    [else (write-string str port)])
|#
         (write-string str port)
         )
         ]
      [(char? x)
         (cond [rd (write-char #\# port) (write-char #\\ port)
                   (case x
                     [(#\newline) (write-string "newline" port)]
                     [(#\space) (write-string "space" port)]
                     [(#\tab) (write-string "tab" port)]
                     [else (write-char x port)]
                    )]   
               [else (write-char x port)]
         )
         ]
      [(number? x) (write-string (number->string x) port)]
      [(##null? x) (write-char #\( port) (write-char #\) port)]
      [(promise? x) (write-string "#<promise>" port)]
      [(input-port? x) (write-string "#<input-port>" port)]
      [(output-port? x) (write-string "#<output-port>" port)]
      [(procedure? x) (write-string "#<procedure>" port)]
      [(eof-object? x) (write-string "#<eof>" port)]
      [(##eq? (%undefined) x) (write-string "#<undefined>" port)]
      [(##eq? (%quiet-undefined) x) (write-string "#<quiet-undefined>" port)]
      [(##eq? #t x) (write-char #\# port) (write-char #\t port)]
      [(##eq? #f x) (write-char #\# port) (write-char #\f port)]
      [else (write-string "#<unknown object>" port)]
    )))   

(define display (lambda (x . args)
                        (let ([l (length args)])
                          (if (##eq? l 0)
                              (begin (%output-to-port x #f standard-output-port) (flush-output-port))
                              (begin (%output-to-port x #f (car args)) (flush-output-port (car args)))
                          )                          
                        )))

(define write   (lambda (x . args)
                        (let ([l (length args)])
                          (if (##eq? l 0)
                              (begin (%output-to-port x #t standard-output-port) (flush-output-port))
                              (begin (%output-to-port x #t (car args)) (flush-output-port (car args)))
                          ))))
