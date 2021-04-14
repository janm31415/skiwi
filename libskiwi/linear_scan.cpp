#include "linear_scan.h"
#include "liveness_range.h"
#include "visitor.h"
#include <cassert>
#include <variant>

#include "concurrency.h"

SKIWI_BEGIN

namespace
  {
  struct get_scan_index_helper
    {
    uint64_t si;

    template <class F>
    void operator()(F& i)
      {
      si = i.scan_index;
      }

    void operator()(Literal& i)
      {
      get_scan_index_helper gsih;
      std::visit(gsih, i);
      si = gsih.si;
      }
    };

  uint64_t get_scan_index(Expression& expr)
    {
    get_scan_index_helper gsih;
    std::visit(gsih, expr);
    return gsih.si;
    }

  struct get_pre_scan_index_helper
    {
    uint64_t si;

    template <class F>
    void operator()(F& i)
      {
      si = i.pre_scan_index;
      }

    void operator()(Literal& i)
      {
      get_pre_scan_index_helper gsih;
      std::visit(gsih, i);
      si = gsih.si;
      }
    };

  uint64_t get_pre_scan_index(Expression& expr)
    {
    get_pre_scan_index_helper gpsih;
    std::visit(gpsih, expr);
    return gpsih.si;
    }

  struct naive_scan_visitor : public base_visitor<naive_scan_visitor>
    {
    virtual bool _previsit(Let& obj)
      {
      obj.live_ranges.clear();
      for (size_t i = 0; i < obj.bindings.size(); ++i)
        {
        liveness_range lr;
        lr.first = get_scan_index(obj.bindings[i].second);
        lr.last = obj.scan_index;
        obj.live_ranges.push_back(lr);
        }
      return true;
      }

    virtual bool _previsit(Lambda& obj)
      {
      obj.live_ranges.clear();
      for (size_t i = 0; i < obj.variables.size(); ++i)
        {
        liveness_range lr;
        lr.first = get_pre_scan_index(obj.body.front());
        lr.last = obj.scan_index;
        obj.live_ranges.push_back(lr);
        }
      return true;
      }
    };

  struct naive_scan_helper
    {
    std::vector<Expression*> expressions;

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
          l.live_ranges.clear();
          for (size_t i = 0; i < l.variables.size(); ++i)
            {
            liveness_range lr;
            lr.first = get_pre_scan_index(l.body.front());
            lr.last = l.scan_index;
            l.live_ranges.push_back(lr);
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
          expressions.push_back(&l.body.front());
          for (auto rit = l.bindings.rbegin(); rit != l.bindings.rend(); ++rit)
            expressions.push_back(&(*rit).second);
          l.live_ranges.clear();
          for (size_t i = 0; i < l.bindings.size(); ++i)
            {
            liveness_range lr;
            lr.first = get_scan_index(l.bindings[i].second);
            lr.last = l.scan_index;
            l.live_ranges.push_back(lr);
            }
          }
        else
          throw std::runtime_error("Compiler error!: Linear scan conversion: not implemented");
        }
      }
    };

  struct find_liveness_of_variable : public base_visitor<find_liveness_of_variable>
    {
    std::map<std::string, uint64_t> var_last_occurence_map;

    virtual void _postvisit(Variable& v)
      {
      auto it = var_last_occurence_map.find(v.name);
      if (it == var_last_occurence_map.end())
        var_last_occurence_map[v.name] = v.scan_index;
      else
        {
        if (v.scan_index > it->second)
          it->second = v.scan_index;
        }
      }
    };

  struct find_liveness_of_variable_helper
    {
    std::vector<Expression*> expressions;
    std::map<std::string, uint64_t> var_last_occurence_map;

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
          auto it = var_last_occurence_map.find(v.name);
          if (it == var_last_occurence_map.end())
            var_last_occurence_map[v.name] = v.scan_index;
          else
            {
            if (v.scan_index > it->second)
              it->second = v.scan_index;
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
          Let& l = std::get<Let>(e);
          expressions.push_back(&l.body.front());
          for (auto rit = l.bindings.rbegin(); rit != l.bindings.rend(); ++rit)
            expressions.push_back(&(*rit).second);
          }
        else
          throw std::runtime_error("Compiler error!: Linear scan: not implemented");
        }
      }
    };

  struct linear_scan_visitor : public base_visitor<linear_scan_visitor>
    {
    std::map<std::string, uint64_t>* p_var_last_occurence_map;
    virtual bool _previsit(Let& obj)
      {
      obj.live_ranges.clear();
      for (size_t i = 0; i < obj.bindings.size(); ++i)
        {
        liveness_range lr;
        lr.first = get_pre_scan_index(obj.body.front());
        lr.last = (*p_var_last_occurence_map)[obj.bindings[i].first];
        obj.live_ranges.push_back(lr);
        }
      return true;
      }

    virtual bool _previsit(Lambda& obj)
      {
      obj.live_ranges.clear();
      for (size_t i = 0; i < obj.variables.size(); ++i)
        {
        liveness_range lr;
        lr.first = get_pre_scan_index(obj.body.front());
        lr.last = (*p_var_last_occurence_map)[obj.variables[i]];
        obj.live_ranges.push_back(lr);
        }
      return true;
      }
    };

  struct linear_scan_helper
    {
    std::vector<Expression*> expressions;
    std::map<std::string, uint64_t>* p_var_last_occurence_map;

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
          l.live_ranges.clear();
          for (size_t i = 0; i < l.variables.size(); ++i)
            {
            liveness_range lr;
            lr.first = get_pre_scan_index(l.body.front());
            lr.last = (*p_var_last_occurence_map)[l.variables[i]];
            l.live_ranges.push_back(lr);
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
          expressions.push_back(&l.body.front());
          for (auto rit = l.bindings.rbegin(); rit != l.bindings.rend(); ++rit)
            expressions.push_back(&(*rit).second);
          l.live_ranges.clear();
          for (size_t i = 0; i < l.bindings.size(); ++i)
            {
            liveness_range lr;
            lr.first = get_pre_scan_index(l.body.front());
            lr.last = (*p_var_last_occurence_map)[l.bindings[i].first];
            l.live_ranges.push_back(lr);
            }
          }
        else
          throw std::runtime_error("Compiler error!: linear scan: not implemented");
        }
      }
    };
  }

