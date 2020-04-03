#include "quote_conversion.h"

#include <cassert>
#include <sstream>
#include <inttypes.h>
#include <stdio.h>

#include "compile_error.h"
#include "visitor.h"
#include "types.h"

SKIWI_BEGIN

namespace
  {
  int64_t s64(const char *s)
    {
    uint64_t i;
    char c;
    sscanf(s, "%" SCNu64 "%c", &i, &c);
    return (int64_t)i;
    }

  struct quote_conversion_visitor : public base_visitor<quote_conversion_visitor>
    {
    std::map<std::string, cell>* new_quotes;
    std::map<std::string, uint64_t>* quote_to_index;
    std::vector<Expression> quotes;
    uint64_t index;
    environment_map env;
    repl_data* rd;
    context* p_ctxt;

    virtual bool _previsit(Expression& e)
      {
      if (std::holds_alternative<Quote>(e))
        _convert(e);
      return true;
      }

    bool is_char(const cell& c, char& ch)
      {
      if (c.value.substr(0, 2) == "#\\")
        {
        std::string charval = c.value.substr(2);
        if (charval.empty())
          {
          ch = 32;
          return true;
          }
        else if (charval.length() == 1)
          {
          ch = charval[0];
          return true;
          }
        else if (charval == "backspace")
          {
          ch = 8;
          return true;
          }
        else if (charval == "tab")
          {
          ch = 9;
          return true;
          }
        else if (charval == "newline")
          {
          ch = 10;
          return true;
          }
        else if (charval == "linefeed")
          {
          ch = 10;
          return true;
          }
        else if (charval == "vtab")
          {
          ch = 11;
          return true;
          }
        else if (charval == "page")
          {
          ch = 12;
          return true;
          }
        else if (charval == "return")
          {
          ch = 13;
          return true;
          }
        else if (charval == "space")
          {
          ch = 32;
          return true;
          }
        }
      return false;
      }

    Expression _parse(cell& in)
      {
      Expression result;
      Expression* last_result = nullptr;
      std::vector<Expression*> parent;
      std::vector<cell> todo;
      todo.push_back(in);
      while (!todo.empty())
        {
        Expression res;
        cell c = todo.back();
        todo.pop_back();
        size_t parents_to_add = 0;
        if (!parent.empty())
          {
          last_result = parent.back();
          parent.pop_back();
          }
        switch (c.type)
          {
          case ct_fixnum:
          {
          Fixnum f;
          f.value = s64(c.value.c_str());
          Literal l = f;
          res = l;
          break;
          }
          case ct_flonum:
          {
          Flonum f;
          f.value = atof(c.value.c_str());
          Literal l = f;
          res = l;
          break;
          }
          case ct_string:
          {
          std::string str = c.value.substr(1, c.value.size() - 2);
          String s;
          s.value = str;
          Literal l = s;
          res = l;
          break;
          }
          case ct_symbol:
          {
          char chval;
          if (c == true_sym)
            {
            True t;
            Literal l = t;
            res = l;
            }
          else if (c == false_sym)
            {
            False f;
            Literal l = f;
            res = l;
            }
          else if (is_char(c, chval))
            {
            Character ch;
            ch.value = chval;
            Literal l = ch;
            res = l;
            }
          else
            {
            FunCall f;
            Variable v;
            v.name = "string->symbol";
            String s;
            s.value = c.value;
            Literal lit = s;
            Expression expr = lit;
            f.fun.push_back(v);
            f.arguments.push_back(expr);
            res = f;
            }
          break;
          }
          case ct_pair:
          {
          if (c == nil_sym)
            {
            Nil n;
            Literal l = n;
            res = l;
            }
          else
            {
            if (c.pair.size() != 2)
              throw_error(invalid_number_of_arguments);
            PrimitiveCall p;
            p.primitive_name = "cons";
            todo.push_back(c.pair[1]);
            todo.push_back(c.pair[0]);
            parents_to_add = 2;
            res = p;
            }
          break;
          }
          case ct_vector:
          {
          PrimitiveCall p;
          p.primitive_name = "vector";
          for (auto it = c.vec.rbegin(); it != c.vec.rend(); ++it)
            {
            todo.push_back(*it);
            }
          parents_to_add = c.vec.size();
          res = p;
          break;
          }
          default:
          {
          throw_error(not_implemented);
          }
          }
        if (last_result)
          {
          std::get<PrimitiveCall>(*last_result).arguments.push_back(res);       
          parent.insert(parent.end(), parents_to_add, &std::get<PrimitiveCall>(*last_result).arguments.back());
          }
        else
          {
          result = res; 
          parent.insert(parent.end(), parents_to_add, &result);
          }
        }
      return result;
      }

    /*
    // This is the recursive version of the above method. Can give stack overflow.
    Expression _parse(cell& c)
      {
      char chval;
      switch (c.type)
        {
        case ct_fixnum:
        {
        Fixnum f;
        f.value = s64(c.value.c_str());
        Literal l = f;
        Expression expr(l);
        return expr;
        }
        case ct_flonum:
        {
        Flonum f;
        f.value = atof(c.value.c_str());
        Literal l = f;
        Expression expr(l);
        return expr;
        }
        case ct_string:
        {
        std::string str = c.value.substr(1, c.value.size() - 2);
        String s;
        s.value = str;
        Literal l = s;
        Expression expr(l);
        return expr;
        }
        case ct_symbol:
        {
        if (c == true_sym)
          {
          True t;
          Literal l = t;
          Expression expr(l);
          return expr;
          }
        else if (c == false_sym)
          {
          False f;
          Literal l = f;
          Expression expr(l);
          return expr;
          }
        else if (is_char(c, chval))
          {
          Character ch;
          ch.value = chval;
          Literal l = ch;
          Expression expr(l);
          return expr;
          }
        else
          {
          FunCall f;
          Variable v;
          v.name = "string->symbol";
          String s;
          s.value = c.value;
          Literal lit = s;
          Expression expr = lit;
          f.fun.push_back(v);
          f.arguments.push_back(expr);
          Expression res = f;
          return res;
          }
        }
        case ct_pair:
        {
        if (c == nil_sym)
          {
          Nil n;
          Literal l = n;
          Expression expr(l);
          return expr;
          }
        else
          {
          if (c.pair.size() != 2)
            throw_error(invalid_number_of_arguments);
          PrimitiveCall p;
          p.primitive_name = "cons";
          p.arguments.push_back(_parse(c.pair[0]));
          p.arguments.push_back(_parse(c.pair[1]));
          Expression expr(p);
          return expr;
          }
        }
        case ct_vector:
        {
        PrimitiveCall p;
        p.primitive_name = "vector";
        for (auto& arg : c.vec)
          p.arguments.push_back(_parse(arg));
        Expression expr(p);
        return expr;
        }
        default:
        {
        throw_error(not_implemented);
        }
        }
      return Nop();
      }
      */
    void _convert(Expression& e)
      {
      assert(std::holds_alternative<Quote>(e));
      Quote& q = std::get<Quote>(e);
      std::stringstream str;
      str << q.arg;
      auto it = new_quotes->find(str.str());
      if (it != new_quotes->end())
        {
        auto it2 = quote_to_index->find(str.str());
        if (it2 == quote_to_index->end())
          {
          Set s;
          s.originates_from_quote = true;
          std::stringstream qs;
          qs << "#%q" << index;
          quote_to_index->insert(std::pair<std::string, uint64_t>(str.str(), index));
          ++index;
          s.name = qs.str();
          s.value.push_back(_parse(q.arg));
          quotes.push_back(s);
          environment_entry ee;
          ee.st = environment_entry::st_global;
          ee.pos = (uint64_t)rd->global_index * 8;
          ++(rd->global_index);
          env->push_outer(s.name, ee);
          uint64_t* addr = p_ctxt->globals + (ee.pos >> 3);
          *addr = reserved_tag; // This is a new address, previously equal to unalloc_tag. To avoid that gc stops here when cleaning, we change its value to reserved_tag.
          }
        }
      assert(quote_to_index->find(str.str()) != quote_to_index->end());
      Variable v;
      std::stringstream qs;
      qs << "#%q" << quote_to_index->find(str.str())->second;
      v.name = qs.str();
      e = v;
      }
    };
  }

