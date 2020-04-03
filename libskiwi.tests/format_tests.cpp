#include "format_tests.h"
#include "test_assert.h"

#include <libskiwi/format.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>
#include <cctype>

SKIWI_BEGIN

namespace
  {

  std::string cleanup(std::string str)
    {
    std::string res;
    std::istringstream s(str);
    std::string line;
    while (std::getline(s, line))
      {
      line.erase(std::find_if(line.rbegin(), line.rend(), [](int ch)
        {
        return !std::isspace(ch);
        }).base(), line.end());
      auto pos = line.find_first_not_of(' ');
      if (pos != std::string::npos)
        {
        res.append(line);
        res.push_back('\n');
        }
      }
    if (!res.empty())
      res.pop_back(); // remove last newline char
    return res;
    }

  void format_test_invalid_input()
    {
    format_options ops;
    auto res = format("a", ops);
    TEST_EQ("a", res);

    res = format("(a (b (c (d e) (f g)) (h i j)) (k l m n", ops);
    TEST_EQ("(a (b (c (d e) (f g)) (h i j)) (k l m n", res);

    res = format("a (b (c (d e) (f g)) (h i j)) (k l m n))", ops);
    TEST_EQ("a (b (c (d e) (f g)) (h i j)) (k l m n))", res);
    }

  void format_test_1()
    {
    format_options ops;
    ops.max_width = 10;
    TEST_EQ(cleanup(R"(
(a (b (c (d e) 
         (f g)) 
      (h i j)) 
   (k l m n))
)"), cleanup(format("(a (b (c (d e) (f g)) (h i j)) (k l m n))", ops)));
    }

  void format_test_2()
    {
    format_options ops;
    ops.max_width = 10;
    TEST_EQ(cleanup(R"(
(if (= a something) 
    (if (= b otherthing) 
        (foo)))
)"), cleanup(format("(if (= a something)(if (= b otherthing)(foo)))", ops)));
    }

  void format_test_3()
    {
    format_options ops;
    ops.max_width = 10;
    auto res = format("(let([x 5])(define foo(lambda(y) (bar x y))) (define bar(lambda(a b) (+(* a b) a))) (foo(+ x 3)))", ops);
    TEST_EQ(cleanup(R"(
(let ([x 5])
     (define foo
       (lambda
         (y)
         (bar x y)))
     (define bar
       (lambda
         (a b)
         (+ (* a b)
            a)))
     (foo (+ x 3)))
)"), cleanup(res));
    ops.max_width = 60;
    res = format("(let([x 5])(define foo(lambda(y) (bar x y))) (define bar(lambda(a b) (+(* a b) a))) (foo(+ x 3)))", ops);
    TEST_EQ(cleanup(R"(
(let ([x 5]) (define foo (lambda (y) (bar x y)))
             (define bar (lambda (a b) (+ (* a b) a)))
             (foo (+ x 3)))
)"), cleanup(res));
    }

  void format_test_4()
    {
    format_options ops;
    ops.max_width = 10;
    auto res = cleanup(format("()))", ops));
    TEST_EQ("()))", res);
    res = format("(((((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))", ops);
    TEST_EQ("(((((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))", res);
    res = format("((()", ops);
    TEST_EQ("((()", res);
    res = format("(((((((((((((((((((((((((())))))))", ops);
    TEST_EQ("(((((((((((((((((((((((((())))))))", res);
    }
  }

void format_test_5()
  {
  format_options ops;
  ops.max_width = 7;
  auto res = cleanup(format("(let([x 5]) (a) (b) (c))", ops));
  TEST_EQ(cleanup(R"(
(let ([x 5])
     (a)
     (b)
     (c))
)"), res);
  }

void format_test_6()
  {
  format_options ops;
  ops.max_width = 10;
  auto res = cleanup(format("(define append (lambda (l m)(if (null? l) m (cons(car l) (append(cdr l) m)))))", ops));
  TEST_EQ(cleanup(R"(
(define append
  (lambda (l m)
          (if (null? l)
              m
              (cons
                (car l)
                (append
                  (cdr l) m)))))
)"), res);

  ops.max_width = 20;
  res = cleanup(format("(define append (lambda (l m)(if (null? l) m (cons(car l) (append(cdr l) m)))))", ops));
  TEST_EQ(cleanup(R"(
(define append (lambda
                 (l m)
                 (if (null? l)
                     m
                     (cons
                       (car l)
                       (append
                         (cdr l) m)))))
)"), res);

  ops.max_width = 30;
  res = cleanup(format("(define append (lambda (l m)(if (null? l) m (cons(car l) (append(cdr l) m)))))", ops));
  TEST_EQ(cleanup(R"(
(define append (lambda (l m)
                       (if (null? l)
                           m (cons
                               (car l)
                               (append
                                 (cdr l) m)))))
)"), res);

  ops.max_width = 40;
  res = cleanup(format("(define append (lambda (l m)(if (null? l) m (cons(car l) (append(cdr l) m)))))", ops));
  TEST_EQ(cleanup(R"(
(define append (lambda (l m)
                       (if (null? l) m (cons
                                         (car l)
                                         (append
                                           (cdr l) m)))))
)"), res);

  ops.max_width = 50;
  res = cleanup(format("(define append (lambda (l m)(if (null? l) m (cons(car l) (append(cdr l) m)))))", ops));
  TEST_EQ(cleanup(R"(
(define append (lambda (l m)
                       (if (null? l) m (cons (car l)
                                             (append
                                               (cdr l) m)))))
)"), res);

  ops.max_width = 60;
  res = cleanup(format("(define append (lambda (l m)(if (null? l) m (cons(car l) (append(cdr l) m)))))", ops));
  TEST_EQ(cleanup(R"(
(define append (lambda (l m) (if (null? l) m (cons (car l)
                                                   (append (cdr l)
                                                           m)))))
)"), res);

  ops.max_width = 70;
  res = cleanup(format("(define append (lambda (l m)(if (null? l) m (cons(car l) (append(cdr l) m)))))", ops));
  TEST_EQ(cleanup(R"(
(define append (lambda (l m) (if (null? l) m (cons (car l) (append (cdr l)
                                                                   m)))))
)"), res);

  ops.max_width = 80;
  res = cleanup(format("(define append (lambda (l m)(if (null? l) m (cons(car l) (append(cdr l) m)))))", ops));
  TEST_EQ(cleanup(R"(
(define append (lambda (l m) (if (null? l) m (cons (car l) (append (cdr l) m)))))
)"), res);
  }

void format_test_7()
  {
  format_options ops;
  ops.max_width = 10;
  auto res = cleanup(format("(define newton lambda(guess function derivative epsilon) (define guess2 (- guess (/ (function guess) (derivative guess)))) (if (< (abs(- guess guess2)) epsilon) guess2 (newton guess2 function derivative epsilon)))", ops));
  TEST_EQ(cleanup(R"(
(define newton lambda
  (guess function derivative epsilon)
  (define guess2
    (- guess
      (/ (function guess)
         (derivative guess))))
  (if (< (abs
           (- guess guess2))
         epsilon)
      guess2
      (newton guess2 function derivative epsilon)))
)"), res);

  res = cleanup(format("(case (car '(c d)) [(a) 'a] [(b) 'b])", ops));
  TEST_EQ(cleanup(R"(
(case (car
        '(c d))
        [(a)
         'a]
        [(b)
         'b])
)"), res);

  res = cleanup(format(R"((string #\j #\a #\n #\n #\e #\m #\a #\n))", ops));
  TEST_EQ(cleanup(R"(
(string #\j #\a #\n #\n #\e #\m #\a #\n)
)"), res);

  res = cleanup(format("quote (\"Hello\" \"World\")", ops));
  TEST_EQ(cleanup(R"(
quote ("Hello" "World")
)"), res);

  res = cleanup(format("(letrec([f (lambda(i)  (when (<= i 100000) (let([x (list 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)]) (f(add1 i)))))]) (f 0) 100)", ops));
  TEST_EQ(cleanup(R"(
(letrec ([f (lambda
              (i)
              (when
                (<= i 100000)
                (let
                  ([x (list 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)])
                  (f (add1 i)))))])
        (f 0)
        100)
)"), res);
  }

void format_test_8()
  {
  format_options ops;
  ops.max_width = 10;
  auto res = cleanup(format("(let([f(lambda(a0 a1 . args) (vector a0 a1))]) (f 10 20 30 40 50 60 70 80 90 100)) ", ops));
  TEST_EQ(cleanup(R"(
(let ([f (lambda
           (a0 a1 . args)
           (vector a0 a1))])
     (f 10 20 30 40 50 60 70 80 90 100))
)"), res);
  }

SKIWI_END

void run_all_format_tests()
  {
  using namespace SKIWI;
  format_test_invalid_input();
  format_test_1();
  format_test_2();
  format_test_3();
  format_test_4();
  format_test_5();
  format_test_6();
  format_test_7();
  format_test_8();
  }