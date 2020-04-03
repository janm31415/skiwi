

(define *modules* '())
(define *loaded-modules* '())

;;
(define (module-name->strings ls res)
  (if (null? ls)
      res
      (let ((str (cond ((symbol? (car ls)) (symbol->string (car ls)))
                       ((number? (car ls)) (number->string (car ls)))
                       ((string? (car ls)) (car ls))
                       (else (error "invalid module name" (car ls))))))
        (module-name->strings (cdr ls) (cons "/" (cons str res))))))
        
;;        
(define (module-name->file name)
  (apply string-append
   (reverse (cons ".scm" (cdr (module-name->strings name '()))))))
   
;;   
(define (module-name->path name)
  (let ((file (module-name->file name))
        (module-path (getenv "SKIWI_MODULE_PATH")))
    (if module-path
        (string-append module-path file)  
        (begin (error "invalid skiwi module path") #f)
    )
  )
) 
       
;;
(define (load-module name)
  (let ((path (module-name->path name)))
    (if path (load path))
  )
)

(define (find-module name)
  (cond
   ((assoc name *modules*) => cdr)
   (else #f)))
   
(define (find-loaded-module name)
  (cond
   ((assoc name *loaded-modules*) => cdr)
   (else #f)))  

(define (add-module! name module)
  (set! *modules* (cons (cons name module) *modules*))) 

(define (add-loaded-module! name module)
  (set! *loaded-modules* (cons (cons name module) *loaded-modules*))) 
   
(define (imported? name)
  (let ((m (find-loaded-module name)))
    (if m #t #f)
  )
) 

(define (import name)
  (if (not (imported? name))
      (let ((m (find-module name)))
        (if (not m)
            (error "Invalid module")
            (begin
              (load-module (vector-ref m 1))
              (add-loaded-module! name m)
              #t
            )
        )
      )
      #t
  )    
)

(define (make-module name files) (vector name files))
 
(defmacro define-module (name filelist)
  `(let ((m (make-module (quote ,name) (let ((files (quote ,filelist))) 
                                (if (list? files)
                                    files
                                    (list files)
                                )    
                              )
            )))
   (if (not (find-module (quote ,name)))
            (begin (add-module! (quote ,name) m) #t)
            (error "module already defined")
            )
   )
)
          
(load-module '(packages))