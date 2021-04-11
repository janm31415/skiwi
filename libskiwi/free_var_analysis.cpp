#include "free_var_analysis.h"
#include "visitor.h"
#include <set>
#include <string>
#include <algorithm>
#include <iterator>

SKIWI_BEGIN

namespace
  {

  struct free_var_analysis_state
    {
    enum struct e_fva_state
      {
      T_INIT,
      T_STEP_1,
      T_STEP_2
      };
    Expression* p_expr;
    e_fva_state state;

    free_var_analysis_state() : p_expr(nullptr), state(e_fva_state::T_INIT) {}
    free_var_analysis_state(Expression* ip_expr) : p_expr(ip_expr), state(e_fva_state::T_INIT) {}
    free_var_analysis_state(Expression* ip_expr, e_fva_state s) : p_expr(ip_expr), state(s) {}
    };

  struct free_var_analysis_helper
    {
    std::vector<free_var_analysis_state> expressions;
    environment_map env;
    std::vector<std::set<std::string>> local_variables;
    std::vector<std::set<std::string>> free_variables;

    free_var_analysis_helper(const environment_map& input_env)
      {
      env = input_env;
      local_variables.emplace_back();
      free_variables.emplace_back();
      }

    void treat_expressions()
      {
      while (!expressions.empty())
        {
        free_var_analysis_state st = expressions.back();
        expressions.pop_back();
        Expression& e = *st.p_expr;

        switch (st.state)
          {
          case free_var_analysis_state::e_fva_state::T_INIT:
          {
          if (std::holds_alternative<Literal>(e))
            {

            }
          else if (std::holds_alternative<Variable>(e))
            {
            Variable& v = std::get<Variable>(e);
            if (local_variables.back().find(v.name) == local_variables.back().end() && !env->has(v.name))
              free_variables.back().insert(v.name);
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
            if (local_variables.back().find(s.name) == local_variables.back().end() && !env->has(s.name))
              free_variables.back().insert(s.name);
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
            l.free_variables.clear();
            local_variables.emplace_back();
            free_variables.emplace_back();
            for (const auto& var : l.variables)
              local_variables.back().insert(var);
            expressions.emplace_back(&e, free_var_analysis_state::e_fva_state::T_STEP_1);
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
            expressions.emplace_back(&e, free_var_analysis_state::e_fva_state::T_STEP_1);
            //expressions.push_back(&l.body.front());
            for (auto rit = l.bindings.rbegin(); rit != l.bindings.rend(); ++rit)
              expressions.push_back(&(*rit).second);
            }
          else
            throw std::runtime_error("Compiler error!: free variable analysis: not implemented");
          break;
          }
          case free_var_analysis_state::e_fva_state::T_STEP_1:
          {
          if (std::holds_alternative<Lambda>(e))
            {
            Lambda& l = std::get<Lambda>(e);
            for (const auto& var : free_variables.back())
              l.free_variables.push_back(var);
            local_variables.pop_back();
            std::vector<std::string> free_vars;
            std::set_difference(free_variables.back().begin(), free_variables.back().end(), local_variables.back().begin(), local_variables.back().end(), std::back_inserter(free_vars));
            free_variables.pop_back();
            for (const auto& var : free_vars)
              free_variables.back().insert(var);
            }
          else if (std::holds_alternative<Let>(e))
            {
            Let& l = std::get<Let>(e);
            local_variables.emplace_back();
            free_variables.emplace_back();
            for (const auto& var : l.bindings)
              local_variables.back().insert(var.first);
            expressions.emplace_back(&e, free_var_analysis_state::e_fva_state::T_STEP_2);
            expressions.push_back(&l.body.front());
           /*
            std::vector<std::string> free_vars;
            std::set_difference(fvav.free_variables.begin(), fvav.free_variables.end(), local_variables.begin(), local_variables.end(), std::back_inserter(free_vars));
            for (const auto& var : free_vars)
              free_variables.insert(var);
              */
            }
          else
            throw std::runtime_error("Compiler error!: free variable analysis: not implemented");
          break;
          }
          case free_var_analysis_state::e_fva_state::T_STEP_2:
          {
          if (std::holds_alternative<Let>(e))
            {
            //Let& l = std::get<Let>(e);           
            local_variables.pop_back();
            std::vector<std::string> free_vars;
            std::set_difference(free_variables.back().begin(), free_variables.back().end(), local_variables.back().begin(), local_variables.back().end(), std::back_inserter(free_vars));
            free_variables.pop_back();
            for (const auto& var : free_vars)
              free_variables.back().insert(var);
            }
          else
            throw std::runtime_error("Compiler error!: free variable analysis: not implemented");
          break;
          }
          }
        }
      }
    };

  struct free_var_analysis_visitor : public base_visitor<free_var_analysis_visitor>
    {
    environment_map env;
    std::set<std::string> local_variables;
    std::set<std::string> free_variables;

    virtual bool _previsit(Lambda& l)
      {
      l.free_variables.clear();
      free_var_analysis_visitor fvav;
      fvav.env = env;
      for (const auto& var : l.variables)
        fvav.local_variables.insert(var);
      visitor<Expression, free_var_analysis_visitor>::visit(l.body.front(), &fvav);
      for (const auto& var : fvav.free_variables)
        l.free_variables.push_back(var);
      std::vector<std::string> free_vars;
      std::set_difference(fvav.free_variables.begin(), fvav.free_variables.end(), local_variables.begin(), local_variables.end(), std::back_inserter(free_vars));
      for (const auto& var : free_vars)
        free_variables.insert(var);
      return false;
      }

    virtual bool _previsit(Let& l)
      {
      for (auto& arg : l.bindings)
        visitor<Expression, free_var_analysis_visitor>::visit(arg.second, this);
      free_var_analysis_visitor fvav;
      fvav.env = env;
      for (const auto& var : l.bindings)
        fvav.local_variables.insert(var.first);
      visitor<Expression, free_var_analysis_visitor>::visit(l.body.front(), &fvav);
      std::vector<std::string> free_vars;
      std::set_difference(fvav.free_variables.begin(), fvav.free_variables.end(), local_variables.begin(), local_variables.end(), std::back_inserter(free_vars));
      for (const auto& var : free_vars)
        free_variables.insert(var);
      return false;
      }

    virtual bool _previsit(Variable& v)
      {
      if (local_variables.find(v.name) == local_variables.end() && !env->has(v.name))
        free_variables.insert(v.name);
      return true;
      }

    virtual bool _previsit(Set& s)
      {
      if (local_variables.find(s.name) == local_variables.end() && !env->has(s.name))
        free_variables.insert(s.name);
      return true;
      }
    };
  }

void free_variable_analysis(Program& prog, environment_map& env)
  {
  assert(prog.global_define_env_allocated);
  assert(env.get());
  //free_var_analysis_visitor fvav;
  //fvav.env = env;  
  //visitor<Program, free_var_analysis_visitor>::visit(prog, &fvav);
  free_var_analysis_helper fvah(env);
  for (auto& expr : prog.expressions)
    fvah.expressions.push_back(&expr);
  std::reverse(fvah.expressions.begin(), fvah.expressions.end());
  fvah.treat_expressions();
  prog.free_variables_analysed = true;
  }

void free_variable_analysis(Lambda& lam, environment_map& env)
  {
  //free_var_analysis_visitor fvav;
  //fvav.env = env;
  //visitor<Lambda, free_var_analysis_visitor>::visit(lam, &fvav);
  free_var_analysis_helper fvah(env);
  Expression expr(lam);
  fvah.expressions.push_back(&expr);
  fvah.treat_expressions();
  }

SKIWI_END
