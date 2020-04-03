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
  /*
  struct find_liveness_of_variable : public base_visitor<find_liveness_of_variable>
    {
    liveness_range lr;

    std::string var_name;

    virtual void _postvisit(Variable& v)
      {
      if (v.name == var_name)
        {
        if (lr.last < v.scan_index)
          lr.last = v.scan_index;
        }
      }


    };

  struct linear_scan_visitor : public base_visitor<linear_scan_visitor>
    {
    virtual bool _previsit(Let& obj)
      {
      obj.live_ranges.clear();
      for (size_t i = 0; i < obj.bindings.size(); ++i)
        {
        find_liveness_of_variable flv;
        flv.var_name = obj.bindings[i].first;
        flv.lr.first = get_scan_index(obj.bindings[i].second);
        flv.lr.last = flv.lr.first;
        visitor<Expression, find_liveness_of_variable>::visit(obj.body.front(), &flv);
        obj.live_ranges.push_back(flv.lr);
        }
      return true;
      }

    virtual bool _previsit(Lambda& obj)
      {
      obj.live_ranges.clear();
      for (size_t i = 0; i < obj.variables.size(); ++i)
        {
        find_liveness_of_variable flv;
        flv.var_name = obj.variables[i];
        flv.lr.first = get_pre_scan_index(obj.body.front());
        flv.lr.last = flv.lr.first;        
        visitor<Expression, find_liveness_of_variable>::visit(obj.body.front(), &flv);
        obj.live_ranges.push_back(flv.lr);
        }
      return true;
      }
    };
    */

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
          naive_scan_visitor nsv;
          visitor<Expression, naive_scan_visitor>::visit(arg, &nsv);
          }
        else
          {
          find_liveness_of_variable flov;
          visitor<Expression, find_liveness_of_variable>::visit(arg, &flov);
          linear_scan_visitor lsv;
          lsv.p_var_last_occurence_map = &flov.var_last_occurence_map;
          visitor<Expression, linear_scan_visitor>::visit(arg, &lsv);
          }
        }, ops);
      }
    else
      {
      if (lsa == lsa_naive)
        {
        naive_scan_visitor nsv;
        visitor<Program, naive_scan_visitor>::visit(prog, &nsv);
        }
      else
        {
        find_liveness_of_variable flov;
        visitor<Program, find_liveness_of_variable>::visit(prog, &flov);
        linear_scan_visitor lsv;
        lsv.p_var_last_occurence_map = &flov.var_last_occurence_map;
        visitor<Program, linear_scan_visitor>::visit(prog, &lsv);
        }
      }
    }
  /*
  if (lsa == lsa_naive)
    {
    naive_scan_visitor nsv;
    visitor<Program, naive_scan_visitor>::visit(prog, &nsv);
    }
  else
    {
    linear_scan_visitor lsv;
    visitor<Program, linear_scan_visitor>::visit(prog, &lsv);
    }
    */
  prog.linear_scan = true;
  }

SKIWI_END
