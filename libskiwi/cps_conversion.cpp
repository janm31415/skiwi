#include "cps_conversion.h"
#include "tail_call_analysis.h"
#include "tail_calls_check.h"
#include "visitor.h"
#include "debug_find.h"

#include <cassert>
#include <sstream>
#include <map>
#include <deque>
#include <list>

#include "concurrency.h"

SKIWI_BEGIN

namespace
  {

  bool is_simple(const Expression& e)
    {
    std::vector<const Expression*> expressions;
    expressions.push_back(&e);
    while (!expressions.empty())
      {
      const Expression* p_expr = expressions.back();
      expressions.pop_back();
      if (std::holds_alternative<Literal>(*p_expr))
        continue;
      if (std::holds_alternative<Quote>(*p_expr))
        continue;
      if (std::holds_alternative<Variable>(*p_expr))
        continue;
      if (std::holds_alternative<PrimitiveCall>(*p_expr))
        {
        const PrimitiveCall& p = std::get<PrimitiveCall>(*p_expr);
        for (const auto& arg : p.arguments)
          expressions.push_back(&arg);
        continue;
        }
      if (std::holds_alternative<ForeignCall>(*p_expr))
        {
        const ForeignCall& p = std::get<ForeignCall>(*p_expr);
        for (const auto& arg : p.arguments)
          expressions.push_back(&arg);
        continue;
        }
      if (std::holds_alternative<Set>(*p_expr))
        {
        expressions.push_back(&std::get<Set>(*p_expr).value.front());
        continue;
        }
      if (std::holds_alternative<If>(*p_expr))
        {
        const If& i = std::get<If>(*p_expr);
        for (const auto& arg : i.arguments)
          expressions.push_back(&arg);
        continue;
        }
      if (std::holds_alternative<Let>(*p_expr))
        {
        const Let& l = std::get<Let>(*p_expr);
        for (const auto& arg : l.bindings)
          expressions.push_back(&arg.second);
        expressions.push_back(&l.body.front());
        continue;
        }
      if (std::holds_alternative<Begin>(*p_expr))
        {
        const Begin& b = std::get<Begin>(*p_expr);
        for (const auto& arg : b.arguments)
            expressions.push_back(&arg);
        continue;
        }
      if (std::holds_alternative<Nop>(*p_expr))
        {
        continue;
        }
      return false;
      }
    return true;
    }

  std::string make_var_name(uint64_t index)
    {
    std::stringstream str;
    str << "#%k" << index;
    return str.str();
    }
/*
  struct cps_conversion_visitor : public base_visitor<cps_conversion_visitor>
    {
    uint64_t index;
    Expression continuation;
    bool continuation_can_be_moved;

    cps_conversion_visitor() : continuation_can_be_moved(true) {}

    bool continuation_is_lambda()
      {
      return std::holds_alternative<Lambda>(continuation);
      }

    bool continuation_is_variable()
      {
      return std::holds_alternative<Variable>(continuation);
      }

    bool continuation_is_valid()
      {
      if (!continuation_is_lambda() && !continuation_is_variable())
        return false;
      Program prog;
      prog.expressions.push_back(continuation);
      tail_call_analysis(prog);
      return only_tail_calls(prog);
      }

    bool continuation_is_lambda_with_one_parameter_without_free_vars()
      {
      if (!continuation_is_lambda())
        return false;
      if (std::get<Lambda>(continuation).variables.size() != 1)
        return false;

      return true;
      }

    void cps_convert_literal_or_variable_or_nop_or_quote(Expression& e)
      {
      assert(std::holds_alternative<Literal>(e) || std::holds_alternative<Variable>(e) || std::holds_alternative<Nop>(e) || std::holds_alternative<Quote>(e));
      if (continuation_is_lambda_with_one_parameter_without_free_vars())
        {
        Lambda& lam = std::get<Lambda>(continuation);
        Let l;
        l.bindings.emplace_back(lam.variables.front(), std::move(e));
        if (continuation_can_be_moved)
          l.body.swap(lam.body);
        else
          l.body = lam.body;
        e = std::move(l);
        }
      else
        {
        FunCall f;
        if (continuation_can_be_moved)
          f.fun.emplace_back(std::move(continuation));
        else
          f.fun.push_back(continuation);
        f.arguments.emplace_back(std::move(e));
        e = std::move(f);
        }
      }

    void cps_convert_set_nonsimple(Expression& e)
      {
      assert(std::holds_alternative<Set>(e));
      Set& s = std::get<Set>(e);
      assert(!is_simple(s.value.front()));
      Expression e1 = s.value.front();

      cps_conversion_visitor ccv;
      ccv.index = index + 1;
      Lambda l;
      l.variables.emplace_back(make_var_name(ccv.index));
      Set new_s;
      new_s.name = s.name;
      new_s.originates_from_define = s.originates_from_define;
      new_s.originates_from_quote = s.originates_from_quote;
      Variable v;
      v.name = make_var_name(ccv.index);
      new_s.value.emplace_back(std::move(v));
      Begin b;

      if (continuation_is_lambda_with_one_parameter_without_free_vars())
        {
        Lambda& lam = std::get<Lambda>(continuation);
        Let let;
        let.bindings.emplace_back(lam.variables.front(), std::move(new_s));
        if (continuation_can_be_moved)
          let.body.swap(lam.body);
        else
          let.body = lam.body;
        b.arguments.emplace_back(std::move(let));
        }
      else
        {
        FunCall f;
        if (continuation_can_be_moved)
          f.fun.emplace_back(std::move(continuation));
        else
          f.fun.push_back(continuation);
        f.arguments.emplace_back(std::move(new_s));
        b.arguments.emplace_back(std::move(f));
        }
      l.body.emplace_back(std::move(b));

      //ccv.continuation = l;
      ccv.continuation = Lambda();
      std::swap(std::get<Lambda>(ccv.continuation), l); // this is a very substantial speedup trick!!
      assert(ccv.continuation_is_valid());
      e.swap(e1);
      visitor<Expression, cps_conversion_visitor>::visit(e, &ccv);      
      index = ccv.index;
      }

    void cps_convert_set_simple(Expression& e)
      {
      assert(std::holds_alternative<Set>(e));
      assert(is_simple(std::get<Set>(e).value.front()));
      if (continuation_is_lambda_with_one_parameter_without_free_vars())
        {
        Lambda& lam = std::get<Lambda>(continuation);
        Let l;
        l.bindings.emplace_back(lam.variables.front(), std::move(e));
        if (continuation_can_be_moved)
          l.body.swap(lam.body);
        else
          l.body = lam.body;
        e = std::move(l);
        }
      else
        {
        FunCall f;
        if (continuation_can_be_moved)
          f.fun.emplace_back(std::move(continuation));
        else
          f.fun.push_back(continuation);
        f.arguments.emplace_back(std::move(e));
        e = std::move(f);
        }
      }

    void cps_convert_set(Expression& e)
      {
      assert(std::holds_alternative<Set>(e));
      Set& s = std::get<Set>(e);

      if (is_simple(s.value.front()))
        cps_convert_set_simple(e);
      else
        cps_convert_set_nonsimple(e);

      assert(find(e, [](const Expression& e) {return std::holds_alternative<Set>(e); }));
      }

    void cps_convert_if(Expression& e)
      {
      assert(std::holds_alternative<If>(e));
      If& i = std::get<If>(e);

      continuation_can_be_moved = false;

      for (size_t j = 1; j < i.arguments.size(); ++j)
        visitor<Expression, cps_conversion_visitor>::visit(i.arguments[j], this);

      if (is_simple(i.arguments.front()))
        return;

      cps_conversion_visitor ccv;
      ccv.index = index + 1;
      Lambda l;
      l.variables.emplace_back(make_var_name(ccv.index));

      Variable v;
      v.name = make_var_name(ccv.index);

      Expression exp0 = std::move(i.arguments[0]);

      i.arguments[0] = v;

      Begin b;
      b.arguments.emplace_back(std::move(i));
      l.body.emplace_back(std::move(b));
      //ccv.continuation = l;
      ccv.continuation = Lambda();
      std::swap(std::get<Lambda>(ccv.continuation), l); // this is a very substantial speedup trick!!
      assert(ccv.continuation_is_valid());

      e = std::move(exp0);
      visitor<Expression, cps_conversion_visitor>::visit(e, &ccv);

      index = ccv.index;
      }

    void cps_convert_begin_simple(Expression& e)
      {
      assert(std::holds_alternative<Begin>(e));
      Begin& b = std::get<Begin>(e);      
      if (b.arguments.empty())
        {
        Nop n;
        b.arguments.emplace_back(Nop());                
        }
      assert(is_simple(e));
      visitor<Expression, cps_conversion_visitor>::visit(b.arguments.back(), this);
      }

    void cps_convert_begin(Expression& e)
      {
      assert(std::holds_alternative<Begin>(e));
      Begin& b = std::get<Begin>(e);
      if (is_simple(e) || b.arguments.empty())
        cps_convert_begin_simple(e);
      else if (b.arguments.size() == 1)
        {
        visitor<Expression, cps_conversion_visitor>::visit(b.arguments.front(), this);
        }
      else
        {
        size_t first_non_simple_expr = 0;
        for (; first_non_simple_expr < b.arguments.size(); ++first_non_simple_expr)
          {
          if (!is_simple(b.arguments[first_non_simple_expr]))
            break;
          }
        if (first_non_simple_expr == (b.arguments.size() - 1))
          {
          visitor<Expression, cps_conversion_visitor>::visit(b.arguments.back(), this);
          }
        else if (first_non_simple_expr > 0)
          {
          Begin new_b;
          new_b.arguments.insert(new_b.arguments.end(), b.arguments.begin(), b.arguments.begin() + first_non_simple_expr);
          b.arguments.erase(b.arguments.begin(), b.arguments.begin() + first_non_simple_expr);
          new_b.arguments.emplace_back(std::move(b));
          b = std::move(new_b);
          cps_convert_begin(b.arguments.back());
          }
        else
          {
          Begin remainder;
          remainder.arguments.reserve(b.arguments.size()-1);
          //remainder.arguments.insert(remainder.arguments.end(), b.arguments.begin() + 1, b.arguments.end());
          for (auto it = b.arguments.begin()+1; it != b.arguments.end(); ++it)
            remainder.arguments.emplace_back(std::move(*it));
          Expression e2(std::move(remainder));
          visitor<Expression, cps_conversion_visitor>::visit(e2, this);
          cps_conversion_visitor ccv;
          ccv.index = index + 1;
          Lambda l;
          l.variables.emplace_back(make_var_name(ccv.index));
          if (std::holds_alternative<Begin>(e2))
            l.body.emplace_back(std::move(e2));
          else
            {
            Begin lb;
            lb.arguments.emplace_back(std::move(e2));
            l.body.emplace_back(std::move(lb));
            //this scenario is triggered by CHECK_EQUAL("130", run("(letrec([f 12][g(lambda(n) (set! f n))])(g 130) f) "));
            }
          //ccv.continuation = l;
          ccv.continuation = Lambda();
          std::swap(std::get<Lambda>(ccv.continuation), l); // this is a very substantial speedup trick!!
          assert(ccv.continuation_is_valid());
          Expression arg = std::move(b.arguments[0]);
          e = std::move(arg);
          visitor<Expression, cps_conversion_visitor>::visit(e, &ccv);
          index = ccv.index;
          }
        }
      }

    void cps_convert_prim_nonsimple(Expression& e)
      {
      assert(std::holds_alternative<PrimitiveCall>(e));
      PrimitiveCall& p = std::get<PrimitiveCall>(e);
      if (p.arguments.empty())
        {
        cps_convert_prim_simple(e);
        return;
        }
      assert(!p.arguments.empty());
      std::map<size_t, std::string> nonsimple_vars;
      std::vector<bool> simple(p.arguments.size());
      for (size_t j = 0; j < p.arguments.size(); ++j)
        {
        simple[j] = is_simple(p.arguments[j]);
        if (!simple[j])
          {
          nonsimple_vars[j] = make_var_name(index + j + 1);
          }
        }
      index += p.arguments.size();

      Lambda bottom_l;
      bottom_l.variables.push_back(nonsimple_vars.rbegin()->second);
      Begin bottom_b;
      PrimitiveCall bottom_p;
      bottom_p.primitive_name = p.primitive_name;
      for (size_t j = 0; j < p.arguments.size(); ++j)
        {
        if (simple[j])
          bottom_p.arguments.emplace_back(std::move(p.arguments[j]));
        else
          {
          Variable v;
          v.name = nonsimple_vars.find(j)->second;
          bottom_p.arguments.emplace_back(std::move(v));
          }
        }
      if (continuation_is_lambda_with_one_parameter_without_free_vars())
        {
        Lambda& lam = std::get<Lambda>(continuation);
        Let let;
        let.bindings.emplace_back(lam.variables.front(), std::move(bottom_p));
        if (continuation_can_be_moved)
          let.body.swap(lam.body);
        else
          let.body = lam.body;
        bottom_b.arguments.emplace_back(std::move(let));
        }
      else
        {
        FunCall f;
        if (continuation_can_be_moved)
          f.fun.emplace_back(std::move(continuation));
        else
          f.fun.push_back(continuation);
        f.arguments.emplace_back(std::move(bottom_p));
        bottom_b.arguments.emplace_back(std::move(f));
        }
      bottom_l.body.emplace_back(std::move(bottom_b));

      cps_conversion_visitor ccv;
      ccv.index = index + 1;
      ccv.continuation = Lambda();
      std::swap(std::get<Lambda>(ccv.continuation), bottom_l); // this is a very substantial speedup trick!!
      assert(ccv.continuation_is_valid());
      visitor<Expression, cps_conversion_visitor>::visit(p.arguments[nonsimple_vars.rbegin()->first], &ccv);
      index = ccv.index;

      auto it = nonsimple_vars.rbegin();
      auto prev_it = it;
      ++it;
      for (; it != nonsimple_vars.rend(); ++it, ++prev_it)
        {
        Lambda l;
        l.variables.push_back(it->second);
        Begin b;
        b.arguments.emplace_back(std::move(p.arguments[prev_it->first]));
        l.body.emplace_back(std::move(b));
        cps_conversion_visitor ccv2;
        ccv2.index = index + 1;
        //ccv2.continuation = l;
        ccv2.continuation = Lambda();
        std::swap(std::get<Lambda>(ccv2.continuation), l); // this is a very substantial speedup trick!!
        assert(ccv2.continuation_is_valid());
        visitor<Expression, cps_conversion_visitor>::visit(p.arguments[it->first], &ccv2);
        index = ccv2.index;
        }

      Expression expr(std::move(p.arguments[nonsimple_vars.begin()->first]));
      e.swap(expr);
      }

    void cps_convert_prim_simple(Expression& e)
      {
      assert(std::holds_alternative<PrimitiveCall>(e));

      if (continuation_is_lambda_with_one_parameter_without_free_vars())
        {
        Lambda& lam = std::get<Lambda>(continuation);
        Let l;
        l.bindings.emplace_back(lam.variables.front(), std::move(e));
        if (continuation_can_be_moved)
          l.body.swap(lam.body);
        else
          l.body = lam.body;
        e = std::move(l);
        }
      else
        {
        FunCall f;
        if (continuation_can_be_moved)
          f.fun.emplace_back(std::move(continuation));
        else
          f.fun.push_back(continuation);
        f.arguments.emplace_back(std::move(e));
        e = std::move(f);
        }
      }

    void cps_convert_prim(Expression& e)
      {
      if (is_simple(e))
        cps_convert_prim_simple(e);
      else
        cps_convert_prim_nonsimple(e);
      }


    void cps_convert_foreign_nonsimple(Expression& e)
      {
      assert(std::holds_alternative<ForeignCall>(e));
      ForeignCall& p = std::get<ForeignCall>(e);
      assert(!p.arguments.empty());
      std::map<size_t, std::string> nonsimple_vars;
      std::vector<bool> simple(p.arguments.size());
      for (size_t j = 0; j < p.arguments.size(); ++j)
        {
        simple[j] = is_simple(p.arguments[j]);
        if (!simple[j])
          {
          nonsimple_vars[j] = make_var_name(index + j + 1);
          }
        }
      index += p.arguments.size();

      Lambda bottom_l;
      bottom_l.variables.push_back(nonsimple_vars.rbegin()->second);
      Begin bottom_b;
      ForeignCall bottom_p;
      bottom_p.foreign_name = p.foreign_name;
      for (size_t j = 0; j < p.arguments.size(); ++j)
        {
        if (simple[j])
          bottom_p.arguments.emplace_back(std::move(p.arguments[j]));
        else
          {
          Variable v;
          v.name = nonsimple_vars.find(j)->second;
          bottom_p.arguments.emplace_back(std::move(v));
          }
        }
      if (continuation_is_lambda_with_one_parameter_without_free_vars())
        {
        Lambda& lam = std::get<Lambda>(continuation);
        Let let;
        let.bindings.emplace_back(lam.variables.front(), std::move(bottom_p));
        if (continuation_can_be_moved)
          let.body.swap(lam.body);
        else
          let.body = lam.body;
        bottom_b.arguments.emplace_back(std::move(let));
        }
      else
        {
        FunCall f;
        if (continuation_can_be_moved)
          f.fun.emplace_back(std::move(continuation));
        else
          f.fun.push_back(continuation);
        f.arguments.emplace_back(std::move(bottom_p));
        bottom_b.arguments.emplace_back(std::move(f));
        }
      bottom_l.body.emplace_back(std::move(bottom_b));

      cps_conversion_visitor ccv;
      ccv.index = index + 1;
      ccv.continuation = Lambda();
      std::swap(std::get<Lambda>(ccv.continuation), bottom_l); // this is a very substantial speedup trick!!
      assert(ccv.continuation_is_valid());
      visitor<Expression, cps_conversion_visitor>::visit(p.arguments[nonsimple_vars.rbegin()->first], &ccv);
      index = ccv.index;

      auto it = nonsimple_vars.rbegin();
      auto prev_it = it;
      ++it;
      for (; it != nonsimple_vars.rend(); ++it, ++prev_it)
        {
        Lambda l;
        l.variables.push_back(it->second);
        Begin b;
        b.arguments.emplace_back(std::move(p.arguments[prev_it->first]));
        l.body.emplace_back(std::move(b));
        cps_conversion_visitor ccv2;
        ccv2.index = index + 1;
        //ccv2.continuation = l;
        ccv2.continuation = Lambda();
        std::swap(std::get<Lambda>(ccv2.continuation), l); // this is a very substantial speedup trick!!
        assert(ccv2.continuation_is_valid());
        visitor<Expression, cps_conversion_visitor>::visit(p.arguments[it->first], &ccv2);
        index = ccv2.index;
        }

      Expression expr = std::move(p.arguments[nonsimple_vars.begin()->first]);
      e.swap(expr);
      }

    void cps_convert_foreign_simple(Expression& e)
      {
      assert(std::holds_alternative<ForeignCall>(e));

      if (continuation_is_lambda_with_one_parameter_without_free_vars())
        {
        Lambda& lam = std::get<Lambda>(continuation);
        Let l;
        l.bindings.emplace_back(lam.variables.front(), std::move(e));
        if (continuation_can_be_moved)
          l.body.swap(lam.body);
        else
          l.body = lam.body;
        e = std::move(l);
        }
      else
        {
        FunCall f;
        if (continuation_can_be_moved)
          f.fun.emplace_back(std::move(continuation));
        else
          f.fun.push_back(continuation);
        f.arguments.emplace_back(std::move(e));
        e = std::move(f);
        }
      }

    void cps_convert_foreign(Expression& e)
      {
      if (is_simple(e))
        cps_convert_foreign_simple(e);
      else
        cps_convert_foreign_nonsimple(e);
      }

    void cps_convert_lambda(Expression& e)
      {
      assert(std::holds_alternative<Lambda>(e));
      Lambda& l = std::get<Lambda>(e);
      cps_conversion_visitor ccv;
      ccv.index = index + 1;
      Variable k;
      k.name = make_var_name(ccv.index);
      ccv.continuation = k;
      assert(ccv.continuation_is_valid());
      visitor<Expression, cps_conversion_visitor>::visit(l.body.front(), &ccv);
      index = ccv.index;
      l.variables.insert(l.variables.begin(), k.name);

      if (continuation_is_lambda_with_one_parameter_without_free_vars())
        {
        Lambda& lam = std::get<Lambda>(continuation);
        Let let;
        let.bindings.emplace_back(lam.variables.front(), std::move(l));
        if (continuation_can_be_moved)
          let.body.swap(lam.body);
        else
          let.body = lam.body;
        e = std::move(let);
        }
      else
        {
        FunCall f;
        if (continuation_can_be_moved)
          f.fun.emplace_back(std::move(continuation));
        else
          f.fun.push_back(continuation);
        f.arguments.emplace_back(std::move(l));
        e = std::move(f);
        }
      }

    void cps_convert_funcall(Expression& e)
      {
      assert(std::holds_alternative<FunCall>(e));
      FunCall& f = std::get<FunCall>(e);

      std::vector<Expression> arguments;
      arguments.reserve(f.arguments.size() + 1);
      arguments.emplace_back(std::move(f.fun.front()));
      for (auto& arg : f.arguments)
        arguments.emplace_back(std::move(arg));

      std::vector<bool> simple_arg(arguments.size());

      std::map<size_t, std::string> nonsimple_vars;

      std::string r0 = make_var_name(index + 1);
      nonsimple_vars[0] = r0;
      for (size_t j = 1; j < arguments.size(); ++j)
        {
        simple_arg[j] = is_simple(arguments[j]);
        if (!simple_arg[j])
          {
          nonsimple_vars[j] = make_var_name(index + j + 1);
          }
        }
      index += arguments.size();


      Lambda bottom_l;
      bottom_l.variables.push_back(nonsimple_vars.rbegin()->second);
      Begin bottom_b;
      FunCall bottom_f;
      Variable bottom_v;
      bottom_v.name = r0;
      bottom_f.fun.emplace_back(std::move(bottom_v));
      if (continuation_can_be_moved)
        bottom_f.arguments.emplace_back(std::move(continuation));
      else
        bottom_f.arguments.push_back(continuation);
      for (size_t j = 1; j < arguments.size(); ++j)
        {
        if (simple_arg[j])
          bottom_f.arguments.emplace_back(std::move(arguments[j]));
        else
          {
          Variable r;
          r.name = nonsimple_vars.find(j)->second;
          bottom_f.arguments.emplace_back(std::move(r));
          }
        }
      bottom_b.arguments.emplace_back(std::move(bottom_f));
      bottom_l.body.emplace_back(std::move(bottom_b));

      cps_conversion_visitor ccv;
      ccv.index = index + 1;
      ccv.continuation = Lambda();
      std::swap(std::get<Lambda>(ccv.continuation), bottom_l); // this is a very substantial speedup trick!!
      assert(ccv.continuation_is_valid());
      visitor<Expression, cps_conversion_visitor>::visit(arguments[nonsimple_vars.rbegin()->first], &ccv);
      index = ccv.index;

      auto it = nonsimple_vars.rbegin();
      ++it;
      for (; it != nonsimple_vars.rend(); ++it)
        {
        Lambda l;
        l.variables.push_back(it->second);
        Begin b;
        auto prev_it = it;
        --prev_it;
        b.arguments.emplace_back(std::move(arguments[prev_it->first]));
        l.body.emplace_back(std::move(b));
        cps_conversion_visitor ccv2;
        ccv2.index = index + 1;
        ccv2.continuation = Lambda();
        std::swap(std::get<Lambda>(ccv2.continuation), l); // this is a very substantial speedup trick!!
        assert(ccv2.continuation_is_valid());
        visitor<Expression, cps_conversion_visitor>::visit(arguments[it->first], &ccv2);
        index = ccv2.index;
        }

      Expression expr = std::move(arguments[nonsimple_vars.begin()->first]);
      e.swap(expr);
      }


    void cps_convert_let_nonsimple(Expression& e)
      {
      assert(std::holds_alternative<Let>(e));
      Let& l = std::get<Let>(e);
      _previsit(l.body.front());

      Expression expr(std::move(l.body.front()));
      for (size_t j = 0; j < l.bindings.size(); ++j)
        {
        size_t id = l.bindings.size() - j - 1;
        Lambda lam;
        lam.variables.push_back(l.bindings[id].first);
        if (std::holds_alternative<Begin>(expr))
          {
          lam.body.emplace_back(std::move(expr));
          }
        else
          {
          Begin b;
          b.arguments.emplace_back(std::move(expr));
          lam.body.emplace_back(std::move(b));
          }
        cps_conversion_visitor ccv;
        ccv.index = index + 1;
        ccv.continuation = Lambda();
        std::swap(std::get<Lambda>(ccv.continuation), lam); // this is a very substantial speedup trick!!
        assert(ccv.continuation_is_valid());
        visitor<Expression, cps_conversion_visitor>::visit(l.bindings[id].second, &ccv);
        expr = std::move(l.bindings[id].second);
        index = ccv.index;
        }
      e.swap(expr);
      }

    void cps_convert_let(Expression& e)
      {
      assert(std::holds_alternative<Let>(e));
      Let& l = std::get<Let>(e);

      std::vector<bool> simple(l.bindings.size());
      bool at_least_one_simple = false;
      bool all_simple = true;
      for (size_t j = 0; j < l.bindings.size(); ++j)
        {
        simple[j] = is_simple(l.bindings[j].second);
        at_least_one_simple |= simple[j];
        all_simple &= simple[j];
        }

      if (!at_least_one_simple)
        cps_convert_let_nonsimple(e);
      else if (all_simple)
        _previsit(l.body.front());
      else
        {
        std::vector<std::pair<std::string, Expression>> simple_bindings;
        std::vector<std::pair<std::string, Expression>> nonsimple_bindings;
        for (size_t j = 0; j < l.bindings.size(); ++j)
          {
          if (simple[j])
            simple_bindings.emplace_back(std::move(l.bindings[j]));
          else
            nonsimple_bindings.emplace_back(std::move(l.bindings[j]));
          }       
        Let new_let;
        new_let.bindings.swap(nonsimple_bindings);
        new_let.body.swap(l.body);
        new_let.bt = bt_let;
        l.body.emplace_back(Begin());
        std::get<Begin>(l.body.front()).arguments.emplace_back(std::move(new_let));
        l.bindings.swap(simple_bindings);
        cps_convert_let_nonsimple(std::get<Begin>(l.body.front()).arguments.front());
        }
      }

    virtual bool _previsit(Expression& e)
      {
      if (std::holds_alternative<Literal>(e))
        {
        cps_convert_literal_or_variable_or_nop_or_quote(e);
        return false;
        }
      else if (std::holds_alternative<Variable>(e))
        {
        cps_convert_literal_or_variable_or_nop_or_quote(e);
        return false;
        }
      else if (std::holds_alternative<Nop>(e))
        {
        cps_convert_literal_or_variable_or_nop_or_quote(e);
        return false;
        }
      else if (std::holds_alternative<Quote>(e))
        {
        cps_convert_literal_or_variable_or_nop_or_quote(e);
        return false;
        }
      else if (std::holds_alternative<Set>(e))
        {
        cps_convert_set(e);
        return false;
        }
      else if (std::holds_alternative<If>(e))
        {
        cps_convert_if(e);
        return false;
        }
      else if (std::holds_alternative<Begin>(e))
        {
        cps_convert_begin(e);
        return false;
        }
      else if (std::holds_alternative<PrimitiveCall>(e))
        {
        cps_convert_prim(e);
        return false;
        }
      else if (std::holds_alternative<ForeignCall>(e))
        {
        cps_convert_foreign(e);
        return false;
        }
      else if (std::holds_alternative<Lambda>(e))
        {
        cps_convert_lambda(e);
        return false;
        }
      else if (std::holds_alternative<FunCall>(e))
        {
        cps_convert_funcall(e);
        return false;
        }
      else if (std::holds_alternative<Let>(e))
        {
        cps_convert_let(e);
        return false;
        }
      return true;
      }
    };
    */
  }
