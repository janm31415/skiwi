#include "free_var_analysis.h"
#include "visitor.h"
#include <set>
#include <string>
#include <algorithm>
#include <iterator>

SKIWI_BEGIN

namespace
  {
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
  free_var_analysis_visitor fvav;
  fvav.env = env;  
  visitor<Program, free_var_analysis_visitor>::visit(prog, &fvav);
  prog.free_variables_analysed = true;
  }

void free_variable_analysis(Lambda& lam, environment_map& env)
  {
  free_var_analysis_visitor fvav;
  fvav.env = env;
  visitor<Lambda, free_var_analysis_visitor>::visit(lam, &fvav);
  }

SKIWI_END
