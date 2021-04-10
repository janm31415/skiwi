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
  /*
  struct find_assignable_variables : public base_visitor<find_assignable_variables>
    {
    virtual bool _previsit(Lambda& l)
      {
      find_assignable_variables new_v;
      visitor<Expression, find_assignable_variables>::visit(l.body.front(), &new_v);      
      l.assignable_variables = new_v.assignable_variables;      
      assignable_variables.insert(new_v.assignable_variables.begin(), new_v.assignable_variables.end());
      return false;
      }

    virtual bool _previsit(Let& l)
      {
      find_assignable_variables new_v;
      for (auto& var : l.bindings)
        {
        visitor<Expression, find_assignable_variables>::visit(var.second, &new_v);
        }
      visitor<Expression, find_assignable_variables>::visit(l.body.front(), &new_v);
      l.assignable_variables = new_v.assignable_variables;
      assignable_variables.insert(new_v.assignable_variables.begin(), new_v.assignable_variables.end());
      return false;
      }

    virtual bool _previsit(Set& s)
      {
      if (!s.originates_from_define && !s.originates_from_quote)
        assignable_variables.insert(s.name);
      return true;
      }

    std::set<std::string> assignable_variables;
    };
  */

  struct convert_assignable_variables : public base_visitor<convert_assignable_variables>
    {
    std::shared_ptr<environment<bool>> env;

    convert_assignable_variables()
      {
      env = std::make_shared<environment<bool>>(nullptr);
      }

    convert_assignable_variables(const std::shared_ptr<environment<bool>>& p_outer)
      {
      env = std::make_shared<environment<bool>>(p_outer);
      }


    virtual bool _previsit(Lambda& l)
      {
      convert_assignable_variables new_v(env);
      std::vector<std::pair<std::string, Expression>> new_bindings;

      for (size_t i = 0; i < l.variables.size(); ++i)
        {
        const std::string& var_name = l.variables[i];
        if (l.assignable_variables.find(var_name) != l.assignable_variables.end())
          {
          std::string original = var_name;
          std::string adapted = "#%" + var_name;
          new_v.env->push(original, true);
          env->push(adapted, false);
          Variable var;
          var.name = adapted;
          PrimitiveCall prim;
          prim.primitive_name = "vector";
          prim.arguments.emplace_back(std::move(var));
          new_bindings.emplace_back(original, prim);
          l.variables[i] = adapted;
          }
        else
          new_v.env->push(var_name, false);
        }
      if (new_bindings.empty())
        {
        visitor<Expression, convert_assignable_variables>::visit(l.body.front(), &new_v);
        }
      else
        {
        Let new_let;
        new_let.bindings.swap(new_bindings);
        new_let.body.swap(l.body);
        Begin new_begin;
        new_begin.arguments.emplace_back(std::move(new_let));
        l.body.emplace_back(std::move(new_begin));
        visitor<Expression, convert_assignable_variables>::visit(std::get<Let>(std::get<Begin>(l.body.front()).arguments.front()).body.front(), &new_v);
        }
      return false;
      }

    virtual bool _previsit(Let& l)
      {
      convert_assignable_variables new_v(env);
      std::vector<std::pair<std::string, Expression>> new_bindings;

      for (auto& binding : l.bindings)
        {
        visitor<Expression, convert_assignable_variables>::visit(binding.second, &new_v);
        if (l.assignable_variables.find(binding.first) != l.assignable_variables.end())
          {
          std::string original = binding.first;
          std::string adapted = "#%" + original;
          new_v.env->push(original, true);
          env->push(adapted, false);
          Variable var;
          var.name = adapted;
          PrimitiveCall prim;
          prim.primitive_name = "vector";
          prim.arguments.emplace_back(std::move(var));
          new_bindings.emplace_back(original, prim);
          binding.first = adapted;
          }
        else
          new_v.env->push(binding.first, false);
        }
      if (new_bindings.empty())
        {
        visitor<Expression, convert_assignable_variables>::visit(l.body.front(), &new_v);
        }
      else
        {
        Let new_let;
        new_let.bindings.swap(new_bindings);
        new_let.body.swap(l.body);
        for (auto& binding : new_let.bindings)
          visitor<Expression, convert_assignable_variables>::visit(binding.second, &new_v);
        Begin new_begin;
        new_begin.arguments.emplace_back(std::move(new_let));
        l.body.emplace_back(std::move(new_begin));
        visitor<Expression, convert_assignable_variables>::visit(std::get<Let>(std::get<Begin>(l.body.front()).arguments.front()).body.front(), &new_v);
        }
      return false;
      }

    virtual bool _previsit(Expression& e)
      {
      if (std::holds_alternative<Variable>(e))
        {
        Variable& v = std::get<Variable>(e);
        bool b;        
        if (!env->find(b, v.name))
          return true; // v.name must be a globally defined variable from another repl          
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
          return false;
          }
        }
      else if (std::holds_alternative<Set>(e))
        {
        Set& s = std::get<Set>(e);
        if (s.originates_from_define || s.originates_from_quote)
          {
          env->push(s.name, false);
          return true;
          }
        bool b;
        if (!env->find(b, s.name))
          return true; // this scenario is possible when s.name is a global variable defined in a repl before, triggered by test scheme_tests_4
          //throw std::runtime_error("error in assignable_variable_conversion");
        if (!b)
          return true;        
        visitor<Expression, convert_assignable_variables>::visit(s.value.front(), this);
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
        return false;
        }
      return true;
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
        //visitor<Expression, find_assignable_variables>::visit(arg, &fav);
        fav.expressions.push_back(&arg);
        fav.treat_expressions();
        convert_assignable_variables cav;
        visitor<Expression, convert_assignable_variables>::visit(arg, &cav);
        }, ops);
      }
    else
      {
      find_assignable_variables fav;
      //visitor<Program, find_assignable_variables>::visit(prog, &fav);
      for (auto& expr : prog.expressions)
        fav.expressions.push_back(&expr);
      std::reverse(fav.expressions.begin(), fav.expressions.end());
      fav.treat_expressions();
      convert_assignable_variables cav;
      visitor<Program, convert_assignable_variables>::visit(prog, &cav);     
      }
    }
  prog.assignable_variables_converted = true;
  }

SKIWI_END