/*
void cps_conversion_old(Program& prog, const compiler_options& ops)
  {
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
        cps_conversion_visitor ccv;
        ccv.index = 0;
        auto& arg = beg.arguments[i];
        Lambda l;
        l.variables.push_back(make_var_name(ccv.index));
        PrimitiveCall prim;
        prim.primitive_name = "halt";
        Variable v;
        v.name = make_var_name(ccv.index);
        prim.arguments.emplace_back(std::move(v));
        Begin b;
        b.arguments.emplace_back(std::move(prim));
        l.body.emplace_back(std::move(b));
        ccv.continuation = Lambda();
        std::swap(std::get<Lambda>(ccv.continuation), l); // this is a very substantial speedup trick!!
        assert(ccv.continuation_is_valid());
        visitor<Expression, cps_conversion_visitor>::visit(arg, &ccv);
        }, ops);
      }
    else
      {
      cps_conversion_visitor ccv;
      ccv.index = 0;
      Lambda l;
      l.variables.push_back(make_var_name(ccv.index));
      PrimitiveCall prim;
      prim.primitive_name = "halt";
      Variable v;
      v.name = make_var_name(ccv.index);
      prim.arguments.emplace_back(std::move(v));
      Begin b;
      b.arguments.emplace_back(std::move(prim));
      l.body.emplace_back(std::move(b));
      ccv.continuation = Lambda();
      std::swap(std::get<Lambda>(ccv.continuation), l); // this is a very substantial speedup trick!!
      assert(ccv.continuation_is_valid());
      visitor<Expression, cps_conversion_visitor>::visit(e, &ccv);
      }
    }
  prog.cps_converted = true;
  
  }
*/




