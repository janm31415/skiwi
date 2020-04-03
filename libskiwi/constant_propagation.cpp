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
                replace_variable_visitor rvv;
                rvv.var_name = it->first;
                rvv.replace_by_this_expr = lit;
                visitor<Expression, replace_variable_visitor>::visit(l.body.front(), &rvv);
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
                replace_variable_visitor rvv;
                rvv.var_name = it->first;
                rvv.replace_by_this_expr = std::get<Variable>(it->second);
                visitor<Expression, replace_variable_visitor>::visit(l.body.front(), &rvv);
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

  is_unmutable_variable_visitor uvv;
  visitor<Program, is_unmutable_variable_visitor>::visit(prog, &uvv);

  constant_propagation_visitor cpv;
  cpv.p_is_unmutable = &uvv.is_unmutable;
  visitor<Program, constant_propagation_visitor>::visit(prog, &cpv);

  prog.constant_propagated = true;
  }

SKIWI_END
