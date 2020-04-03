#include "lambda_to_let_conversion.h"
#include "visitor.h"

#include <cassert>

SKIWI_BEGIN

namespace
  {

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
  lambda_to_let_convertor llc;
  visitor<Program, lambda_to_let_convertor>::visit(prog, &llc);

  prog.lambda_to_let_converted = true;
  }

SKIWI_END