namespace {

struct cps_conversion_helper
    {
    std::vector<uint64_t> index;
    std::vector<Expression> continuation;
    std::vector<bool> continuation_can_be_moved;
    
    std::vector<Expression*> expressions_to_treat;

    cps_conversion_helper() {}

    bool continuation_is_lambda()
      {
      return std::holds_alternative<Lambda>(continuation.back());
      }

    bool continuation_is_variable()
      {
      return std::holds_alternative<Variable>(continuation.back());
      }

    bool continuation_is_valid()
      {
      if (!continuation_is_lambda() && !continuation_is_variable())
        return false;
      Program prog;
      prog.expressions.push_back(continuation.back());
      tail_call_analysis(prog);
      return only_tail_calls(prog);
      }

    bool continuation_is_lambda_with_one_parameter_without_free_vars()
      {
      if (!continuation_is_lambda())
        return false;
      if (std::get<Lambda>(continuation.back()).variables.size() != 1)
        return false;

      return true;
      }

    void cps_convert_literal_or_variable_or_nop_or_quote(Expression& e)
      {
      assert(std::holds_alternative<Literal>(e) || std::holds_alternative<Variable>(e) || std::holds_alternative<Nop>(e) || std::holds_alternative<Quote>(e));
      if (continuation_is_lambda_with_one_parameter_without_free_vars())
        {
        Lambda& lam = std::get<Lambda>(continuation.back());
        Let l;
        l.bindings.emplace_back(lam.variables.front(), std::move(e));
        if (continuation_can_be_moved.back())
          l.body.swap(lam.body);
        else
          l.body = lam.body;
        e = std::move(l);
        }
      else
        {
        FunCall f;
        if (continuation_can_be_moved.back())
          f.fun.emplace_back(std::move(continuation.back()));
        else
          f.fun.push_back(continuation.back());
        f.arguments.emplace_back(std::move(e));
        e = std::move(f);
        }
      }

