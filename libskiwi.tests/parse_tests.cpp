///////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////

#include "parse_tests.h"
#include "test_assert.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <algorithm>

#include <libskiwi/parse.h>
#include <libskiwi/tokenize.h>
#include <libskiwi/dump.h>

SKIWI_BEGIN

namespace
  {

  void parse_fixnum()
    {
    auto tokens = tokenize("5");
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    TEST_EQ(int(1), (int)prog.expressions.size());
    TEST_ASSERT(std::holds_alternative<Literal>(prog.expressions.front()));
    TEST_ASSERT(std::holds_alternative<Fixnum>(std::get<Literal>(prog.expressions.front())));
    TEST_EQ(5, std::get<Fixnum>(std::get<Literal>(prog.expressions.front())).value);
    TEST_EQ(1, std::get<Fixnum>(std::get<Literal>(prog.expressions.front())).line_nr);
    TEST_EQ(1, std::get<Fixnum>(std::get<Literal>(prog.expressions.front())).column_nr);

    tokens = tokenize("-5555");
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    TEST_EQ(int(1), (int)prog.expressions.size());
    TEST_ASSERT(std::holds_alternative<Literal>(prog.expressions.front()));
    TEST_ASSERT(std::holds_alternative<Fixnum>(std::get<Literal>(prog.expressions.front())));
    TEST_EQ(-5555, std::get<Fixnum>(std::get<Literal>(prog.expressions.front())).value);
    TEST_EQ(1, std::get<Fixnum>(std::get<Literal>(prog.expressions.front())).line_nr);
    TEST_EQ(1, std::get<Fixnum>(std::get<Literal>(prog.expressions.front())).column_nr);
    }

  void parse_flonum()
    {
    auto tokens = tokenize("3.14");
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    TEST_EQ(int(1), (int)prog.expressions.size());
    TEST_ASSERT(std::holds_alternative<Literal>(prog.expressions.front()));
    TEST_ASSERT(std::holds_alternative<Flonum>(std::get<Literal>(prog.expressions.front())));
    TEST_EQ(3.14, std::get<Flonum>(std::get<Literal>(prog.expressions.front())).value);
    TEST_EQ(1, std::get<Flonum>(std::get<Literal>(prog.expressions.front())).line_nr);
    TEST_EQ(1, std::get<Flonum>(std::get<Literal>(prog.expressions.front())).column_nr);
    }


  void parse_nil()
    {
    auto tokens = tokenize("()");
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    TEST_EQ(int(1), (int)prog.expressions.size());
    TEST_ASSERT(std::holds_alternative<Literal>(prog.expressions.front()));
    TEST_ASSERT(std::holds_alternative<Nil>(std::get<Literal>(prog.expressions.front())));
    TEST_EQ(1, std::get<Nil>(std::get<Literal>(prog.expressions.front())).line_nr);
    TEST_EQ(1, std::get<Nil>(std::get<Literal>(prog.expressions.front())).column_nr);
    }

  void parse_true()
    {
    auto tokens = tokenize("#t");
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    TEST_EQ(int(1), (int)prog.expressions.size());
    TEST_ASSERT(std::holds_alternative<Literal>(prog.expressions.front()));
    TEST_ASSERT(std::holds_alternative<True>(std::get<Literal>(prog.expressions.front())));
    TEST_EQ(1, std::get<True>(std::get<Literal>(prog.expressions.front())).line_nr);
    TEST_EQ(1, std::get<True>(std::get<Literal>(prog.expressions.front())).column_nr);
    }

  void parse_false()
    {
    auto tokens = tokenize("#f");
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    TEST_EQ(int(1), (int)prog.expressions.size());
    TEST_ASSERT(std::holds_alternative<Literal>(prog.expressions.front()));
    TEST_ASSERT(std::holds_alternative<False>(std::get<Literal>(prog.expressions.front())));
    TEST_EQ(1, std::get<False>(std::get<Literal>(prog.expressions.front())).line_nr);
    TEST_EQ(1, std::get<False>(std::get<Literal>(prog.expressions.front())).column_nr);
    }

  void parse_string()
    {
    auto tokens = tokenize("\"test\"");
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    TEST_EQ(int(1), (int)prog.expressions.size());
    TEST_ASSERT(std::holds_alternative<Literal>(prog.expressions.front()));
    TEST_ASSERT(std::holds_alternative<String>(std::get<Literal>(prog.expressions.front())));
    TEST_ASSERT(std::get<String>(std::get<Literal>(prog.expressions.front())).value == "test");
    TEST_EQ(1, std::get<String>(std::get<Literal>(prog.expressions.front())).line_nr);
    TEST_EQ(1, std::get<String>(std::get<Literal>(prog.expressions.front())).column_nr);
    }

  void parse_primitive_call()
    {
    auto tokens = tokenize("(+ 1 2)");
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    TEST_EQ(int(1), (int)prog.expressions.size());
    TEST_ASSERT(std::holds_alternative<PrimitiveCall>(prog.expressions.front()));
    TEST_ASSERT(std::get<PrimitiveCall>(prog.expressions.front()).primitive_name == "+");
    TEST_ASSERT(std::get<PrimitiveCall>(prog.expressions.front()).arguments.size() == 2);
    TEST_EQ(1, std::get<PrimitiveCall>(prog.expressions.front()).line_nr);
    TEST_EQ(2, std::get<PrimitiveCall>(prog.expressions.front()).column_nr);
    }

  void parse_let()
    {
    std::string script = R"( (let ([x 5]) x) )";
    auto tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    TEST_EQ(int(1), (int)prog.expressions.size());
    TEST_ASSERT(std::holds_alternative<Let>(prog.expressions.front()));

    script = R"( (let () 12) )";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    TEST_EQ(int(1), (int)prog.expressions.size());
    TEST_ASSERT(std::holds_alternative<Let>(prog.expressions.front()));

    script = R"( (let ([f (lambda (x y) (fx+ x y))] [g (lambda (x) (fx+ x 12))]) (app f 16 (app f (app g 0) (fx+ 1 (app g 0)))))  )";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    TEST_EQ(int(1), (int)prog.expressions.size());
    TEST_ASSERT(std::holds_alternative<Let>(prog.expressions.front()));

    script = R"( (let ([f (lambda (x y) (fx+ x y))] [g (lambda (x) (fx+ x 12))]) (f 16 (f (g 0) (fx+ 1 (g 0)))))  )";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    TEST_EQ(int(1), (int)prog.expressions.size());
    TEST_ASSERT(std::holds_alternative<Let>(prog.expressions.front()));
    }

  namespace
    {
    std::string to_string(Program& prog)
      {
      std::stringstream str;
      dump(str, prog);
      return str.str();
      }
    }

  void parse_quasiquote()
    {
    auto script = R"( `(a b c) )";
    auto tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    TEST_EQ("( quasiquote (a b c) ) ", to_string(prog));

    script = R"( `(list ,(+ 1 2) 4) )";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    //TEST_EQ("( quasiquote (list (, + 1 2) 4) ) ", to_string(prog));
    TEST_EQ("( quasiquote (list (unquote (+ 1 2)) 4) ) ", to_string(prog));

    script = R"( `(a ,(+ 1 2) ,@(map abs '(4 -5 6)) b))";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    //TEST_EQ("( quasiquote (a (, + 1 2) (,@ map abs ' (4 -5 6)) b) ) ", to_string(prog));
    TEST_EQ("( quasiquote (a (unquote (+ 1 2)) (unquote-splicing (map abs (quote (4 -5 6)))) b) ) ", to_string(prog));
    }

  }

SKIWI_END

void run_all_parse_tests()
  {
  using namespace SKIWI;
  parse_fixnum();
  parse_flonum();
  parse_nil();
  parse_true();
  parse_false();
  parse_string();
  parse_primitive_call();
  parse_let();
  parse_quasiquote();
  }