void quote_conversion(Program& prog, repl_data& data, environment_map& env, context& ctxt)
  {
  assert(!prog.cps_converted);
  assert(prog.alpha_converted);
  assert(prog.quotes_collected);

  quote_conversion_visitor qcv;
  qcv.new_quotes = &prog.quotes;
  qcv.quote_to_index = &data.quote_to_index;
  qcv.index = data.alpha_conversion_index;
  qcv.env = env;
  qcv.rd = &data;
  qcv.p_ctxt = &ctxt;

  visitor<Program, quote_conversion_visitor>::visit(prog, &qcv);
  data.alpha_conversion_index = qcv.index;
  prog.quotes_converted = true;
  if (prog.expressions.empty())
    return;
  if (std::holds_alternative<Begin>(prog.expressions.front()))
    {
    std::get<Begin>(prog.expressions.front()).arguments.insert(std::get<Begin>(prog.expressions.front()).arguments.begin(), qcv.quotes.begin(), qcv.quotes.end());
    }
  else
    {
    Begin new_begin;
    new_begin.arguments = qcv.quotes;
    new_begin.arguments.insert(new_begin.arguments.end(), prog.expressions.begin(), prog.expressions.end());
    prog.expressions.clear();
    prog.expressions.push_back(new_begin);
    }
  }

SKIWI_END
