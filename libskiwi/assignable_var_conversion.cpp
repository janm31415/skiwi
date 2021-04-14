#include "assignable_var_conversion.h"
#include "visitor.h"
#include "environment.h"
#include "concurrency.h"

#include <memory>
#include <string>
#include <set>



SKIWI_BEGIN

namespace
  {

  struct find_assignable_variables_state
    {
    enum struct e_fav_state
      {
      T_INIT,
      T_STEP_1,
      };
    Expression* p_expr;
    e_fav_state state;

    find_assignable_variables_state() : p_expr(nullptr), state(e_fav_state::T_INIT) {}
    find_assignable_variables_state(Expression* ip_expr) : p_expr(ip_expr), state(e_fav_state::T_INIT) {}
    find_assignable_variables_state(Expression* ip_expr, e_fav_state s) : p_expr(ip_expr), state(s) {}
    };

  struct find_assignable_variables
    {
    std::vector<find_assignable_variables_state> expressions;
    std::vector<std::set<std::string>> assignable_variables;

    find_assignable_variables()
      {
      assignable_variables.emplace_back();
      }

    void treat_expressions()
      {
      while (!expressions.empty())
        {
        find_assignable_variables_state st = expressions.back();
        expressions.pop_back();
        Expression& e = *st.p_expr;

        switch (st.state)
          {
          case find_assignable_variables_state::e_fav_state::T_INIT:
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
            if (!s.originates_from_define && !s.originates_from_quote)
              assignable_variables.back().insert(s.name);
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
            assignable_variables.emplace_back();
            expressions.emplace_back(&e, find_assignable_variables_state::e_fav_state::T_STEP_1);
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
            assignable_variables.emplace_back();
            expressions.emplace_back(&e, find_assignable_variables_state::e_fav_state::T_STEP_1);
            expressions.push_back(&l.body.front());
            for (auto rit = l.bindings.rbegin(); rit != l.bindings.rend(); ++rit)
              expressions.push_back(&(*rit).second);
            }
          else
            throw std::runtime_error("Compiler error!: assignable var conversion: not implemented");
          break;
          }
          case find_assignable_variables_state::e_fav_state::T_STEP_1:
          {
          if (std::holds_alternative<Lambda>(e))
            {
            Lambda& l = std::get<Lambda>(e);
            l.assignable_variables = assignable_variables.back();
            assignable_variables.pop_back();
            assignable_variables.back().insert(l.assignable_variables.begin(), l.assignable_variables.end());
            }
          else if (std::holds_alternative<Let>(e))
            {
            Let& l = std::get<Let>(e);
            l.assignable_variables = assignable_variables.back();
            assignable_variables.pop_back();
            assignable_variables.back().insert(l.assignable_variables.begin(), l.assignable_variables.end());
            }
          else
            throw std::runtime_error("Compiler error!: assignable var conversion: not implemented");
          break;
          }
          }
        }
      }
    };

  struct convert_assignable_variables_state
    {
    enum struct e_cav_state
      {
      T_INIT,
      T_STEP_1,
      T_STEP_2
      };
    Expression* p_expr;
    e_cav_state state;

    convert_assignable_variables_state() : p_expr(nullptr), state(e_cav_state::T_INIT) {}
    convert_assignable_variables_state(Expression* ip_expr) : p_expr(ip_expr), state(e_cav_state::T_INIT) {}
    convert_assignable_variables_state(Expression* ip_expr, e_cav_state s) : p_expr(ip_expr), state(s) {}
    };

  struct convert_assignable_variables
    {
    std::vector<std::shared_ptr<environment<bool>>> env;
    std::vector<convert_assignable_variables_state> expressions;

    convert_assignable_variables()
      {
      env.push_back(std::make_shared<environment<bool>>(nullptr));
      }


    void treat_expressions()
      {
      while (!expressions.empty())
        {
        convert_assignable_variables_state st = expressions.back();
        expressions.pop_back();
        Expression& e = *st.p_expr;

        switch (st.state)
          {
          case convert_assignable_variables_state::e_cav_state::T_INIT:
          {
          if (std::holds_alternative<Literal>(e))
            {

            }
          else if (std::holds_alternative<Variable>(e))
            {
            Variable& v = std::get<Variable>(e);
            bool b;
            if (!env.back()->find(b, v.name))
              continue; // v.name must be a globally defined variable from another repl
            if (b)
              {
              PrimitiveCall prim;
              prim.primitive_name = "vector-ref";
              prim.arguments.emplace_back(std::move(v));
              Fixnum z;
              z.value = 0;
              prim.arguments.emplace_back(std::move(z));
              e = PrimitiveCall();
              std::swap(std::get<PrimitiveCall>(e), prim);
              }
            continue;
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
            if (s.originates_from_define || s.originates_from_quote)
              {
              env.back()->push(s.name, false);
              expressions.push_back(&std::get<Set>(e).value.front());
              continue;
              }
            bool b;
            if (!env.back()->find(b, s.name))
              {// this scenario is possible when s.name is a global variable defined in a repl before, triggered by test scheme_tests_4
              //throw std::runtime_error("error in assignable_variable_conversion");
              expressions.push_back(&std::get<Set>(e).value.front());
              continue;
              }
            if (!b)
              {
              expressions.push_back(&std::get<Set>(e).value.front());
              continue;
              }
            expressions.emplace_back(&e, convert_assignable_variables_state::e_cav_state::T_STEP_1);
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
            //expressions.push_back(&l.body.front());

            auto current_env = env.back();
            env.push_back(std::make_shared<environment<bool>>(current_env));
            std::vector<std::pair<std::string, Expression>> new_bindings;
            for (size_t i = 0; i < l.variables.size(); ++i)
              {
              const std::string& var_name = l.variables[i];
              if (l.assignable_variables.find(var_name) != l.assignable_variables.end())
                {
                std::string original = var_name;
                std::string adapted = "#%" + var_name;
                env.back()->push(original, true);
                current_env->push(adapted, false);
                Variable var;
                var.name = adapted;
                PrimitiveCall prim;
                prim.primitive_name = "vector";
                prim.arguments.emplace_back(std::move(var));
                new_bindings.emplace_back(original, prim);
                l.variables[i] = adapted;
                }
              else
                env.back()->push(var_name, false);
              }

            expressions.emplace_back(&e, convert_assignable_variables_state::e_cav_state::T_STEP_1);
            if (new_bindings.empty())
              {
              expressions.push_back(&l.body.front());
              //visitor<Expression, convert_assignable_variables>::visit(l.body.front(), &new_v);
              //expressions.push_back(&l.body.front());
              }
            else
              {
              Let new_let;
              new_let.bindings.swap(new_bindings);
              new_let.body.swap(l.body);
              Begin new_begin;
              new_begin.arguments.emplace_back(std::move(new_let));
              l.body.emplace_back(std::move(new_begin));
              //visitor<Expression, convert_assignable_variables>::visit(std::get<Let>(std::get<Begin>(l.body.front()).arguments.front()).body.front(), &new_v);
              expressions.push_back(&std::get<Let>(std::get<Begin>(l.body.front()).arguments.front()).body.front());
              }
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

            //expressions.push_back(&l.body.front());
            //for (auto rit = l.bindings.rbegin(); rit != l.bindings.rend(); ++rit)
            //  expressions.push_back(&(*rit).second);

            auto current_env = env.back();
            env.push_back(std::make_shared<environment<bool>>(current_env));

            expressions.emplace_back(&e, convert_assignable_variables_state::e_cav_state::T_STEP_1);
            for (auto rit = l.bindings.rbegin(); rit != l.bindings.rend(); ++rit)
              expressions.push_back(&(*rit).second);

            }
          else
            throw std::runtime_error("Compiler error!: assignable var conversion: not implemented");
          break;
          }
          case convert_assignable_variables_state::e_cav_state::T_STEP_1:
          {
          if (std::holds_alternative<Lambda>(e))
            {
            //Lambda& l = std::get<Lambda>(e);
            env.pop_back();
            }
          else if (std::holds_alternative<Let>(e))
            {
            expressions.emplace_back(&e, convert_assignable_variables_state::e_cav_state::T_STEP_2);
            Let& l = std::get<Let>(e);
            std::vector<std::pair<std::string, Expression>> new_bindings;
            auto old_env = env[env.size() - 2];
            for (auto& binding : l.bindings)
              {
              if (l.assignable_variables.find(binding.first) != l.assignable_variables.end())
                {
                std::string original = binding.first;
                std::string adapted = "#%" + original;
                env.back()->push(original, true);
                old_env->push(adapted, false);
                Variable var;
                var.name = adapted;
                PrimitiveCall prim;
                prim.primitive_name = "vector";
                prim.arguments.emplace_back(std::move(var));
                new_bindings.emplace_back(original, prim);
                binding.first = adapted;
                }
              else
                env.back()->push(binding.first, false);
              }
            if (new_bindings.empty())
              {
              //visitor<Expression, convert_assignable_variables>::visit(l.body.front(), &new_v);
              expressions.push_back(&l.body.front());
              }
            else
              {
              Let new_let;
              new_let.bindings.swap(new_bindings);
              new_let.body.swap(l.body);
              //for (auto& binding : new_let.bindings)
              //  visitor<Expression, convert_assignable_variables>::visit(binding.second, &new_v);
              Begin new_begin;
              new_begin.arguments.emplace_back(std::move(new_let));
              l.body.emplace_back(std::move(new_begin));
              //visitor<Expression, convert_assignable_variables>::visit(std::get<Let>(std::get<Begin>(l.body.front()).arguments.front()).body.front(), &new_v);
              expressions.push_back(&std::get<Let>(std::get<Begin>(l.body.front()).arguments.front()).body.front());
              for (auto rit = new_let.bindings.rbegin(); rit != new_let.bindings.rend(); ++rit)
                {
                expressions.push_back(&(*rit).second);
                }
              }
            }
          else if (std::holds_alternative<Set>(e))
            {
            Set& s = std::get<Set>(e);
            PrimitiveCall prim;
            prim.primitive_name = "vector-set!";
            Variable v;
            v.name = s.name;
            prim.arguments.emplace_back(std::move(v));
            Fixnum z;
            z.value = 0;
            prim.arguments.emplace_back(std::move(z));
            prim.arguments.emplace_back(std::move(s.value.front()));
            e = PrimitiveCall();
            std::swap(std::get<PrimitiveCall>(e), prim);
            }
          else
            throw std::runtime_error("Compiler error!: assignable var conversion: not implemented");
          break;
          }
          case convert_assignable_variables_state::e_cav_state::T_STEP_2:
          {
          if (std::holds_alternative<Let>(e))
            {
            //Let& l = std::get<Let>(e);
            env.pop_back();
            }
          else
            throw std::runtime_error("Compiler error!: assignable var conversion: not implemented");
          break;
          }
          }
        }
      }
    };

  }