    void cps_convert_set_nonsimple(Expression& e)
      {
      assert(std::holds_alternative<Set>(e));
      Set& s = std::get<Set>(e);
      assert(!is_simple(s.value.front()));
      Expression e1 = s.value.front();

      //cps_conversion_visitor ccv;
      index.push_back(index.back()+1);
      //ccv.index = index.back() + 1;
      Lambda l;
      l.variables.emplace_back(make_var_name(index.back()));
      Set new_s;
      new_s.name = s.name;
      new_s.originates_from_define = s.originates_from_define;
      new_s.originates_from_quote = s.originates_from_quote;
      Variable v;
      v.name = make_var_name(index.back());
      new_s.value.emplace_back(std::move(v));
      Begin b;

      if (continuation_is_lambda_with_one_parameter_without_free_vars())
        {
        Lambda& lam = std::get<Lambda>(continuation.back());
        Let let;
        let.bindings.emplace_back(lam.variables.front(), std::move(new_s));
        if (continuation_can_be_moved.back())
          let.body.swap(lam.body);
        else
          let.body = lam.body;
        b.arguments.emplace_back(std::move(let));
        }
      else
        {
        FunCall f;
        if (continuation_can_be_moved.back())
          f.fun.emplace_back(std::move(continuation.back()));
        else
          f.fun.push_back(continuation.back());
        f.arguments.emplace_back(std::move(new_s));
        b.arguments.emplace_back(std::move(f));
        }
      l.body.emplace_back(std::move(b));

      //ccv.continuation = l;
      continuation.emplace_back(Lambda());
      std::swap(std::get<Lambda>(continuation.back()), l); // this is a very substantial speedup trick!!
      //ccv.continuation = Lambda();
      //std::swap(std::get<Lambda>(ccv.continuation), l); // this is a very substantial speedup trick!!
      assert(continuation_is_valid());
      e.swap(e1);
      //visitor<Expression, cps_conversion_visitor>::visit(e, &ccv);
      size_t current_size = expressions_to_treat.size();
      expressions_to_treat.push_back(&e);
      treat_expressions(current_size);
      //index.back() = ccv.index;
      index.pop_back();
      continuation.pop_back();
      }

