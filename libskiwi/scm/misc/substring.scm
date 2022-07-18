;This program takes a string (the pattern) and returns a function which takes a string (the target) and either returns #f or the index in the target in which the pattern first occurs as a substring.
;
; example usage:
;   (define findJan (substring-search-maker "Jan"))
; <lambda>
;   (findJan "Januari")
; 0
;   (findJan "adf")
; #f
;   (findJan "allezJan")
; 5
;   (findJan "allezjan")
; #f

(define (substring-search-maker pattern-string)

  (define num-chars-in-charset 256)  ;; Update this, e.g. for ISO Latin 1


  (define (build-shift-vector pattern-string)
    (let* ( (pat-len (string-length pattern-string))
             (shift-vec (make-vector num-chars-in-charset (+ pat-len 1)))
             (max-pat-index (- pat-len 1))
          )
      (let loop ( (index 0) )
        (vector-set! shift-vec
                      (char->integer (string-ref pattern-string index))
                      (- pat-len index)
        )
         (if (< index max-pat-index)
             (loop (+ index 1))
             shift-vec)
  ) ) )


  (let ( (shift-vec (build-shift-vector pattern-string))
          (pat-len   (string-length pattern-string))
       )

   (lambda (target-string)

      (let* ( (tar-len (string-length target-string))
               (max-tar-index (- tar-len 1))
               (max-pat-index (- pat-len 1))
            )
         (let outer ( (start-index 0) )
           (if (> (+ pat-len start-index) tar-len)
                #f
                (let inner ( (p-ind 0) (t-ind start-index) )
                   (cond
                    ((> p-ind max-pat-index)  ; nothing left to check
                     #f                       ; fail
                    )
                    ((char=? (string-ref pattern-string p-ind)
                             (string-ref target-string  t-ind))
                     (if (= p-ind max-pat-index)
                         start-index  ;; success -- return start index of match
                         (inner (+ p-ind 1) (+ t-ind 1)) ; keep checking
                    )
                   )
                   ((> (+ pat-len start-index) max-tar-index) #f) ; fail
                    (else
                      (outer (+ start-index
                                (vector-ref shift-vec
                                            (char->integer
                                                 (string-ref target-string
                                                             (+ start-index pat-len)
                   ) )      )  )           )    )
                  ) ; end-cond
          ) ) )
   )  ) ; end-lambda
) )


(define (substring? str pattern-string)
  (let ((f (substring-search-maker pattern-string)))
    (f str)
  )
)

