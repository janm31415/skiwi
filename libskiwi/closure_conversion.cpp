#include "closure_conversion.h"
#include "visitor.h"
#include <set>
#include <string>
#include <algorithm>
#include <iterator>
#include <sstream>

#include "concurrency.h"

SKIWI_BEGIN

/*
Source: Marc Feeley's 90 minute Scheme->C compiler
*/

namespace
  {

  Fixnum _make_fixnum(int64_t i)
    {
    Fixnum f;
    f.line_nr = -1;
    f.column_nr = -1;
    f.value = i;
    return f;
    }

  Variable _make_var(const std::string& name)
    {
    Variable v;
    v.name = name;
    return v;
    }

  struct replace_free_vars : public base_visitor<replace_free_vars>
    {
    replace_free_vars() : p_closure_variables(nullptr) {}

    const std::string* p_lambda_ref;

    const std::vector<std::string>* p_local_variables;
    const std::vector<std::string>* p_closure_variables;

    virtual bool _previsit(Lambda&)
      {
      return false;
      }

    virtual bool _previsit(Expression& e)
      {
      assert(!p_lambda_ref->empty());
      
      if (std::holds_alternative<Variable>(e))
        {
        Variable& v = std::get<Variable>(e);
        auto it1 = std::find(p_local_variables->begin(), p_local_variables->end(), v.name);
        if ((it1 == p_local_variables->end()))
          {
          auto it = std::find(p_closure_variables->begin(), p_closure_variables->end(), v.name);
          if (it != p_closure_variables->end())
            {
            PrimitiveCall p;
            p.primitive_name = "closure-ref";
            p.arguments.push_back(_make_var(*p_lambda_ref));
            p.arguments.push_back(_make_fixnum(1 + std::distance(p_closure_variables->begin(), it)));
            e = p;
            return false;
            }
          }
        return true;
        }
      return true;
      }
    };

  struct resolve_free_variables_visitor : public base_visitor< resolve_free_variables_visitor>
    {
    std::vector<Lambda*> active_lambda;

    virtual bool _previsit(Lambda& lam)
      {
      active_lambda.push_back(&lam);
      return true;
      }

    virtual bool _previsit(Expression& e)
      {
      if (std::holds_alternative<Variable>(e))
        {
        if (active_lambda.empty())
          return true;

        Variable& v = std::get<Variable>(e);
        auto it = std::find(active_lambda.back()->free_variables.begin(), active_lambda.back()->free_variables.end(), v.name);
        if (it != active_lambda.back()->free_variables.end())
          {
          PrimitiveCall p;
          p.primitive_name = "closure-ref";
          p.arguments.push_back(_make_var(active_lambda.back()->variables.front()));
          p.arguments.push_back(_make_fixnum(1 + std::distance(active_lambda.back()->free_variables.begin(), it)));
          e = PrimitiveCall();
          std::swap(std::get<PrimitiveCall>(e), p);
          return false;
          }
        }
      return true;
      }

    virtual void _postvisit(Lambda&)
      {
      active_lambda.pop_back();
      }
    };

  struct closure_conversion_visitor : public base_visitor<closure_conversion_visitor>
    {
    uint64_t self_index;

    virtual void _postvisit(Expression& e)
      {
      if (std::holds_alternative<Lambda>(e))
        {
        Lambda& l = std::get<Lambda>(e);
        std::stringstream str;
        str << "#%self" << self_index;
        std::string lambda_ref = str.str();
        ++self_index;
        l.variables.insert(l.variables.begin(), lambda_ref);
        
        /*
        if (!l.free_variables.empty())
          {
          replace_free_vars rfv;
          rfv.p_lambda_ref = &lambda_ref;
          rfv.p_local_variables = &(l.variables);
          rfv.p_closure_variables = &(l.free_variables);
          visitor<Expression, replace_free_vars>::visit(l.body.front(), &rfv);
          }
        */

        PrimitiveCall p;
        p.primitive_name = "closure";
        p.arguments.emplace_back(Lambda());
        for (const auto& free_var : l.free_variables)
          {
          p.arguments.emplace_back(_make_var(free_var));
          std::get<Variable>(p.arguments.back()).line_nr = l.line_nr; // could be improved, but good guess for now
          std::get<Variable>(p.arguments.back()).column_nr = l.column_nr;
          }
        std::swap(std::get<Lambda>(p.arguments[0]), l); // this is a very substantial speedup trick!!
        e = PrimitiveCall();
        std::swap(std::get<PrimitiveCall>(e), p);
        }
      }

    /*
    virtual bool _previsit(Expression& e)
      {
      if (std::holds_alternative<Lambda>(e))
        {
        Lambda& l = std::get<Lambda>(e);
        visitor<Expression, closure_conversion_visitor>::visit(l.body.front(), this);
        std::stringstream str;
        str << "#%self" << self_index;
        std::string lambda_ref = str.str();
        ++self_index;
        l.variables.insert(l.variables.begin(), lambda_ref);

        if (!l.free_variables.empty())
          {
          replace_free_vars rfv;
          rfv.p_lambda_ref = &lambda_ref;
          rfv.p_local_variables = &(l.variables);
          rfv.p_closure_variables = &(l.free_variables);
          visitor<Expression, replace_free_vars>::visit(l.body.front(), &rfv);
          }

        PrimitiveCall p;
        p.primitive_name = "closure";
        p.arguments.push_back(l);
        for (const auto& free_var : l.free_variables)
          {
          p.arguments.push_back(_make_var(free_var));
          std::get<Variable>(p.arguments.back()).line_nr = l.line_nr; // could be improved, but good guess for now
          std::get<Variable>(p.arguments.back()).column_nr = l.column_nr;
          }
        e = p;
        return false;
        }
      return true;
      }
      */
    };
  }

void closure_conversion(Program& prog, const compiler_options& ops)
  {
  assert(prog.free_variables_analysed);
  /*
  closure_conversion_visitor ccv;
  ccv.self_index = 0;
  visitor<Program, closure_conversion_visitor>::visit(prog, &ccv);
  */
  if (prog.expressions.size() == 1)
    {
    Expression& e = prog.expressions.front();
    if (std::holds_alternative<Begin>(e))
      {
      Begin& beg = std::get<Begin>(e);
      size_t sz = beg.arguments.size();
      parallel_for(size_t(0), sz, [&](size_t i)
        {
        auto& arg = beg.arguments[i];
        closure_conversion_visitor ccv;
        ccv.self_index = 0;
        visitor<Expression, closure_conversion_visitor>::visit(arg, &ccv);

        resolve_free_variables_visitor rfvv;
        visitor<Expression, resolve_free_variables_visitor>::visit(arg, &rfvv);

        }, ops);
      }
    else
      {
      closure_conversion_visitor ccv;
      ccv.self_index = 0;
      visitor<Program, closure_conversion_visitor>::visit(prog, &ccv);
      resolve_free_variables_visitor rfvv;
      visitor<Program, resolve_free_variables_visitor>::visit(prog, &rfvv);
      }
    }
  else
    {
    closure_conversion_visitor ccv;
    ccv.self_index = 0;
    visitor<Program, closure_conversion_visitor>::visit(prog, &ccv);
    resolve_free_variables_visitor rfvv;
    visitor<Program, resolve_free_variables_visitor>::visit(prog, &rfvv);
    }


  prog.closure_converted = true;
  }

SKIWI_END
