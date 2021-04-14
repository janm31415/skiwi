#include "constant_folding.h"

#include <cassert>
#include <sstream>
#include <inttypes.h>
#include <stdio.h>
#include <map>
#include <cmath>
#include <algorithm>

#include "compile_error.h"
#include "visitor.h"

SKIWI_BEGIN

namespace
  {

  bool is_literal(Literal& lit, Expression& e)
    {
    if (std::holds_alternative<Literal>(e))
      {
      lit = std::get<Literal>(e);
      return true;
      }
    return false;
    }

  bool is_fixnum(Fixnum& f, Literal& lit)
    {
    if (std::holds_alternative<Fixnum>(lit))
      {
      f = std::get<Fixnum>(lit);
      return true;
      }
    return false;
    }

  bool is_flonum(Flonum& f, Literal& lit)
    {
    if (std::holds_alternative<Flonum>(lit))
      {
      f = std::get<Flonum>(lit);
      return true;
      }
    return false;
    }

  bool is_character(Character& c, Literal& lit)
    {
    if (std::holds_alternative<Character>(lit))
      {
      c = std::get<Character>(lit);
      return true;
      }
    return false;
    }

  bool is_symbol(Symbol& s, Literal& lit)
    {
    if (std::holds_alternative<Symbol>(lit))
      {
      s = std::get<Symbol>(lit);
      return true;
      }
    return false;
    }

  bool is_string(String& s, Literal& lit)
    {
    if (std::holds_alternative<String>(lit))
      {
      s = std::get<String>(lit);
      return true;
      }
    return false;
    }

  bool is_string(Literal& lit)
    {
    return std::holds_alternative<String>(lit);
    }

  bool is_symbol(Literal& lit)
    {
    return std::holds_alternative<Symbol>(lit);
    }

  bool is_fixnum(Literal& lit)
    {
    return std::holds_alternative<Fixnum>(lit);
    }

  bool is_flonum(Literal& lit)
    {
    return std::holds_alternative<Flonum>(lit);
    }

  bool is_character(Literal& lit)
    {
    return std::holds_alternative<Character>(lit);
    }

  bool is_nil(Literal& lit)
    {
    return std::holds_alternative<Nil>(lit);
    }

  bool is_true(Literal& lit)
    {
    return std::holds_alternative<True>(lit);
    }

  bool is_false(Literal& lit)
    {
    return std::holds_alternative<False>(lit);
    }

  True make_true()
    {
    return True();
    }

  False make_false()
    {
    return False();
    }

  Fixnum make_fixnum(int64_t value)
    {
    Fixnum f;
    f.value = value;
    return f;
    }

  Flonum make_flonum(double value)
    {
    Flonum f;
    f.value = value;
    return f;
    }

  Character make_character(char value)
    {
    Character c;
    c.value = value;
    return c;
    }

  void fold_dummy(PrimitiveCall&, Expression&)
    {
    }

  void fold_fx_add(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Fixnum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_fixnum(f1, lit1) && is_fixnum(f2, lit2))
        {
        e = make_fixnum(f1.value + f2.value);
        }
      }
    }

  void fold_fx_sub(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Fixnum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_fixnum(f1, lit1) && is_fixnum(f2, lit2))
        {
        e = make_fixnum(f1.value - f2.value);
        }
      }
    }

  void fold_fx_mul(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Fixnum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_fixnum(f1, lit1) && is_fixnum(f2, lit2))
        {
        e = make_fixnum(f1.value * f2.value);
        }
      }
    }

  void fold_fx_div(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Fixnum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_fixnum(f1, lit1) && is_fixnum(f2, lit2))
        {
        if (f2.value)
          e = make_fixnum(f1.value / f2.value);
        else
          throw_error(division_by_zero);
        }
      }
    }

  void fold_fx_less(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Fixnum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_fixnum(f1, lit1) && is_fixnum(f2, lit2))
        {
        if (f1.value < f2.value)
          e = make_true();
        else
          e = make_false();
        }
      }
    }

  void fold_fx_leq(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Fixnum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_fixnum(f1, lit1) && is_fixnum(f2, lit2))
        {
        if (f1.value <= f2.value)
          e = make_true();
        else
          e = make_false();
        }
      }
    }

  void fold_fx_greater(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Fixnum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_fixnum(f1, lit1) && is_fixnum(f2, lit2))
        {
        if (f1.value > f2.value)
          e = make_true();
        else
          e = make_false();
        }
      }
    }

  void fold_fx_geq(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Fixnum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_fixnum(f1, lit1) && is_fixnum(f2, lit2))
        {
        if (f1.value >= f2.value)
          e = make_true();
        else
          e = make_false();
        }
      }
    }

  void fold_fx_equal(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Fixnum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_fixnum(f1, lit1) && is_fixnum(f2, lit2))
        {
        if (f1.value == f2.value)
          e = make_true();
        else
          e = make_false();
        }
      }
    }

  void fold_fx_not_equal(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Fixnum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_fixnum(f1, lit1) && is_fixnum(f2, lit2))
        {
        if (f1.value != f2.value)
          e = make_true();
        else
          e = make_false();
        }
      }
    }

  void fold_fx_max(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Fixnum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_fixnum(f1, lit1) && is_fixnum(f2, lit2))
        {
        e = make_fixnum(std::max<int64_t>(f1.value, f2.value));
        }
      }
    }

  void fold_fx_min(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Fixnum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_fixnum(f1, lit1) && is_fixnum(f2, lit2))
        {
        e = make_fixnum(std::min<int64_t>(f1.value, f2.value));
        }
      }
    }

  void fold_fx_add1(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Fixnum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_fixnum(f1, lit1))
        {
        e = make_fixnum(f1.value + 1);
        }
      }
    }

  void fold_fx_sub1(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Fixnum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_fixnum(f1, lit1))
        {
        e = make_fixnum(f1.value - 1);
        }
      }
    }

  void fold_fx_is_zero(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1, lit2;
    Fixnum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_fixnum(f1, lit1))
        {
        if (f1.value == 0)
          e = make_true();
        else
          e = make_false();
        }
      }
    }

  void fold_fl_add(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Flonum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_flonum(f1, lit1) && is_flonum(f2, lit2))
        {
        e = make_flonum(f1.value + f2.value);
        }
      }
    }

  void fold_fl_sub(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Flonum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_flonum(f1, lit1) && is_flonum(f2, lit2))
        {
        e = make_flonum(f1.value - f2.value);
        }
      }
    }

  void fold_fl_mul(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Flonum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_flonum(f1, lit1) && is_flonum(f2, lit2))
        {
        e = make_flonum(f1.value * f2.value);
        }
      }
    }

  void fold_fl_div(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Flonum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_flonum(f1, lit1) && is_flonum(f2, lit2))
        {
        e = make_flonum(f1.value / f2.value);
        }
      }
    }

  void fold_fl_less(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Flonum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_flonum(f1, lit1) && is_flonum(f2, lit2))
        {
        if (f1.value < f2.value)
          e = make_true();
        else
          e = make_false();
        }
      }
    }

  void fold_fl_leq(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Flonum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_flonum(f1, lit1) && is_flonum(f2, lit2))
        {
        if (f1.value <= f2.value)
          e = make_true();
        else
          e = make_false();
        }
      }
    }

  void fold_fl_greater(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Flonum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_flonum(f1, lit1) && is_flonum(f2, lit2))
        {
        if (f1.value > f2.value)
          e = make_true();
        else
          e = make_false();
        }
      }
    }

  void fold_fl_geq(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Flonum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_flonum(f1, lit1) && is_flonum(f2, lit2))
        {
        if (f1.value >= f2.value)
          e = make_true();
        else
          e = make_false();
        }
      }
    }

  void fold_fl_equal(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Flonum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_flonum(f1, lit1) && is_flonum(f2, lit2))
        {
        if (f1.value == f2.value)
          e = make_true();
        else
          e = make_false();
        }
      }
    }

  void fold_fl_not_equal(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Flonum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_flonum(f1, lit1) && is_flonum(f2, lit2))
        {
        if (f1.value != f2.value)
          e = make_true();
        else
          e = make_false();
        }
      }
    }

  void fold_fl_max(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Flonum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_flonum(f1, lit1) && is_flonum(f2, lit2))
        {
        e = make_flonum(std::max<double>(f1.value, f2.value));
        }
      }
    }

  void fold_fl_min(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Flonum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_flonum(f1, lit1) && is_flonum(f2, lit2))
        {
        e = make_flonum(std::min<double>(f1.value, f2.value));
        }
      }
    }

  void fold_fl_add1(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Flonum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_flonum(f1, lit1))
        {
        e = make_flonum(f1.value + 1.0);
        }
      }
    }

  void fold_fl_sub1(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Flonum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_flonum(f1, lit1))
        {
        e = make_flonum(f1.value - 1.0);
        }
      }
    }

  void fold_fl_is_zero(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1, lit2;
    Flonum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_flonum(f1, lit1))
        {
        if (f1.value == 0.0)
          e = make_true();
        else
          e = make_false();
        }
      }
    }

  void fold_is_fixnum(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit;
    if (is_literal(lit, p.arguments.front()))
      {
      if (is_fixnum(lit))
        e = make_true();
      else
        e = make_false();
      }
    }

  void fold_is_flonum(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit;
    if (is_literal(lit, p.arguments.front()))
      {
      if (is_flonum(lit))
        e = make_true();
      else
        e = make_false();
      }
    }

  void fold_is_pair(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit;
    if (is_literal(lit, p.arguments.front()))
      e = make_false();
    }

  void fold_is_string(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit;
    if (is_literal(lit, p.arguments.front()))
      {
      if (is_string(lit))
        e = make_true();
      else
        e = make_false();
      }
    }

  void fold_is_vector(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit;
    if (is_literal(lit, p.arguments.front()))
      e = make_false();
    }

  void fold_is_symbol(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit;
    if (is_literal(lit, p.arguments.front()))
      {
      if (is_symbol(lit))
        e = make_true();
      else
        e = make_false();
      }
    }

  void fold_is_closure(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit;
    if (is_literal(lit, p.arguments.front()))
      e = make_false();
    }

  void fold_is_procedure(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit;
    if (is_literal(lit, p.arguments.front()))
      e = make_false();
    }

  void fold_is_promise(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit;
    if (is_literal(lit, p.arguments.front()))
      e = make_false();
    }

  void fold_is_port(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit;
    if (is_literal(lit, p.arguments.front()))
      e = make_false();
    }

  void fold_is_eof_object(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit;
    if (is_literal(lit, p.arguments.front()))
      e = make_false();
    }

  void fold_is_input_port(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit;
    if (is_literal(lit, p.arguments.front()))
      e = make_false();
    }

  void fold_is_output_port(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit;
    if (is_literal(lit, p.arguments.front()))
      e = make_false();
    }

  void fold_is_boolean(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit;
    if (is_literal(lit, p.arguments.front()))
      {
      if (is_true(lit) || is_false(lit))
        e = make_true();
      else
        e = make_false();
      }
    }

  void fold_is_char(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit;
    if (is_literal(lit, p.arguments.front()))
      {
      if (is_character(lit))
        e = make_true();
      else
        e = make_false();
      }
    }

  void fold_is_null(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit;
    if (is_literal(lit, p.arguments.front()))
      {
      if (is_nil(lit))
        e = make_true();
      else
        e = make_false();
      }
    }

  void fold_not(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit;
    if (is_literal(lit, p.arguments.front()))
      {
      if (is_true(lit))
        e = make_false();
      else if (is_false(lit))
        e = make_true();
      }
    }

  void fold_is_eq(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      Fixnum f1, f2;
      Flonum fl1, fl2;
      Character ch1, ch2;
      Symbol s1, s2;
      if (is_fixnum(f1, lit1) && is_fixnum(f2, lit2))
        {
        if (f1.value == f2.value)
          e = make_true();
        else
          e = make_false();
        }
      else if (is_flonum(fl1, lit1) && is_flonum(fl2, lit2))
        {
        if (fl1.value == fl2.value)
          e = make_true();
        else
          e = make_false();
        }
      else if (is_true(lit1) && is_true(lit2))
        {
        e = make_true();
        }
      else if (is_false(lit1) && is_false(lit2))
        {
        e = make_true();
        }
      else if (is_nil(lit1) && is_nil(lit2))
        {
        e = make_true();
        }
      else if (is_symbol(s1, lit1) && is_symbol(s2, lit2))
        {
        if (s1.value == s2.value)
          e = make_true();
        else
          e = make_false();
        }
      }
    }

  void fold_pi(PrimitiveCall&, Expression& e)
    {
    e = make_flonum(3.141592653589793116);
    }

  void fold_fxsin(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Fixnum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_fixnum(f1, lit1))
        {
        e = make_flonum(std::sin((double)f1.value));
        }
      }
    }

  void fold_fxcos(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Fixnum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_fixnum(f1, lit1))
        {
        e = make_flonum(std::cos((double)f1.value));
        }
      }
    }

  void fold_fxtan(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Fixnum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_fixnum(f1, lit1))
        {
        e = make_flonum(std::tan((double)f1.value));
        }
      }
    }

  void fold_fxasin(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Fixnum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_fixnum(f1, lit1))
        {
        e = make_flonum(std::asin((double)f1.value));
        }
      }
    }

  void fold_fxacos(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Fixnum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_fixnum(f1, lit1))
        {
        e = make_flonum(std::acos((double)f1.value));
        }
      }
    }

  void fold_fxatan1(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Fixnum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_fixnum(f1, lit1))
        {
        e = make_flonum(std::atan((double)f1.value));
        }
      }
    }

  void fold_fxlog(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Fixnum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_fixnum(f1, lit1))
        {
        e = make_flonum(std::log((double)f1.value));
        }
      }
    }

  void fold_fxround(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Fixnum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_fixnum(f1, lit1))
        {
        e = make_flonum(std::round((double)f1.value));
        }
      }
    }

  void fold_fxtruncate(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Fixnum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_fixnum(f1, lit1))
        {
        e = f1;
        }
      }
    }

  void fold_fxsqrt(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Fixnum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_fixnum(f1, lit1))
        {
        e = make_flonum(std::sqrt((double)f1.value));
        }
      }
    }

  void fold_flsin(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Flonum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_flonum(f1, lit1))
        {
        e = make_flonum(std::sin(f1.value));
        }
      }
    }

  void fold_flcos(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Flonum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_flonum(f1, lit1))
        {
        e = make_flonum(std::cos(f1.value));
        }
      }
    }

  void fold_fltan(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Flonum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_flonum(f1, lit1))
        {
        e = make_flonum(std::tan(f1.value));
        }
      }
    }

  void fold_flasin(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Flonum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_flonum(f1, lit1))
        {
        e = make_flonum(std::asin(f1.value));
        }
      }
    }

  void fold_flacos(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Flonum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_flonum(f1, lit1))
        {
        e = make_flonum(std::acos(f1.value));
        }
      }
    }

  void fold_flatan1(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Flonum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_flonum(f1, lit1))
        {
        e = make_flonum(std::atan(f1.value));
        }
      }
    }

  void fold_fllog(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Flonum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_flonum(f1, lit1))
        {
        e = make_flonum(std::log(f1.value));
        }
      }
    }

  void fold_flround(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Flonum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_flonum(f1, lit1))
        {
        e = make_flonum(std::round(f1.value));
        }
      }
    }

  void fold_fltruncate(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Flonum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_flonum(f1, lit1))
        {
        e = make_fixnum((int64_t)(f1.value));
        }
      }
    }

  void fold_flsqrt(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Flonum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_flonum(f1, lit1))
        {
        e = make_flonum(std::sqrt(f1.value));
        }
      }
    }

  void fold_bitwise_and(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Fixnum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_fixnum(f1, lit1) && is_fixnum(f2, lit2))
        {
        e = make_fixnum(f1.value & f2.value);
        }
      }
    }

  void fold_bitwise_not(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Fixnum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_fixnum(f1, lit1))
        {
        e = make_fixnum(~f1.value);
        }
      }
    }

  void fold_bitwise_or(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Fixnum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_fixnum(f1, lit1) && is_fixnum(f2, lit2))
        {
        e = make_fixnum(f1.value | f2.value);
        }
      }
    }

  void fold_bitwise_xor(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Fixnum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_fixnum(f1, lit1) && is_fixnum(f2, lit2))
        {
        e = make_fixnum(f1.value ^ f2.value);
        }
      }
    }

  void fold_char_less(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Character ch1, ch2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_character(ch1, lit1) && is_character(ch2, lit2))
        {
        if (ch1.value < ch2.value)
          e = make_true();
        else
          e = make_false();
        }
      }
    }

  void fold_char_greater(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Character ch1, ch2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_character(ch1, lit1) && is_character(ch2, lit2))
        {
        if (ch1.value > ch2.value)
          e = make_true();
        else
          e = make_false();
        }
      }
    }

  void fold_char_leq(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Character ch1, ch2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_character(ch1, lit1) && is_character(ch2, lit2))
        {
        if (ch1.value <= ch2.value)
          e = make_true();
        else
          e = make_false();
        }
      }
    }

  void fold_char_geq(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Character ch1, ch2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_character(ch1, lit1) && is_character(ch2, lit2))
        {
        if (ch1.value >= ch2.value)
          e = make_true();
        else
          e = make_false();
        }
      }
    }

  void fold_char_equal(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Character ch1, ch2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_character(ch1, lit1) && is_character(ch2, lit2))
        {
        if (ch1.value == ch2.value)
          e = make_true();
        else
          e = make_false();
        }
      }
    }

  void fold_fixnum_to_flonum(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Fixnum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_fixnum(f1, lit1))
        {
        e = make_flonum((double)f1.value);
        }
      }
    }

  void fold_flonum_to_fixnum(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Flonum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_flonum(f1, lit1))
        {
        e = make_fixnum((int64_t)f1.value);
        }
      }
    }

  void fold_fixnum_to_char(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Fixnum f1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_fixnum(f1, lit1))
        {
        e = make_character((char)f1.value);
        }
      }
    }

  void fold_char_to_fixnum(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 1);
    Literal lit1;
    Character ch1;
    if (is_literal(lit1, p.arguments[0]))
      {
      if (is_character(ch1, lit1))
        {
        e = make_fixnum((int64_t)ch1.value);
        }
      }
    }

  void fold_arithmetic_shift(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Fixnum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_fixnum(f1, lit1) && is_fixnum(f2, lit2))
        {
        if (f2.value > 0)
          e = make_fixnum(f1.value << f2.value);
        else
          e = make_fixnum(f1.value >> (-f2.value));
        }
      }
    }

  void fold_remainder(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Fixnum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_fixnum(f1, lit1) && is_fixnum(f2, lit2))
        {
        if (f2.value)
          e = make_fixnum(f1.value % f2.value);
        else
          throw_error(division_by_zero);
        }
      }
    }

  void fold_quotient(PrimitiveCall& p, Expression& e)
    {
    assert(p.arguments.size() == 2);
    Literal lit1, lit2;
    Fixnum f1, f2;
    if (is_literal(lit1, p.arguments[0]) && is_literal(lit2, p.arguments[1]))
      {
      if (is_fixnum(f1, lit1) && is_fixnum(f2, lit2))
        {
        if (f2.value)
          e = make_fixnum(f1.value / f2.value);
        else
          throw_error(division_by_zero);
        }
      }
    }

  struct constant_folding_visitor : public base_visitor<constant_folding_visitor>
    {

    typedef void(*fun_ptr)(PrimitiveCall&, Expression&);

    std::map<std::string, fun_ptr> get_primitives_that_can_be_folded() const
      {
      std::map<std::string, fun_ptr> ptcbf;

      ptcbf.insert(std::pair<std::string, fun_ptr>("##remainder", &fold_remainder));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##quotient", &fold_quotient));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##arithmetic-shift", &fold_arithmetic_shift));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##%quiet-undefined", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##%undefined", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##vector-length", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fixnum->flonum", &fold_fixnum_to_flonum));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##flonum->fixnum", &fold_flonum_to_fixnum));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fixnum->char", &fold_fixnum_to_char));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##char->fixnum", &fold_char_to_fixnum));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##char<?", &fold_char_less));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##char>?", &fold_char_greater));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##char<=?", &fold_char_leq));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##char>=?", &fold_char_geq));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##char=?", &fold_char_equal));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##bitwise-and", &fold_bitwise_and));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##bitwise-not", &fold_bitwise_not));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##bitwise-or", &fold_bitwise_or));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##bitwise-xor", &fold_bitwise_xor));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-sign", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-mantissa", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-exponent", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-pi", &fold_pi));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fxsin", &fold_fxsin));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fxcos", &fold_fxcos));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fxtan", &fold_fxtan));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fxasin", &fold_fxasin));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fxacos", &fold_fxacos));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fxatan1", &fold_fxatan1));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fxlog", &fold_fxlog));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fxround", &fold_fxround));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fxtruncate", &fold_fxtruncate));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fxsqrt", &fold_fxsqrt));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-flsin", &fold_flsin));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-flcos", &fold_flcos));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fltan", &fold_fltan));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-flasin", &fold_flasin));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-flacos", &fold_flacos));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-flatan1", &fold_flatan1));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fllog", &fold_fllog));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-flround", &fold_flround));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fltruncate", &fold_fltruncate));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-flsqrt", &fold_flsqrt));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##memq", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##assq", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##not", &fold_not));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##cons", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##car", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##cdr", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##set-car!", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##set-cdr!", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##eq?", &fold_is_eq));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fxadd1", &fold_fx_add1));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fxsub1", &fold_fx_sub1));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fxmax", &fold_fx_max));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fxmin", &fold_fx_min));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fxzero?", &fold_fx_is_zero));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fx+", &fold_fx_add));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fx-", &fold_fx_sub));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fx*", &fold_fx_mul));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fx/", &fold_fx_div));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fx<?", &fold_fx_less));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fx>?", &fold_fx_greater));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fx<=?", &fold_fx_leq));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fx>=?", &fold_fx_geq));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fx=?", &fold_fx_equal));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fx!=?", &fold_fx_not_equal));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fladd1", &fold_fl_add1));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##flsub1", &fold_fl_sub1));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##flmax", &fold_fl_max));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##flmin", &fold_fl_min));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##flzero?", &fold_fl_is_zero));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fl+", &fold_fl_add));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fl-", &fold_fl_sub));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fl*", &fold_fl_mul));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fl/", &fold_fl_div));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fl<?", &fold_fl_less));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fl>?", &fold_fl_greater));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fl<=?", &fold_fl_leq));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fl>=?", &fold_fl_geq));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fl=?", &fold_fl_equal));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fl!=?", &fold_fl_not_equal));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fixnum?", &fold_is_fixnum));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##flonum?", &fold_is_flonum));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##pair?", &fold_is_pair));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##vector?", &fold_is_vector));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##string?", &fold_is_string));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##symbol?", &fold_is_symbol));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##closure?", &fold_is_closure));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##procedure?", &fold_is_procedure));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##boolean?", &fold_is_boolean));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##null?", &fold_is_null));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##eof-object?", &fold_is_eof_object));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##char?", &fold_is_char));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##promise?", &fold_is_promise));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##port?", &fold_is_port));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##input-port?", &fold_is_input_port));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##output-port?", &fold_is_output_port));

      return ptcbf;
      }

    void _fold_primitive(PrimitiveCall& p, Expression& e)
      {
      static std::map<std::string, fun_ptr> ptcbf = get_primitives_that_can_be_folded();

      auto it = ptcbf.find(p.primitive_name);
      if (it != ptcbf.end())
        {
        it->second(p, e);
        }
      }

    bool is_nonmutable(const Expression& e)
      {
      if (std::holds_alternative<Literal>(e))
        return true;
      if (std::holds_alternative<Variable>(e))
        return true;
      if (std::holds_alternative<PrimitiveCall>(e))
        {
        const PrimitiveCall& p = std::get<PrimitiveCall>(e);
        for (const auto& arg : p.arguments)
          if (!is_nonmutable(arg))
            return false;
        return true;
        }
      if (std::holds_alternative<Quote>(e))
        return true;
      if (std::holds_alternative<If>(e))
        {
        const If& i = std::get<If>(e);
        for (const auto& arg : i.arguments)
          if (!is_nonmutable(arg))
            return false;
        return true;
        }
      if (std::holds_alternative<Let>(e))
        {
        const Let& l = std::get<Let>(e);
        for (const auto& arg : l.bindings)
          if (!is_nonmutable(arg.second))
            return false;
        return is_nonmutable(l.body.front());
        }
      if (std::holds_alternative<Begin>(e))
        {
        const Begin& b = std::get<Begin>(e);
        for (const auto& arg : b.arguments)
          if (!is_nonmutable(arg))
            return false;
        return true;
        }
      if (std::holds_alternative<Nop>(e))
        {
        return true;
        }
      if (std::holds_alternative<ForeignCall>(e))
        {
        const ForeignCall& p = std::get<ForeignCall>(e);
        for (const auto& arg : p.arguments)
          if (!is_nonmutable(arg))
            return false;
        return true;
        }
      // set and funcall are mutable operations (i.e. ops that have side effects)
      return false;
      }

    void _fold_if(If& i, Expression& e)
      {
      assert(i.arguments.size() == 3);
      if (std::holds_alternative<Literal>(i.arguments.front()))
        {
        Literal& lit = std::get<Literal>(i.arguments.front());
        if (std::holds_alternative<False>(lit))
          {
          auto new_val(std::move(i.arguments[2]));
          e = std::move(new_val);
          }
        else
          {
          auto new_val(std::move(i.arguments[1]));
          e = std::move(new_val);
          }
        }

      if (std::holds_alternative<If>(e))
        {
        If& f = std::get<If>(e);
        Literal lit1, lit2;
        if (is_literal(lit1, f.arguments[1]) && is_literal(lit2, f.arguments[2]))
          {
          Fixnum f1, f2;
          Flonum fl1, fl2;
          Character ch1, ch2;
          Symbol s1, s2;
          String st1, st2;
          if (is_fixnum(f1, lit1) && is_fixnum(f2, lit2) && f1.value == f2.value)
            {
            if (is_nonmutable(f.arguments[0]))
              e = lit1;
            }
          else if (is_flonum(fl1, lit1) && is_flonum(fl2, lit2) && fl1.value == fl2.value)
            {
            if (is_nonmutable(f.arguments[0]))
              e = lit1;
            }
          else if (is_symbol(s1, lit1) && is_symbol(s2, lit2) && s1.value == s2.value)
            {
            if (is_nonmutable(f.arguments[0]))
              e = lit1;
            }
          else if (is_string(st1, lit1) && is_string(st2, lit2) && st1.value == st2.value)
            {
            if (is_nonmutable(f.arguments[0]))
              e = lit1;
            }
          else if (is_true(lit1) && is_true(lit2))
            {
            if (is_nonmutable(f.arguments[0]))
              e = lit1;
            }
          else if (is_false(lit1) && is_false(lit2))
            {
            if (is_nonmutable(f.arguments[0]))
              e = lit1;
            }
          else if (is_nil(lit1) && is_nil(lit2))
            {
            if (is_nonmutable(f.arguments[0]))
              e = lit1;
            }
          }
        }

      if (std::holds_alternative<If>(e))
        {
        // rewrite if (if (##flonum x) #t #f ) ... to if (##flonum x) ...
        If& f = std::get<If>(e);
        if (std::holds_alternative<If>(f.arguments[0]))
          {
          If& f2 = std::get<If>(f.arguments[0]);
          Literal lit1, lit2;
          if (is_literal(lit1, f2.arguments[1]) && is_literal(lit2, f2.arguments[2]))
            {
            if (is_true(lit1) && is_false(lit2))
              {
              Expression expr(std::move(f2.arguments[0]));
              f.arguments[0] = std::move(expr);
              }
            }
          }
        }

      }

    virtual void _postvisit(Expression& e)
      {
      if (std::holds_alternative<PrimitiveCall>(e))
        {
        _fold_primitive(std::get<PrimitiveCall>(e), e);
        }
      else if (std::holds_alternative<If>(e))
        {
        _fold_if(std::get<If>(e), e);
        }
      }

    };


  struct constant_folding_state
    {
    enum struct e_cf_state
      {
      T_INIT,
      T_STEP_1
      };
    Expression* p_expr;
    e_cf_state state;

    constant_folding_state() : p_expr(nullptr), state(e_cf_state::T_INIT) {}
    constant_folding_state(Expression* ip_expr) : p_expr(ip_expr), state(e_cf_state::T_INIT) {}
    constant_folding_state(Expression* ip_expr, e_cf_state s) : p_expr(ip_expr), state(s) {}
    };

  struct constant_folding_helper
    {
    std::vector<constant_folding_state> expressions;


    typedef void(*fun_ptr)(PrimitiveCall&, Expression&);

    std::map<std::string, fun_ptr> get_primitives_that_can_be_folded() const
      {
      std::map<std::string, fun_ptr> ptcbf;

      ptcbf.insert(std::pair<std::string, fun_ptr>("##remainder", &fold_remainder));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##quotient", &fold_quotient));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##arithmetic-shift", &fold_arithmetic_shift));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##%quiet-undefined", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##%undefined", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##vector-length", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fixnum->flonum", &fold_fixnum_to_flonum));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##flonum->fixnum", &fold_flonum_to_fixnum));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fixnum->char", &fold_fixnum_to_char));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##char->fixnum", &fold_char_to_fixnum));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##char<?", &fold_char_less));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##char>?", &fold_char_greater));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##char<=?", &fold_char_leq));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##char>=?", &fold_char_geq));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##char=?", &fold_char_equal));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##bitwise-and", &fold_bitwise_and));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##bitwise-not", &fold_bitwise_not));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##bitwise-or", &fold_bitwise_or));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##bitwise-xor", &fold_bitwise_xor));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-sign", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-mantissa", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-exponent", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-pi", &fold_pi));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fxsin", &fold_fxsin));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fxcos", &fold_fxcos));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fxtan", &fold_fxtan));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fxasin", &fold_fxasin));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fxacos", &fold_fxacos));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fxatan1", &fold_fxatan1));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fxlog", &fold_fxlog));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fxround", &fold_fxround));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fxtruncate", &fold_fxtruncate));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fxsqrt", &fold_fxsqrt));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-flsin", &fold_flsin));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-flcos", &fold_flcos));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fltan", &fold_fltan));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-flasin", &fold_flasin));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-flacos", &fold_flacos));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-flatan1", &fold_flatan1));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fllog", &fold_fllog));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-flround", &fold_flround));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-fltruncate", &fold_fltruncate));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##ieee754-flsqrt", &fold_flsqrt));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##memq", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##assq", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##not", &fold_not));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##cons", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##car", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##cdr", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##set-car!", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##set-cdr!", &fold_dummy));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##eq?", &fold_is_eq));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fxadd1", &fold_fx_add1));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fxsub1", &fold_fx_sub1));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fxmax", &fold_fx_max));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fxmin", &fold_fx_min));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fxzero?", &fold_fx_is_zero));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fx+", &fold_fx_add));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fx-", &fold_fx_sub));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fx*", &fold_fx_mul));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fx/", &fold_fx_div));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fx<?", &fold_fx_less));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fx>?", &fold_fx_greater));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fx<=?", &fold_fx_leq));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fx>=?", &fold_fx_geq));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fx=?", &fold_fx_equal));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fx!=?", &fold_fx_not_equal));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fladd1", &fold_fl_add1));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##flsub1", &fold_fl_sub1));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##flmax", &fold_fl_max));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##flmin", &fold_fl_min));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##flzero?", &fold_fl_is_zero));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fl+", &fold_fl_add));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fl-", &fold_fl_sub));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fl*", &fold_fl_mul));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fl/", &fold_fl_div));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fl<?", &fold_fl_less));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fl>?", &fold_fl_greater));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fl<=?", &fold_fl_leq));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fl>=?", &fold_fl_geq));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fl=?", &fold_fl_equal));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fl!=?", &fold_fl_not_equal));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##fixnum?", &fold_is_fixnum));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##flonum?", &fold_is_flonum));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##pair?", &fold_is_pair));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##vector?", &fold_is_vector));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##string?", &fold_is_string));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##symbol?", &fold_is_symbol));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##closure?", &fold_is_closure));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##procedure?", &fold_is_procedure));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##boolean?", &fold_is_boolean));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##null?", &fold_is_null));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##eof-object?", &fold_is_eof_object));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##char?", &fold_is_char));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##promise?", &fold_is_promise));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##port?", &fold_is_port));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##input-port?", &fold_is_input_port));
      ptcbf.insert(std::pair<std::string, fun_ptr>("##output-port?", &fold_is_output_port));

      return ptcbf;
      }

    void _fold_primitive(PrimitiveCall& p, Expression& e)
      {
      static std::map<std::string, fun_ptr> ptcbf = get_primitives_that_can_be_folded();

      auto it = ptcbf.find(p.primitive_name);
      if (it != ptcbf.end())
        {
        it->second(p, e);
        }
      }

    bool is_nonmutable(const Expression& e)
      {
      if (std::holds_alternative<Literal>(e))
        return true;
      if (std::holds_alternative<Variable>(e))
        return true;
      if (std::holds_alternative<PrimitiveCall>(e))
        {
        const PrimitiveCall& p = std::get<PrimitiveCall>(e);
        for (const auto& arg : p.arguments)
          if (!is_nonmutable(arg))
            return false;
        return true;
        }
      if (std::holds_alternative<Quote>(e))
        return true;
      if (std::holds_alternative<If>(e))
        {
        const If& i = std::get<If>(e);
        for (const auto& arg : i.arguments)
          if (!is_nonmutable(arg))
            return false;
        return true;
        }
      if (std::holds_alternative<Let>(e))
        {
        const Let& l = std::get<Let>(e);
        for (const auto& arg : l.bindings)
          if (!is_nonmutable(arg.second))
            return false;
        return is_nonmutable(l.body.front());
        }
      if (std::holds_alternative<Begin>(e))
        {
        const Begin& b = std::get<Begin>(e);
        for (const auto& arg : b.arguments)
          if (!is_nonmutable(arg))
            return false;
        return true;
        }
      if (std::holds_alternative<Nop>(e))
        {
        return true;
        }
      if (std::holds_alternative<ForeignCall>(e))
        {
        const ForeignCall& p = std::get<ForeignCall>(e);
        for (const auto& arg : p.arguments)
          if (!is_nonmutable(arg))
            return false;
        return true;
        }
      // set and funcall are mutable operations (i.e. ops that have side effects)
      return false;
      }

    void _fold_if(If& i, Expression& e)
      {
      assert(i.arguments.size() == 3);
      if (std::holds_alternative<Literal>(i.arguments.front()))
        {
        Literal& lit = std::get<Literal>(i.arguments.front());
        if (std::holds_alternative<False>(lit))
          {
          auto new_val(std::move(i.arguments[2]));
          e = std::move(new_val);
          }
        else
          {
          auto new_val(std::move(i.arguments[1]));
          e = std::move(new_val);
          }
        }

      if (std::holds_alternative<If>(e))
        {
        If& f = std::get<If>(e);
        Literal lit1, lit2;
        if (is_literal(lit1, f.arguments[1]) && is_literal(lit2, f.arguments[2]))
          {
          Fixnum f1, f2;
          Flonum fl1, fl2;
          Character ch1, ch2;
          Symbol s1, s2;
          String st1, st2;
          if (is_fixnum(f1, lit1) && is_fixnum(f2, lit2) && f1.value == f2.value)
            {
            if (is_nonmutable(f.arguments[0]))
              e = lit1;
            }
          else if (is_flonum(fl1, lit1) && is_flonum(fl2, lit2) && fl1.value == fl2.value)
            {
            if (is_nonmutable(f.arguments[0]))
              e = lit1;
            }
          else if (is_symbol(s1, lit1) && is_symbol(s2, lit2) && s1.value == s2.value)
            {
            if (is_nonmutable(f.arguments[0]))
              e = lit1;
            }
          else if (is_string(st1, lit1) && is_string(st2, lit2) && st1.value == st2.value)
            {
            if (is_nonmutable(f.arguments[0]))
              e = lit1;
            }
          else if (is_true(lit1) && is_true(lit2))
            {
            if (is_nonmutable(f.arguments[0]))
              e = lit1;
            }
          else if (is_false(lit1) && is_false(lit2))
            {
            if (is_nonmutable(f.arguments[0]))
              e = lit1;
            }
          else if (is_nil(lit1) && is_nil(lit2))
            {
            if (is_nonmutable(f.arguments[0]))
              e = lit1;
            }
          }
        }

      if (std::holds_alternative<If>(e))
        {
        // rewrite if (if (##flonum x) #t #f ) ... to if (##flonum x) ...
        If& f = std::get<If>(e);
        if (std::holds_alternative<If>(f.arguments[0]))
          {
          If& f2 = std::get<If>(f.arguments[0]);
          Literal lit1, lit2;
          if (is_literal(lit1, f2.arguments[1]) && is_literal(lit2, f2.arguments[2]))
            {
            if (is_true(lit1) && is_false(lit2))
              {
              Expression expr(std::move(f2.arguments[0]));
              f.arguments[0] = std::move(expr);
              }
            }
          }
        }

      }

    void treat_expressions()
      {
      while (!expressions.empty())
        {
        constant_folding_state cf_state = expressions.back();
        expressions.pop_back();
        Expression& e = *cf_state.p_expr;
        if (std::holds_alternative<Literal>(e))
          {

          }
        else if (std::holds_alternative<Variable>(e))
          {

          }
        else if (std::holds_alternative<Nop>(e))
          {

          }
        else if (std::holds_alternative<Quote>(e))
          {

          }
        else if (std::holds_alternative<Set>(e))
          {
          //Set& s = std::get<Set>(e);
          expressions.push_back(&std::get<Set>(e).value.front());
          }
        else if (std::holds_alternative<If>(e))
          {
          if (cf_state.state == constant_folding_state::e_cf_state::T_INIT)
            {
            expressions.emplace_back(&e, constant_folding_state::e_cf_state::T_STEP_1);
            for (auto rit = std::get<If>(e).arguments.rbegin(); rit != std::get<If>(e).arguments.rend(); ++rit)
              expressions.push_back(&(*rit));
            }
          else
            {
            _fold_if(std::get<If>(e), e);
            }
          }
        else if (std::holds_alternative<Begin>(e))
          {
          for (auto rit = std::get<Begin>(e).arguments.rbegin(); rit != std::get<Begin>(e).arguments.rend(); ++rit)
            expressions.push_back(&(*rit));
          }
        else if (std::holds_alternative<PrimitiveCall>(e))
          {
          if (cf_state.state == constant_folding_state::e_cf_state::T_INIT)
            {
            expressions.emplace_back(&e, constant_folding_state::e_cf_state::T_STEP_1);
            for (auto rit = std::get<PrimitiveCall>(e).arguments.rbegin(); rit != std::get<PrimitiveCall>(e).arguments.rend(); ++rit)
              expressions.push_back(&(*rit));
            }
          else
            {
            _fold_primitive(std::get<PrimitiveCall>(e), e);
            }
          }
        else if (std::holds_alternative<ForeignCall>(e))
          {
          for (auto rit = std::get<ForeignCall>(e).arguments.rbegin(); rit != std::get<ForeignCall>(e).arguments.rend(); ++rit)
            expressions.push_back(&(*rit));
          }
        else if (std::holds_alternative<Lambda>(e))
          {
          Lambda& l = std::get<Lambda>(e);
          expressions.push_back(&l.body.front());
          }
        else if (std::holds_alternative<FunCall>(e))
          {
          expressions.push_back(&std::get<FunCall>(e).fun.front());
          for (auto rit = std::get<FunCall>(e).arguments.rbegin(); rit != std::get<FunCall>(e).arguments.rend(); ++rit)
            expressions.push_back(&(*rit));
          }
        else if (std::holds_alternative<Let>(e))
          {
          Let& l = std::get<Let>(e);
          expressions.push_back(&l.body.front());
          for (auto rit = l.bindings.rbegin(); rit != l.bindings.rend(); ++rit)
            expressions.push_back(&(*rit).second);
          }
        else
          throw std::runtime_error("Compiler error!: constant folding: not implemented");
        }
      }
    };
  }

void constant_folding(Program& prog)
  {

  //constant_folding_visitor cfv;
  //visitor<Program, constant_folding_visitor>::visit(prog, &cfv);

  constant_folding_helper cfh;
  for (auto& expr : prog.expressions)
    cfh.expressions.push_back(&expr);
  std::reverse(cfh.expressions.begin(), cfh.expressions.end());
  cfh.treat_expressions();

  prog.constant_folded = true;
  }

SKIWI_END