    void cps_convert_set_simple(Expression& e)
      {
      assert(std::holds_alternative<Set>(e));
      assert(is_simple(std::get<Set>(e).value.front()));
      if (continuation_is_lambda_with_one_parameter_without_free_vars())
        {
        Lambda& lam = std::get<Lambda>(continuation.back());
        Let l;
        l.bindings.emplace_back(lam.variables.front(), std::move(e));
        if (continuation_can_be_moved.back())
          l.body.swap(lam.body);
        else
          l.body = lam.body;
        e = std::move(l);
        }
      else
        {
        FunCall f;
        if (continuation_can_be_moved.back())
          f.fun.emplace_back(std::move(continuation.back()));
        else
          f.fun.push_back(continuation.back());
        f.arguments.emplace_back(std::move(e));
        e = std::move(f);
        }
      }

    void cps_convert_set(Expression& e)
      {
      assert(std::holds_alternative<Set>(e));
      Set& s = std::get<Set>(e);

      if (is_simple(s.value.front()))
        cps_convert_set_simple(e);
      else
        cps_convert_set_nonsimple(e);

      assert(find(e, [](const Expression& e) {return std::holds_alternative<Set>(e); }));
      }

    void cps_convert_if(Expression& e)
      {
      assert(std::holds_alternative<If>(e));
      If& i = std::get<If>(e);

      continuation_can_be_moved.back() = false;

      size_t current_size = expressions_to_treat.size();
      for (size_t j = 1; j < i.arguments.size(); ++j)
          expressions_to_treat.push_back(&i.arguments[j]);
      treat_expressions(current_size);
          
      
      //for (size_t j = 1; j < i.arguments.size(); ++j)
      //  visitor<Expression, cps_conversion_visitor>::visit(i.arguments[j], this);

      if (is_simple(i.arguments.front()))
        return;

      //cps_conversion_visitor ccv;
      //ccv.index = index.back() + 1;
      index.push_back(index.back()+1);
      Lambda l;
      l.variables.emplace_back(make_var_name(index.back()));

      Variable v;
      v.name = make_var_name(index.back());

      Expression exp0 = std::move(i.arguments[0]);

      i.arguments[0] = v;

      Begin b;
      b.arguments.emplace_back(std::move(i));
      l.body.emplace_back(std::move(b));
      //ccv.continuation = l;
      //ccv.continuation = Lambda();
      //std::swap(std::get<Lambda>(ccv.continuation), l); // this is a very substantial speedup trick!!
      continuation.emplace_back(Lambda());
      std::swap(std::get<Lambda>(continuation.back()), l); // this is a very substantial speedup trick!!
      assert(continuation_is_valid());

      continuation_can_be_moved.push_back(true);
      e = std::move(exp0);
      //visitor<Expression, cps_conversion_visitor>::visit(e, &ccv);
      current_size = expressions_to_treat.size();
      expressions_to_treat.push_back(&e);
      treat_expressions(current_size);

      //index.back() = ccv.index;
      auto ind = index.back();
      index.pop_back();
      index.back() = ind;
      continuation.pop_back();
      continuation_can_be_moved.pop_back();
      }

    void cps_convert_begin_simple(Expression& e)
      {
      assert(std::holds_alternative<Begin>(e));
      Begin& b = std::get<Begin>(e);
      if (b.arguments.empty())
        {
        Nop n;
        b.arguments.emplace_back(Nop());
        }
      assert(is_simple(e));
      size_t current_size = expressions_to_treat.size();
      expressions_to_treat.push_back(&b.arguments.back());
      treat_expressions(current_size);
      //visitor<Expression, cps_conversion_visitor>::visit(b.arguments.back(), this);
      }

    void cps_convert_begin(Expression& e)
      {
      assert(std::holds_alternative<Begin>(e));
      Begin& b = std::get<Begin>(e);
      if (is_simple(e) || b.arguments.empty())
        cps_convert_begin_simple(e);
      else if (b.arguments.size() == 1)
        {
        size_t current_size = expressions_to_treat.size();
        expressions_to_treat.push_back(&b.arguments.front());
        treat_expressions(current_size);
        //visitor<Expression, cps_conversion_visitor>::visit(b.arguments.front(), this);
        }
      else
        {
        size_t first_non_simple_expr = 0;
        for (; first_non_simple_expr < b.arguments.size(); ++first_non_simple_expr)
          {
          if (!is_simple(b.arguments[first_non_simple_expr]))
            break;
          }
        if (first_non_simple_expr == (b.arguments.size() - 1))
          {
          size_t current_size = expressions_to_treat.size();
          expressions_to_treat.push_back(&b.arguments.back());
          treat_expressions(current_size);
          //visitor<Expression, cps_conversion_visitor>::visit(b.arguments.back(), this);
          }
        else if (first_non_simple_expr > 0)
          {
          Begin new_b;
          new_b.arguments.insert(new_b.arguments.end(), b.arguments.begin(), b.arguments.begin() + first_non_simple_expr);
          b.arguments.erase(b.arguments.begin(), b.arguments.begin() + first_non_simple_expr);
          new_b.arguments.emplace_back(std::move(b));
          b = std::move(new_b);
          cps_convert_begin(b.arguments.back());
          }
        else
          {
          Begin remainder;
          remainder.arguments.reserve(b.arguments.size()-1);
          //remainder.arguments.insert(remainder.arguments.end(), b.arguments.begin() + 1, b.arguments.end());
          for (auto it = b.arguments.begin()+1; it != b.arguments.end(); ++it)
            remainder.arguments.emplace_back(std::move(*it));
          Expression e2(std::move(remainder));
          size_t current_size = expressions_to_treat.size();
          expressions_to_treat.push_back(&e2);
          treat_expressions(current_size);
          //visitor<Expression, cps_conversion_visitor>::visit(e2, this);
          //cps_conversion_visitor ccv;
          //ccv.index = index.back() + 1;
          index.push_back(index.back()+1);
          Lambda l;
          l.variables.emplace_back(make_var_name(index.back()));
          if (std::holds_alternative<Begin>(e2))
            l.body.emplace_back(std::move(e2));
          else
            {
            Begin lb;
            lb.arguments.emplace_back(std::move(e2));
            l.body.emplace_back(std::move(lb));
            //this scenario is triggered by CHECK_EQUAL("130", run("(letrec([f 12][g(lambda(n) (set! f n))])(g 130) f) "));
            }
          //ccv.continuation = l;
          //continuation = Lambda();
          //std::swap(std::get<Lambda>(ccv.continuation), l); // this is a very substantial speedup trick!!
          continuation.emplace_back(Lambda());
          std::swap(std::get<Lambda>(continuation.back()), l); // this is a very substantial speedup trick!!
          assert(continuation_is_valid());
          Expression arg = std::move(b.arguments[0]);
          e = std::move(arg);
          //visitor<Expression, cps_conversion_visitor>::visit(e, &ccv);
          current_size = expressions_to_treat.size();
          expressions_to_treat.push_back(&e);
          treat_expressions(current_size);
          auto ind = index.back();
          index.pop_back();
          index.back() = ind;
          continuation.pop_back();
          //index.back() = ccv.index;
          }
        }
      }

