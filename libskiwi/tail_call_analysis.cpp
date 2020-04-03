#include "tail_call_analysis.h"
#include "visitor.h"
#include <variant>

SKIWI_BEGIN

namespace
  {
  struct unset_tail_pos
    {
    template <class F>
    void operator()(F& i)
      {
      i.tail_position = false;
      }

    void operator()(Literal& i)
      {
      unset_tail_pos stp;
      std::visit(stp, i);
      }
    };

  struct tail_call_init : public base_visitor<tail_call_init>
    {

    virtual bool _previsit(Expression& e)
      {
      unset_tail_pos stp;
      std::visit(stp, e);
      return true;
      }
    };

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

  struct set_tail_pos
    {
    template <class F>
    void operator()(F& i)
      {
      i.tail_position = true;
      }

    void operator()(Literal& i)
      {
      set_tail_pos stp;
      std::visit(stp, i);
      }
    };

  struct tail_call_analysis_visitor : public base_visitor<tail_call_analysis_visitor>
    {    
    bool in_tail_position;
    tail_call_analysis_visitor() : in_tail_position(false) {}

    void set_tail_position(Expression& e)
      {
      set_tail_pos stp;
      std::visit(stp, e);
      }

    virtual bool _previsit(Variable&) 
      { 
      return true;
      }

    virtual bool _previsit(Begin& b) 
      { 
      if (b.tail_position && !b.arguments.empty())
        {
        set_tail_position(b.arguments.back());
        }
      return true;
      }

    virtual bool _previsit(FunCall&) 
      { 
      return true;
      }

    virtual bool _previsit(If& i) 
      {
      if (i.tail_position)
        {
        if (i.arguments.size() >= 2)
          set_tail_position(i.arguments[1]);
        if (i.arguments.size() >= 3)
          set_tail_position(i.arguments[2]);
        }
      return true;
      }

    virtual bool _previsit(Lambda& l) 
      { 
      set_tail_position(l.body.front());
      return true;
      }

    virtual bool _previsit(Let& l)
      { 
      if (l.tail_position)
        {
        set_tail_position(l.body.front());
        }
      return true;
      }

    virtual bool _previsit(Literal&) 
      { 
      return true;
      }

    virtual bool _previsit(ForeignCall&)
      {
      return true;
      }

    virtual bool _previsit(PrimitiveCall&) 
      { 
      return true;
      }

    virtual bool _previsit(Set&)
      { 
      return true;
      }

    virtual bool _previsit(Program& p)
      {
      if (!p.expressions.empty())
        {
        if (p.expressions.size() == 1 && std::holds_alternative<Begin>(p.expressions.front()))
          {
          for (auto& expr : std::get<Begin>(p.expressions.front()).arguments)
            set_tail_position(expr);
          }
        else
          set_tail_position(p.expressions.back());
        }
      return true;
      }
    };
  }

void tail_call_analysis(Program& prog)
  {
  if (prog.tail_call_analysis)
    {
    tail_call_init tci;
    visitor<Program, tail_call_init>::visit(prog, &tci);
    }
  tail_call_analysis_visitor tcav;
  visitor<Program, tail_call_analysis_visitor>::visit(prog, &tcav);
  prog.tail_call_analysis = true;
  }

SKIWI_END
