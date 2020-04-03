#include "single_begin_conversion.h"
#include "visitor.h"
#include <cassert>

SKIWI_BEGIN

namespace
  {
  struct remove_nested_begins : public base_visitor<remove_nested_begins>
    {
    virtual void _postvisit(Begin& b)
      {
      auto it = b.arguments.begin();
      while (it != b.arguments.end())
        {
        if (std::holds_alternative<Begin>(*it))
          {
          Begin b2 = std::get<Begin>(*it);
          it = b.arguments.erase(it);
          it = b.arguments.insert(it, b2.arguments.begin(), b2.arguments.end());
          }
        else
          ++it;
        }
      }

    virtual void _postvisit(Program& p)
      {
      auto it = p.expressions.begin();
      while (it != p.expressions.end())
        {
        if (std::holds_alternative<Begin>(*it))
          {
          Begin b2 = std::get<Begin>(*it);
          it = p.expressions.erase(it);
          it = p.expressions.insert(it, b2.arguments.begin(), b2.arguments.end());
          }
        else
          ++it;
        }
      }
    };
  }

void single_begin_conversion(Program& prog)
  {
  remove_nested_begins rnb;
  visitor<Program, remove_nested_begins>::visit(prog, &rnb);

  if (prog.expressions.size() > 1)
    {
    Begin b;
    b.arguments = prog.expressions;
    prog.expressions.clear();
    prog.expressions.push_back(b);
    }
  assert(prog.expressions.size() <= 1);

  prog.single_begin_conversion = true;
  }

void remove_nested_begin_expressions(Program& p)
  {
  remove_nested_begins rnb;
  visitor<Program, remove_nested_begins>::visit(p, &rnb);
  }

void remove_nested_begin_expressions(Lambda& lam)
  {
  remove_nested_begins rnb;
  visitor<Lambda, remove_nested_begins>::visit(lam, &rnb);
  }

void remove_nested_begin_expressions(Let& let)
  {
  remove_nested_begins rnb;
  visitor<Let, remove_nested_begins>::visit(let, &rnb);
  }

SKIWI_END