    void cps_convert_prim_nonsimple(Expression& e)
      {
      assert(std::holds_alternative<PrimitiveCall>(e));
      PrimitiveCall& p = std::get<PrimitiveCall>(e);
      if (p.arguments.empty())
        {
        cps_convert_prim_simple(e);
        return;
        }
      assert(!p.arguments.empty());
      std::map<size_t, std::string> nonsimple_vars;
      std::vector<bool> simple(p.arguments.size());
      for (size_t j = 0; j < p.arguments.size(); ++j)
        {
        simple[j] = is_simple(p.arguments[j]);
        if (!simple[j])
          {
          nonsimple_vars[j] = make_var_name(index.back() + j + 1);
          }
        }
      index.back() += p.arguments.size();

      Lambda bottom_l;
      bottom_l.variables.push_back(nonsimple_vars.rbegin()->second);
      Begin bottom_b;
      PrimitiveCall bottom_p;
      bottom_p.primitive_name = p.primitive_name;
      for (size_t j = 0; j < p.arguments.size(); ++j)
        {
        if (simple[j])
          bottom_p.arguments.emplace_back(std::move(p.arguments[j]));
        else
          {
          Variable v;
          v.name = nonsimple_vars.find(j)->second;
          bottom_p.arguments.emplace_back(std::move(v));
          }
        }
      if (continuation_is_lambda_with_one_parameter_without_free_vars())
        {
        Lambda& lam = std::get<Lambda>(continuation.back());
        Let let;
        let.bindings.emplace_back(lam.variables.front(), std::move(bottom_p));
        if (continuation_can_be_moved.back())
          let.body.swap(lam.body);
        else
          let.body = lam.body;
        bottom_b.arguments.emplace_back(std::move(let));
        }
      else
        {
        FunCall f;
        if (continuation_can_be_moved.back())
          f.fun.emplace_back(std::move(continuation.back()));
        else
          f.fun.push_back(continuation.back());
        f.arguments.emplace_back(std::move(bottom_p));
        bottom_b.arguments.emplace_back(std::move(f));
        }
      bottom_l.body.emplace_back(std::move(bottom_b));

      //cps_conversion_visitor ccv;
      //ccv.index = index.back() + 1;
      index.push_back(index.back()+1);
      //ccv.continuation = Lambda();
      continuation.emplace_back(Lambda());
      //std::swap(std::get<Lambda>(ccv.continuation), bottom_l); // this is a very substantial speedup trick!!
      std::swap(std::get<Lambda>(continuation.back()), bottom_l); // this is a very substantial speedup trick!!
      assert(continuation_is_valid());
      //visitor<Expression, cps_conversion_visitor>::visit(p.arguments[nonsimple_vars.rbegin()->first], &ccv);
      size_t current_size = expressions_to_treat.size();
      expressions_to_treat.push_back(&p.arguments[nonsimple_vars.rbegin()->first]);
      treat_expressions(current_size);
      //index.back() = ccv.index;
      auto ind = index.back();
      index.pop_back();
      index.back() = ind;
      continuation.pop_back();

      auto it = nonsimple_vars.rbegin();
      auto prev_it = it;
      ++it;
      for (; it != nonsimple_vars.rend(); ++it, ++prev_it)
        {
        Lambda l;
        l.variables.push_back(it->second);
        Begin b;
        b.arguments.emplace_back(std::move(p.arguments[prev_it->first]));
        l.body.emplace_back(std::move(b));
        //cps_conversion_visitor ccv2;
        //ccv2.index = index.back() + 1;
        index.push_back(index.back()+1);
        //ccv2.continuation = l;
        //ccv2.continuation = Lambda();
        continuation.emplace_back(Lambda());
        //std::swap(std::get<Lambda>(ccv2.continuation), l); // this is a very substantial speedup trick!!
        std::swap(std::get<Lambda>(continuation.back()), l); // this is a very substantial speedup trick!!
        assert(continuation_is_valid());
        //visitor<Expression, cps_conversion_visitor>::visit(p.arguments[it->first], &ccv2);
        size_t current_size = expressions_to_treat.size();
        expressions_to_treat.push_back(&p.arguments[it->first]);
        treat_expressions(current_size);
        //index.back() = ccv2.index;
        ind = index.back();
        index.pop_back();
        index.back() = ind;
        continuation.pop_back();
        }

      Expression expr(std::move(p.arguments[nonsimple_vars.begin()->first]));
      e.swap(expr);
      }

    void cps_convert_prim_simple(Expression& e)
      {
      assert(std::holds_alternative<PrimitiveCall>(e));

      if (continuation_is_lambda_with_one_parameter_without_free_vars())
        {
        Lambda& lam = std::get<Lambda>(continuation.back());
        Let l;
        l.bindings.emplace_back(lam.variables.front(), std::move(e));
        if (continuation_can_be_moved.back())
          l.body.swap(lam.body);
        else
          l.body = lam.body;
        e = std::move(l);
        }
      else
        {
        FunCall f;
        if (continuation_can_be_moved.back())
          f.fun.emplace_back(std::move(continuation.back()));
        else
          f.fun.push_back(continuation.back());
        f.arguments.emplace_back(std::move(e));
        e = std::move(f);
        }
      }

    void cps_convert_prim(Expression& e)
      {
      if (is_simple(e))
        cps_convert_prim_simple(e);
      else
        cps_convert_prim_nonsimple(e);
      }


