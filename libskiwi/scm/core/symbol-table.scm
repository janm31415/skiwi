(define %symbol-table (make-vector 256 (list)))

(define string->symbol  (lambda (str) 
                                (let* ([i (string-hash str)]
                                       [b (vector-ref %symbol-table i)])
                                       (let loop ([bucket b])
                                                 (cond [(##null? bucket) 
                                                          (let ([sym (%allocate-symbol str)])
                                                               (vector-set! %symbol-table i (##cons sym b)) 
                                                               sym
                                                          )
                                                       ]
                                                       [(%eqv? str (symbol->string (car bucket))) (car bucket)]
                                                       [else (loop (cdr bucket))]
                                                  )))))