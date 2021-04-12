#include "lambda_to_let_conversion.h"
#include "visitor.h"

#include <cassert>

SKIWI_BEGIN

namespace
  {
  
  struct lamda_to_let_state
    {
    enum struct e_ll_state
      {
      T_INIT,
      T_STEP_1
      };
    Expression* p_expr;
    e_ll_state state;

    lamda_to_let_state() : p_expr(nullptr), state(e_ll_state::T_INIT) {}
    lamda_to_let_state(Expression* ip_expr) : p_expr(ip_expr), state(e_ll_state::T_INIT) {}
    lamda_to_let_state(Expression* ip_expr, e_ll_state s) : p_expr(ip_expr), state(s) {}
    };

  struct lambda_to_let_helper
    {
    /*
    ((lambda (p1 p2 ...) body) v1 v2 ...)

    (let ([p1 v1] [p2 v2] ...) body)
    */
    
    std::vector<lamda_to_let_state> expressions;
            
    void _convert(FunCall& f, Expression& e)
      {
      assert(std::holds_alternative<Lambda>(f.fun.front()));
      Lambda& lam = std::get<Lambda>(f.fun.front());
      //assert(lam.assignable_variables.empty());
      assert(!lam.variable_arity);
      assert(lam.variables.size() == f.arguments.size());
      Let let;
      let.body.swap(lam.body);
      for (size_t i = 0; i < lam.variables.size(); ++i)
        {
        let.bindings.emplace_back(lam.variables[i], std::move(f.arguments[i]));
        }
      e = Let();
      std::swap(std::get<Let>(e), let);
      }
      
    void treat_expressions()
      {
      while (!expressions.empty())
        {
        lamda_to_let_state ll_state = expressions.back();
        expressions.pop_back();
        Expression& e = *ll_state.p_expr;
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
          expressions.push_back(&l.body.front());
          }
        else if (std::holds_alternative<FunCall>(e))
          {
          if (ll_state.state == lamda_to_let_state::e_ll_state::T_INIT)
            {
            expressions.emplace_back(&e, lamda_to_let_state::e_ll_state::T_STEP_1);
            expressions.push_back(&std::get<FunCall>(e).fun.front());
            for (auto rit = std::get<FunCall>(e).arguments.rbegin(); rit != std::get<FunCall>(e).arguments.rend(); ++rit)
              expressions.push_back(&(*rit));
            }
          else
            {
            FunCall& f = std::get<FunCall>(e);
            if (std::holds_alternative<Lambda>(f.fun.front()))
              {
              Lambda& lam = std::get<Lambda>(f.fun.front());
              if (!lam.variable_arity)
                _convert(f, e);
              }
            }
          }
        else if (std::holds_alternative<Let>(e))
          {
          Let& l = std::get<Let>(e);
          expressions.push_back(&l.body.front());
          for (auto rit = l.bindings.rbegin(); rit != l.bindings.rend(); ++rit)
            expressions.push_back(&(*rit).second);
          }
        else
          throw std::runtime_error("Compiler error!: Lambda to let conversion: not implemented");
        }
      }
    };
    
  struct lambda_to_let_convertor : public base_visitor<lambda_to_let_convertor>
    {
    /*
    ((lambda (p1 p2 ...) body) v1 v2 ...)

    (let ([p1 v1] [p2 v2] ...) body)
    */

    void _convert(FunCall& f, Expression& e)
      {
      assert(std::holds_alternative<Lambda>(f.fun.front()));
      Lambda& lam = std::get<Lambda>(f.fun.front());
      //assert(lam.assignable_variables.empty());
      assert(!lam.variable_arity);
      assert(lam.variables.size() == f.arguments.size());
      Let let;
      let.body.swap(lam.body);
      for (size_t i = 0; i < lam.variables.size(); ++i)
        {
        let.bindings.emplace_back(lam.variables[i], std::move(f.arguments[i]));
        }
      e = Let();
      std::swap(std::get<Let>(e), let);
      }

    virtual void _postvisit(Expression& e)
      {
      if (std::holds_alternative<FunCall>(e))
        {
        FunCall& f = std::get<FunCall>(e);
        if (std::holds_alternative<Lambda>(f.fun.front()))
          {
          Lambda& lam = std::get<Lambda>(f.fun.front());
          if (!lam.variable_arity)
            _convert(f, e);
          }
        }
      }

    };

  }

void lambda_to_let_conversion(Program& prog)
  {
  //lambda_to_let_convertor llc;
  //visitor<Program, lambda_to_let_convertor>::visit(prog, &llc);
  
  lambda_to_let_helper llh;
  for (auto& expr : prog.expressions)
    llh.expressions.push_back(&expr);
  std::reverse(llh.expressions.begin(), llh.expressions.end());
  llh.treat_expressions();

  prog.lambda_to_let_converted = true;
  }

SKIWI_END
