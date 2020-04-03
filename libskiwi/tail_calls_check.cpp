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
  }


bool only_tail_calls(Program& prog)
  {
  assert(prog.tail_call_analysis);
  tail_calls_check tcc;
  visitor<Program, tail_calls_check>::visit(prog, &tcc);
  return tcc.only_tails;
  }

SKIWI_END