    void cps_convert_foreign_nonsimple(Expression& e)
      {
      assert(std::holds_alternative<ForeignCall>(e));
      ForeignCall& p = std::get<ForeignCall>(e);
      assert(!p.arguments.empty());
      std::map<size_t, std::string> nonsimple_vars;
      std::vector<bool> simple(p.arguments.size());
      for (size_t j = 0; j < p.arguments.size(); ++j)
        {
        simple[j] = is_simple(p.arguments[j]);
        if (!simple[j])
          {
          nonsimple_vars[j] = make_var_name(index.back() + j + 1);
          }
        }
      index.back() += p.arguments.size();

      Lambda bottom_l;
      bottom_l.variables.push_back(nonsimple_vars.rbegin()->second);
      Begin bottom_b;
      ForeignCall bottom_p;
      bottom_p.foreign_name = p.foreign_name;
      for (size_t j = 0; j < p.arguments.size(); ++j)
        {
        if (simple[j])
          bottom_p.arguments.emplace_back(std::move(p.arguments[j]));
        else
          {
          Variable v;
          v.name = nonsimple_vars.find(j)->second;
          bottom_p.arguments.emplace_back(std::move(v));
          }
        }
      if (continuation_is_lambda_with_one_parameter_without_free_vars())
        {
        Lambda& lam = std::get<Lambda>(continuation.back());
        Let let;
        let.bindings.emplace_back(lam.variables.front(), std::move(bottom_p));
        if (continuation_can_be_moved.back())
          let.body.swap(lam.body);
        else
          let.body = lam.body;
        bottom_b.arguments.emplace_back(std::move(let));
        }
      else
        {
        FunCall f;
        if (continuation_can_be_moved.back())
          f.fun.emplace_back(std::move(continuation.back()));
        else
          f.fun.push_back(continuation.back());
        f.arguments.emplace_back(std::move(bottom_p));
        bottom_b.arguments.emplace_back(std::move(f));
        }
      bottom_l.body.emplace_back(std::move(bottom_b));

      //cps_conversion_visitor ccv;
      //ccv.index = index.back() + 1;
      //ccv.continuation = Lambda();
      index.push_back(index.back()+1);
      continuation.emplace_back(Lambda());
      //std::swap(std::get<Lambda>(ccv.continuation), bottom_l); // this is a very substantial speedup trick!!
      std::swap(std::get<Lambda>(continuation.back()), bottom_l); // this is a very substantial speedup trick!!
      assert(continuation_is_valid());
      //visitor<Expression, cps_conversion_visitor>::visit(p.arguments[nonsimple_vars.rbegin()->first], &ccv);
      size_t current_size = expressions_to_treat.size();
      expressions_to_treat.push_back(&p.arguments[nonsimple_vars.rbegin()->first]);
      treat_expressions(current_size);
      //index.back() = ccv.index;
      auto ind = index.back();
      index.pop_back();
      index.back() = ind;
      continuation.pop_back();

      auto it = nonsimple_vars.rbegin();
      auto prev_it = it;
      ++it;
      for (; it != nonsimple_vars.rend(); ++it, ++prev_it)
        {
        Lambda l;
        l.variables.push_back(it->second);
        Begin b;
        b.arguments.emplace_back(std::move(p.arguments[prev_it->first]));
        l.body.emplace_back(std::move(b));
        //cps_conversion_visitor ccv2;
        //ccv2.index = index.back() + 1;
        //ccv2.continuation = l;
        //ccv2.continuation = Lambda();
        index.push_back(index.back()+1);
        continuation.emplace_back(Lambda());
        //std::swap(std::get<Lambda>(ccv2.continuation), l); // this is a very substantial speedup trick!!
        std::swap(std::get<Lambda>(continuation.back()), l); // this is a very substantial speedup trick!!
        assert(continuation_is_valid());
        //visitor<Expression, cps_conversion_visitor>::visit(p.arguments[it->first], &ccv2);
        current_size = expressions_to_treat.size();
        expressions_to_treat.push_back(&p.arguments[it->first]);
        treat_expressions(current_size);
        ind = index.back();
        index.pop_back();
        index.back() = ind;
        continuation.pop_back();
        }

      Expression expr = std::move(p.arguments[nonsimple_vars.begin()->first]);
      e.swap(expr);
      }

    void cps_convert_foreign_simple(Expression& e)
      {
      assert(std::holds_alternative<ForeignCall>(e));

      if (continuation_is_lambda_with_one_parameter_without_free_vars())
        {
        Lambda& lam = std::get<Lambda>(continuation.back());
        Let l;
        l.bindings.emplace_back(lam.variables.front(), std::move(e));
        if (continuation_can_be_moved.back())
          l.body.swap(lam.body);
        else
          l.body = lam.body;
        e = std::move(l);
        }
      else
        {
        FunCall f;
        if (continuation_can_be_moved.back())
          f.fun.emplace_back(std::move(continuation.back()));
        else
          f.fun.push_back(continuation.back());
        f.arguments.emplace_back(std::move(e));
        e = std::move(f);
        }
      }

    void cps_convert_foreign(Expression& e)
      {
      if (is_simple(e))
        cps_convert_foreign_simple(e);
      else
        cps_convert_foreign_nonsimple(e);
      }

    void cps_convert_lambda(Expression& e)
      {
      assert(std::holds_alternative<Lambda>(e));
      Lambda& l = std::get<Lambda>(e);
      //cps_conversion_visitor ccv;
      //ccv.index = index.back() + 1;
      index.push_back(index.back()+1);
      Variable k;
      k.name = make_var_name(index.back());
      //ccv.continuation = k;
      continuation.push_back(k);
      assert(continuation_is_valid());
      size_t current_size = expressions_to_treat.size();
      expressions_to_treat.push_back(&l.body.front());
      treat_expressions(current_size);
      //visitor<Expression, cps_conversion_visitor>::visit(l.body.front(), &ccv);
      //index.back() = ccv.index;
      auto ind = index.back();
      index.pop_back();
      index.back() = ind;
      continuation.pop_back();
      l.variables.insert(l.variables.begin(), k.name);

      if (continuation_is_lambda_with_one_parameter_without_free_vars())
        {
        Lambda& lam = std::get<Lambda>(continuation.back());
        Let let;
        let.bindings.emplace_back(lam.variables.front(), std::move(l));
        if (continuation_can_be_moved.back())
          let.body.swap(lam.body);
        else
          let.body = lam.body;
        e = std::move(let);
        }
      else
        {
        FunCall f;
        if (continuation_can_be_moved.back())
          f.fun.emplace_back(std::move(continuation.back()));
        else
          f.fun.push_back(continuation.back());
        f.arguments.emplace_back(std::move(l));
        e = std::move(f);
        }
      }

    void cps_convert_funcall(Expression& e)
      {
      assert(std::holds_alternative<FunCall>(e));
      FunCall& f = std::get<FunCall>(e);

      std::vector<Expression> arguments;
      arguments.reserve(f.arguments.size() + 1);
      arguments.emplace_back(std::move(f.fun.front()));
      for (auto& arg : f.arguments)
        arguments.emplace_back(std::move(arg));

      std::vector<bool> simple_arg(arguments.size());

      std::map<size_t, std::string> nonsimple_vars;

      std::string r0 = make_var_name(index.back() + 1);
      nonsimple_vars[0] = r0;
      for (size_t j = 1; j < arguments.size(); ++j)
        {
        simple_arg[j] = is_simple(arguments[j]);
        if (!simple_arg[j])
          {
          nonsimple_vars[j] = make_var_name(index.back() + j + 1);
          }
        }
      index.back() += arguments.size();


      Lambda bottom_l;
      bottom_l.variables.push_back(nonsimple_vars.rbegin()->second);
      Begin bottom_b;
      FunCall bottom_f;
      Variable bottom_v;
      bottom_v.name = r0;
      bottom_f.fun.emplace_back(std::move(bottom_v));
      if (continuation_can_be_moved.back())
        bottom_f.arguments.emplace_back(std::move(continuation.back()));
      else
        bottom_f.arguments.push_back(continuation.back());
      for (size_t j = 1; j < arguments.size(); ++j)
        {
        if (simple_arg[j])
          bottom_f.arguments.emplace_back(std::move(arguments[j]));
        else
          {
          Variable r;
          r.name = nonsimple_vars.find(j)->second;
          bottom_f.arguments.emplace_back(std::move(r));
          }
        }
      bottom_b.arguments.emplace_back(std::move(bottom_f));
      bottom_l.body.emplace_back(std::move(bottom_b));

      //cps_conversion_visitor ccv;
      //ccv.index = index.back() + 1;
      //ccv.continuation = Lambda();
      //std::swap(std::get<Lambda>(ccv.continuation), bottom_l); // this is a very substantial speedup trick!!
      index.push_back(index.back()+1);
      continuation.emplace_back(Lambda());
      std::swap(std::get<Lambda>(continuation.back()), bottom_l); // this is a very substantial speedup trick!!
      assert(continuation_is_valid());
      //visitor<Expression, cps_conversion_visitor>::visit(arguments[nonsimple_vars.rbegin()->first], &ccv);
      size_t current_size = expressions_to_treat.size();
      expressions_to_treat.push_back(&arguments[nonsimple_vars.rbegin()->first]);
      treat_expressions(current_size);
      //index.back()= ccv.index;
      auto ind = index.back();
      index.pop_back();
      index.back() = ind;
      continuation.pop_back();

      auto it = nonsimple_vars.rbegin();
      ++it;
      for (; it != nonsimple_vars.rend(); ++it)
        {
        Lambda l;
        l.variables.push_back(it->second);
        Begin b;
        auto prev_it = it;
        --prev_it;
        b.arguments.emplace_back(std::move(arguments[prev_it->first]));
        l.body.emplace_back(std::move(b));
        //cps_conversion_visitor ccv2;
        //ccv2.index = index.back() + 1;
        //ccv2.continuation = Lambda();
        //std::swap(std::get<Lambda>(ccv2.continuation), l); // this is a very substantial speedup trick!!
        index.push_back(index.back()+1);
        continuation.emplace_back(Lambda());
        std::swap(std::get<Lambda>(continuation.back()), l); // this is a very substantial speedup trick!!
        assert(continuation_is_valid());
        //visitor<Expression, cps_conversion_visitor>::visit(arguments[it->first], &ccv2);
        //index.back()= ccv2.index;
        current_size = expressions_to_treat.size();
        expressions_to_treat.push_back(&arguments[it->first]);
        treat_expressions(current_size);
        auto ind = index.back();
        index.pop_back();
        index.back() = ind;
        continuation.pop_back();
        }

      Expression expr = std::move(arguments[nonsimple_vars.begin()->first]);
      e.swap(expr);
      }


