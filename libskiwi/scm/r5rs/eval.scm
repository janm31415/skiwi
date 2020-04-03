(import 'srfi-6)

(define (eval x)
  (let ((s (open-output-string)))
    (%output-to-port x #t s)
    (let ((result (%eval (get-output-string s))))
      (close-output-port s)
      result
    )
  )
)