void linear_scan(Program& prog, linear_scan_algorithm lsa, const compiler_options& ops)
  {
  assert(prog.linear_scan_indices_computed);
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
        if (lsa == lsa_naive)
          {
          //naive_scan_visitor nsv;
          //visitor<Expression, naive_scan_visitor>::visit(arg, &nsv);
          naive_scan_helper nsh;
          nsh.expressions.push_back(&arg);
          nsh.treat_expressions();
          }
        else
          {
          //find_liveness_of_variable flov;
          //visitor<Expression, find_liveness_of_variable>::visit(arg, &flov);
          //linear_scan_visitor lsv;
          //lsv.p_var_last_occurence_map = &flov.var_last_occurence_map;
          //visitor<Expression, linear_scan_visitor>::visit(arg, &lsv);
          find_liveness_of_variable_helper flovh;
          flovh.expressions.push_back(&arg);
          flovh.treat_expressions();
          linear_scan_helper lsh;
          lsh.p_var_last_occurence_map = &flovh.var_last_occurence_map;
          lsh.expressions.push_back(&arg);
          lsh.treat_expressions();
          }
        }, ops);
      }
    else
      {
      if (lsa == lsa_naive)
        {
        //naive_scan_visitor nsv;
        //visitor<Program, naive_scan_visitor>::visit(prog, &nsv);
        naive_scan_helper nsh;
        for (auto& expr : prog.expressions)
          nsh.expressions.push_back(&expr);
        std::reverse(nsh.expressions.begin(), nsh.expressions.end());
        nsh.treat_expressions();
        }
      else
        {
        //find_liveness_of_variable flov;
        //visitor<Program, find_liveness_of_variable>::visit(prog, &flov);
        //linear_scan_visitor lsv;
        //lsv.p_var_last_occurence_map = &flov.var_last_occurence_map;
        //visitor<Program, linear_scan_visitor>::visit(prog, &lsv);
        find_liveness_of_variable_helper flovh;
        for (auto& expr : prog.expressions)
          flovh.expressions.push_back(&expr);
        std::reverse(flovh.expressions.begin(), flovh.expressions.end());
        flovh.treat_expressions();
        linear_scan_helper lsh;
        lsh.p_var_last_occurence_map = &flovh.var_last_occurence_map;
        for (auto& expr : prog.expressions)
          lsh.expressions.push_back(&expr);
        std::reverse(lsh.expressions.begin(), lsh.expressions.end());
        lsh.treat_expressions();
        }
      }
    }

  prog.linear_scan = true;
  }

SKIWI_END