    void cps_convert_let_nonsimple(Expression& e)
      {
      assert(std::holds_alternative<Let>(e));
      Let& l = std::get<Let>(e);
      //_previsit(l.body.front());
      size_t current_size = expressions_to_treat.size();
      expressions_to_treat.push_back(&l.body.front());
      treat_expressions(current_size);

      Expression expr(std::move(l.body.front()));
      for (size_t j = 0; j < l.bindings.size(); ++j)
        {
        size_t id = l.bindings.size() - j - 1;
        Lambda lam;
        lam.variables.push_back(l.bindings[id].first);
        if (std::holds_alternative<Begin>(expr))
          {
          lam.body.emplace_back(std::move(expr));
          }
        else
          {
          Begin b;
          b.arguments.emplace_back(std::move(expr));
          lam.body.emplace_back(std::move(b));
          }
        //cps_conversion_visitor ccv;
        //ccv.index = index.back() + 1;
        //ccv.continuation = Lambda();
        //std::swap(std::get<Lambda>(ccv.continuation), lam); // this is a very substantial speedup trick!!
        index.push_back(index.back()+1);
        continuation.push_back(Lambda());
        std::swap(std::get<Lambda>(continuation.back()), lam); // this is a very substantial speedup trick!!
        assert(continuation_is_valid());
        //visitor<Expression, cps_conversion_visitor>::visit(l.bindings[id].second, &ccv);
        size_t current_size = expressions_to_treat.size();
        expressions_to_treat.push_back(&l.bindings[id].second);
        treat_expressions(current_size);
        auto ind = index.back();
        index.pop_back();
        index.back() = ind;
        continuation.pop_back();
        expr = std::move(l.bindings[id].second);
        //index.back() = ccv.index;
        }
      e.swap(expr);
      }

    void cps_convert_let(Expression& e)
      {
      assert(std::holds_alternative<Let>(e));
      Let& l = std::get<Let>(e);

      std::vector<bool> simple(l.bindings.size());
      bool at_least_one_simple = false;
      bool all_simple = true;
      for (size_t j = 0; j < l.bindings.size(); ++j)
        {
        simple[j] = is_simple(l.bindings[j].second);
        at_least_one_simple |= simple[j];
        all_simple &= simple[j];
        }

      if (!at_least_one_simple)
        cps_convert_let_nonsimple(e);
      else if (all_simple)
        {
        //_previsit(l.body.front());
        size_t current_size = expressions_to_treat.size();
        expressions_to_treat.push_back(&l.body.front());
        treat_expressions(current_size);
        }
      else
        {
        std::vector<std::pair<std::string, Expression>> simple_bindings;
        std::vector<std::pair<std::string, Expression>> nonsimple_bindings;
        for (size_t j = 0; j < l.bindings.size(); ++j)
          {
          if (simple[j])
            simple_bindings.emplace_back(std::move(l.bindings[j]));
          else
            nonsimple_bindings.emplace_back(std::move(l.bindings[j]));
          }
        Let new_let;
        new_let.bindings.swap(nonsimple_bindings);
        new_let.body.swap(l.body);
        new_let.bt = bt_let;
        l.body.emplace_back(Begin());
        std::get<Begin>(l.body.front()).arguments.emplace_back(std::move(new_let));
        l.bindings.swap(simple_bindings);
        cps_convert_let_nonsimple(std::get<Begin>(l.body.front()).arguments.front());
        }
      }

    void treat_expressions(size_t target)
      {
      while (expressions_to_treat.size() > target)
        {
        Expression* p_expr = expressions_to_treat.back();
        expressions_to_treat.pop_back();
        Expression& e = *p_expr;
        if (std::holds_alternative<Literal>(e))
          {
          cps_convert_literal_or_variable_or_nop_or_quote(e);
          }
        else if (std::holds_alternative<Variable>(e))
          {
          cps_convert_literal_or_variable_or_nop_or_quote(e);
          }
        else if (std::holds_alternative<Nop>(e))
          {
          cps_convert_literal_or_variable_or_nop_or_quote(e);
          }
        else if (std::holds_alternative<Quote>(e))
          {
          cps_convert_literal_or_variable_or_nop_or_quote(e);
          }
        else if (std::holds_alternative<Set>(e))
          {
          cps_convert_set(e);
          }
        else if (std::holds_alternative<If>(e))
          {
          cps_convert_if(e);
          }
        else if (std::holds_alternative<Begin>(e))
          {
          cps_convert_begin(e);
          }
        else if (std::holds_alternative<PrimitiveCall>(e))
          {
          cps_convert_prim(e);
          }
        else if (std::holds_alternative<ForeignCall>(e))
          {
          cps_convert_foreign(e);
          }
        else if (std::holds_alternative<Lambda>(e))
          {
          cps_convert_lambda(e);
          }
        else if (std::holds_alternative<FunCall>(e))
          {
          cps_convert_funcall(e);
          }
        else if (std::holds_alternative<Let>(e))
          {
          cps_convert_let(e);
          }
        }
      }
    };
  }

void cps_conversion(Program& prog, const compiler_options& ops)
  {
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
        cps_conversion_helper ccv;
        ccv.continuation_can_be_moved.push_back(true);
        ccv.index.push_back(0);
        auto& arg = beg.arguments[i];
        Lambda l;
        l.variables.push_back(make_var_name(ccv.index.back()));
        PrimitiveCall prim;
        prim.primitive_name = "halt";
        Variable v;
        v.name = make_var_name(ccv.index.back());
        prim.arguments.emplace_back(std::move(v));
        Begin b;
        b.arguments.emplace_back(std::move(prim));
        l.body.emplace_back(std::move(b));
        ccv.continuation.push_back(Lambda());
        std::swap(std::get<Lambda>(ccv.continuation.back()), l); // this is a very substantial speedup trick!!
        assert(ccv.continuation_is_valid());
        //ccv.visit(arg);
        ccv.expressions_to_treat.push_back(&arg);
        ccv.treat_expressions(0);
        }, ops);
      }
    else
      {
      cps_conversion_helper ccv;
      ccv.continuation_can_be_moved.push_back(true);
      ccv.index.push_back(0);
      Lambda l;
      l.variables.push_back(make_var_name(ccv.index.back()));
      PrimitiveCall prim;
      prim.primitive_name = "halt";
      Variable v;
      v.name = make_var_name(ccv.index.back());
      prim.arguments.emplace_back(std::move(v));
      Begin b;
      b.arguments.emplace_back(std::move(prim));
      l.body.emplace_back(std::move(b));
      ccv.continuation.push_back(Lambda());
      std::swap(std::get<Lambda>(ccv.continuation.back()), l); // this is a very substantial speedup trick!!
      assert(ccv.continuation_is_valid());
      //ccv.visit(e);
      ccv.expressions_to_treat.push_back(&e);
      ccv.treat_expressions(0);
      }
    }
  prog.cps_converted = true;
  }
SKIWI_END
