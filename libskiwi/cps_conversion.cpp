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

#define cps_assert(EX)

//#define cps_assert(EX) assert(EX)

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

  struct cps_conversion_state
    {
    enum struct e_conversion_state
      {
      T_INIT,
      T_STEP_1,
      T_STEP_2,
      T_STEP_3      
      };
    Expression* p_expr;
    e_conversion_state state;
    std::vector<std::pair<size_t, std::string>> nonsimple_vars;
    uint32_t index;

    cps_conversion_state() : p_expr(nullptr), state(e_conversion_state::T_INIT) {}
    cps_conversion_state(Expression* ip_expr) : p_expr(ip_expr), state(e_conversion_state::T_INIT) {}
    cps_conversion_state(Expression* ip_expr, e_conversion_state s) : p_expr(ip_expr), state(s) {}
    };

  struct cps_conversion_helper
    {
    std::vector<uint64_t> index;
    std::vector<Expression> continuation;
    std::vector<bool> continuation_can_be_moved;

    std::vector<cps_conversion_state> expressions_to_treat;

    Expression dummy_if;
    Expression dummy_begin;

    cps_conversion_helper()
      {
      dummy_if = If();
      dummy_begin = Begin();
      }

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
      cps_assert(std::holds_alternative<Literal>(e) || std::holds_alternative<Variable>(e) || std::holds_alternative<Nop>(e) || std::holds_alternative<Quote>(e));
      if (continuation_is_lambda_with_one_parameter_without_free_vars())
        {
        Lambda& lam = std::get<Lambda>(continuation.back());
        Let l;
        l.bindings.emplace_back(lam.variables.front(), std::move(e));
        if (continuation_can_be_moved.back())
          l.body = std::move(lam.body);
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
      cps_assert(std::holds_alternative<Set>(e));
      Set& s = std::get<Set>(e);
      cps_assert(!is_simple(s.value.front()));
      Expression& e1 = s.value.front();

      //cps_conversion_visitor ccv;
      index.push_back(index.back() + 1);
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
      continuation_can_be_moved.push_back(true);
      std::swap(std::get<Lambda>(continuation.back()), l); // this is a very substantial speedup trick!!
      //ccv.continuation = Lambda();
      //std::swap(std::get<Lambda>(ccv.continuation), l); // this is a very substantial speedup trick!!
      cps_assert(continuation_is_valid());
      //e.swap(e1);
      //visitor<Expression, cps_conversion_visitor>::visit(e, &ccv);

      //size_t current_size = expressions_to_treat.size();
      expressions_to_treat.emplace_back(&e, cps_conversion_state::e_conversion_state::T_STEP_1);
      expressions_to_treat.push_back(&e1);
      //treat_expressions(current_size);
      //index.back() = ccv.index;
      //index.pop_back();
      //continuation.pop_back();
      //continuation_can_be_moved.pop_back();
      }

    void cps_convert_set_nonsimple_step1(Expression& e)
      {
      auto ind = index.back();
      index.pop_back();
      index.back() = ind;
      continuation.pop_back();
      continuation_can_be_moved.pop_back();

      Set& s = std::get<Set>(e);
      Expression& e1 = s.value.front();
      Expression new_expr;
      new_expr.swap(e);
      e = std::move(e1);
      }

    void cps_convert_set_simple(Expression& e)
      {
      cps_assert(std::holds_alternative<Set>(e));
      cps_assert(is_simple(std::get<Set>(e).value.front()));
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
      cps_assert(std::holds_alternative<Set>(e));
      Set& s = std::get<Set>(e);

      if (is_simple(s.value.front()))
        cps_convert_set_simple(e);
      else
        cps_convert_set_nonsimple(e);

      cps_assert(find(e, [](const Expression& e) {return std::holds_alternative<Set>(e); }));
      }

    void cps_convert_if(Expression& e)
      {
      cps_assert(std::holds_alternative<If>(e));
      If& i = std::get<If>(e);

      continuation_can_be_moved.back() = false;

      if (!is_simple(i.arguments.front()))
        expressions_to_treat.emplace_back(&e, cps_conversion_state::e_conversion_state::T_STEP_1);

      for (size_t j = 1; j < i.arguments.size(); ++j)
        expressions_to_treat.push_back(&i.arguments[j]);
      //treat_expressions(current_size);


      //for (size_t j = 1; j < i.arguments.size(); ++j)
      //  visitor<Expression, cps_conversion_visitor>::visit(i.arguments[j], this);

      //if (is_simple(i.arguments.front()))
      //  return;

      }

    void cps_convert_if_step1(Expression& e)
      {
      cps_assert(std::holds_alternative<If>(e));
      If& i = std::get<If>(e);
      //cps_conversion_visitor ccv;
      //ccv.index = index.back() + 1;
      index.push_back(index.back() + 1);
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
      continuation_can_be_moved.push_back(true);
      std::swap(std::get<Lambda>(continuation.back()), l); // this is a very substantial speedup trick!!
      cps_assert(continuation_is_valid());

      e = std::move(exp0);
      //visitor<Expression, cps_conversion_visitor>::visit(e, &ccv);
      //current_size = expressions_to_treat.size();
      expressions_to_treat.emplace_back(&dummy_if, cps_conversion_state::e_conversion_state::T_STEP_2);
      expressions_to_treat.push_back(&e);
      //treat_expressions(current_size);
      }

    void cps_convert_if_step2()
      {
      //cps_assert(std::holds_alternative<If>(e));
      auto ind = index.back();
      index.pop_back();
      index.back() = ind;
      continuation.pop_back();
      continuation_can_be_moved.pop_back();
      }

    void cps_convert_begin_simple(Expression& e)
      {
      cps_assert(std::holds_alternative<Begin>(e));
      Begin& b = std::get<Begin>(e);
      if (b.arguments.empty())
        {
        Nop n;
        b.arguments.emplace_back(Nop());
        }
      cps_assert(is_simple(e));
      //size_t current_size = expressions_to_treat.size();
      expressions_to_treat.push_back(&b.arguments.back());
      //treat_expressions(current_size);
      //visitor<Expression, cps_conversion_visitor>::visit(b.arguments.back(), this);
      }

    void cps_convert_begin(Expression& e)
      {
      cps_assert(std::holds_alternative<Begin>(e));
      Begin& b = std::get<Begin>(e);
      if (is_simple(e) || b.arguments.empty())
        cps_convert_begin_simple(e);
      else if (b.arguments.size() == 1)
        {
        //size_t current_size = expressions_to_treat.size();
        expressions_to_treat.push_back(&b.arguments.front());
        //treat_expressions(current_size);
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
          //size_t current_size = expressions_to_treat.size();
          expressions_to_treat.push_back(&b.arguments.back());
          //treat_expressions(current_size);
          //visitor<Expression, cps_conversion_visitor>::visit(b.arguments.back(), this);
          }
        else if (first_non_simple_expr > 0)
          {
          Begin new_b;
          new_b.arguments.insert(new_b.arguments.end(), b.arguments.begin(), b.arguments.begin() + first_non_simple_expr);
          b.arguments.erase(b.arguments.begin(), b.arguments.begin() + first_non_simple_expr);
          new_b.arguments.emplace_back(std::move(b));
          b = std::move(new_b);
          //cps_convert_begin(b.arguments.back());
          expressions_to_treat.push_back(&b.arguments.back());
          }
        else
          {
          Begin remainder;
          remainder.arguments.reserve(b.arguments.size() - 1);
          for (auto it = b.arguments.begin() + 1; it != b.arguments.end(); ++it)
            remainder.arguments.emplace_back(std::move(*it));
          //Expression e2(std::move(remainder));
          b.arguments.resize(2);
          b.arguments[1] = std::move(remainder);
          //size_t current_size = expressions_to_treat.size();
          expressions_to_treat.emplace_back(&e, cps_conversion_state::e_conversion_state::T_STEP_1);
          expressions_to_treat.push_back(&b.arguments[1]);
          /*
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
          continuation.emplace_back(Lambda());
          continuation_can_be_moved.push_back(true);
          std::swap(std::get<Lambda>(continuation.back()), l); // this is a very substantial speedup trick!!
          cps_assert(continuation_is_valid());
          Expression arg = std::move(b.arguments[0]);
          e = std::move(arg);
          current_size = expressions_to_treat.size();
          expressions_to_treat.push_back(&e);
          treat_expressions(current_size);
          auto ind = index.back();
          index.pop_back();
          index.back() = ind;
          continuation.pop_back();
          continuation_can_be_moved.pop_back();
          */
          }
        }
      }

    void cps_convert_begin_step1(Expression& e)
      {
      cps_assert(std::holds_alternative<Begin>(e));
      Begin& b = std::get<Begin>(e);
      index.push_back(index.back() + 1);
      Lambda l;
      Expression& e2 = b.arguments[1];
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
      continuation.emplace_back(Lambda());
      continuation_can_be_moved.push_back(true);
      std::swap(std::get<Lambda>(continuation.back()), l); // this is a very substantial speedup trick!!
      cps_assert(continuation_is_valid());
      Expression arg = std::move(b.arguments[0]);
      e = std::move(arg);

      expressions_to_treat.emplace_back(&dummy_begin, cps_conversion_state::e_conversion_state::T_STEP_2);
      expressions_to_treat.push_back(&e);
      /*
      auto ind = index.back();
      index.pop_back();
      index.back() = ind;
      continuation.pop_back();
      continuation_can_be_moved.pop_back();
      */
      }

    void cps_convert_begin_step2()
      {
      auto ind = index.back();
      index.pop_back();
      index.back() = ind;
      continuation.pop_back();
      continuation_can_be_moved.pop_back();
      }

    void cps_convert_prim_nonsimple(Expression& e)
      {
      cps_assert(std::holds_alternative<PrimitiveCall>(e));
      PrimitiveCall& p = std::get<PrimitiveCall>(e);
      if (p.arguments.empty())
        {
        cps_convert_prim_simple(e);
        return;
        }
      cps_assert(!p.arguments.empty());
      std::vector<std::pair<size_t, std::string>> nonsimple_vars;
      std::vector<bool> simple(p.arguments.size());
      for (size_t j = 0; j < p.arguments.size(); ++j)
        {
        simple[j] = is_simple(p.arguments[j]);
        if (!simple[j])
          {
          nonsimple_vars.emplace_back(j, make_var_name(index.back() + j + 1));
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
          //v.name = nonsimple_vars.find(j)->second;
          v.name = std::find_if(nonsimple_vars.begin(), nonsimple_vars.end(), [&](const auto& item) { return item.first == j; })->second;
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
      index.push_back(index.back() + 1);
      //ccv.continuation = Lambda();
      continuation.emplace_back(Lambda());
      continuation_can_be_moved.push_back(true);
      //std::swap(std::get<Lambda>(ccv.continuation), bottom_l); // this is a very substantial speedup trick!!
      std::swap(std::get<Lambda>(continuation.back()), bottom_l); // this is a very substantial speedup trick!!
      cps_assert(continuation_is_valid());
      //visitor<Expression, cps_conversion_visitor>::visit(p.arguments[nonsimple_vars.rbegin()->first], &ccv);
      //size_t current_size = expressions_to_treat.size();

      expressions_to_treat.emplace_back(&e, cps_conversion_state::e_conversion_state::T_STEP_1);
      expressions_to_treat.back().nonsimple_vars.swap(nonsimple_vars);
      expressions_to_treat.emplace_back(&p.arguments[expressions_to_treat.back().nonsimple_vars.rbegin()->first]);
      //expressions_to_treat.emplace_back(&std::get<PrimitiveCall>(*expressions_to_treat.back().p_expr).arguments[nonsimple_vars.rbegin()->first]);
      //treat_expressions(current_size);
      }

    void cps_convert_prim_nonsimple_step3(Expression& e, std::vector<std::pair<size_t, std::string>>& nonsimple_vars)
      {
      cps_assert(std::holds_alternative<PrimitiveCall>(e));
      PrimitiveCall& p = std::get<PrimitiveCall>(e);
      cps_assert(!p.arguments.empty());

      auto ind = index.back();
      index.pop_back();
      index.back() = ind;
      continuation.pop_back();
      continuation_can_be_moved.pop_back();

      Expression expr(std::move(p.arguments[nonsimple_vars.begin()->first]));
      e.swap(expr);
      }

    void cps_convert_prim_nonsimple_step2(Expression& e, std::vector<std::pair<size_t, std::string>>& nonsimple_vars, uint32_t var_index)
      {
      cps_assert(std::holds_alternative<PrimitiveCall>(e));
      PrimitiveCall& p = std::get<PrimitiveCall>(e);
      cps_assert(!p.arguments.empty());



      if (var_index > 0)
        {
        expressions_to_treat.emplace_back(&e, cps_conversion_state::e_conversion_state::T_STEP_2);
        expressions_to_treat.back().nonsimple_vars = nonsimple_vars;
        expressions_to_treat.back().index = var_index - 1;
        }

      auto ind = index.back();
      index.pop_back();
      index.back() = ind;
      continuation.pop_back();
      continuation_can_be_moved.pop_back();

      auto it = nonsimple_vars.begin() + var_index;
      auto prev_it = it + 1;

      Lambda l;
      l.variables.push_back(it->second);
      Begin b;
      b.arguments.emplace_back(std::move(p.arguments[prev_it->first]));
      l.body.emplace_back(std::move(b));
      //cps_conversion_visitor ccv2;
      //ccv2.index = index.back() + 1;
      index.push_back(index.back() + 1);
      //ccv2.continuation = l;
      //ccv2.continuation = Lambda();
      continuation.emplace_back(Lambda());
      continuation_can_be_moved.push_back(true);
      //std::swap(std::get<Lambda>(ccv2.continuation), l); // this is a very substantial speedup trick!!
      std::swap(std::get<Lambda>(continuation.back()), l); // this is a very substantial speedup trick!!
      cps_assert(continuation_is_valid());
      //visitor<Expression, cps_conversion_visitor>::visit(p.arguments[it->first], &ccv2);
      //size_t current_size = expressions_to_treat.size();
      expressions_to_treat.push_back(&p.arguments[it->first]);
      //treat_expressions(current_size);

      }

    void cps_convert_prim_nonsimple_step1(Expression& e, std::vector<std::pair<size_t, std::string>>& nonsimple_vars)
      {
      cps_assert(std::holds_alternative<PrimitiveCall>(e));
      cps_assert(!std::get<PrimitiveCall>(e).arguments.empty());

      expressions_to_treat.emplace_back(&e, cps_conversion_state::e_conversion_state::T_STEP_3);
      expressions_to_treat.back().nonsimple_vars = nonsimple_vars;

      //index.back() = ccv.index;
      //auto ind = index.back();
      //index.pop_back();
      //index.back() = ind;
      //continuation.pop_back();

      //auto it = nonsimple_vars.rbegin();
      //auto prev_it = it;
      //++it;

      if (expressions_to_treat.back().nonsimple_vars.size() > 1)
        {
        expressions_to_treat.emplace_back(&e, cps_conversion_state::e_conversion_state::T_STEP_2);
        expressions_to_treat.back().nonsimple_vars = nonsimple_vars;
        expressions_to_treat.back().index = (uint32_t)expressions_to_treat.back().nonsimple_vars.size() - 2;
        //++expressions_to_treat.back().nonsimple_vars_it;
        }
      /*
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
        cps_assert(continuation_is_valid());
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
      e.swap(expr);*/
      }

    void cps_convert_prim_nonsimple_step1_old(Expression& e, std::vector<std::pair<size_t, std::string>>& nonsimple_vars)
      {
      cps_assert(std::holds_alternative<PrimitiveCall>(e));
      PrimitiveCall& p = std::get<PrimitiveCall>(e);
      cps_assert(!p.arguments.empty());

      //index.back() = ccv.index;
      auto ind = index.back();
      index.pop_back();
      index.back() = ind;
      continuation.pop_back();
      continuation_can_be_moved.pop_back();

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
        index.push_back(index.back() + 1);
        //ccv2.continuation = l;
        //ccv2.continuation = Lambda();
        continuation.emplace_back(Lambda());
        continuation_can_be_moved.push_back(true);
        //std::swap(std::get<Lambda>(ccv2.continuation), l); // this is a very substantial speedup trick!!
        std::swap(std::get<Lambda>(continuation.back()), l); // this is a very substantial speedup trick!!
        cps_assert(continuation_is_valid());
        //visitor<Expression, cps_conversion_visitor>::visit(p.arguments[it->first], &ccv2);
        size_t current_size = expressions_to_treat.size();
        expressions_to_treat.push_back(&p.arguments[it->first]);
        treat_expressions(current_size);
        //index.back() = ccv2.index;
        ind = index.back();
        index.pop_back();
        index.back() = ind;
        continuation.pop_back();
        continuation_can_be_moved.pop_back();
        }

      Expression expr(std::move(p.arguments[nonsimple_vars.begin()->first]));
      e.swap(expr);
      }

    void cps_convert_prim_simple(Expression& e)
      {
      cps_assert(std::holds_alternative<PrimitiveCall>(e));

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
      cps_assert(std::holds_alternative<ForeignCall>(e));
      ForeignCall& p = std::get<ForeignCall>(e);
      cps_assert(!p.arguments.empty());
      std::vector<std::pair<size_t, std::string>> nonsimple_vars;
      std::vector<bool> simple(p.arguments.size());
      for (size_t j = 0; j < p.arguments.size(); ++j)
        {
        simple[j] = is_simple(p.arguments[j]);
        if (!simple[j])
          {
          nonsimple_vars.emplace_back(j, make_var_name(index.back() + j + 1));
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
          v.name = std::find_if(nonsimple_vars.begin(), nonsimple_vars.end(), [&](const auto& item) { return item.first == j; })->second;
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
      index.push_back(index.back() + 1);
      continuation.emplace_back(Lambda());
      continuation_can_be_moved.push_back(true);
      //std::swap(std::get<Lambda>(ccv.continuation), bottom_l); // this is a very substantial speedup trick!!
      std::swap(std::get<Lambda>(continuation.back()), bottom_l); // this is a very substantial speedup trick!!
      cps_assert(continuation_is_valid());
      //visitor<Expression, cps_conversion_visitor>::visit(p.arguments[nonsimple_vars.rbegin()->first], &ccv);
      //size_t current_size = expressions_to_treat.size();
      expressions_to_treat.emplace_back(&e, cps_conversion_state::e_conversion_state::T_STEP_1);
      expressions_to_treat.back().nonsimple_vars.swap(nonsimple_vars);
      expressions_to_treat.push_back(&p.arguments[expressions_to_treat.back().nonsimple_vars.rbegin()->first]);
      //treat_expressions(current_size);
      //index.back() = ccv.index;
      /*
      auto ind = index.back();
      index.pop_back();
      index.back() = ind;
      continuation.pop_back();
      continuation_can_be_moved.pop_back();

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
        continuation_can_be_moved.push_back(true);
        //std::swap(std::get<Lambda>(ccv2.continuation), l); // this is a very substantial speedup trick!!
        std::swap(std::get<Lambda>(continuation.back()), l); // this is a very substantial speedup trick!!
        cps_assert(continuation_is_valid());
        //visitor<Expression, cps_conversion_visitor>::visit(p.arguments[it->first], &ccv2);
        current_size = expressions_to_treat.size();
        expressions_to_treat.push_back(&p.arguments[it->first]);
        treat_expressions(current_size);
        ind = index.back();
        index.pop_back();
        index.back() = ind;
        continuation.pop_back();
        continuation_can_be_moved.pop_back();
        }

      Expression expr = std::move(p.arguments[nonsimple_vars.begin()->first]);
      e.swap(expr);
      */
      }

    void cps_convert_foreign_nonsimple_step3(Expression& e, std::vector<std::pair<size_t, std::string>>& nonsimple_vars)
      {
      cps_assert(std::holds_alternative<ForeignCall>(e));
      ForeignCall& p = std::get<ForeignCall>(e);
      cps_assert(!p.arguments.empty());

      auto ind = index.back();
      index.pop_back();
      index.back() = ind;
      continuation.pop_back();
      continuation_can_be_moved.pop_back();

      Expression expr(std::move(p.arguments[nonsimple_vars.begin()->first]));
      e.swap(expr);
      }

    void cps_convert_foreign_nonsimple_step2(Expression& e, std::vector<std::pair<size_t, std::string>>& nonsimple_vars, uint32_t var_index)
      {
      cps_assert(std::holds_alternative<ForeignCall>(e));
      ForeignCall& p = std::get<ForeignCall>(e);
      cps_assert(!p.arguments.empty());



      if (var_index > 0)
        {
        expressions_to_treat.emplace_back(&e, cps_conversion_state::e_conversion_state::T_STEP_2);
        expressions_to_treat.back().nonsimple_vars = nonsimple_vars;
        expressions_to_treat.back().index = var_index - 1;
        }

      auto ind = index.back();
      index.pop_back();
      index.back() = ind;
      continuation.pop_back();
      continuation_can_be_moved.pop_back();

      auto it = nonsimple_vars.begin() + var_index;
      auto prev_it = it + 1;

      Lambda l;
      l.variables.push_back(it->second);
      Begin b;
      b.arguments.emplace_back(std::move(p.arguments[prev_it->first]));
      l.body.emplace_back(std::move(b));
      //cps_conversion_visitor ccv2;
      //ccv2.index = index.back() + 1;
      index.push_back(index.back() + 1);
      //ccv2.continuation = l;
      //ccv2.continuation = Lambda();
      continuation.emplace_back(Lambda());
      continuation_can_be_moved.push_back(true);
      //std::swap(std::get<Lambda>(ccv2.continuation), l); // this is a very substantial speedup trick!!
      std::swap(std::get<Lambda>(continuation.back()), l); // this is a very substantial speedup trick!!
      cps_assert(continuation_is_valid());
      //visitor<Expression, cps_conversion_visitor>::visit(p.arguments[it->first], &ccv2);
      //size_t current_size = expressions_to_treat.size();
      expressions_to_treat.push_back(&p.arguments[it->first]);
      //treat_expressions(current_size);

      }

    void cps_convert_foreign_nonsimple_step1(Expression& e, std::vector<std::pair<size_t, std::string>>& nonsimple_vars)
      {
      cps_assert(std::holds_alternative<ForeignCall>(e));
      cps_assert(!std::get<ForeignCall>(e).arguments.empty());

      expressions_to_treat.emplace_back(&e, cps_conversion_state::e_conversion_state::T_STEP_3);
      expressions_to_treat.back().nonsimple_vars = nonsimple_vars;

      //index.back() = ccv.index;
      //auto ind = index.back();
      //index.pop_back();
      //index.back() = ind;
      //continuation.pop_back();

      //auto it = nonsimple_vars.rbegin();
      //auto prev_it = it;
      //++it;

      if (expressions_to_treat.back().nonsimple_vars.size() > 1)
        {
        expressions_to_treat.emplace_back(&e, cps_conversion_state::e_conversion_state::T_STEP_2);
        expressions_to_treat.back().nonsimple_vars = nonsimple_vars;
        expressions_to_treat.back().index = (uint32_t)expressions_to_treat.back().nonsimple_vars.size() - 2;
        //++expressions_to_treat.back().nonsimple_vars_it;
        }
      /*
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
        cps_assert(continuation_is_valid());
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
      e.swap(expr);*/
      }

    void cps_convert_foreign_simple(Expression& e)
      {
      cps_assert(std::holds_alternative<ForeignCall>(e));

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
      cps_assert(std::holds_alternative<Lambda>(e));
      Lambda& l = std::get<Lambda>(e);
      //cps_conversion_visitor ccv;
      //ccv.index = index.back() + 1;
      index.push_back(index.back() + 1);
      Variable k;
      k.name = make_var_name(index.back());
      //ccv.continuation = k;
      continuation.push_back(k);
      continuation_can_be_moved.push_back(true);

      cps_assert(continuation_is_valid());
      //size_t current_size = expressions_to_treat.size();
      expressions_to_treat.emplace_back(&e, cps_conversion_state::e_conversion_state::T_STEP_1);
      expressions_to_treat.push_back(&l.body.front());
      //treat_expressions(current_size);
      //visitor<Expression, cps_conversion_visitor>::visit(l.body.front(), &ccv);
      //index.back() = ccv.index;
      }

    void cps_convert_lambda_step1(Expression& e)
      {
      cps_assert(std::holds_alternative<Lambda>(e));
      Lambda& l = std::get<Lambda>(e);
      auto ind = index.back();
      index.pop_back();
      std::string k_name = make_var_name(index.back() + 1);
      index.back() = ind;
      continuation.pop_back();
      continuation_can_be_moved.pop_back();
      l.variables.insert(l.variables.begin(), k_name);

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
      cps_assert(std::holds_alternative<FunCall>(e));
      FunCall& f = std::get<FunCall>(e);

      std::vector<Expression*> arguments;
      arguments.reserve(f.arguments.size() + 1);
      arguments.emplace_back(&f.fun.front());
      for (auto& arg : f.arguments)
        arguments.emplace_back(&arg);

      std::vector<bool> simple_arg(arguments.size());

      std::vector<std::pair<size_t, std::string>> nonsimple_vars;

      std::string r0 = make_var_name(index.back() + 1);
      nonsimple_vars.emplace_back(0, r0);
      for (size_t j = 1; j < arguments.size(); ++j)
        {
        simple_arg[j] = is_simple(*arguments[j]);
        if (!simple_arg[j])
          {
          nonsimple_vars.emplace_back(j, make_var_name(index.back() + j + 1));
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
          bottom_f.arguments.emplace_back(std::move(*arguments[j]));
        else
          {
          Variable r;
          //r.name = nonsimple_vars.find(j)->second;
          r.name = std::find_if(nonsimple_vars.begin(), nonsimple_vars.end(), [&](const auto& item) { return item.first == j; })->second;

          bottom_f.arguments.emplace_back(std::move(r));
          }
        }
      bottom_b.arguments.emplace_back(std::move(bottom_f));
      bottom_l.body.emplace_back(std::move(bottom_b));

      //cps_conversion_visitor ccv;
      //ccv.index = index.back() + 1;
      //ccv.continuation = Lambda();
      //std::swap(std::get<Lambda>(ccv.continuation), bottom_l); // this is a very substantial speedup trick!!
      index.push_back(index.back() + 1);
      continuation.emplace_back(Lambda());
      continuation_can_be_moved.push_back(true);
      std::swap(std::get<Lambda>(continuation.back()), bottom_l); // this is a very substantial speedup trick!!
      cps_assert(continuation_is_valid());
      //visitor<Expression, cps_conversion_visitor>::visit(arguments[nonsimple_vars.rbegin()->first], &ccv);

      expressions_to_treat.emplace_back(&e, cps_conversion_state::e_conversion_state::T_STEP_1);
      expressions_to_treat.back().nonsimple_vars.swap(nonsimple_vars);

      expressions_to_treat.push_back(arguments[expressions_to_treat.back().nonsimple_vars.rbegin()->first]);
      /*
      size_t current_size = expressions_to_treat.size();
      expressions_to_treat.push_back(&arguments[nonsimple_vars.rbegin()->first]);
      treat_expressions(current_size);
      //index.back()= ccv.index;
      auto ind = index.back();
      index.pop_back();
      index.back() = ind;
      continuation.pop_back();
      continuation_can_be_moved.pop_back();

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
        continuation_can_be_moved.push_back(true);
        std::swap(std::get<Lambda>(continuation.back()), l); // this is a very substantial speedup trick!!
        cps_assert(continuation_is_valid());
        //visitor<Expression, cps_conversion_visitor>::visit(arguments[it->first], &ccv2);
        //index.back()= ccv2.index;
        current_size = expressions_to_treat.size();
        expressions_to_treat.push_back(&arguments[it->first]);
        treat_expressions(current_size);
        ind = index.back();
        index.pop_back();
        index.back() = ind;
        continuation.pop_back();
        continuation_can_be_moved.pop_back();
        }

      Expression expr = std::move(arguments[nonsimple_vars.begin()->first]);
      e.swap(expr);
      */
      }

    void cps_convert_funcall_step3(Expression& e)
      {
      cps_assert(std::holds_alternative<FunCall>(e));
      FunCall& f = std::get<FunCall>(e);
      //cps_assert(!f.arguments.empty());

      auto ind = index.back();
      index.pop_back();
      index.back() = ind;
      continuation.pop_back();
      continuation_can_be_moved.pop_back();

      //Expression expr(std::move(f.arguments[nonsimple_vars.begin()->first]));
      Expression expr(f.fun.front());
      e.swap(expr);
      }

    void cps_convert_funcall_step2(Expression& e, std::vector<std::pair<size_t, std::string>>& nonsimple_vars, uint32_t var_index)
      {
      cps_assert(std::holds_alternative<FunCall>(e));
      FunCall& f = std::get<FunCall>(e);
      //cps_assert(!f.arguments.empty());

      std::vector<Expression*> arguments;
      arguments.reserve(f.arguments.size() + 1);
      arguments.emplace_back(&f.fun.front());
      for (auto& arg : f.arguments)
        arguments.emplace_back(&arg);

      if (var_index > 0)
        {
        expressions_to_treat.emplace_back(&e, cps_conversion_state::e_conversion_state::T_STEP_2);
        expressions_to_treat.back().nonsimple_vars = nonsimple_vars;
        expressions_to_treat.back().index = var_index - 1;
        }

      auto ind = index.back();
      index.pop_back();
      index.back() = ind;
      continuation.pop_back();
      continuation_can_be_moved.pop_back();

      auto it = nonsimple_vars.begin() + var_index;
      auto prev_it = it + 1;

      Lambda l;
      l.variables.push_back(it->second);
      Begin b;
      b.arguments.emplace_back(std::move(*arguments[prev_it->first]));
      l.body.emplace_back(std::move(b));
      //cps_conversion_visitor ccv2;
      //ccv2.index = index.back() + 1;
      index.push_back(index.back() + 1);
      //ccv2.continuation = l;
      //ccv2.continuation = Lambda();
      continuation.emplace_back(Lambda());
      continuation_can_be_moved.push_back(true);
      //std::swap(std::get<Lambda>(ccv2.continuation), l); // this is a very substantial speedup trick!!
      std::swap(std::get<Lambda>(continuation.back()), l); // this is a very substantial speedup trick!!
      cps_assert(continuation_is_valid());
      //visitor<Expression, cps_conversion_visitor>::visit(p.arguments[it->first], &ccv2);
      //size_t current_size = expressions_to_treat.size();
      expressions_to_treat.push_back(arguments[it->first]);
      //treat_expressions(current_size);

      }

    void cps_convert_funcall_step1(Expression& e, std::vector<std::pair<size_t, std::string>>& nonsimple_vars)
      {
      cps_assert(std::holds_alternative<FunCall>(e));
      //cps_assert(!std::get<FunCall>(e).arguments.empty());

      expressions_to_treat.emplace_back(&e, cps_conversion_state::e_conversion_state::T_STEP_3);
      expressions_to_treat.back().nonsimple_vars = nonsimple_vars;

      if (expressions_to_treat.back().nonsimple_vars.size() > 1)
        {
        expressions_to_treat.emplace_back(&e, cps_conversion_state::e_conversion_state::T_STEP_2);
        expressions_to_treat.back().nonsimple_vars = nonsimple_vars;
        expressions_to_treat.back().index = (uint32_t)expressions_to_treat.back().nonsimple_vars.size() - 2;
        }
      }

    void cps_convert_let_nonsimple(Expression& e)
      {
      cps_assert(std::holds_alternative<Let>(e));
      Let& l = std::get<Let>(e);

      expressions_to_treat.emplace_back(&e, cps_conversion_state::e_conversion_state::T_STEP_1);
      expressions_to_treat.push_back(&l.body.front());
      }

    void cps_convert_let_nonsimple_step1(Expression& e)
      {
      cps_assert(std::holds_alternative<Let>(e));
      Let& l = std::get<Let>(e);
      
      expressions_to_treat.emplace_back(&e, cps_conversion_state::e_conversion_state::T_STEP_2);
      expressions_to_treat.back().index = (uint32_t)l.bindings.size() - 1;
      }

    void cps_convert_let_nonsimple_step2(Expression& e, uint32_t id)
      {
      cps_assert(std::holds_alternative<Let>(e));
      Let& l = std::get<Let>(e);

      Lambda lam;
      lam.variables.push_back(l.bindings[id].first);

      if (std::holds_alternative<Begin>(l.body.front()))
        {
        lam.body.emplace_back(std::move(l.body.front()));
        }
      else
        {
        Begin b;
        b.arguments.emplace_back(std::move(l.body.front()));
        lam.body.emplace_back(std::move(b));
        }
      index.push_back(index.back() + 1);
      continuation.push_back(Lambda());
      continuation_can_be_moved.push_back(true);
      std::swap(std::get<Lambda>(continuation.back()), lam); // this is a very substantial speedup trick!!
      cps_assert(continuation_is_valid());
      //current_size = expressions_to_treat.size();
      expressions_to_treat.emplace_back(&e, cps_conversion_state::e_conversion_state::T_STEP_3);
      expressions_to_treat.back().index = id;
      expressions_to_treat.push_back(&l.bindings[id].second);
      /*
       auto ind = index.back();
       index.pop_back();
       index.back() = ind;
       continuation.pop_back();
       continuation_can_be_moved.pop_back();
       expr = std::move(l.bindings[id].second);
       }
     e.swap(expr);*/
      }

    void cps_convert_let_nonsimple_step3(Expression& e, uint32_t id)
      {
      Let& l = std::get<Let>(e);
      l.body.front() = std::move(l.bindings[id].second);
      auto ind = index.back();
      index.pop_back();
      index.back() = ind;
      continuation.pop_back();
      continuation_can_be_moved.pop_back();
      if (id)
        {
        expressions_to_treat.emplace_back(&e, cps_conversion_state::e_conversion_state::T_STEP_2);
        expressions_to_treat.back().index = id - 1;
        }
      else
        {
        Expression expr = std::move(l.body.front());
        e.swap(expr);
        }
      }

    void cps_convert_let(Expression& e)
      {
      cps_assert(std::holds_alternative<Let>(e));
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
      if (l.bindings.empty())
        {
        at_least_one_simple = true;
        all_simple = true;
        }

      if (!at_least_one_simple)
        cps_convert_let_nonsimple(e);
      else if (all_simple)
        {
        //_previsit(l.body.front());
        //size_t current_size = expressions_to_treat.size();
        expressions_to_treat.push_back(&l.body.front());
        //treat_expressions(current_size);
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
        //Expression* p_expr = expressions_to_treat.back();
        cps_conversion_state cps_state = expressions_to_treat.back();
        expressions_to_treat.pop_back();
        Expression& e = *cps_state.p_expr;
        switch (cps_state.state)
          {
          case cps_conversion_state::e_conversion_state::T_INIT:
          {
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
          break;
          }
          case cps_conversion_state::e_conversion_state::T_STEP_1:
          {
          if (std::holds_alternative<PrimitiveCall>(e))
            {
            cps_convert_prim_nonsimple_step1(e, cps_state.nonsimple_vars);
            }
          else if (std::holds_alternative<Begin>(e))
            {
            cps_convert_begin_step1(e);
            }
          else if (std::holds_alternative<Lambda>(e))
            {
            cps_convert_lambda_step1(e);
            }
          else if (std::holds_alternative<Let>(e))
            {
            cps_convert_let_nonsimple_step1(e);
            }
          else if (std::holds_alternative<FunCall>(e))
            {
            cps_convert_funcall_step1(e, cps_state.nonsimple_vars);
            }
          else if (std::holds_alternative<Set>(e))
            {
            cps_convert_set_nonsimple_step1(e);
            }
          else if (std::holds_alternative<If>(e))
            {
            cps_convert_if_step1(e);
            }
          else if (std::holds_alternative<ForeignCall>(e))
            {
            cps_convert_foreign_nonsimple_step1(e, cps_state.nonsimple_vars);
            }
          else
            {
            throw std::runtime_error("Compiler error!: cps conversion: not implemented");
            }
          break;
          }
          case cps_conversion_state::e_conversion_state::T_STEP_2:
          {
          if (std::holds_alternative<PrimitiveCall>(e))
            {
            cps_convert_prim_nonsimple_step2(e, cps_state.nonsimple_vars, cps_state.index);
            }
          else if (std::holds_alternative<Let>(e))
            {
            cps_convert_let_nonsimple_step2(e, cps_state.index);
            }
          else if (std::holds_alternative<Begin>(e))
            {
            cps_convert_begin_step2();
            }
          else if (std::holds_alternative<FunCall>(e))
            {
            cps_convert_funcall_step2(e, cps_state.nonsimple_vars, cps_state.index);
            }
          else if (std::holds_alternative<If>(e))
            {
            cps_convert_if_step2();
            }
          else if (std::holds_alternative<ForeignCall>(e))
            {
            cps_convert_foreign_nonsimple_step2(e, cps_state.nonsimple_vars, cps_state.index);
            }
          else
            {
            throw std::runtime_error("Compiler error!: cps conversion: not implemented");
            }
          break;
          }
          case cps_conversion_state::e_conversion_state::T_STEP_3:
          {
          if (std::holds_alternative<PrimitiveCall>(e))
            {
            cps_convert_prim_nonsimple_step3(e, cps_state.nonsimple_vars);
            }
          else if (std::holds_alternative<Let>(e))
            {
            cps_convert_let_nonsimple_step3(e, cps_state.index);
            }
          else if (std::holds_alternative<FunCall>(e))
            {
            cps_convert_funcall_step3(e);
            }
          else if (std::holds_alternative<ForeignCall>(e))
            {
            cps_convert_foreign_nonsimple_step3(e, cps_state.nonsimple_vars);
            }
          else
            {
            throw std::runtime_error("Compiler error!: cps conversion: not implemented");
            }
          break;
          }          
          } // switch
        }
      }
    };
  }

void cps_conversion(Program& prog, const compiler_options& ops)
  {
  cps_assert(prog.expressions.size() <= 1);


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
        ccv.continuation_can_be_moved.push_back(true);
        std::swap(std::get<Lambda>(ccv.continuation.back()), l); // this is a very substantial speedup trick!!
        cps_assert(ccv.continuation_is_valid());
        //ccv.visit(arg);
        ccv.expressions_to_treat.push_back(&arg);
        ccv.treat_expressions(0);
        }, ops);
      }
    else
      {
      cps_conversion_helper ccv;
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
      ccv.continuation_can_be_moved.push_back(true);
      std::swap(std::get<Lambda>(ccv.continuation.back()), l); // this is a very substantial speedup trick!!
      cps_assert(ccv.continuation_is_valid());
      //ccv.visit(e);
      ccv.expressions_to_treat.push_back(&e);
      ccv.treat_expressions(0);
      }
    }
  prog.cps_converted = true;
  }
SKIWI_END
