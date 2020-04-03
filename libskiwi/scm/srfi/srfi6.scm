(define open-input-string (lambda (s)     
                                  (make-port #t "input-string" -2 s 0 (string-length s))
                                  ))
(define open-output-string (lambda ()     
                                   (make-port #f "output-string" -2 (make-string 256) 0 256)
                                   ))
(define get-output-string (lambda (s)
                                  (substring (%slot-ref s 3) 0 (%slot-ref s 4))))