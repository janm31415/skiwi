///////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////

#include "conversion_tests.h"
#include "test_assert.h"

#include <asm/assembler.h>

#include <iomanip>
#include <iostream>
#include <fstream>

#include <libskiwi/alpha_conversion.h>
#include <libskiwi/assignable_var_conversion.h>
#include <libskiwi/closure_conversion.h>
#include <libskiwi/constant_propagation.h>
#include <libskiwi/constant_folding.h>
#include <libskiwi/cps_conversion.h>
#include <libskiwi/define_conversion.h>
#include <libskiwi/free_var_analysis.h>
#include <libskiwi/global_define_env.h>
#include <libskiwi/dump.h>
#include <libskiwi/linear_scan_index.h>
#include <libskiwi/linear_scan.h>
#include <libskiwi/simplify_to_core.h>
#include <libskiwi/single_begin_conversion.h>
#include <libskiwi/tail_call_analysis.h>
#include <libskiwi/tail_calls_check.h>
#include <libskiwi/quasiquote_conversion.h>
#include <libskiwi/parse.h>
#include <libskiwi/reader.h>
#include <libskiwi/tokenize.h>

SKIWI_BEGIN

namespace
  {
  std::string to_string(Program& prog)
    {
    std::stringstream str;
    dump(str, prog);
    return str.str();
    }

  void alpha_conversion()
    {
    auto tokens = tokenize("(let ([x 5][y 6]) x)");
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    uint64_t index = 0;
    std::shared_ptr < environment<alpha_conversion_data>> empty;
    alpha_conversion(prog, index, empty);
    TEST_ASSERT(std::get<Let>(prog.expressions.front()).bindings[0].first == "x_0");
    TEST_ASSERT(std::get<Let>(prog.expressions.front()).bindings[1].first == "y_1");
    TEST_ASSERT(std::get<Variable>(std::get<Begin>(std::get<Let>(prog.expressions.front()).body.front()).arguments.front()).name == "x_0");
    TEST_ASSERT(to_string(prog) == "( let ( [ x_0 5 ] [ y_1 6 ] ) ( begin x_0 ) ) ");
    }

  void simplify_to_core_conversion_and()
    {
    auto tokens = tokenize("(and)");
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    simplify_to_core_forms(prog);
    TEST_ASSERT(std::holds_alternative<True>(std::get<Literal>(prog.expressions.front())));

    tokens = tokenize("(and 3)");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    simplify_to_core_forms(prog);
    TEST_ASSERT(std::holds_alternative<Fixnum>(std::get<Literal>(prog.expressions.front())));
    TEST_ASSERT(std::get<Fixnum>(std::get<Literal>(prog.expressions.front())).value == 3);

    tokens = tokenize("(and 3 4)");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    simplify_to_core_forms(prog);
    TEST_ASSERT(std::holds_alternative<If>(prog.expressions.front()));
    TEST_ASSERT(std::get<Fixnum>(std::get<Literal>(std::get<If>(prog.expressions.front()).arguments[0])).value == 3);
    }

  void simplify_to_core_conversion_or()
    {
    auto tokens = tokenize("(or)");
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    simplify_to_core_forms(prog);
    TEST_ASSERT(std::holds_alternative<False>(std::get<Literal>(prog.expressions.front())));

    tokens = tokenize("(or 3)");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    simplify_to_core_forms(prog);
    TEST_ASSERT(std::holds_alternative<Fixnum>(std::get<Literal>(prog.expressions.front())));
    TEST_ASSERT(std::get<Fixnum>(std::get<Literal>(prog.expressions.front())).value == 3);

    tokens = tokenize("(or 3 4)");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    simplify_to_core_forms(prog);
    TEST_ASSERT(std::holds_alternative<Let>(prog.expressions.front()));
    TEST_ASSERT(to_string(prog) == "( let ( [ #%x 3 ] ) ( begin ( if #%x #%x 4 ) ) ) ");
    }

  void simplify_to_core_conversion_letrec()
    {
    auto tokens = tokenize("(letrec ([f (lambda () 5)]) (- 20 (f)))");
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    simplify_to_core_forms(prog);
    TEST_ASSERT(to_string(prog) == "( let ( [ f #undefined ] ) ( begin ( let ( [ #%t0 ( lambda ( ) ( begin 5 ) ) ] ) ( begin ( set! f #%t0 ) ) ) ( - 20 ( f ) ) ) ) ");

    tokens = tokenize("(letrec ([f (lambda (x y) (+ x y))] [g(lambda(x) (+ x 12))])(f 16 (f(g 0) (+ 1 (g 0)))))");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    simplify_to_core_forms(prog);
    TEST_ASSERT(to_string(prog) == "( let ( [ f #undefined ] [ g #undefined ] ) ( begin ( let ( [ #%t0 ( lambda ( x y ) ( begin ( + x y ) ) ) ] [ #%t1 ( lambda ( x ) ( begin ( + x 12 ) ) ) ] ) ( begin ( set! f #%t0 ) ( set! g #%t1 ) ) ) ( f 16 ( f ( g 0 ) ( + 1 ( g 0 ) ) ) ) ) ) ");
    }

  void convert_define()
    {
    auto tokens = tokenize("(let ([x 5]) (define foo (lambda (y) (bar x y))) (define bar (lambda (a b) (+ (* a b) a))) (foo (+ x 3)))");
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    define_conversion(prog);

    TEST_ASSERT(to_string(prog) == "( let ( [ x 5 ] ) ( begin ( letrec ( [ foo ( lambda ( y ) ( begin ( bar x y ) ) ) ] [ bar ( lambda ( a b ) ( begin ( + ( * a b ) a ) ) ) ] ) ( begin ( foo ( + x 3 ) ) ) ) ) ) ");

    tokens = tokenize("(define x 5)");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    define_conversion(prog);
    TEST_ASSERT(to_string(prog) == "( set! x 5 ) ");
    uint64_t alpha_conversion_index = 0;
    std::shared_ptr < environment<alpha_conversion_data>> empty;
    alpha_conversion(prog, alpha_conversion_index, empty);
    TEST_ASSERT(to_string(prog) == "( set! x_0 5 ) ");

    tokens = tokenize("(define y 5) (let ([z y]) ( + z 1) )");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    define_conversion(prog);
    alpha_conversion_index = 0;
    alpha_conversion(prog, alpha_conversion_index, empty);
    TEST_ASSERT(to_string(prog) == "( set! y_0 5 ) ( let ( [ z_1 y_0 ] ) ( begin ( + z_1 1 ) ) ) ");
    }

  void single_begin_conv()
    {
    auto tokens = tokenize("(define x 5) (define y 6)");
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    single_begin_conversion(prog);
    TEST_ASSERT(to_string(prog) == "( begin ( define x 5 ) ( define y 6 ) ) ");
    define_conversion(prog);
    single_begin_conversion(prog);
    TEST_ASSERT(to_string(prog) == "( begin ( set! x 5 ) ( set! y 6 ) ) ");    
    }

  void dump_conversion()
    {
    std::string script = "( + 15 7.2 ) ";
    auto tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    TEST_ASSERT(to_string(prog) == script);

    script = "( test 15 7.2 ) ";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    TEST_ASSERT(to_string(prog) == script);

    script = "( if ( > x 3 ) 7 3 ) ";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    TEST_ASSERT(to_string(prog) == script);

    script = "( set! x ( begin 3 2 ) ) ";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    TEST_ASSERT(to_string(prog) == script);

    script = "( lambda ( x y ) ( begin ( - ( + x 3 ) y ) ) ) ";
    tokens = tokenize("(lambda (x y) (- (+ x 3 ) y) )");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    TEST_ASSERT(to_string(prog) == script);

    script = "( let ( [ x ( + 1 2 ) ] [ y 6 ] ) ( begin y ) ) ";
    tokens = tokenize("(let ([x (+ 1 2)][y 6]) y)");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    TEST_ASSERT(to_string(prog) == script);

    script = "( let ( [ f ( lambda ( ) ( begin 5 ) ) ] ) ( begin ( + ( f ) 6 ) ) ) ";
    tokens = tokenize("(let ([f (lambda () 5)]) (+ (f) 6))");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    TEST_ASSERT(to_string(prog) == script);
    }

  void closure_conversion()
    {
    context ctxt = create_context(1024, 1024, 1024, 1024);
    compiler_options ops;
    ops.parallel = false;
    auto tokens = tokenize("(let ([x 5]) (lambda (y) (lambda () (+ x y))))");
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    environment_map env = std::make_shared<environment<environment_entry>>(nullptr);
    uint64_t alpha_conversion_index = 0;
    repl_data rd;
    std::shared_ptr < environment<alpha_conversion_data>> empty;
    alpha_conversion(prog, alpha_conversion_index, empty);
    global_define_environment_allocation(prog, env, rd, ctxt);
    free_variable_analysis(prog, env);
    closure_conversion(prog, ops);
    TEST_ASSERT(to_string(prog) == "( let ( [ x_0 5 ] ) ( begin ( closure ( lambda ( #%self1 y_1 ) ( begin ( closure ( lambda ( #%self0 ) ( begin ( + ( closure-ref #%self0 1 ) ( closure-ref #%self0 2 ) ) ) ) ( closure-ref #%self1 1 ) y_1 ) ) ) x_0 ) ) ) ");
    tokens = tokenize("(lambda (x y z) (let ([ f (lambda (a b) (+ (* a x) (8 b y)))]) (- (f 1 2) (f 3 4))))");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    alpha_conversion_index = 0;
    alpha_conversion(prog, alpha_conversion_index, empty);
    global_define_environment_allocation(prog, env, rd, ctxt);
    free_variable_analysis(prog, env);
    closure_conversion(prog, ops);
    TEST_ASSERT(to_string(prog) == "( closure ( lambda ( #%self1 x_0 y_1 z_2 ) ( begin ( let ( [ f_5 ( closure ( lambda ( #%self0 a_3 b_4 ) ( begin ( + ( * a_3 ( closure-ref #%self0 1 ) ) ( 8 b_4 ( closure-ref #%self0 2 ) ) ) ) ) x_0 y_1 ) ] ) ( begin ( - ( f_5 1 2 ) ( f_5 3 4 ) ) ) ) ) ) ) ");
    destroy_context(ctxt);
    }

  void assignable_var_conversion()
    {
    compiler_options ops;
    ops.parallel = false;
    auto tokens = tokenize("(let([x 12]) (let([y(let([x #f]) (set! x 14))])  x))");
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    uint64_t alpha_conversion_index = 0;
    std::shared_ptr < environment<alpha_conversion_data>> empty;
    alpha_conversion(prog, alpha_conversion_index, empty);
    assignable_variable_conversion(prog, ops);
    TEST_ASSERT(to_string(prog) == "( let ( [ x_0 12 ] ) ( begin ( let ( [ y_2 ( let ( [ #%x_1 #f ] ) ( begin ( let ( [ x_1 ( vector #%x_1 ) ] ) ( begin ( vector-set! x_1 0 14 ) ) ) ) ) ] ) ( begin x_0 ) ) ) ) ");
    //std::cout << to_string(prog) << std::endl;

    tokens = tokenize("(let ([f (lambda (c) (cons (lambda (v) (set! c v)) (lambda () c)))]) (let ([p (f 0)]) ( (car p) 12) ((cdr p))))");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    alpha_conversion(prog, alpha_conversion_index, empty);
    assignable_variable_conversion(prog, ops);
    //std::cout << to_string(prog) << std::endl;
    TEST_ASSERT(to_string(prog) == "( let ( [ f_5 ( lambda ( #%c_3 ) ( begin ( let ( [ c_3 ( vector #%c_3 ) ] ) ( begin ( cons ( lambda ( v_4 ) ( begin ( vector-set! c_3 0 v_4 ) ) ) ( lambda ( ) ( begin ( vector-ref c_3 0 ) ) ) ) ) ) ) ) ] ) ( begin ( let ( [ p_6 ( f_5 0 ) ] ) ( begin ( ( car p_6 ) 12 ) ( ( cdr p_6 ) ) ) ) ) ) ");

    tokens = tokenize("(let ([x 3]) (set! x 5))");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    alpha_conversion(prog, alpha_conversion_index, empty);
    assignable_variable_conversion(prog, ops);
    //std::cout << to_string(prog) << std::endl;
    TEST_ASSERT(to_string(prog) == "( let ( [ #%x_7 3 ] ) ( begin ( let ( [ x_7 ( vector #%x_7 ) ] ) ( begin ( vector-set! x_7 0 5 ) ) ) ) ) ");
    }

  void constant_propagation_tests()
    {
    auto tokens = tokenize("(let ((x 5)) (let ((y x)) (+ y y)))");
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    constant_propagation(prog);
    TEST_ASSERT(to_string(prog) == "( + 5 5 ) ");

    tokens = tokenize("(if (and #f (f 2)) 123 (g 3))");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    simplify_to_core_forms(prog);
    constant_folding(prog);
    TEST_ASSERT(to_string(prog) == "( g 3 ) ");
    }

  void tail_calls_analysis()
    {
    auto tokens = tokenize("(define fact (lambda (x) (if (= x 0) 1 ( * x ( fact (- x 1))))))");
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    tail_call_analysis(prog);
    TEST_ASSERT(!only_tail_calls(prog));

    tokens = tokenize("(define fact (lambda (x) (define fact-tail (lambda (x accum) (if (= x 0) accum (fact-tail (- x 1) (* x accum))))) (fact-tail x 1) ) )");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    tail_call_analysis(prog);
    TEST_ASSERT(only_tail_calls(prog));
    }

  void cps_conversion()
    {
    compiler_options ops;
    ops.parallel = false;
   //environment_map empty = std::make_shared<environment<environment_entry>>(nullptr);
    auto tokens = tokenize("(15)");
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    cps_conversion(prog, ops);
    TEST_ASSERT(to_string(prog) == "( let ( [ #%k0 15 ] ) ( begin ( halt #%k0 ) ) ) ");
    tail_call_analysis(prog);
    TEST_ASSERT(only_tail_calls(prog));

    tokens = tokenize("(+ 22 (- x 3) x)");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    cps_conversion(prog, ops);
    TEST_ASSERT(to_string(prog) == "( let ( [ #%k0 ( + 22 ( - x 3 ) x ) ] ) ( begin ( halt #%k0 ) ) ) ");
    tail_call_analysis(prog);
    TEST_ASSERT(only_tail_calls(prog));

    tokens = tokenize("(+ 22 (f x) 33 (g y))");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    tail_call_analysis(prog);
    TEST_ASSERT(!only_tail_calls(prog));
    cps_conversion(prog, ops);
    TEST_ASSERT(to_string(prog) == "( let ( [ #%k10 f ] ) ( begin ( #%k10 ( lambda ( #%k2 ) ( begin ( let ( [ #%k6 g ] ) ( begin ( #%k6 ( lambda ( #%k4 ) ( begin ( let ( [ #%k0 ( + 22 #%k2 33 #%k4 ) ] ) ( begin ( halt #%k0 ) ) ) ) ) y ) ) ) ) ) x ) ) ) ");
    //std::cout << to_string(prog) << "\n\n";
    tail_call_analysis(prog);
    TEST_ASSERT(only_tail_calls(prog));

    tokens = tokenize("(set! x 5)");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    cps_conversion(prog, ops);
    TEST_ASSERT(to_string(prog) == "( let ( [ #%k0 ( set! x 5 ) ] ) ( begin ( halt #%k0 ) ) ) ");
    tail_call_analysis(prog);
    TEST_ASSERT(only_tail_calls(prog));

    tokens = tokenize("(set! x (f))");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    cps_conversion(prog, ops);
    TEST_ASSERT(to_string(prog) == "( let ( [ #%k2 f ] ) ( begin ( #%k2 ( lambda ( #%k1 ) ( begin ( let ( [ #%k0 ( set! x #%k1 ) ] ) ( begin ( halt #%k0 ) ) ) ) ) ) ) ) ");
    //std::cout << to_string(prog) << "\n\n";
    tail_call_analysis(prog);
    TEST_ASSERT(only_tail_calls(prog));

    tokens = tokenize("(if #t 2 3)");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    cps_conversion(prog, ops);
    TEST_ASSERT(to_string(prog) == "( if #t ( let ( [ #%k0 2 ] ) ( begin ( halt #%k0 ) ) ) ( let ( [ #%k0 3 ] ) ( begin ( halt #%k0 ) ) ) ) ");
    tail_call_analysis(prog);
    TEST_ASSERT(only_tail_calls(prog));

    tokens = tokenize("(if (f) 2 3)");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    tail_call_analysis(prog);
    TEST_ASSERT(!only_tail_calls(prog));
    cps_conversion(prog, ops);
    TEST_ASSERT(to_string(prog) == "( let ( [ #%k2 f ] ) ( begin ( #%k2 ( lambda ( #%k1 ) ( begin ( if #%k1 ( let ( [ #%k0 2 ] ) ( begin ( halt #%k0 ) ) ) ( let ( [ #%k0 3 ] ) ( begin ( halt #%k0 ) ) ) ) ) ) ) ) ) ");
    tail_call_analysis(prog);
    TEST_ASSERT(only_tail_calls(prog));

    tokens = tokenize("(begin 1 2)");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    cps_conversion(prog, ops);
    //std::cout << to_string(prog) << "\n\n";
    TEST_ASSERT(to_string(prog) == "( begin ( let ( [ #%k0 1 ] ) ( begin ( halt #%k0 ) ) ) ( let ( [ #%k0 2 ] ) ( begin ( halt #%k0 ) ) ) ) ");
    tail_call_analysis(prog);
    TEST_ASSERT(only_tail_calls(prog));

    tokens = tokenize("(begin (a) (b))");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    tail_call_analysis(prog);
    //TEST_ASSERT(!only_tail_calls(prog));
    cps_conversion(prog, ops);
    TEST_ASSERT(to_string(prog) == "( begin ( let ( [ #%k1 a ] ) ( begin ( #%k1 ( lambda ( #%k0 ) ( begin ( halt #%k0 ) ) ) ) ) ) ( let ( [ #%k1 b ] ) ( begin ( #%k1 ( lambda ( #%k0 ) ( begin ( halt #%k0 ) ) ) ) ) ) ) ");
    //std::cout << to_string(prog) << "\n\n";
    tail_call_analysis(prog);
    TEST_ASSERT(only_tail_calls(prog));

    tokens = tokenize("(begin (a) 2)");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    tail_call_analysis(prog);
    //TEST_ASSERT(!only_tail_calls(prog));
    cps_conversion(prog, ops);
    TEST_ASSERT(to_string(prog) == "( begin ( let ( [ #%k1 a ] ) ( begin ( #%k1 ( lambda ( #%k0 ) ( begin ( halt #%k0 ) ) ) ) ) ) ( let ( [ #%k0 2 ] ) ( begin ( halt #%k0 ) ) ) ) ");
    //std::cout << to_string(prog) << "\n\n";
    tail_call_analysis(prog);
    TEST_ASSERT(only_tail_calls(prog));

    tokens = tokenize("(begin 4 3 (a) 2)");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    tail_call_analysis(prog);
    //TEST_ASSERT(!only_tail_calls(prog));
    cps_conversion(prog, ops);
    TEST_ASSERT(to_string(prog) == "( begin ( let ( [ #%k0 4 ] ) ( begin ( halt #%k0 ) ) ) ( let ( [ #%k0 3 ] ) ( begin ( halt #%k0 ) ) ) ( let ( [ #%k1 a ] ) ( begin ( #%k1 ( lambda ( #%k0 ) ( begin ( halt #%k0 ) ) ) ) ) ) ( let ( [ #%k0 2 ] ) ( begin ( halt #%k0 ) ) ) ) ");
    //std::cout << to_string(prog) << "\n\n";
    tail_call_analysis(prog);
    TEST_ASSERT(only_tail_calls(prog));

    tokens = tokenize("(begin 1 2 (a))");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    cps_conversion(prog, ops);
    TEST_ASSERT(to_string(prog) == "( begin ( let ( [ #%k0 1 ] ) ( begin ( halt #%k0 ) ) ) ( let ( [ #%k0 2 ] ) ( begin ( halt #%k0 ) ) ) ( let ( [ #%k1 a ] ) ( begin ( #%k1 ( lambda ( #%k0 ) ( begin ( halt #%k0 ) ) ) ) ) ) ) ");
    //std::cout << to_string(prog) << "\n\n";
    tail_call_analysis(prog);
    TEST_ASSERT(only_tail_calls(prog));

    tokens = tokenize("(+ 1 2 3)");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    cps_conversion(prog, ops);
    TEST_ASSERT(to_string(prog) == "( let ( [ #%k0 ( + 1 2 3 ) ] ) ( begin ( halt #%k0 ) ) ) ");
    tail_call_analysis(prog);
    TEST_ASSERT(only_tail_calls(prog));

    tokens = tokenize("(+ 1 (f) 3)");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    tail_call_analysis(prog);
    TEST_ASSERT(!only_tail_calls(prog));
    cps_conversion(prog, ops);
    TEST_ASSERT(to_string(prog) == "( let ( [ #%k5 f ] ) ( begin ( #%k5 ( lambda ( #%k2 ) ( begin ( let ( [ #%k0 ( + 1 #%k2 3 ) ] ) ( begin ( halt #%k0 ) ) ) ) ) ) ) ) ");
    tail_call_analysis(prog);
    TEST_ASSERT(only_tail_calls(prog));

    tokens = tokenize("(lambda ( a b ) ( + a b))");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    cps_conversion(prog, ops);
    TEST_ASSERT(to_string(prog) == "( let ( [ #%k0 ( lambda ( #%k1 a b ) ( begin ( #%k1 ( + a b ) ) ) ) ] ) ( begin ( halt #%k0 ) ) ) ");
    tail_call_analysis(prog);
    TEST_ASSERT(only_tail_calls(prog));


    tokens = tokenize("(f)");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    cps_conversion(prog, ops);
    TEST_ASSERT(to_string(prog) == "( let ( [ #%k1 f ] ) ( begin ( #%k1 ( lambda ( #%k0 ) ( begin ( halt #%k0 ) ) ) ) ) ) ");
    tail_call_analysis(prog);
    TEST_ASSERT(only_tail_calls(prog));


    tokens = tokenize("(let ([square (lambda (x) (* x x))]) (write (+ (square 10) 1)))");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    tail_call_analysis(prog);
    TEST_ASSERT(!only_tail_calls(prog));
    cps_conversion(prog, ops);
    TEST_ASSERT(to_string(prog) == "( let ( [ square ( lambda ( #%k12 x ) ( begin ( #%k12 ( * x x ) ) ) ) ] ) ( begin ( let ( [ #%k1 write ] ) ( begin ( let ( [ #%k7 square ] ) ( begin ( #%k7 ( lambda ( #%k4 ) ( begin ( let ( [ #%k2 ( + #%k4 1 ) ] ) ( begin ( #%k1 ( lambda ( #%k0 ) ( begin ( halt #%k0 ) ) ) #%k2 ) ) ) ) ) 10 ) ) ) ) ) ) ) ");
    //std::cout << to_string(prog) << "\n\n";
    tail_call_analysis(prog);
    TEST_ASSERT(only_tail_calls(prog));

    tokens = tokenize("(let ([x 3] [y 4]) ( + x y) )");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    cps_conversion(prog, ops);
    TEST_ASSERT(to_string(prog) == "( let ( [ x 3 ] [ y 4 ] ) ( begin ( let ( [ #%k0 ( + x y ) ] ) ( begin ( halt #%k0 ) ) ) ) ) ");
    tail_call_analysis(prog);
    TEST_ASSERT(only_tail_calls(prog));

    tokens = tokenize("(let ([x (f)] [y 4]) ( + x y) )");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    tail_call_analysis(prog);
    TEST_ASSERT(!only_tail_calls(prog));
    cps_conversion(prog, ops);
    TEST_ASSERT(to_string(prog) == "( let ( [ y 4 ] ) ( begin ( let ( [ #%k2 f ] ) ( begin ( #%k2 ( lambda ( x ) ( begin ( let ( [ #%k0 ( + x y ) ] ) ( begin ( halt #%k0 ) ) ) ) ) ) ) ) ) ) ");
    //std::cout << to_string(prog) << "\n\n";
    tail_call_analysis(prog);
    TEST_ASSERT(only_tail_calls(prog));

    tokens = tokenize("(let ([x (f)] [y (g)]) ( + x y) )");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    tail_call_analysis(prog);
    TEST_ASSERT(!only_tail_calls(prog));
    cps_conversion(prog, ops);
    TEST_ASSERT(to_string(prog) == "( let ( [ #%k5 f ] ) ( begin ( #%k5 ( lambda ( x ) ( begin ( let ( [ #%k2 g ] ) ( begin ( #%k2 ( lambda ( y ) ( begin ( let ( [ #%k0 ( + x y ) ] ) ( begin ( halt #%k0 ) ) ) ) ) ) ) ) ) ) ) ) ) ");
    //std::cout << to_string(prog) << "\n\n";
    tail_call_analysis(prog);
    TEST_ASSERT(only_tail_calls(prog));
    }

  void cps_conversion_2()
    {
    compiler_options ops;
    ops.parallel = false;
    //environment_map empty = std::make_shared<environment<environment_entry>>(nullptr);
    std::string script = R"(
(define square
  (lambda (x)
    (* x x)))

(+ (square 5) 1)
)";
    auto tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    define_conversion(prog);
    single_begin_conversion(prog);
    simplify_to_core_forms(prog);
    tail_call_analysis(prog);
    TEST_ASSERT(!only_tail_calls(prog));
    cps_conversion(prog, ops);
    TEST_ASSERT(to_string(prog) == "( begin ( let ( [ #%k1 ( lambda ( #%k2 x ) ( begin ( #%k2 ( * x x ) ) ) ) ] ) ( begin ( let ( [ #%k0 ( set! square #%k1 ) ] ) ( begin ( halt #%k0 ) ) ) ) ) ( let ( [ #%k4 square ] ) ( begin ( #%k4 ( lambda ( #%k1 ) ( begin ( let ( [ #%k0 ( + #%k1 1 ) ] ) ( begin ( halt #%k0 ) ) ) ) ) 5 ) ) ) ) ");
    //std::cout << to_string(prog) << "\n\n";
    tail_call_analysis(prog);
    TEST_ASSERT(only_tail_calls(prog));
    }

  void linear_scan_tests_naive()
    {
    compiler_options ops;
    ops.parallel = false;
    auto tokens = tokenize("(let ([x 5]) x)");
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    compute_linear_scan_index(prog);
    linear_scan(prog, lsa_naive, ops);
    Let l = std::get<Let>(prog.expressions.front());
    TEST_EQ(2, l.live_ranges.front().first);
    TEST_EQ(4, l.live_ranges.front().last);

    tokens = tokenize("(let ([x 5][y 6]) x)");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    compute_linear_scan_index(prog);
    linear_scan(prog, lsa_naive, ops);
    l = std::get<Let>(prog.expressions.front());
    TEST_EQ(2, l.live_ranges.front().first);
    TEST_EQ(5, l.live_ranges.front().last);
    TEST_EQ(3, l.live_ranges.back().first);
    TEST_EQ(5, l.live_ranges.back().last);
    }

  void bug1()
    {
    context ctxt = create_context(1024, 1024, 1024, 1024);
    compiler_options ops;
    ops.parallel = false;
    //environment_map empty_env = std::make_shared<environment<environment_entry>>(nullptr);
    std::string script = "(let ([x 5]) (x))";
    auto tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    define_conversion(prog);
    single_begin_conversion(prog);
    simplify_to_core_forms(prog);
    uint64_t alpha_conversion_index = 0;
    std::shared_ptr < environment<alpha_conversion_data>> empty;
    alpha_conversion(prog, alpha_conversion_index, empty);
    cps_conversion(prog, ops);
    environment_map env = std::make_shared<environment<environment_entry>>(nullptr);
    repl_data rd;
    global_define_environment_allocation(prog, env, rd, ctxt);
    free_variable_analysis(prog, env);
    assignable_variable_conversion(prog, ops);
    closure_conversion(prog, ops);
    tail_call_analysis(prog);
    TEST_ASSERT(only_tail_calls(prog));
    destroy_context(ctxt);
    }

  void quasiquote_conversion_tests()
    {
    std::string script = R"(
`(1 2)
)";
    auto tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( quote (1 2) ) ", to_string(prog));

    script = "`let";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( quote let ) ", to_string(prog));
    /*
    script = "`(,@ a)";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    bool catched = false;
    try
      {
      quasiquote_conversion(prog);
      }
    catch (std::logic_error e)
      {
      catched = true;
      TEST_EQ("error: Bad syntax: ,@", std::string(e.what()));
      }
    TEST_ASSERT(catched);
    */
    script = "`( 1 ,(+ 2 3))";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    //TEST_EQ("( append ( quote (1) ) ( append ( list ( + 2 3 ) ) ( quote () ) ) ) ", to_string(prog));
    TEST_EQ("( cons ( quote 1 ) ( cons ( + 2 3 ) ( quote () ) ) ) ", to_string(prog));

    script = "(quasiquote ( 1 ,(+ 2 3)))";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( cons ( quote 1 ) ( cons ( + 2 3 ) ( quote () ) ) ) ", to_string(prog));

    script = "`( ,(+ 2 3) 1)";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( cons ( + 2 3 ) ( quote (1) ) ) ", to_string(prog));

    script = "(quasiquote( (unquote (+ 2 3)) 1))";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( cons ( + 2 3 ) ( quote (1) ) ) ", to_string(prog));

    script = "`( 1 2 ,x )";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( cons ( quote 1 ) ( cons ( quote 2 ) ( cons x ( quote () ) ) ) ) ", to_string(prog));

    script = "(quasiquote ( 1 2 (unquote x) ))";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( cons ( quote 1 ) ( cons ( quote 2 ) ( cons x ( quote () ) ) ) ) ", to_string(prog));

    script = "`x";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( quote x ) ", to_string(prog));


    script = "(quasiquote x)";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( quote x ) ", to_string(prog));

    script = "`,x";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("x ", to_string(prog));

    script = "(quasiquote (unquote x))";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("x ", to_string(prog));

    script = "`( (,(+ 1 2) 3) 5 )";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( cons ( cons ( + 1 2 ) ( quote (3) ) ) ( quote (5) ) ) ", to_string(prog));

    script = "(quasiquote ( ((unquote (+ 1 2)) 3) 5 ) )";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( cons ( cons ( + 1 2 ) ( quote (3) ) ) ( quote (5) ) ) ", to_string(prog));

    script = "`( (5 4 ,(+ 1 2) 3) 5 )";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( cons ( cons ( quote 5 ) ( cons ( quote 4 ) ( cons ( + 1 2 ) ( quote (3) ) ) ) ) ( quote (5) ) ) ", to_string(prog));


    script = "(quasiquote ( (5 4 (unquote (+ 1 2)) 3) 5 ))";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( cons ( cons ( quote 5 ) ( cons ( quote 4 ) ( cons ( + 1 2 ) ( quote (3) ) ) ) ) ( quote (5) ) ) ", to_string(prog));

    script = "`(,@(cdr '(c)) )";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( cdr ( quote (c) ) ) ", to_string(prog));

    script = "(quasiquote ( (unquote-splicing (cdr '(c))) ))";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( cdr ( quote (c) ) ) ", to_string(prog));

    script = "`((foo ,(- 10 3)) ,@(cdr '(c)) . ,(car '(cons)))";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( cons ( cons ( quote foo ) ( cons ( - 10 3 ) ( quote () ) ) ) ( append ( cdr ( quote (c) ) ) ( car ( quote (cons) ) ) ) ) ", to_string(prog));

    script = "(quasiquote ((foo (unquote (- 10 3))) (unquote-splicing (cdr '(c))) . (unquote (car '(cons)))))";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( cons ( cons ( quote foo ) ( cons ( - 10 3 ) ( quote () ) ) ) ( append ( cdr ( quote (c) ) ) ( car ( quote (cons) ) ) ) ) ", to_string(prog));

    script = "(let ((name 'a)) `(list ,name ',name))";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( let ( [ name ( quote a ) ] ) ( begin ( cons ( quote list ) ( cons name ( cons ( cons ( quote quote ) ( cons name ( quote () ) ) ) ( quote () ) ) ) ) ) ) ", to_string(prog));

    script = "(let ((name (quote a))) (quasiquote (list (unquote name) (quote (unquote name)))))";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( let ( [ name ( quote a ) ] ) ( begin ( cons ( quote list ) ( cons name ( cons ( cons ( quote quote ) ( cons name ( quote () ) ) ) ( quote () ) ) ) ) ) ) ", to_string(prog));

    script = "`(a ,(+ 1 2) ,@(map abs '(4 -5 6)) b)";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( cons ( quote a ) ( cons ( + 1 2 ) ( append ( map abs ( quote (4 -5 6) ) ) ( quote (b) ) ) ) ) ", to_string(prog));

    script = "(quasiquote (a (unquote (+ 1 2)) (unquote-splicing (map abs (quote (4 -5 6)))) b))";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( cons ( quote a ) ( cons ( + 1 2 ) ( append ( map abs ( quote (4 -5 6) ) ) ( quote (b) ) ) ) ) ", to_string(prog));

    script = "`#(10 5 ,(sqrt 4) ,@(map sqrt '(16 9)) 8)";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( list->vector ( cons ( quote 10 ) ( cons ( quote 5 ) ( cons ( sqrt 4 ) ( append ( map sqrt ( quote (16 9) ) ) ( quote (8) ) ) ) ) ) ) ", to_string(prog));

    script = "(quasiquote #(10 5 (unquote (sqrt 4)) (unquote-splicing (map sqrt (quote (16 9)))) 8))";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( list->vector ( cons ( quote 10 ) ( cons ( quote 5 ) ( cons ( sqrt 4 ) ( append ( map sqrt ( quote (16 9) ) ) ( quote (8) ) ) ) ) ) ) ", to_string(prog));

    script = "`(1 2 #(10 ,(+ 2 6)) 5)";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( cons ( quote 1 ) ( cons ( quote 2 ) ( cons ( list->vector ( cons ( quote 10 ) ( cons ( + 2 6 ) ( quote () ) ) ) ) ( quote (5) ) ) ) ) ", to_string(prog));

    script = "(quasiquote (1 2 #(10 (unquote (+ 2 6))) 5))";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( cons ( quote 1 ) ( cons ( quote 2 ) ( cons ( list->vector ( cons ( quote 10 ) ( cons ( + 2 6 ) ( quote () ) ) ) ) ( quote (5) ) ) ) ) ", to_string(prog));

    script = "`( `(,(+ 1 2 ,(+ 3 4) ) ) )";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( cons ( cons ( quote quasiquote ) ( cons ( cons ( cons ( quote unquote ) ( cons ( cons ( quote + ) ( cons ( quote 1 ) ( cons ( quote 2 ) ( cons ( + 3 4 ) ( quote () ) ) ) ) ) ( quote () ) ) ) ( quote () ) ) ( quote () ) ) ) ( quote () ) ) ", to_string(prog));

    script = "`(,(+ 1 2 7) )";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( cons ( + 1 2 7 ) ( quote () ) ) ", to_string(prog));

    script = "`(a `(b ,(+ 1 2) ,(foo ,(+ 1 3) d) e) f)";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    quasiquote_conversion(prog);
    TEST_EQ("( cons ( quote a ) ( cons ( cons ( quote quasiquote ) ( cons ( cons ( quote b ) ( cons ( cons ( quote unquote ) ( cons ( quote (+ 1 2) ) ( quote () ) ) ) ( cons ( cons ( quote unquote ) ( cons ( cons ( quote foo ) ( cons ( + 1 3 ) ( quote (d) ) ) ) ( quote () ) ) ) ( quote (e) ) ) ) ) ( quote () ) ) ) ( quote (f) ) ) ) ", to_string(prog));
    }

  }

SKIWI_END

void run_all_conversion_tests()
  {
  using namespace SKIWI;
  alpha_conversion();
  simplify_to_core_conversion_and();
  simplify_to_core_conversion_or();
  simplify_to_core_conversion_letrec();
  convert_define();
  single_begin_conv();
  dump_conversion();
  closure_conversion();
  assignable_var_conversion();
  tail_calls_analysis();
  cps_conversion();
  cps_conversion_2();
  linear_scan_tests_naive();
  bug1();
  quasiquote_conversion_tests();
  constant_propagation_tests();
  }