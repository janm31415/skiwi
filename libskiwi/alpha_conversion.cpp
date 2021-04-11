#include "alpha_conversion.h"
#include "compile_error.h"
#include "visitor.h"
#include "parse.h"

#include <vector>
#include <cctype>

#include <sstream>

SKIWI_BEGIN

namespace
  {
  std::vector<Set*> g_unhandled_sets;

  struct alpha_conversion_state
    {
    enum struct e_ac_state
      {
      T_INIT,
      T_STEP_1,
      T_STEP_2
      };
    Expression* p_expr;
    e_ac_state state;

    alpha_conversion_state() : p_expr(nullptr), state(e_ac_state::T_INIT) {}
    alpha_conversion_state(Expression* ip_expr) : p_expr(ip_expr), state(e_ac_state::T_INIT) {}
    alpha_conversion_state(Expression* ip_expr, e_ac_state s) : p_expr(ip_expr), state(s) {}
    };

  struct alpha_conversion_helper
    {
    std::vector<alpha_conversion_state> expressions;
    std::vector<uint64_t> index;
    std::vector<std::shared_ptr<environment<alpha_conversion_data>>> env;
    bool modify_names;

    alpha_conversion_helper(uint64_t i, const std::shared_ptr<environment<alpha_conversion_data>>& p_outer, bool mn)
      {
      modify_names = mn;
      index.push_back(i);
      env.push_back(std::make_shared<environment<alpha_conversion_data>>(p_outer));
      }

    std::string make_name(const std::string& original, uint64_t i)
      {
      std::stringstream str;
      str << original << "_" << i;
      return str.str();
      }

    void treat_expressions()
      {
      while (!expressions.empty())
        {
        alpha_conversion_state st = expressions.back();
        expressions.pop_back();
        Expression& e = *st.p_expr;

        switch (st.state)
          {
          case alpha_conversion_state::e_ac_state::T_INIT:
          {
          if (std::holds_alternative<Literal>(e))
            {

            }
          else if (std::holds_alternative<Variable>(e))
            {
            Variable& v = std::get<Variable>(e);
            alpha_conversion_data new_name;
            if (!env.back()->find(new_name, v.name)) // variables are already used in a context with the variable being defined later
              {
              //throw_error(v.line_nr, v.column_nr, v.filename, primitive_unknown, v.name);
              new_name.name = modify_names ? make_name(v.name, index.back()++) : v.name;
              new_name.forward_declaration = true;
              env.back()->push_outer(v.name, new_name);
              }
            v.name = new_name.name;
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
            expressions.emplace_back(&e, alpha_conversion_state::e_ac_state::T_STEP_1);
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
            expressions.emplace_back(&e, alpha_conversion_state::e_ac_state::T_STEP_1);
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
            Lambda& lam = std::get<Lambda>(e);
            expressions.emplace_back(&e, alpha_conversion_state::e_ac_state::T_STEP_1);
            //alpha_conversion_visitor new_visitor(index, env, modify_names);

            index.push_back(index.back());
            env.push_back(std::make_shared<environment<alpha_conversion_data>>(env.back()));

            for (size_t i = 0; i < lam.variables.size(); ++i)
              {
              std::string& var_name = lam.variables[i];
              std::string new_name = modify_names ? make_name(var_name, index.back()++) : var_name;
              env.back()->push(var_name, new_name);
              lam.variables[i] = new_name;
              }
            //visitor<Expression, alpha_conversion_visitor>::visit(lam.body.front(), &new_visitor);
            //index = new_visitor.index;            
            expressions.push_back(&lam.body.front());
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
            expressions.emplace_back(&e, alpha_conversion_state::e_ac_state::T_STEP_1);
            //expressions.push_back(&l.body.front());
            for (auto rit = l.bindings.rbegin(); rit != l.bindings.rend(); ++rit)
              expressions.push_back(&(*rit).second);
            }
          else
            throw std::runtime_error("Compiler error!: alpha conversion: not implemented");
          break;
          }
          case alpha_conversion_state::e_ac_state::T_STEP_1:
          {
          if (std::holds_alternative<Lambda>(e))
            {
            auto ind = index.back();
            index.pop_back();
            index.back() = ind;
            env.pop_back();
            }
          else if (std::holds_alternative<Let>(e))
            {
            Let& l = std::get<Let>(e);
            expressions.emplace_back(&e, alpha_conversion_state::e_ac_state::T_STEP_2);
            index.push_back(index.back());
            env.push_back(std::make_shared<environment<alpha_conversion_data>>(env.back()));
            for (auto& binding : l.bindings)
              {
              std::string original = binding.first;
              std::string adapted = modify_names ? make_name(original, index.back()++) : original;
              env.back()->push(original, adapted);
              binding.first = adapted;
              }

            expressions.push_back(&l.body.front());
            }
          else if (std::holds_alternative<Set>(e))
            {
            Set& s = std::get<Set>(e);
            alpha_conversion_data new_name;

            if (s.originates_from_define || s.originates_from_quote)
              {
              if (!env.back()->find(new_name, s.name))
                {
                new_name.name = modify_names ? make_name(s.name, index.back()++) : s.name;
                env.back()->push(s.name, new_name);
                }
              else
                {
                if (new_name.forward_declaration) // this variable was forward declared, but now it is also defined, so set forward_declaration to false.
                  {
                  new_name.forward_declaration = false;
                  env.back()->push(s.name, new_name);
                  }
                else
                  s.originates_from_define = false; // was already defined before, therefore, treat this define as a set!
                }
              s.name = new_name.name;
              }
            else if (!env.back()->find(new_name, s.name))
              {
              static std::map<std::string, expression_type> expr_map = generate_expression_map();
              auto it = expr_map.find(s.name);
              if (it != expr_map.end() && it->second == et_primitive_call)
                { // we're setting a primitive call, convert it to a define
                new_name.name = modify_names ? make_name(s.name, index.back()++) : s.name;
                env.back()->push(s.name, new_name);
                s.name = new_name.name;
                s.originates_from_define = true;
                }
              else
                g_unhandled_sets.push_back(&s);
              }
            else
              s.name = new_name.name;
            }
          else if (std::holds_alternative<PrimitiveCall>(e))
            {
            PrimitiveCall& p = std::get<PrimitiveCall>(e);
            alpha_conversion_data new_name;
            if (env.back()->find(new_name, p.primitive_name)) // it is possible that a primitive name has been redefined. That case is treated here.
              {
              FunCall f;
              Variable v;
              v.name = new_name.name;
              f.fun.push_back(v);
              f.arguments = p.arguments;
              if (p.as_object)
                e = v;
              else
                e = f;
              }
            }
          else
            throw std::runtime_error("Compiler error!: alpha conversion: not implemented");
          break;
          }
          case alpha_conversion_state::e_ac_state::T_STEP_2:
          {
          if (std::holds_alternative<Let>(e))
            {
            auto ind = index.back();
            index.pop_back();
            index.back() = ind;
            env.pop_back();
            }
          else
            throw std::runtime_error("Compiler error!: alpha conversion: not implemented");
          break;
          }
          }
        }
      }
    };

  struct alpha_conversion_visitor : public base_visitor<alpha_conversion_visitor>
    {
    uint64_t index;
    std::shared_ptr<environment<alpha_conversion_data>> env;
    bool modify_names;

    alpha_conversion_visitor(uint64_t i, const std::shared_ptr<environment<alpha_conversion_data>>& p_outer, bool mn)
      {
      modify_names = mn;
      index = i;
      env = std::make_shared<environment<alpha_conversion_data>>(p_outer);
      }

    virtual bool _previsit(Lambda& lam)
      {
      alpha_conversion_visitor new_visitor(index, env, modify_names);

      for (size_t i = 0; i < lam.variables.size(); ++i)
        {
        std::string& var_name = lam.variables[i];
        std::string new_name = modify_names ? make_name(var_name, new_visitor.index++) : var_name;
        new_visitor.env->push(var_name, new_name);
        lam.variables[i] = new_name;
        }
      visitor<Expression, alpha_conversion_visitor>::visit(lam.body.front(), &new_visitor);
      index = new_visitor.index;
      return false; // return false because we already treated the lambda body
      }

    virtual bool _previsit(Variable& v)
      {
      alpha_conversion_data new_name;
      if (!env->find(new_name, v.name)) // variables are already used in a context with the variable being defined later
        {
        //throw_error(v.line_nr, v.column_nr, v.filename, primitive_unknown, v.name);
        new_name.name = modify_names ? make_name(v.name, index++) : v.name;
        new_name.forward_declaration = true;
        env->push_outer(v.name, new_name);
        }
      v.name = new_name.name;
      return true;
      }

    virtual bool _previsit(Let& l)
      {
      for (auto& arg : l.bindings)
        visitor<Expression, alpha_conversion_visitor>::visit(arg.second, this);

      alpha_conversion_visitor new_visitor(index, env, modify_names);
      for (auto& binding : l.bindings)
        {
        std::string original = binding.first;
        std::string adapted = modify_names ? make_name(original, new_visitor.index++) : original;
        new_visitor.env->push(original, adapted);
        binding.first = adapted;
        }
      visitor<Expression, alpha_conversion_visitor>::visit(l.body.front(), &new_visitor);
      index = new_visitor.index;
      return false;
      }

    virtual void _postvisit(Set& s)
      {
      alpha_conversion_data new_name;

      if (s.originates_from_define || s.originates_from_quote)
        {
        if (!env->find(new_name, s.name))
          {
          new_name.name = modify_names ? make_name(s.name, index++) : s.name;
          env->push(s.name, new_name);
          }
        else
          {
          if (new_name.forward_declaration) // this variable was forward declared, but now it is also defined, so set forward_declaration to false.
            {
            new_name.forward_declaration = false;
            env->push(s.name, new_name);
            }
          else
            s.originates_from_define = false; // was already defined before, therefore, treat this define as a set!
          }
        s.name = new_name.name;
        }
      else if (!env->find(new_name, s.name))
        {
        static std::map<std::string, expression_type> expr_map = generate_expression_map();
        auto it = expr_map.find(s.name);
        if (it != expr_map.end() && it->second == et_primitive_call)
          { // we're setting a primitive call, convert it to a define
          new_name.name = modify_names ? make_name(s.name, index++) : s.name;
          env->push(s.name, new_name);
          s.name = new_name.name;
          s.originates_from_define = true;
          }
        else
          g_unhandled_sets.push_back(&s);
        }
      else
        s.name = new_name.name;


      //return true;
      }

    virtual void _postvisit(Expression& e)
      {
      if (std::holds_alternative<PrimitiveCall>(e))
        {
        PrimitiveCall& p = std::get<PrimitiveCall>(e);
        alpha_conversion_data new_name;
        if (env->find(new_name, p.primitive_name)) // it is possible that a primitive name has been redefined. That case is treated here.
          {
          FunCall f;
          Variable v;
          v.name = new_name.name;
          f.fun.push_back(v);
          f.arguments = p.arguments;
          if (p.as_object)
            e = v;
          else
            e = f;
          }
        }
      }

    std::string make_name(const std::string& original, uint64_t i)
      {
      std::stringstream str;
      str << original << "_" << i;
      return str.str();
      }

    };
  }

