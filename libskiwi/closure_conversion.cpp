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
    
  struct closure_conversion_state
    {
    enum struct e_cc_state
      {
      T_INIT,
      T_STEP_1,
      };
    Expression* p_expr;
    e_cc_state state;
    
    closure_conversion_state() : p_expr(nullptr), state(e_cc_state::T_INIT) {}
    closure_conversion_state(Expression* ip_expr) : p_expr(ip_expr), state(e_cc_state::T_INIT) {}
    closure_conversion_state(Expression* ip_expr, e_cc_state s) : p_expr(ip_expr), state(s) {}
    };
  
  struct closure_conversion_helper
    {
    std::vector<closure_conversion_state> expressions;
    uint64_t self_index;
    
    closure_conversion_helper()
      {
      self_index = 0;
      }
      
    void treat_expressions()
      {
      while (!expressions.empty())
        {
        closure_conversion_state st = expressions.back();
        expressions.pop_back();
        Expression& e = *st.p_expr;
        
        switch (st.state)
          {
          case closure_conversion_state::e_cc_state::T_INIT:
          {
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
              Set& s = std::get<Set>(e);
              expressions.push_back(&std::get<Set>(e).value.front());
              }
            else if (std::holds_alternative<If>(e))
              {
              for (auto rit = std::get<If>(e).arguments.rbegin(); rit != std::get<If>(e).arguments.rend(); ++rit)
                expressions.push_back(&(*rit));
              }
            else if (std::holds_alternative<Begin>(e))
              {
              for (auto rit = std::get<Begin>(e).arguments.rbegin(); rit != std::get<Begin>(e).arguments.rend(); ++rit)
                expressions.push_back(&(*rit));
              }
            else if (std::holds_alternative<PrimitiveCall>(e))
              {
              for (auto rit = std::get<PrimitiveCall>(e).arguments.rbegin(); rit != std::get<PrimitiveCall>(e).arguments.rend(); ++rit)
                expressions.push_back(&(*rit));
              }
            else if (std::holds_alternative<ForeignCall>(e))
              {
              for (auto rit = std::get<ForeignCall>(e).arguments.rbegin(); rit != std::get<ForeignCall>(e).arguments.rend(); ++rit)
                expressions.push_back(&(*rit));
              }
            else if (std::holds_alternative<Lambda>(e))
              {
              Lambda& l = std::get<Lambda>(e);
              expressions.emplace_back(&e, closure_conversion_state::e_cc_state::T_STEP_1);
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
              throw std::runtime_error("Compiler error!: closure conversion: not implemented");
            break;
            }
          case closure_conversion_state::e_cc_state::T_STEP_1:
            {
            if (std::holds_alternative<Lambda>(e))
              {
              Lambda& l = std::get<Lambda>(e);
              std::stringstream str;
              str << "#%self" << self_index;
              std::string lambda_ref = str.str();
              ++self_index;
              l.variables.insert(l.variables.begin(), lambda_ref);
              

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
            else
              throw std::runtime_error("Compiler error!: closure conversion: not implemented");
            break;
            }
          }
        }
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
        //closure_conversion_visitor ccv;
        //ccv.self_index = 0;
        //visitor<Expression, closure_conversion_visitor>::visit(arg, &ccv);
        closure_conversion_helper cch;
        cch.expressions.push_back(&arg);
        cch.treat_expressions();

        resolve_free_variables_visitor rfvv;
        visitor<Expression, resolve_free_variables_visitor>::visit(arg, &rfvv);

        }, ops);
      }
    else
      {
      //closure_conversion_visitor ccv;
      //ccv.self_index = 0;
      //visitor<Program, closure_conversion_visitor>::visit(prog, &ccv);
      closure_conversion_helper cch;
      for (auto& expr : prog.expressions)
        cch.expressions.push_back(&expr);
      std::reverse(cch.expressions.begin(), cch.expressions.end());
      cch.treat_expressions();
      
      resolve_free_variables_visitor rfvv;
      visitor<Program, resolve_free_variables_visitor>::visit(prog, &rfvv);
      }
    }
  else
    {
    //closure_conversion_visitor ccv;
    //ccv.self_index = 0;
    //visitor<Program, closure_conversion_visitor>::visit(prog, &ccv);
    closure_conversion_helper cch;
    for (auto& expr : prog.expressions)
      cch.expressions.push_back(&expr);
    std::reverse(cch.expressions.begin(), cch.expressions.end());
    cch.treat_expressions();
    resolve_free_variables_visitor rfvv;
    visitor<Program, resolve_free_variables_visitor>::visit(prog, &rfvv);
    }


  prog.closure_converted = true;
  }

SKIWI_END
