#include "tail_calls_check.h"
#include "visitor.h"

SKIWI_BEGIN

namespace
  {

  /*  
  A procedure call is called a tail call if it occurs in a tail position.
  A tail position is defined recursively as follows:

  - the body of a procedure is in tail position
  - if a let expression is in tail position, then the body
    of the let is in tail position
  - if the condition expression (if test conseq altern) is in tail
    position, then the conseq and altern branches are also in tail position.
  - all other expressions are not in tail position.
  */

  struct tail_calls_check : public base_visitor<tail_calls_check>
    {
    bool only_tails;

    tail_calls_check() : only_tails(true)
      {
      }

    virtual bool _previsit(Variable&) 
      { 
      return only_tails;
      }

    virtual bool _previsit(Begin&) 
      { 
      return only_tails;
      }

    virtual bool _previsit(FunCall& f) 
      { 
      if (!f.tail_position)
        only_tails = false;
      return only_tails;
      }

    virtual bool _previsit(If&) 
      { 
      return only_tails;
      }

    virtual bool _previsit(Lambda&) 
      { 
      return only_tails;
      }

    virtual bool _previsit(Let&)
      { 
      return only_tails;
      }

    virtual bool _previsit(Literal&) 
      { 
      return only_tails;
      }

    virtual bool _previsit(PrimitiveCall&) 
      { 
      return only_tails;
      }

    virtual bool _previsit(ForeignCall&)
      {
      return only_tails;
      }

    virtual bool _previsit(Set&)
      { 
      return only_tails;
      }
    };
    
  struct tail_calls_check_helper
    {
    std::vector<Expression*> expressions;
    bool only_tails;

    tail_calls_check_helper()
      {
      only_tails = true;
      }
      
    void treat_expressions()
      {
      while (!expressions.empty())
        {
        Expression* p_expr = expressions.back();
        expressions.pop_back();
        Expression& e = *p_expr;
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
          FunCall& f = std::get<FunCall>(e);
          if (!f.tail_position)
            {
            only_tails = false;
            break;
            }
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
          throw std::runtime_error("Compiler error!: Tail calls check: not implemented");
        }
      }
    };
  }


bool only_tail_calls(Program& prog)
  {
  assert(prog.tail_call_analysis);
  //tail_calls_check tcc;
  //visitor<Program, tail_calls_check>::visit(prog, &tcc);
  
  tail_calls_check_helper tcch;
  for (auto& expr : prog.expressions)
    tcch.expressions.push_back(&expr);
    
  tcch.treat_expressions();
  
  return tcch.only_tails;
  }

SKIWI_END