void alpha_conversion(Program& prog, uint64_t& alpha_conversion_index, std::shared_ptr<environment<alpha_conversion_data>>& env, bool modify_names)
  {
  g_unhandled_sets.clear();
  //alpha_conversion_visitor acv(alpha_conversion_index, env, modify_names);
  //visitor<Program, alpha_conversion_visitor>::visit(prog, &acv);

  alpha_conversion_helper ach(alpha_conversion_index, env, modify_names);
  for (auto& expr : prog.expressions)
    ach.expressions.push_back(&expr);
  std::reverse(ach.expressions.begin(), ach.expressions.end());
  ach.treat_expressions();

  //env = acv.env;
  env = ach.env.back();
  env->rollup();

  for (auto& unhandled_set : g_unhandled_sets)
    {
    alpha_conversion_data new_name;
    if (!env->find(new_name, unhandled_set->name))
      {
      throw std::runtime_error("alpha conversion: set! error");
      //could not resolve this set!, so treat it as a define
      //new_name.name = acv.make_name(unhandled_set->name, acv.index++);
      //unhandled_set->originates_from_define = true;
      //env->push(unhandled_set->name, new_name);
      //unhandled_set->name = new_name.name;
      }
    else
      unhandled_set->name = new_name.name;
    }


  //alpha_conversion_index = acv.index;
  alpha_conversion_index = ach.index.back();
  prog.alpha_converted = true;
  }

std::string get_variable_name_before_alpha(const std::string& variable_name_after_alpha)
  {
  auto pos = variable_name_after_alpha.find_last_of('_');
  std::string name_before_alpha_conversion = variable_name_after_alpha.substr(0, pos);
  if (pos != std::string::npos)
    {
    std::string alpha_index_nr = variable_name_after_alpha.substr(pos + 1);
    bool is_number = !alpha_index_nr.empty() &&
      std::find_if(alpha_index_nr.begin(), alpha_index_nr.end(), [](char c) { return !std::isdigit(c); }) == alpha_index_nr.end();
    if (!is_number)
      name_before_alpha_conversion = variable_name_after_alpha;
    }
  return name_before_alpha_conversion;
  }

SKIWI_END
