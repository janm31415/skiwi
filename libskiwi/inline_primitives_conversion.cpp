#include "inline_primitives_conversion.h"

#include <cassert>
#include <sstream>
#include <inttypes.h>
#include <stdio.h>
#include <map>

#include "compile_error.h"
#include "parse.h"
#include "tokenize.h"
#include "visitor.h"
#include "simplify_to_core.h"

SKIWI_BEGIN

namespace
  {
  bool g_safe_primitives = true;
  bool g_standard_bindings = false;
  uint64_t g_alpha_conversion_index = 0;

  typedef void(*fun_ptr)(PrimitiveCall&, Expression&);

  std::string make_name(const std::string& original, uint64_t i)
    {
    std::stringstream str;
    str << original << "_" << i;
    return str.str();
    }

  std::vector<std::string> make_let_names(int nr_args)
    {
    assert(nr_args <= 4);
    std::array<std::string, 4> temp = { {"x", "y", "z", "w"} };
    std::vector<std::string> let_names;
    for (int i = 0; i < nr_args; ++i)
      {
      let_names.push_back(make_name(temp[i], g_alpha_conversion_index++));
      }
    return let_names;
    }

  std::string build_script(int nr_args, bool check_binding, bool safe_primitives, std::string prim_name, std::string inline_name, std::string first_arg_check, std::string second_arg_check)
    {
    /*
    this method makes a script of the form
        (let ([x 0])
      (if (##eq? car ###car)
          (if (##pair? x))
              (##car x)          
              (car x))
          (car x)))
    */
    assert(nr_args > 0);

    int nr_of_checks = 0;
    if (!first_arg_check.empty())
      ++nr_of_checks;
    if (!second_arg_check.empty())
      ++nr_of_checks;

    std::vector<std::string> let_names = make_let_names(nr_args);
    std::stringstream str;
    str << "(let (";
    for (int i = 0; i < nr_args; ++i)
      {
      str << "[" << let_names[i] << " 0] ";
      }
    str << ")";

    if (check_binding)
      {
      str << "(if (##eq? " << prim_name << " ###" << prim_name << ")";
      }

    if (safe_primitives && nr_of_checks)
      {
      str << "(if ";
      if (nr_of_checks > 1)
        str << "(and ";
      if (!first_arg_check.empty())
        {
        str << "(" << first_arg_check << " " << let_names[0] << ")";
        }
      if (!second_arg_check.empty())
        {
        str << "(" << second_arg_check << " " << let_names[1] << ")";
        }
      if (nr_of_checks > 1)
        str << ")"; // close and
      }

    str << "(" << inline_name;
    for (int i = 0; i < nr_args; ++i)
      str << " " << let_names[i];
    str << ")"; // close inline call   
        
    if (safe_primitives)
      {
      str << "( " << prim_name;
      for (int i = 0; i < nr_args; ++i)
        str << " " << let_names[i];
      str << "))";
      }

    if (check_binding)
      {
      str << "( " << prim_name;
      for (int i = 0; i < nr_args; ++i)
        str << " " << let_names[i];
      str << "))";
      }
    str << ")"; // close let
    return str.str();
    }

  std::string build_simple_script(int nr_args, bool check_binding, std::string prim_name, std::string inline_name)
    {
    /*
    this method makes a script of the form
    (let ([x 0])
      (if (##eq? fixnum? ###fixnum?)
          (##fixnum? x)          
          (fixnum? x)))
    */
    assert(nr_args > 0);

    std::vector<std::string> let_names = make_let_names(nr_args);
    std::stringstream str;
    str << "(let (";
    for (int i = 0; i < nr_args; ++i)
      {
      str << "[" << let_names[i] << " 0] ";
      }
    str << ")";

    if (check_binding)
      {
      str << "(if (##eq? " << prim_name << " ###" << prim_name << ")";
      }
    
    str << "(" << inline_name;
    for (int i = 0; i < nr_args; ++i)
      str << " " << let_names[i];
    str << ")"; // close inline call   
    if (check_binding)
      {
      str << "( " << prim_name;
      for (int i = 0; i < nr_args; ++i)
        str << " " << let_names[i];
      str << "))";
      }
    str << ")"; // close let
    return str.str();
    }

  std::string build_numerical_script(int nr_args, bool check_binding, std::string prim_name, std::string inline_fixnum_name, std::string inline_flonum_name)
    {
    /*
    this method makes a script of the form
    (let ([x 0] [y 0])
      (if (##eq? + ###+)
          (if (and (##fixnum? y) (##fixnum? x))
              (##fx+ x y)
              (if (and (##flonum? y) (##flonum? x))
                  (##fl+ x y)
                  (+ x y)))
          (+ x y)))
    */
    assert(nr_args > 0);

    std::vector<std::string> let_names = make_let_names(nr_args);
    std::stringstream str;
    str << "(let (";
    for (int i = 0; i < nr_args; ++i)
      {
      str << "[" << let_names[i] << " 0] ";
      }
    str << ")";


    if (check_binding)
      {
      str << "(if (##eq? " << prim_name << " ###" << prim_name << ")";
      }
    str << "(if ";
    if (nr_args > 1)
      {
      str << "(and ";
      }
    for (int i = 0; i < nr_args; ++i)
      str << "(##fixnum? " << let_names[i] << ")";
    if (nr_args > 1)
      str << ")"; // close and
    str << "(" << inline_fixnum_name;
    for (int i = 0; i < nr_args; ++i)
      str << " " << let_names[i];
    str << ")"; // close fixnum call
    str << "(if ";
    if (nr_args > 1)
      {
      str << "(and ";
      }
    for (int i = 0; i < nr_args; ++i)
      str << "(##flonum? " << let_names[i] << ")";
    if (nr_args > 1)
      str << ")"; // close and
    str << "(" << inline_flonum_name;
    for (int i = 0; i < nr_args; ++i)
      str << " " << let_names[i];
    str << ")"; // close flonum call
    str << "( " << prim_name;
    for (int i = 0; i < nr_args; ++i)
      str << " " << let_names[i];
    str << ")))";
    if (check_binding)
      {
      str << "( " << prim_name;
      for (int i = 0; i < nr_args; ++i)
        str << " " << let_names[i];
      str << "))";
      }
    str << ")"; // close let
    return str.str();
    }

  std::string build_numerical_script_flonum_first(int nr_args, bool check_binding, std::string prim_name, std::string inline_fixnum_name, std::string inline_flonum_name)
    {
    /*
    this method makes a script of the form
    (let ([x 0] [y 0])
      (if (##eq? + ###+)
          (if (and (##flonum? y) (##flonum? x))
              (##fl+ x y)
              (if (and (##fixnum? y) (##fixnum? x))
                  (##fx+ x y)
                  (+ x y)))
          (+ x y)))
    */
    assert(nr_args > 0);

    std::vector<std::string> let_names = make_let_names(nr_args);
    std::stringstream str;
    str << "(let (";
    for (int i = 0; i < nr_args; ++i)
      {
      str << "[" << let_names[i] << " 0] ";
      }
    str << ")";


    if (check_binding)
      {
      str << "(if (##eq? " << prim_name << " ###" << prim_name << ")";
      }
    str << "(if ";
    if (nr_args > 1)
      {
      str << "(and ";
      }
    for (int i = 0; i < nr_args; ++i)
      str << "(##flonum? " << let_names[i] << ")";
    if (nr_args > 1)
      str << ")"; // close and
    str << "(" << inline_flonum_name;
    for (int i = 0; i < nr_args; ++i)
      str << " " << let_names[i];
    str << ")"; // close flonum call
    str << "(if ";
    if (nr_args > 1)
      {
      str << "(and ";
      }
    for (int i = 0; i < nr_args; ++i)
      str << "(##fixnum? " << let_names[i] << ")";
    if (nr_args > 1)
      str << ")"; // close and
    str << "(" << inline_fixnum_name;
    for (int i = 0; i < nr_args; ++i)
      str << " " << let_names[i];
    str << ")"; // close fixnum call
    str << "( " << prim_name;
    for (int i = 0; i < nr_args; ++i)
      str << " " << let_names[i];
    str << ")))";
    if (check_binding)
      {
      str << "( " << prim_name;
      for (int i = 0; i < nr_args; ++i)
        str << " " << let_names[i];
      str << "))";
      }
    str << ")"; // close let
    return str.str();
    }

  std::string build_numerical_fx_script(int nr_args, bool check_binding, bool safe_primitives, std::string prim_name, std::string inline_fixnum_name)
    {
    /*
    this method makes a script of the form
    (let ([x 0] [y 0])
      (if (##eq? fx+ ###fx+)
          (if (and (##fixnum? y) (##fixnum? x))
              (##fx+ x y)
              (fx+ x y))
          (fx+ x y)))
    */
    assert(nr_args > 0);

    std::vector<std::string> let_names = make_let_names(nr_args);
    std::stringstream str;
    str << "(let (";
    for (int i = 0; i < nr_args; ++i)
      {
      str << "[" << let_names[i] << " 0] ";
      }
    str << ")";


    if (check_binding)
      {
      str << "(if (##eq? " << prim_name << " ###" << prim_name << ")";
      }
    if (safe_primitives)
      {
      str << "(if ";
      if (nr_args > 1)
        {
        str << "(and ";
        }
      for (int i = 0; i < nr_args; ++i)
        str << "(##fixnum? " << let_names[i] << ")";
      if (nr_args > 1)
        str << ")"; // close and
      }
    str << "(" << inline_fixnum_name;
    for (int i = 0; i < nr_args; ++i)
      str << " " << let_names[i];
    str << ")"; // close fixnum call          
    if (safe_primitives)
      {
      str << "( " << prim_name;
      for (int i = 0; i < nr_args; ++i)
        str << " " << let_names[i];
      str << "))";
      }
    if (check_binding)
      {
      str << "( " << prim_name;
      for (int i = 0; i < nr_args; ++i)
        str << " " << let_names[i];
      str << "))";
      }
    str << ")"; // close let
    return str.str();
    }

  std::string build_numerical_fl_script(int nr_args, bool check_binding, bool safe_primitives, std::string prim_name, std::string inline_flonum_name)
    {
    /*
    this method makes a script of the form
    (let ([x 0] [y 0])
      (if (##eq? / ###/)
          (if (and (##flonum? y) (##flonum? x))
              (##fl/ x y)
              (/ x y))
          (/ x y)))
    */
    assert(nr_args > 0);

    std::vector<std::string> let_names = make_let_names(nr_args);
    std::stringstream str;
    str << "(let (";
    for (int i = 0; i < nr_args; ++i)
      {
      str << "[" << let_names[i] << " 0] ";
      }
    str << ")";


    if (check_binding)
      {
      str << "(if (##eq? " << prim_name << " ###" << prim_name << ")";
      }
    if (safe_primitives)
      {
      str << "(if ";
      if (nr_args > 1)
        {
        str << "(and ";
        }
      for (int i = 0; i < nr_args; ++i)
        str << "(##flonum? " << let_names[i] << ")";
      if (nr_args > 1)
        str << ")"; // close and
      }
    str << "(" << inline_flonum_name;
    for (int i = 0; i < nr_args; ++i)
      str << " " << let_names[i];
    str << ")"; // close fixnum call          
    if (safe_primitives)
      {
      str << "( " << prim_name;
      for (int i = 0; i < nr_args; ++i)
        str << " " << let_names[i];
      str << "))";
      }
    if (check_binding)
      {
      str << "( " << prim_name;
      for (int i = 0; i < nr_args; ++i)
        str << " " << let_names[i];
      str << "))";
      }
    str << ")"; // close let
    return str.str();
    }

  void make_inline_procedure(PrimitiveCall& p, Expression& e, int nr_args, bool check_binding, bool safe_primitives, std::string prim_name, std::string inline_name, std::string first_arg_check = "", std::string second_arg_check = "")
    {
    if (p.arguments.size() != nr_args)
      return;

    std::string convert_script = build_script(nr_args, check_binding, safe_primitives, prim_name, inline_name, first_arg_check, second_arg_check);

    std::vector<token> tokens = tokenize(convert_script);
    std::reverse(tokens.begin(), tokens.end());
    Program prog = make_program(tokens);
    simplify_to_core_forms(prog);

    assert(prog.expressions.size() == 1);
    assert(std::holds_alternative<Let>(prog.expressions.front()));
    Let& let = std::get<Let>(prog.expressions.front());
    for (int i = 0; i < nr_args; ++i)
      let.bindings[i].second = std::move(p.arguments[i]);
    //e = let;
    e = Let();
    std::swap(std::get<Let>(e), let);
    }

  void make_inline_simple_procedure(PrimitiveCall& p, Expression& e, int nr_args, bool check_binding, std::string prim_name, std::string inline_name)
    {
    if (p.arguments.size() != nr_args)
      return;

    std::string convert_script = build_simple_script(nr_args, check_binding, prim_name, inline_name);

    std::vector<token> tokens = tokenize(convert_script);
    std::reverse(tokens.begin(), tokens.end());
    Program prog = make_program(tokens);
    simplify_to_core_forms(prog);

    assert(prog.expressions.size() == 1);
    assert(std::holds_alternative<Let>(prog.expressions.front()));
    Let& let = std::get<Let>(prog.expressions.front());
    for (int i = 0; i < nr_args; ++i)
      let.bindings[i].second = std::move(p.arguments[i]);
    //e = let;
    e = Let();
    std::swap(std::get<Let>(e), let);
    }

  void make_inline_numerical_procedure(PrimitiveCall& p, Expression& e, int nr_args, bool check_binding, std::string prim_name, std::string inline_fixnum_name, std::string inline_flonum_name)
    {
    if (p.arguments.size() != nr_args)
      return;

    std::string convert_script = build_numerical_script(nr_args, check_binding, prim_name, inline_fixnum_name, inline_flonum_name);

    std::vector<token> tokens = tokenize(convert_script);
    std::reverse(tokens.begin(), tokens.end());
    Program prog = make_program(tokens);
    simplify_to_core_forms(prog);

    assert(prog.expressions.size() == 1);
    assert(std::holds_alternative<Let>(prog.expressions.front()));
    Let& let = std::get<Let>(prog.expressions.front());
    for (int i = 0; i < nr_args; ++i)
      let.bindings[i].second = std::move(p.arguments[i]);
    //e = let;
    e = Let();
    std::swap(std::get<Let>(e), let);
    }

  void make_inline_numerical_procedure_flonum_first(PrimitiveCall& p, Expression& e, int nr_args, bool check_binding, std::string prim_name, std::string inline_fixnum_name, std::string inline_flonum_name)
    {
    if (p.arguments.size() != nr_args)
      return;

    std::string convert_script = build_numerical_script_flonum_first(nr_args, check_binding, prim_name, inline_fixnum_name, inline_flonum_name);

    std::vector<token> tokens = tokenize(convert_script);
    std::reverse(tokens.begin(), tokens.end());
    Program prog = make_program(tokens);
    simplify_to_core_forms(prog);

    assert(prog.expressions.size() == 1);
    assert(std::holds_alternative<Let>(prog.expressions.front()));
    Let& let = std::get<Let>(prog.expressions.front());
    for (int i = 0; i < nr_args; ++i)
      let.bindings[i].second = std::move(p.arguments[i]);
    //e = let;
    e = Let();
    std::swap(std::get<Let>(e), let);
    }

  void make_inline_numerical_fx_procedure(PrimitiveCall& p, Expression& e, int nr_args, bool check_binding, bool safe_primitives, std::string prim_name, std::string inline_fixnum_name)
    {
    if (p.arguments.size() != nr_args)
      return;

    std::string convert_script = build_numerical_fx_script(nr_args, check_binding, safe_primitives, prim_name, inline_fixnum_name);

    std::vector<token> tokens = tokenize(convert_script);
    std::reverse(tokens.begin(), tokens.end());
    Program prog = make_program(tokens);
    simplify_to_core_forms(prog);

    assert(prog.expressions.size() == 1);
    assert(std::holds_alternative<Let>(prog.expressions.front()));
    Let& let = std::get<Let>(prog.expressions.front());
    for (int i = 0; i < nr_args; ++i)
      let.bindings[i].second = std::move(p.arguments[i]);
    //e = let;
    e = Let();
    std::swap(std::get<Let>(e), let);
    }

  void make_inline_numerical_fl_procedure(PrimitiveCall& p, Expression& e, int nr_args, bool check_binding, bool safe_primitives, std::string prim_name, std::string inline_flonum_name)
    {
    if (p.arguments.size() != nr_args)
      return;

    std::string convert_script = build_numerical_fl_script(nr_args, check_binding, safe_primitives, prim_name, inline_flonum_name);

    std::vector<token> tokens = tokenize(convert_script);
    std::reverse(tokens.begin(), tokens.end());
    Program prog = make_program(tokens);
    simplify_to_core_forms(prog);

    assert(prog.expressions.size() == 1);
    assert(std::holds_alternative<Let>(prog.expressions.front()));
    Let& let = std::get<Let>(prog.expressions.front());
    for (int i = 0; i < nr_args; ++i)
      let.bindings[i].second = std::move(p.arguments[i]);
    //e = let;
    e = Let();
    std::swap(std::get<Let>(e), let);
    }

  void inline_add1(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure(p, e, 1, !g_standard_bindings, "add1", "##fxadd1", "##fladd1");
    }

  void inline_sub1(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure(p, e, 1, !g_standard_bindings, "sub1", "##fxsub1", "##flsub1");
    }

  void inline_max(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure(p, e, 2, !g_standard_bindings, "max", "##fxmax", "##flmax");
    }

  void inline_min(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure(p, e, 2, !g_standard_bindings, "min", "##fxmin", "##flmin");
    }

  void inline_is_zero(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure(p, e, 1, !g_standard_bindings, "zero?", "##fxzero?", "##flzero?");
    }

  void inline_add(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure(p, e, 2, !g_standard_bindings, "+", "##fx+", "##fl+");
    }

  void inline_sub(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure(p, e, 2, !g_standard_bindings, "-", "##fx-", "##fl-");
    }

  void inline_mul(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure(p, e, 2, !g_standard_bindings, "*", "##fx*", "##fl*");
    }

  void inline_div(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_fl_procedure(p, e, 2, !g_standard_bindings, true, "/", "##fl/");
    }

  void inline_less(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure(p, e, 2, !g_standard_bindings, "<", "##fx<?", "##fl<?");
    }

  void inline_leq(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure(p, e, 2, !g_standard_bindings, "<=", "##fx<=?", "##fl<=?");
    }

  void inline_greater(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure(p, e, 2, !g_standard_bindings, ">", "##fx>?", "##fl>?");
    }

  void inline_geq(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure(p, e, 2, !g_standard_bindings, ">=", "##fx>=?", "##fl>=?");
    }

  void inline_numerically_equal(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure(p, e, 2, !g_standard_bindings, "=", "##fx=?", "##fl=?");
    }

  void inline_numerically_not_equal(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure(p, e, 2, !g_standard_bindings, "!=", "##fx!=?", "##fl!=?");
    }

  void inline_bitwise_and(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_fx_procedure(p, e, 2, !g_standard_bindings, g_safe_primitives, "bitwise-and", "##bitwise-and");
    }

  void inline_bitwise_not(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_fx_procedure(p, e, 1, !g_standard_bindings, g_safe_primitives, "bitwise-not", "##bitwise-not");
    }

  void inline_bitwise_or(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_fx_procedure(p, e, 2, !g_standard_bindings, g_safe_primitives, "bitwise-or", "##bitwise-or");
    }

  void inline_bitwise_xor(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_fx_procedure(p, e, 2, !g_standard_bindings, g_safe_primitives, "bitwise-xor", "##bitwise-xor");
    }

  void inline_fxadd1(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_fx_procedure(p, e, 1, !g_standard_bindings, g_safe_primitives, "fxadd1", "##fxadd1");
    }

  void inline_fxsub1(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_fx_procedure(p, e, 1, !g_standard_bindings, g_safe_primitives, "fxsub1", "##fxsub1");
    }

  void inline_fx_is_zero(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_fx_procedure(p, e, 1, !g_standard_bindings, g_safe_primitives, "fxzero?", "##fxzero?");
    }

  void inline_fxadd(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_fx_procedure(p, e, 2, !g_standard_bindings, g_safe_primitives, "fx+", "##fx+");
    }

  void inline_fxsub(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_fx_procedure(p, e, 2, !g_standard_bindings, g_safe_primitives, "fx-", "##fx-");
    }

  void inline_fxmul(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_fx_procedure(p, e, 2, !g_standard_bindings, g_safe_primitives, "fx*", "##fx*");
    }

  void inline_fxdiv(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_fx_procedure(p, e, 2, !g_standard_bindings, g_safe_primitives, "fx/", "##fx/");
    }

  void inline_fxless(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_fx_procedure(p, e, 2, !g_standard_bindings, g_safe_primitives, "fx<?", "##fx<?");
    }

  void inline_fxleq(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_fx_procedure(p, e, 2, !g_standard_bindings, g_safe_primitives, "fx<=?", "##fx<=?");
    }

  void inline_fxgreater(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_fx_procedure(p, e, 2, !g_standard_bindings, g_safe_primitives, "fx>?", "##fx>?");
    }

  void inline_fxgeq(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_fx_procedure(p, e, 2, !g_standard_bindings, g_safe_primitives, "fx>=?", "##fx>=?");
    }

  void inline_fxequal(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_fx_procedure(p, e, 2, !g_standard_bindings, g_safe_primitives, "fx=?", "##fx=?");
    }

  void inline_is_fixnum(PrimitiveCall& p, Expression& e)
    {
    make_inline_simple_procedure(p, e, 1, !g_standard_bindings, "fixnum?", "##fixnum?");
    }

  void inline_is_flonum(PrimitiveCall& p, Expression& e)
    {
    make_inline_simple_procedure(p, e, 1, !g_standard_bindings, "flonum?", "##flonum?");
    }

  void inline_is_pair(PrimitiveCall& p, Expression& e)
    {
    make_inline_simple_procedure(p, e, 1, !g_standard_bindings, "pair?", "##pair?");
    }

  void inline_is_vector(PrimitiveCall& p, Expression& e)
    {
    make_inline_simple_procedure(p, e, 1, !g_standard_bindings, "vector?", "##vector?");
    }

  void inline_is_string(PrimitiveCall& p, Expression& e)
    {
    make_inline_simple_procedure(p, e, 1, !g_standard_bindings, "string?", "##string?");
    }

  void inline_is_symbol(PrimitiveCall& p, Expression& e)
    {
    make_inline_simple_procedure(p, e, 1, !g_standard_bindings, "symbol?", "##symbol?");
    }

  void inline_is_closure(PrimitiveCall& p, Expression& e)
    {
    make_inline_simple_procedure(p, e, 1, !g_standard_bindings, "closure?", "##closure?");
    }

  void inline_is_procedure(PrimitiveCall& p, Expression& e)
    {
    make_inline_simple_procedure(p, e, 1, !g_standard_bindings, "procedure?", "##procedure?");
    }

  void inline_is_port(PrimitiveCall& p, Expression& e)
    {
    make_inline_simple_procedure(p, e, 1, !g_standard_bindings, "port?", "##port?");
    }

  void inline_is_input_port(PrimitiveCall& p, Expression& e)
    {
    make_inline_simple_procedure(p, e, 1, !g_standard_bindings, "input-port?", "##input-port?");
    }

  void inline_is_output_port(PrimitiveCall& p, Expression& e)
    {
    make_inline_simple_procedure(p, e, 1, !g_standard_bindings, "output-port?", "##output-port?");
    }

  void inline_is_boolean(PrimitiveCall& p, Expression& e)
    {
    make_inline_simple_procedure(p, e, 1, !g_standard_bindings, "boolean?", "##boolean?");
    }

  void inline_is_null(PrimitiveCall& p, Expression& e)
    {
    make_inline_simple_procedure(p, e, 1, !g_standard_bindings, "null?", "##null?");
    }

  void inline_is_eof_object(PrimitiveCall& p, Expression& e)
    {
    make_inline_simple_procedure(p, e, 1, !g_standard_bindings, "eof-object?", "##eof-object?");
    }

  void inline_is_char(PrimitiveCall& p, Expression& e)
    {
    make_inline_simple_procedure(p, e, 1, !g_standard_bindings, "char?", "##char?");
    }

  void inline_is_promise(PrimitiveCall& p, Expression& e)
    {
    make_inline_simple_procedure(p, e, 1, !g_standard_bindings, "promise?", "##promise?");
    }

  void inline_is_eq(PrimitiveCall& p, Expression& e)
    {
    make_inline_simple_procedure(p, e, 1, !g_standard_bindings, "eq?", "##eq?");
    }

  void inline_cons(PrimitiveCall& p, Expression& e)
    {
    make_inline_simple_procedure(p, e, 2, !g_standard_bindings, "cons", "##cons");
    }

  void inline_charless(PrimitiveCall& p, Expression& e)
    {
    make_inline_procedure(p, e, 1, !g_standard_bindings, g_safe_primitives, "char<?", "##char<?", "##char?");
    }

  void inline_charleq(PrimitiveCall& p, Expression& e)
    {
    make_inline_procedure(p, e, 1, !g_standard_bindings, g_safe_primitives, "char<=?", "##char<=?", "##char?");
    }

  void inline_chargreater(PrimitiveCall& p, Expression& e)
    {
    make_inline_procedure(p, e, 1, !g_standard_bindings, g_safe_primitives, "char>?", "##char>?", "##char?");
    }

  void inline_chargeq(PrimitiveCall& p, Expression& e)
    {
    make_inline_procedure(p, e, 1, !g_standard_bindings, g_safe_primitives, "char>=?", "##char>=?", "##char?");
    }

  void inline_charequal(PrimitiveCall& p, Expression& e)
    {
    make_inline_procedure(p, e, 1, !g_standard_bindings, g_safe_primitives, "char=?", "##char=?", "##char?");
    }

  void inline_car(PrimitiveCall& p, Expression& e)
    {
    make_inline_procedure(p, e, 1, !g_standard_bindings, g_safe_primitives, "car", "##car", "##pair?");    
    }

  void inline_cdr(PrimitiveCall& p, Expression& e)
    {
    make_inline_procedure(p, e, 1, !g_standard_bindings, g_safe_primitives, "cdr", "##cdr", "##pair?");    
    }

  void inline_set_car(PrimitiveCall& p, Expression& e)
    {
    make_inline_procedure(p, e, 2, !g_standard_bindings, g_safe_primitives, "set-car!", "##set-car!", "##pair?");
    }

  void inline_set_cdr(PrimitiveCall& p, Expression& e)
    {
    make_inline_procedure(p, e, 2, !g_standard_bindings, g_safe_primitives, "set-cdr!", "##set-cdr!", "##pair?");
    }

  void inline_not(PrimitiveCall& p, Expression& e)
    {
    make_inline_simple_procedure(p, e, 1, !g_standard_bindings, "not", "##not");
    }

  void inline_memq(PrimitiveCall& p, Expression& e)
    {
    make_inline_simple_procedure(p, e, 2, !g_standard_bindings, "memq", "##memq");
    }

  void inline_assq(PrimitiveCall& p, Expression& e)
    {
    make_inline_simple_procedure(p, e, 2, !g_standard_bindings, "assq", "##assq");
    }

  void inline_ieee754_sign(PrimitiveCall& p, Expression& e)
    {
    make_inline_procedure(p, e, 1, !g_standard_bindings, g_safe_primitives, "ieee754-sign", "##ieee754-sign", "##flonum?");
    }

  void inline_ieee754_mantissa(PrimitiveCall& p, Expression& e)
    {
    make_inline_procedure(p, e, 1, !g_standard_bindings, g_safe_primitives, "ieee754-mantissa", "##ieee754-mantissa", "##flonum?");
    }

  void inline_ieee754_exponent(PrimitiveCall& p, Expression& e)
    {
    make_inline_procedure(p, e, 1, !g_standard_bindings, g_safe_primitives, "ieee754-exponent", "##ieee754-exponent", "##flonum?");
    }

  void inline_ieee754_pi(PrimitiveCall& p, Expression& e)
    {
    if (!p.arguments.empty())
      return;
    std::string script_with_check = R"(
    (if (##eq? ieee754-pi ###ieee754-pi)
        (##ieee754-pi)
        (ieee754-pi))
)";

    std::string script_without_check = R"(   
        (##ieee754-pi)      
)";

    std::vector<token> tokens = tokenize(g_standard_bindings ? script_without_check : script_with_check);
    std::reverse(tokens.begin(), tokens.end());
    Program prog = make_program(tokens);
    assert(prog.expressions.size() == 1);    
    e = prog.expressions.front();
    }

  void inline_ieee754_sin(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure_flonum_first(p, e, 1, !g_standard_bindings, "ieee754-sin", "##ieee754-fxsin", "##ieee754-flsin");
    }

  void inline_ieee754_cos(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure_flonum_first(p, e, 1, !g_standard_bindings, "ieee754-cos", "##ieee754-fxcos", "##ieee754-flcos");
    }

  void inline_ieee754_tan(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure_flonum_first(p, e, 1, !g_standard_bindings, "ieee754-tan", "##ieee754-fxtan", "##ieee754-fltan");
    }

  void inline_ieee754_asin(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure_flonum_first(p, e, 1, !g_standard_bindings, "ieee754-asin", "##ieee754-fxasin", "##ieee754-flasin");
    }

  void inline_ieee754_acos(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure_flonum_first(p, e, 1, !g_standard_bindings, "ieee754-acos", "##ieee754-fxacos", "##ieee754-flacos");
    }

  void inline_ieee754_atan1(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure_flonum_first(p, e, 1, !g_standard_bindings, "ieee754-atan1", "##ieee754-fxatan1", "##ieee754-flatan1");
    }

  void inline_ieee754_log(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure_flonum_first(p, e, 1, !g_standard_bindings, "ieee754-log", "##ieee754-fxlog", "##ieee754-fllog");
    }

  void inline_ieee754_round(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure_flonum_first(p, e, 1, !g_standard_bindings, "ieee754-round", "##ieee754-fxround", "##ieee754-flround");
    }

  void inline_ieee754_truncate(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure_flonum_first(p, e, 1, !g_standard_bindings, "ieee754-truncate", "##ieee754-fxtruncate", "##ieee754-fltruncate");
    }

  void inline_ieee754_sqrt(PrimitiveCall& p, Expression& e)
    {
    make_inline_numerical_procedure_flonum_first(p, e, 1, !g_standard_bindings, "ieee754-sqrt", "##ieee754-fxsqrt", "##ieee754-flsqrt");
    }

  void inline_fixnum_to_char(PrimitiveCall& p, Expression& e)
    {
    make_inline_procedure(p, e, 1, !g_standard_bindings, g_safe_primitives, "fixnum->char", "##fixnum->char", "##fixnum?");
    }

  void inline_char_to_fixnum(PrimitiveCall& p, Expression& e)
    {
    make_inline_procedure(p, e, 1, !g_standard_bindings, g_safe_primitives, "char->fixnum", "##char->fixnum", "##char?");
    }

  void inline_vector_length(PrimitiveCall& p, Expression& e)
    {
    make_inline_procedure(p, e, 1, !g_standard_bindings, g_safe_primitives, "vector-length", "##vector-length", "##vector?");
    }

  void inline_fixnum_to_flonum(PrimitiveCall& p, Expression& e)
    {
    make_inline_procedure(p, e, 1, !g_standard_bindings, g_safe_primitives, "fixnum->flonum", "##fixnum->flonum", "##fixnum?");
    }

  void inline_flonum_to_fixnum(PrimitiveCall& p, Expression& e)
    {
    make_inline_procedure(p, e, 1, !g_standard_bindings, g_safe_primitives, "flonum->fixnum", "##flonum->fixnum", "##flonum?");
    }

  void inline_quiet_undefined(PrimitiveCall& p, Expression&)
    {
    p.primitive_name = "##%quiet-undefined";
    }

  void inline_undefined(PrimitiveCall& p, Expression&)
    {
    p.primitive_name = "##%undefined";
    }

  void inline_arithmetic_shift(PrimitiveCall& p, Expression& e)
    {
    make_inline_procedure(p, e, 2, !g_standard_bindings, g_safe_primitives, "arithmetic-shift", "##arithmetic-shift", "##fixnum?", "##fixnum?");
    }

  void inline_remainder(PrimitiveCall& p, Expression& e)
    {
    make_inline_procedure(p, e, 2, !g_standard_bindings, true, "remainder", "##remainder", "##fixnum?", "##fixnum?");
    }

  void inline_quotient(PrimitiveCall& p, Expression& e)
    {
    make_inline_procedure(p, e, 2, !g_standard_bindings, true, "quotient", "##quotient", "##fixnum?", "##fixnum?");
    }

  struct inline_primitives_visitor : public base_visitor<inline_primitives_visitor>
    {

    std::map<std::string, fun_ptr> get_primitives_that_can_be_inlined() const
      {
      std::map<std::string, fun_ptr> ptcbi;
      ptcbi.insert(std::pair<std::string, fun_ptr>("remainder", &inline_remainder));
      ptcbi.insert(std::pair<std::string, fun_ptr>("quotient", &inline_quotient));
      ptcbi.insert(std::pair<std::string, fun_ptr>("arithmetic-shift", &inline_arithmetic_shift));
      ptcbi.insert(std::pair<std::string, fun_ptr>("%quiet-undefined", &inline_quiet_undefined));
      ptcbi.insert(std::pair<std::string, fun_ptr>("%undefined", &inline_undefined));
      ptcbi.insert(std::pair<std::string, fun_ptr>("vector-length", &inline_vector_length));
      ptcbi.insert(std::pair<std::string, fun_ptr>("fixnum->flonum", &inline_fixnum_to_flonum));
      ptcbi.insert(std::pair<std::string, fun_ptr>("flonum->fixnum", &inline_flonum_to_fixnum));
      ptcbi.insert(std::pair<std::string, fun_ptr>("fixnum->char", &inline_fixnum_to_char));
      ptcbi.insert(std::pair<std::string, fun_ptr>("char->fixnum", &inline_char_to_fixnum));
      ptcbi.insert(std::pair<std::string, fun_ptr>("bitwise-and", &inline_bitwise_and));
      ptcbi.insert(std::pair<std::string, fun_ptr>("bitwise-not", &inline_bitwise_not));
      ptcbi.insert(std::pair<std::string, fun_ptr>("bitwise-or", &inline_bitwise_or));
      ptcbi.insert(std::pair<std::string, fun_ptr>("bitwise-xor", &inline_bitwise_xor));

      ptcbi.insert(std::pair<std::string, fun_ptr>("ieee754-sign", &inline_ieee754_sign));
      ptcbi.insert(std::pair<std::string, fun_ptr>("ieee754-mantissa", &inline_ieee754_mantissa));
      ptcbi.insert(std::pair<std::string, fun_ptr>("ieee754-exponent", &inline_ieee754_exponent));
      ptcbi.insert(std::pair<std::string, fun_ptr>("ieee754-pi", &inline_ieee754_pi));
      ptcbi.insert(std::pair<std::string, fun_ptr>("ieee754-sin", &inline_ieee754_sin));
      ptcbi.insert(std::pair<std::string, fun_ptr>("ieee754-cos", &inline_ieee754_cos));
      ptcbi.insert(std::pair<std::string, fun_ptr>("ieee754-tan", &inline_ieee754_tan));
      ptcbi.insert(std::pair<std::string, fun_ptr>("ieee754-asin", &inline_ieee754_asin));
      ptcbi.insert(std::pair<std::string, fun_ptr>("ieee754-acos", &inline_ieee754_acos));
      ptcbi.insert(std::pair<std::string, fun_ptr>("ieee754-atan1", &inline_ieee754_atan1));
      ptcbi.insert(std::pair<std::string, fun_ptr>("ieee754-log", &inline_ieee754_log));
      ptcbi.insert(std::pair<std::string, fun_ptr>("ieee754-round", &inline_ieee754_round));
      ptcbi.insert(std::pair<std::string, fun_ptr>("ieee754-truncate", &inline_ieee754_truncate));
      ptcbi.insert(std::pair<std::string, fun_ptr>("ieee754-sqrt", &inline_ieee754_sqrt));
      

      ptcbi.insert(std::pair<std::string, fun_ptr>("memq", &inline_memq));
      ptcbi.insert(std::pair<std::string, fun_ptr>("assq", &inline_assq));
      ptcbi.insert(std::pair<std::string, fun_ptr>("not", &inline_not));
      ptcbi.insert(std::pair<std::string, fun_ptr>("cons", &inline_cons));
      ptcbi.insert(std::pair<std::string, fun_ptr>("car", &inline_car));
      ptcbi.insert(std::pair<std::string, fun_ptr>("cdr", &inline_cdr));
      ptcbi.insert(std::pair<std::string, fun_ptr>("set-car!", &inline_set_car));
      ptcbi.insert(std::pair<std::string, fun_ptr>("set-cdr!", &inline_set_cdr));
      ptcbi.insert(std::pair<std::string, fun_ptr>("fixnum?", &inline_is_fixnum));
      ptcbi.insert(std::pair<std::string, fun_ptr>("flonum?", &inline_is_flonum));
      ptcbi.insert(std::pair<std::string, fun_ptr>("pair?", &inline_is_pair));
      ptcbi.insert(std::pair<std::string, fun_ptr>("vector?", &inline_is_vector));
      ptcbi.insert(std::pair<std::string, fun_ptr>("string?", &inline_is_string));
      ptcbi.insert(std::pair<std::string, fun_ptr>("symbol?", &inline_is_symbol));
      ptcbi.insert(std::pair<std::string, fun_ptr>("closure?", &inline_is_closure));
      ptcbi.insert(std::pair<std::string, fun_ptr>("port?", &inline_is_port));
      ptcbi.insert(std::pair<std::string, fun_ptr>("input-port?", &inline_is_input_port));
      ptcbi.insert(std::pair<std::string, fun_ptr>("output-port?", &inline_is_output_port));
      ptcbi.insert(std::pair<std::string, fun_ptr>("procedure?", &inline_is_procedure));
      ptcbi.insert(std::pair<std::string, fun_ptr>("boolean?", &inline_is_boolean));
      ptcbi.insert(std::pair<std::string, fun_ptr>("null?", &inline_is_null));
      ptcbi.insert(std::pair<std::string, fun_ptr>("eof-object?", &inline_is_eof_object));
      ptcbi.insert(std::pair<std::string, fun_ptr>("char?", &inline_is_char));
      ptcbi.insert(std::pair<std::string, fun_ptr>("promise?", &inline_is_promise));      
      ptcbi.insert(std::pair<std::string, fun_ptr>("eq?", &inline_is_eq));
      ptcbi.insert(std::pair<std::string, fun_ptr>("add1", &inline_add1));
      ptcbi.insert(std::pair<std::string, fun_ptr>("sub1", &inline_sub1));
      ptcbi.insert(std::pair<std::string, fun_ptr>("max", &inline_max));
      ptcbi.insert(std::pair<std::string, fun_ptr>("min", &inline_min));
      ptcbi.insert(std::pair<std::string, fun_ptr>("zero?", &inline_is_zero));
      ptcbi.insert(std::pair<std::string, fun_ptr>("+", &inline_add));
      ptcbi.insert(std::pair<std::string, fun_ptr>("-", &inline_sub));
      ptcbi.insert(std::pair<std::string, fun_ptr>("*", &inline_mul));
      ptcbi.insert(std::pair<std::string, fun_ptr>("/", &inline_div));
      ptcbi.insert(std::pair<std::string, fun_ptr>("<", &inline_less));
      ptcbi.insert(std::pair<std::string, fun_ptr>("<=", &inline_leq));
      ptcbi.insert(std::pair<std::string, fun_ptr>(">", &inline_greater));
      ptcbi.insert(std::pair<std::string, fun_ptr>(">=", &inline_geq));
      ptcbi.insert(std::pair<std::string, fun_ptr>("=", &inline_numerically_equal));
      ptcbi.insert(std::pair<std::string, fun_ptr>("!=", &inline_numerically_not_equal));

      ptcbi.insert(std::pair<std::string, fun_ptr>("fxadd1", &inline_fxadd1));
      ptcbi.insert(std::pair<std::string, fun_ptr>("fxsub1", &inline_fxsub1));
      ptcbi.insert(std::pair<std::string, fun_ptr>("fxzero?", &inline_fx_is_zero));
      ptcbi.insert(std::pair<std::string, fun_ptr>("fx+", &inline_fxadd));
      ptcbi.insert(std::pair<std::string, fun_ptr>("fx-", &inline_fxsub));
      ptcbi.insert(std::pair<std::string, fun_ptr>("fx*", &inline_fxmul));
      ptcbi.insert(std::pair<std::string, fun_ptr>("fx/", &inline_fxdiv));
      ptcbi.insert(std::pair<std::string, fun_ptr>("fx<?", &inline_fxless));
      ptcbi.insert(std::pair<std::string, fun_ptr>("fx<=?", &inline_fxleq));
      ptcbi.insert(std::pair<std::string, fun_ptr>("fx>?", &inline_fxgreater));
      ptcbi.insert(std::pair<std::string, fun_ptr>("fx>=?", &inline_fxgeq));
      ptcbi.insert(std::pair<std::string, fun_ptr>("fx=?", &inline_fxequal));

      ptcbi.insert(std::pair<std::string, fun_ptr>("char<?", &inline_charless));
      ptcbi.insert(std::pair<std::string, fun_ptr>("char<=?", &inline_charleq));
      ptcbi.insert(std::pair<std::string, fun_ptr>("char>?", &inline_chargreater));
      ptcbi.insert(std::pair<std::string, fun_ptr>("char>=?", &inline_chargeq));
      ptcbi.insert(std::pair<std::string, fun_ptr>("char=?", &inline_charequal));

      return ptcbi;
      }

    void _check_for_inline(PrimitiveCall& p, Expression& e)
      {
      static std::map<std::string, fun_ptr> ptcbi = get_primitives_that_can_be_inlined();

      auto it = ptcbi.find(p.primitive_name);
      if (it != ptcbi.end())
        {
        it->second(p, e);
        }
      }

    virtual void _postvisit(Expression& e)
      {
      if (std::holds_alternative<PrimitiveCall>(e))
        _check_for_inline(std::get<PrimitiveCall>(e), e);
      }

    };
  }

void inline_primitives(Program& prog, uint64_t& alpha_conversion_index, bool safe_primitives, bool standard_bindings)
  {
  g_safe_primitives = safe_primitives;
  g_standard_bindings = standard_bindings;
  g_alpha_conversion_index = alpha_conversion_index;
  inline_primitives_visitor ipv;
  visitor<Program, inline_primitives_visitor>::visit(prog, &ipv);
  prog.inline_primitives_converted = true;
  alpha_conversion_index = g_alpha_conversion_index;
  }

SKIWI_END
