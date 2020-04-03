#include "define_conversion.h"
#include "compile_error.h"
#include "debug_find.h"
#include "single_begin_conversion.h"
#include "visitor.h"
#include <algorithm>
#include <cassert>
#include <sstream>

SKIWI_BEGIN

namespace
  {
  namespace
    {
    Variable _make_var(const std::string& name)
      {
      Variable v;
      v.name = name;
      return v;
      }

    bool is_name(const Expression& expr)
      {
      if (std::holds_alternative<PrimitiveCall>(expr))
        return std::get<PrimitiveCall>(expr).as_object;
      return std::holds_alternative<Variable>(expr);
      }

    std::string get_name(const Expression& expr)
      {
      if (std::holds_alternative<Variable>(expr))
        return std::get<Variable>(expr).name;
      if (std::holds_alternative<PrimitiveCall>(expr))
        return std::get<PrimitiveCall>(expr).primitive_name;
      return "";
      }
    }
  struct define_conversion_visitor : public base_visitor<define_conversion_visitor>
    {
    void rewrite(PrimitiveCall& p)
      {
      if (p.arguments.size() < 2)
        throw_error(p.line_nr, p.column_nr, p.filename, invalid_number_of_arguments);
      assert(p.arguments.size() >= 2);

      bool alternative_syntax = false;
      if (std::holds_alternative<FunCall>(p.arguments.front()))
        alternative_syntax = true;
      if (std::holds_alternative<PrimitiveCall>(p.arguments.front()) && !std::get<PrimitiveCall>(p.arguments.front()).as_object)
        alternative_syntax = true;

      if (alternative_syntax)
        {
        FunCall f;
        if (std::holds_alternative<FunCall>(p.arguments.front()))
          f = std::get<FunCall>(p.arguments.front());
        else
          {
          PrimitiveCall& prim = std::get<PrimitiveCall>(p.arguments.front());
          Variable v;
          v.name = prim.primitive_name;
          f.fun.push_back(v);
          f.arguments = prim.arguments;
          }
        if (!std::holds_alternative<Variable>(f.fun.front()))
          throw_error(p.line_nr, p.column_nr, p.filename, invalid_argument);
        Lambda lam;
        if (f.arguments.size() >= 2 && std::holds_alternative<Literal>(f.arguments[f.arguments.size()-2]))
          {
          Literal& lit = std::get<Literal>(f.arguments[f.arguments.size() - 2]);
          if (std::holds_alternative<Flonum>(lit))
            {
            Flonum& fl = std::get<Flonum>(lit);
            if (fl.value != 0.0)
              throw_error(p.line_nr, p.column_nr, p.filename, invalid_argument);
            for (size_t j = 0; j < f.arguments.size(); ++j)
              {
              if (j == (f.arguments.size() - 2))
                continue;
              if (!is_name(f.arguments[j]))
                throw_error(p.line_nr, p.column_nr, p.filename, invalid_argument);
              lam.variables.push_back(get_name(f.arguments[j]));
              }                        
            lam.variable_arity = true;
            }
          else
            throw_error(p.line_nr, p.column_nr, p.filename, invalid_argument);
          }
        else
          {
          for (const auto& arg : f.arguments)
            {
            if (!is_name(arg))
              throw_error(p.line_nr, p.column_nr, p.filename, invalid_argument);
            lam.variables.push_back(get_name(arg));
            }
          }
        Begin lam_begin;
        lam_begin.arguments.insert(lam_begin.arguments.begin(), p.arguments.begin()+1, p.arguments.end());
        lam.body.push_back(lam_begin);
        remove_nested_begin_expressions(lam);
        p.arguments.resize(2);
        p.arguments[0] = f.fun.front();
        p.arguments[1] = lam;
        visitor<PrimitiveCall, define_conversion_visitor>::visit(p, this);
        }
      }

    template <typename T>
    void _convert_internal_define(T& l)
      {
      std::vector<size_t> define_args;
      std::vector<PrimitiveCall> define_exprs;
      size_t index = 0;
      for (auto& arg : std::get<Begin>(l.body.front()).arguments)
        {
        if (std::holds_alternative<PrimitiveCall>(arg))
          {
          PrimitiveCall& p = std::get<PrimitiveCall>(arg);
          if (p.primitive_name == "define")
            {
            rewrite(p);

            define_args.push_back(index);
            define_exprs.push_back(p);
            }
          }
        ++index;
        }
      if (!define_args.empty())
        {
        auto& body = std::get<Begin>(l.body.front()).arguments;
        std::for_each(define_args.crbegin(), define_args.crend(), [&](size_t index) { body.erase(begin(body) + index); });
        Let new_let;
        new_let.bt = bt_letrec;
        for (const auto& def : define_exprs)
          {
          assert(def.primitive_name == "define");
          if (def.arguments.size() != 2)
            throw_error(def.line_nr, def.column_nr, def.filename, invalid_number_of_arguments);
          if (!std::holds_alternative<Variable>(def.arguments.front()))
            throw_error(def.line_nr, def.column_nr, def.filename, invalid_argument);
          new_let.bindings.emplace_back(std::get<Variable>(def.arguments.front()).name, def.arguments[1]);
          }
        new_let.body.push_back(l.body.front());
        body.clear();
        body.push_back(new_let);
        }
      }

    virtual void _postvisit(Let& l)
      {
      //remove_nested_begin_expressions(l);
      _convert_internal_define(l);
      }

    virtual void _postvisit(Lambda& l)
      {
      //remove_nested_begin_expressions(l);
      _convert_internal_define(l);
      }    

    void modify_exprs(std::vector<Expression>& exprs)
      {      
      for (auto& expr : exprs)
        {
        if (std::holds_alternative<PrimitiveCall>(expr))
          {
          PrimitiveCall& p = std::get<PrimitiveCall>(expr);
          if (p.primitive_name == "define")
            {            
            rewrite(p);
            if (p.arguments.size() != 2)
              throw_error(p.line_nr, p.column_nr, p.filename, invalid_number_of_arguments);
            Set s;
            if (std::holds_alternative<Variable>(p.arguments.front()))
              {
              s.name = std::get<Variable>(p.arguments.front()).name;
              }
            else if (std::holds_alternative<PrimitiveCall>(p.arguments.front()))
              {
              s.name = std::get<PrimitiveCall>(p.arguments.front()).primitive_name;
              }
            else            
              throw_error(p.line_nr, p.column_nr, p.filename, invalid_argument);
                       
            s.value.push_back(p.arguments[1]);
            s.originates_from_define = true;
            expr = s;
            }
          }
        }

      }

    virtual void _postvisit(Program& prog)
      {
      if (prog.expressions.size() == 1 && std::holds_alternative<Begin>(prog.expressions.front()))
        {
        Begin& b = std::get<Begin>(prog.expressions.front());
        modify_exprs(b.arguments);
        }
      else
        modify_exprs(prog.expressions);
      }
    };
  }

void define_conversion(Program& prog)
  {
  assert(!prog.simplified_to_core_forms);
  assert(!prog.closure_converted);
  remove_nested_begin_expressions(prog);  
  define_conversion_visitor dcv;
  visitor<Program, define_conversion_visitor>::visit(prog, &dcv);
  prog.define_converted = true;

  int line_nr, column_nr;
  std::string filename;
  if (find(prog, [&](const Expression& e) {if (std::holds_alternative<PrimitiveCall>(e) && std::get<PrimitiveCall>(e).primitive_name == "define")
    {
    line_nr = std::get<PrimitiveCall>(e).line_nr;
    column_nr = std::get<PrimitiveCall>(e).column_nr;
    filename = std::get<PrimitiveCall>(e).filename;
    return true;
    }
  else return false; }))
    {
    throw_error(line_nr, column_nr, filename, define_invalid_place);
    }

  }

SKIWI_END