void assignable_variable_conversion(Program& prog, const compiler_options& ops)
  {
  /*
  assert(prog.alpha_converted);
  find_assignable_variables fav;
  visitor<Program, find_assignable_variables>::visit(prog, &fav);
  convert_assignable_variables cav;
  //for (const auto& v : fav.assignable_variables)
  //  cav.env->push(v, true);
  visitor<Program, convert_assignable_variables>::visit(prog, &cav);
  prog.assignable_variables_converted = true;
  */
  assert(prog.alpha_converted);
  assert(prog.expressions.size() <= 1);

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
        find_assignable_variables fav;
        fav.expressions.push_back(&arg);
        fav.treat_expressions();
        convert_assignable_variables cav;
        cav.expressions.push_back(&arg);
        cav.treat_expressions();
        }, ops);
      }
    else
      {
      find_assignable_variables fav;
      for (auto& expr : prog.expressions)
        fav.expressions.push_back(&expr);
      std::reverse(fav.expressions.begin(), fav.expressions.end());
      fav.treat_expressions();
      convert_assignable_variables cav;
      for (auto& expr : prog.expressions)
        cav.expressions.push_back(&expr);
      std::reverse(cav.expressions.begin(), cav.expressions.end());
      cav.treat_expressions();
      }
    }
  prog.assignable_variables_converted = true;
  }

SKIWI_END
