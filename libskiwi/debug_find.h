#pragma once

#include "namespace.h"
#include "visitor.h"
#include "parse.h"

SKIWI_BEGIN

template <class TFind>
struct debug_find_visitor : public base_visitor<debug_find_visitor<TFind>>
  {
  debug_find_visitor(TFind i_pred) : pred(i_pred), found(false)
    {
    }

  bool found;
  TFind pred;

  virtual bool _previsit(Expression& e)
    {
    if (pred(e))
      found = true;

    return true;
    }
  };

template <class TFind>
struct debug_find_helper
  {
  std::vector<Expression*> expressions;
  bool found;
  TFind pred;

  debug_find_helper(TFind i_pred) : pred(i_pred), found(false)
    {
    }

  void treat_expressions()
    {
    while (!expressions.empty())
      {
      Expression* p_expr = expressions.back();
      expressions.pop_back();
      Expression& e = *p_expr;
      if (pred(e))
        {
        found = true;
        break;
        }
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
      else if (std::holds_alternative<Case>(e))
        {
        Case& c = std::get<Case>(e);
        expressions.push_back(&c.val_expr.front());
        assert(c.datum_args.size() == c.then_bodies.size());
        for (size_t i = 0; i < c.datum_args.size(); ++i)
          {
          for (auto& arg : c.then_bodies[i])
            expressions.push_back(&arg);
          }
        for (auto& arg : c.else_body)
          expressions.push_back(&arg);
        }
      else if (std::holds_alternative<Cond>(e))
        {
        Cond& c = std::get<Cond>(e);
        for (auto& arg_v : c.arguments)
          for (auto& arg : arg_v)
            expressions.push_back(&arg);
        }
      else if (std::holds_alternative<Do>(e))
        {
        Do& d = std::get<Do>(e);
        for (auto& binding : d.bindings)
          for (auto& arg : binding)
            expressions.push_back(&arg);
        for (auto& tst : d.test)
          expressions.push_back(&tst);
        for (auto& command : d.commands)
          expressions.push_back(&command);
        }
      else
        throw std::runtime_error("Compiler error!: debug find: not implemented");
      }
    }
  };

template <class T, class TFind>
bool find(T& expr, TFind pred)
  {
  //debug_find_visitor<TFind> dfv(pred);
  //visitor<T, debug_find_visitor<TFind>>::visit(expr, &dfv);
  //return dfv.found;
  debug_find_helper<TFind> dfh(pred);
  dfh.expressions.push_back(&expr);
  dfh.treat_expressions();
  return dfh.found;
  }

template <class TFind>
bool find(Program& prog, TFind pred)
  {
  //debug_find_visitor<TFind> dfv(pred);
  //visitor<T, debug_find_visitor<TFind>>::visit(expr, &dfv);
  //return dfv.found;
  debug_find_helper<TFind> dfh(pred);
  for (auto& expr: prog.expressions)
    dfh.expressions.push_back(&expr);
  dfh.treat_expressions();
  return dfh.found;
  }

SKIWI_END
