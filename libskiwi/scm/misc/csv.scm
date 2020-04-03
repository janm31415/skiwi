(import 'srfi-6)

(define (read-line . port)
  (define (eat p c)
    (if (and (not (eof-object? (peek-char p)))
             (char=? (peek-char p) c))
        (read-char p)))
  (let ((p (if (null? port) (current-input-port) (car port))))
    (let loop ((c (read-char p)) (line '()))
      (cond [(eof-object? c) (if (null? line) c (list->string (reverse line)))]
            [(char=? #\newline c) (eat p #\return) (list->string (reverse line))]
            [(char=? #\return c) (eat p #\newline) (list->string (reverse line))]
            [else (loop (read-char p) (cons c line))]))))

;; Parse a LINE from a CSV file and return the list of "cells" in it as
;; strings.  Takes special care that comma characters "," inside strings are
;; correctly handled.  Also double quotes inside strings are unquoted.
(define (parse-line in)
  (let loop ((c (read-char in))
             (current (open-output-string))
             (row '())
             (in-string? #f))
    (cond ((eof-object? c)
           (reverse (cons (get-output-string current) row)))
          ((equal? c #\newline)
           ;; Recognize #\newline + #\return combinations
           (when (equal? (peek-char in) #\return) (read-char in))
           (reverse (cons (get-output-string current) row)))
          ((equal? c #\return)
           ;; Recognize #\return + #\newline combinations
           (when (equal? (peek-char in) #\newline) (read-char in))
           (reverse (cons (get-output-string current) row)))
          ((and (eqv? c #\,) (not in-string?))
           (loop (read-char in)
                 (open-output-string)
                 (cons (get-output-string current) row)
                 #f))
          ((and in-string? (eqv? c #\") (eqv? (peek-char in) #\"))
           (write-char c current)
           (read-char in)             ; consume the next char
           (loop (read-char in) current row in-string?))
          ((eqv? c #\")
           (write-char c current)
           (loop (read-char in) current row (not in-string?)))
          (#t
           (write-char c current)
           (loop (read-char in) current row in-string?)))))  


(define (preview-csv path rows)
  (let ([p (open-input-file path)])
    (let loop ([row (read-line p)]
               [results '()]
               [iter rows])
      (cond [(or (eof-object? row) (< iter 1))
             (close-input-port p)
             (reverse results)]
            [else
             (loop (read-line p) (cons (parse-line (open-input-string row)) results) (sub1 iter))]))))

(define (read-csv path)
  (preview-csv path 4611686018427387903))

(define (csv-map proc csvlst)
  (map (curry map proc) csvlst))

(define (csv->numbers csvlst)
  (csv-map string->number csvlst))

(define (quote-string str)
  (let* ([in (open-input-string str)]
         [str-list (string->list str)]
         [str-length (length str-list)])
    (if (not (or (member #\, str-list) (member #\" str-list)))
        str  ;; return string unchanged b/c no commas or double quotes
        (let loop ([c (read-char in)]
                   [result "\""]
                   [ctr 0])
          (cond [(eof-object? c)
                 (string-append result "\"")]
                [(and (char=? c #\") (or (= ctr 0) (= ctr (sub1 str-length))))
                 ;; don't add double-quote character to string when it is at start or end of string
                 (loop (read-char in) (string-append result "") (add1 ctr))]
                 ;; 2x double-quotes for double-quotes inside string (not at start or end)
                [(char=? c #\")
                 (loop (read-char in) (string-append result "\"\"") (add1 ctr))]
                [else
                 (loop (read-char in) (string-append result (string c)) (add1 ctr))])))))
                 
                 
(define (delimit-list ls)
  (define (iterate ls result first?)
    (if (null? ls)
        result
        (let* ([item (car ls)]
               [sep (if first? "" ",")]
               [item-new (cond [(char? item) (string item)]
                               [(symbol? item) (symbol->string item)]
                               [(real? item) (number->string
                                              (if (exact? item)
                                                  (exact->inexact item)
                                                  item))]
               [else (quote-string item)])])
          (iterate (cdr ls) (string-append result sep item-new) #f))))
  (iterate ls "" #t))                 
  
  

(define (write-csv ls path)
  (let ([p (open-output-file path)])
    (let loop ([ls-local ls])
      (cond [(null? ls-local)
             (close-output-port p)]
            [else
             (write-string (delimit-list (car ls-local)) p)
             (newline p)
             (loop (cdr ls-local))]))))