#include "constant_propagation.h"

#include <cassert>
#include <sstream>
#include <inttypes.h>
#include <stdio.h>
#include <map>

#include "compile_error.h"
#include "visitor.h"

SKIWI_BEGIN

namespace
  {

  struct is_unmutable_variable_helper
    {
    std::vector<Expression*> expressions;
    std::map<std::string, bool> is_unmutable;
      
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
          Variable& v = std::get<Variable>(e);
          auto it = is_unmutable.find(v.name);
          if (it == is_unmutable.end())
            {
            is_unmutable[v.name] = false;
            }
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
          is_unmutable[s.name] = false;
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
          for (auto& v : l.variables)
            {
            is_unmutable[v] = false;
            }
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
          for (auto& b : l.bindings)
            {
            auto it = is_unmutable.find(b.first);
            if (it == is_unmutable.end())
              {
              is_unmutable[b.first] = true;
              }
            }
          expressions.push_back(&l.body.front());
          for (auto rit = l.bindings.rbegin(); rit != l.bindings.rend(); ++rit)
            expressions.push_back(&(*rit).second);
          }
        else
          throw std::runtime_error("Compiler error!: is unmutable variable helper: not implemented");
        }
      }
    };

  struct is_unmutable_variable_visitor : public base_visitor<is_unmutable_variable_visitor>
    {
    std::map<std::string, bool> is_unmutable;

    virtual bool _previsit(Let& l)
      {
      for (auto& b : l.bindings)
        {
        auto it = is_unmutable.find(b.first);
        if (it == is_unmutable.end())
          {
          is_unmutable[b.first] = true;
          }
        }
      return true;
      }

    virtual bool _previsit(Lambda& l)
      {
      for (auto& v : l.variables)
        {
        is_unmutable[v] = false;
        }
      return true;
      }

    virtual bool _previsit(Variable& v)
      {
      auto it = is_unmutable.find(v.name);
      if (it == is_unmutable.end())
        {
        is_unmutable[v.name] = false;
        }
      return true;
      }

    virtual bool _previsit(Set& s)
      {
      is_unmutable[s.name] = false;
      return true;
      }

    };
    
  struct replace_variable_helper
    {
    std::vector<Expression*> expressions;
    std::string var_name;
    Expression replace_by_this_expr;
      
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
          Variable& v = std::get<Variable>(e);
          if (v.name == var_name)
            e = replace_by_this_expr;
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
          throw std::runtime_error("Compiler error!: replace variable helper: not implemented");
        }
      }
    };

  struct replace_variable_visitor : public base_visitor<replace_variable_visitor>
    {
    std::string var_name;
    Expression replace_by_this_expr;

    virtual void _postvisit(Expression& e)
      {
      if (std::holds_alternative<Variable>(e) && (std::get<Variable>(e).name == var_name))
        {
        e = replace_by_this_expr;
        }
      }

    };


  struct constant_propagation_state
    {
    enum struct e_cc_state
      {
      T_INIT,
      T_STEP_1
      };
    Expression* p_expr;
    e_cc_state state;

    constant_propagation_state() : p_expr(nullptr), state(e_cc_state::T_INIT) {}
    constant_propagation_state(Expression* ip_expr) : p_expr(ip_expr), state(e_cc_state::T_INIT) {}
    constant_propagation_state(Expression* ip_expr, e_cc_state s) : p_expr(ip_expr), state(s) {}
    };

  struct constant_propagation_helper
    {
    std::vector<constant_propagation_state> expressions;
    std::map<std::string, bool>* p_is_unmutable;
    
    void treat_let(Expression& e)
      {
      Let& l = std::get<Let>(e);
      auto it = l.bindings.begin();
      while (it != l.bindings.end())
        {
        auto iter = p_is_unmutable->find(it->first);
        bool is_unmutable = false;
        if (iter != p_is_unmutable->end())
          is_unmutable = iter->second;
        if (is_unmutable)
          {
          if (std::holds_alternative<Literal>(it->second))
            {
            Literal& lit = std::get<Literal>(it->second);
            if (!std::holds_alternative<String>(lit))
              {
              //replace_variable_visitor rvv;
              //rvv.var_name = it->first;
              //rvv.replace_by_this_expr = lit;
              //visitor<Expression, replace_variable_visitor>::visit(l.body.front(), &rvv);
              replace_variable_helper rvh;
              rvh.var_name = it->first;
              rvh.replace_by_this_expr = lit;
              rvh.expressions.push_back(&l.body.front());
              rvh.treat_expressions();
              it = l.bindings.erase(it);
              }
            else
              ++it;
            }
          else if (std::holds_alternative<Variable>(it->second))
            {
            bool var_is_unmutable = false;
            auto var_iter = p_is_unmutable->find(std::get<Variable>(it->second).name);
            if (var_iter != p_is_unmutable->end())
              var_is_unmutable = var_iter->second;
            if (var_is_unmutable)
              {
              //replace_variable_visitor rvv;
              //rvv.var_name = it->first;
              //rvv.replace_by_this_expr = std::get<Variable>(it->second);
              //visitor<Expression, replace_variable_visitor>::visit(l.body.front(), &rvv);
              replace_variable_helper rvh;
              rvh.var_name = it->first;
              rvh.replace_by_this_expr = std::get<Variable>(it->second);
              rvh.expressions.push_back(&l.body.front());
              rvh.treat_expressions();
              it = l.bindings.erase(it);
              }
            else
              ++it;
            }
          else
            ++it;
          }
        else
          ++it;
        }
      if (l.bindings.empty())
        {
        Expression new_expr;
        if (std::holds_alternative<Begin>(l.body.front()) && (std::get<Begin>(l.body.front()).arguments.size() == 1))
          {
          new_expr = std::move(std::get<Begin>(l.body.front()).arguments.front());
          }
        else
          new_expr = std::move(l.body.front());
        e = std::move(new_expr);
        }
      }
    
 
    void treat_expressions()
      {
      while (!expressions.empty())
        {
        constant_propagation_state cp_state = expressions.back();
        expressions.pop_back();
        Expression& e = *cp_state.p_expr;
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
          expressions.push_back(&std::get<FunCall>(e).fun.front());
          for (auto rit = std::get<FunCall>(e).arguments.rbegin(); rit != std::get<FunCall>(e).arguments.rend(); ++rit)
            expressions.push_back(&(*rit));
          }
        else if (std::holds_alternative<Let>(e))
          {
          if (cp_state.state == constant_propagation_state::e_cc_state::T_INIT)
            {
            Let& l = std::get<Let>(e);
            expressions.emplace_back(&e, constant_propagation_state::e_cc_state::T_STEP_1);
            expressions.push_back(&l.body.front());
            for (auto rit = l.bindings.rbegin(); rit != l.bindings.rend(); ++rit)
              expressions.push_back(&(*rit).second);
            }
          else
            {
            treat_let(e);
            }
          }
        else
          throw std::runtime_error("Compiler error!: constant propagation: not implemented");
        }
      }
    };

  struct constant_propagation_visitor : public base_visitor<constant_propagation_visitor>
    {
    std::map<std::string, bool>* p_is_unmutable;

    virtual void _postvisit(Expression& e)
      {
      if (std::holds_alternative<Let>(e))
        {
        Let& l = std::get<Let>(e);
        auto it = l.bindings.begin();
        while (it != l.bindings.end())
          {
          auto iter = p_is_unmutable->find(it->first);
          bool is_unmutable = false;
          if (iter != p_is_unmutable->end())
            is_unmutable = iter->second;
          if (is_unmutable)
            {
            if (std::holds_alternative<Literal>(it->second))
              {
              Literal& lit = std::get<Literal>(it->second);
              if (!std::holds_alternative<String>(lit))
                {
                //replace_variable_visitor rvv;
                //rvv.var_name = it->first;
                //rvv.replace_by_this_expr = lit;
                //visitor<Expression, replace_variable_visitor>::visit(l.body.front(), &rvv);
                replace_variable_helper rvh;
                rvh.var_name = it->first;
                rvh.replace_by_this_expr = lit;
                rvh.expressions.push_back(&l.body.front());
                rvh.treat_expressions();
                it = l.bindings.erase(it);
                }
              else
                ++it;
              }
            else if (std::holds_alternative<Variable>(it->second))
              {
              bool var_is_unmutable = false;
              auto var_iter = p_is_unmutable->find(std::get<Variable>(it->second).name);
              if (var_iter != p_is_unmutable->end())
                var_is_unmutable = var_iter->second;
              if (var_is_unmutable)
                {
                //replace_variable_visitor rvv;
                //rvv.var_name = it->first;
                //rvv.replace_by_this_expr = std::get<Variable>(it->second);
                //visitor<Expression, replace_variable_visitor>::visit(l.body.front(), &rvv);
                replace_variable_helper rvh;
                rvh.var_name = it->first;
                rvh.replace_by_this_expr = std::get<Variable>(it->second);
                rvh.expressions.push_back(&l.body.front());
                rvh.treat_expressions();
                it = l.bindings.erase(it);
                }
              else
                ++it;
              }
            else
              ++it;
            }
          else
            ++it;
          }
        if (l.bindings.empty())
          {
          Expression new_expr;
          if (std::holds_alternative<Begin>(l.body.front()) && (std::get<Begin>(l.body.front()).arguments.size() == 1))
            {
            new_expr = std::move(std::get<Begin>(l.body.front()).arguments.front());
            }
          else
            new_expr = std::move(l.body.front());
          e = std::move(new_expr);
          }
        }
      }
 
    };
  }

void constant_propagation(Program& prog)
  {    
  //assert(prog.alpha_converted);

  //is_unmutable_variable_visitor uvv;
  //visitor<Program, is_unmutable_variable_visitor>::visit(prog, &uvv);
  is_unmutable_variable_helper iuvh;
  for (auto& expr: prog.expressions)
    iuvh.expressions.push_back(&expr);
  std::reverse(iuvh.expressions.begin(), iuvh.expressions.end());
  iuvh.treat_expressions();

  //constant_propagation_visitor cpv;
  //cpv.p_is_unmutable = &uvv.is_unmutable;
  //cpv.p_is_unmutable = &iuvh.is_unmutable;
  //visitor<Program, constant_propagation_visitor>::visit(prog, &cpv);
  
  constant_propagation_helper cph;
  cph.p_is_unmutable = &iuvh.is_unmutable;
  for (auto& expr: prog.expressions)
    cph.expressions.push_back(&expr);
  std::reverse(cph.expressions.begin(), cph.expressions.end());
  cph.treat_expressions();

  prog.constant_propagated = true;
  }

SKIWI_END
