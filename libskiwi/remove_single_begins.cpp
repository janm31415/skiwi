#include "remove_single_begins.h"
#include <cassert>
#include <variant>

#include "concurrency.h"

SKIWI_BEGIN

namespace
  {

  struct remove_single_begin_state
    {
    enum struct e_rsb_state
      {
      T_INIT,
      T_STEP_1
      };
    Expression* p_expr;
    e_rsb_state state;

    remove_single_begin_state() : p_expr(nullptr), state(e_rsb_state::T_INIT) {}
    remove_single_begin_state(Expression* ip_expr) : p_expr(ip_expr), state(e_rsb_state::T_INIT) {}
    remove_single_begin_state(Expression* ip_expr, e_rsb_state s) : p_expr(ip_expr), state(s) {}
    };

  struct remove_single_begin_helper
    {
    std::vector<remove_single_begin_state> expressions;

    void treat_expressions()
      {
      while (!expressions.empty())
        {
        auto rsb_state = expressions.back();
        expressions.pop_back();
        Expression& e = *rsb_state.p_expr;
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
          Begin& b = std::get<Begin>(e);
          if (rsb_state.state == remove_single_begin_state::e_rsb_state::T_INIT)
            {
            if (b.arguments.size() == 1)
              expressions.emplace_back(&e, remove_single_begin_state::e_rsb_state::T_STEP_1);
            for (auto rit = b.arguments.rbegin(); rit != b.arguments.rend(); ++rit)
              expressions.push_back(&(*rit));
            }
          else
            {
            assert(b.arguments.size() == 1);
            Expression new_e = std::move(b.arguments[0]);
            e = std::move(new_e);
            }
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
        else
          throw std::runtime_error("Compiler error!: Remove single begin: not implemented");
        }
      }
    };

  }


void remove_single_begins(Program& prog)
  {
  remove_single_begin_helper sbh;
  for (auto& expr : prog.expressions)
    sbh.expressions.push_back(&expr);
  sbh.treat_expressions();
  prog.single_begins_removed = true;
  }

SKIWI_END
