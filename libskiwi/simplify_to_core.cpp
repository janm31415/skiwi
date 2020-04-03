#include "simplify_to_core.h"
#include "compile_error.h"
#include "visitor.h"
#include <cassert>
#include <sstream>

SKIWI_BEGIN

namespace
  {
  Literal _make_false()
    {
    Literal lit;
    False fa;
    fa.line_nr = -1;
    fa.column_nr = -1;
    lit = fa;
    return lit;
    }

  Literal _make_true()
    {
    Literal lit;
    True fa;
    fa.line_nr = -1;
    fa.column_nr = -1;
    lit = fa;
    return lit;
    }

  If _make_if(Expression a, Expression b, Expression c)
    {
    If i;
    i.line_nr = -1;
    i.column_nr = -1;
    i.arguments.push_back(a);
    i.arguments.push_back(b);
    i.arguments.push_back(c);
    return i;
    }

  Variable _make_var(const std::string& name)
    {
    Variable v;
    v.name = name;
    return v;
    }

  template <class Iter>
  PrimitiveCall _make_primitivecall(const std::string& name, Iter first, Iter last)
    {
    PrimitiveCall p;
    p.line_nr = -1;
    p.column_nr = -1;
    p.primitive_name = name;
    p.arguments.insert(p.arguments.end(), first, last);
    return p;
    }



  struct simplify_to_core_visitor : public base_visitor<simplify_to_core_visitor>
    {
    uint64_t letrec_index;

    void _revisit_expr(Expression& e)
      {
      /*
      simplify_to_core_visitor stcv;
      stcv.letrec_index = letrec_index;
      visitor<Expression, simplify_to_core_visitor>::visit(e, &stcv);
      letrec_index = stcv.letrec_index;
      */
      visitor<Expression, simplify_to_core_visitor>::visit(e, this);
      }

    void _convert_do(Expression& e)
      {
      assert(std::holds_alternative<Do>(e));
      Do& d = std::get<Do>(e);
      Let letr;
      letr.bt = bt_letrec;
      Begin letr_begin;
      FunCall call_loop;
      call_loop.fun.push_back(_make_var("loop"));
      for (const auto& bind : d.bindings)
        {
        assert(bind.size() == 3); // if not, make_do in parse.cpp is incorrect
        call_loop.arguments.push_back(bind[1]); // the inits
        }
      letr_begin.arguments.push_back(call_loop);
      letr.body.push_back(letr_begin);
      Lambda lam;
      for (const auto& bind : d.bindings)
        {
        assert(std::holds_alternative<Variable>(bind.front())); // if not, make_do in parse.cpp is incorrect
        lam.variables.push_back(std::get<Variable>(bind.front()).name);
        }
      Begin lam_begin;
      
      If i;
      i.arguments.push_back(d.test.front());
      Begin if_arg1;
      for (size_t j = 1; j < d.test.size(); ++j)
        if_arg1.arguments.push_back(d.test[j]);
      i.arguments.push_back(if_arg1);
      Begin if_arg2;
      for (const auto& c : d.commands)
        if_arg2.arguments.push_back(c);

      FunCall call_loop2;
      call_loop2.fun.push_back(_make_var("loop"));
      for (const auto& bind : d.bindings)
        {
        assert(bind.size() == 3); // if not, make_do in parse.cpp is incorrect
        call_loop2.arguments.push_back(bind[2]); // the steps
        }
      if_arg2.arguments.push_back(call_loop2);

      i.arguments.push_back(if_arg2);

      lam_begin.arguments.push_back(i);

      lam.body.push_back(lam_begin);
      letr.bindings.emplace_back("loop", lam);
      e = letr;
      }

    void _convert_case(Expression& e)
      {
      assert(std::holds_alternative<Case>(e));
      Case& c = std::get<Case>(e);
      Expression& v = c.val_expr.front();
      if (!std::holds_alternative<Variable>(v))
        {
        Variable var;
        var.name = "case-var";
        Let new_let;
        new_let.bindings.emplace_back(var.name, v);
        Begin new_begin;
        c.val_expr.front() = var;
        new_begin.arguments.push_back(c);
        new_let.body.push_back(new_begin);
        e = new_let;
        }
      else
        {
        Variable& var = std::get<Variable>(v);
        If i;
        if (c.datum_args.empty())
          {
          i.arguments.push_back(_make_false());
          i.arguments.push_back(_make_false());
          }
        else
          {
          auto first_datum = c.datum_args.front();
          c.datum_args.erase(c.datum_args.begin());
          auto first_body = c.then_bodies.front();
          c.then_bodies.erase(c.then_bodies.begin());
          Quote datum;
          datum.arg = first_datum;
          PrimitiveCall p;
          p.primitive_name = "memv";
          p.arguments.push_back(var);
          p.arguments.push_back(datum);
          i.arguments.push_back(p);
          Begin first_body_begin;
          first_body_begin.arguments = first_body;
          i.arguments.push_back(first_body_begin);
          }
        if (c.datum_args.empty())
          {
          Begin else_body_begin;
          else_body_begin.arguments = c.else_body;
          i.arguments.push_back(else_body_begin);
          }
        else
          {
          i.arguments.push_back(c);
          }
        e = i;
        }
      }

    void _convert_cond(Expression& e)
      {
      assert(std::holds_alternative<Cond>(e));
      Cond& c = std::get<Cond>(e);
      if (!c.arguments.empty())
        {
        std::vector<std::pair<std::string, Expression>> bindings;
        bindings.push_back(std::pair<std::string, Expression>("cond-var", c.arguments.front().front()));
        Variable v;
        v.name = "cond-var";
        Expression var(v);
        std::vector<Expression> if_arg;
        if_arg.push_back(var);
        if (c.arguments.front().size() > 1)
          {
          if (c.is_proc.front())
            {
            FunCall new_fun;
            new_fun.fun.push_back(c.arguments.front()[1]);
            new_fun.arguments.push_back(var);            
            if_arg.push_back(new_fun);
            }
          else
            {
            Begin new_begin;
            for (size_t j = 1; j < c.arguments.front().size(); ++j)
              new_begin.arguments.push_back(c.arguments.front()[j]);
            if_arg.push_back(new_begin);
            }
          }
        else
          if_arg.push_back(var);
        c.arguments.erase(c.arguments.begin());
        c.is_proc.erase(c.is_proc.begin());
        if (!c.arguments.empty())
          if_arg.push_back(c);
        If i;
        i.arguments = if_arg;
        if (i.arguments.size() == 2)
          i.arguments.push_back(Nop());
        assert(i.arguments.size() == 3);
        //Let new_let(bindings, Expression(new_if), false, line_nr);
        Let new_let;
        new_let.bindings = bindings;
        Begin let_begin;
        let_begin.arguments.push_back(i);
        new_let.body.push_back(let_begin);

        e = new_let;
        }
      else
        {
        throw_error(c.line_nr, c.column_nr, c.filename, invalid_number_of_arguments, "cond");        
        /*
        std::vector<Expression> if_arg;
        ConditionalIf new_if(if_arg, line_nr);
        expr.cond_if = new_if;
        expr.type = Expression::T_CONDITIONAL_IF;
        expr.accept(*this);
        */
        }
      }

    void _convert_let_star(Let& l)
      {
      assert(l.bt == bt_let_star);
      if (l.bindings.size() <= 1)
        {
        l.bt = bt_let;        
        }
      else
        {
        Let new_let;
        new_let.bindings.push_back(l.bindings.front());
        l.bindings.erase(l.bindings.begin());
        Begin b;
        b.arguments.push_back(l);
        new_let.body.push_back(b);
        l = new_let;
        _convert_let_star(std::get<Let>(std::get<Begin>(l.body.front()).arguments.front()));
        }
      }

    void _convert_letrec(Let& l)
      {
      assert(l.bt == bt_letrec);
      /*
      (letrec([x1 e1] ...[xn en]) body)

      becomes:
        (let([x1 undefined] ...[xn undefined])
        (let([t1 e1] ...[tn en])
          (set! x1 t1)
          ...
          (set! xn tn))
          body)
      */
      Let internal_let;
      Begin internal_begin;
      for (auto& b : l.bindings)
        {
        std::stringstream str;
        str << "#%t" << letrec_index;
        internal_let.bindings.emplace_back(str.str(), b.second);
        Set s;
        s.name = b.first;
        s.value.push_back(_make_var(str.str()));
        internal_begin.arguments.push_back(s);
        b.second = Nop();
        ++letrec_index;
        }
      internal_let.body.push_back(internal_begin);
      Begin& b = std::get<Begin>(l.body.front());
      b.arguments.insert(b.arguments.begin(), internal_let);
      l.bt = bt_let;
      }

    void _convert_named_let(Expression& e, Let& l)
      {
      /*
      (let <variable 0> ([<variable 1> <init 1>] ...) <body>)
      ==  
      ((letrec ([<variable 0> (lambda (<variable 1> ...) <body>)]) <variable 0>) <init 1> ...)
      */
      assert(l.bt == bt_let);
      assert(l.named_let);

      Lambda lam;
      lam.body = l.body;
      for (const auto& b : l.bindings)
        lam.variables.push_back(b.first);

      Let letr;
      letr.bt = bt_letrec;
      std::pair<std::string, Expression> bind(l.let_name, lam);
      letr.bindings.push_back(bind);
      Begin letr_beg;
      letr_beg.arguments.push_back(_make_var(l.let_name));
      letr.body.push_back(letr_beg);

      FunCall f;
      f.fun.push_back(letr);
      for (const auto& b : l.bindings)
        f.arguments.push_back(b.second);

      e = f;
      }

    void _convert_and(Expression& e, PrimitiveCall& p)
      {
      /*
      syntax (and expr ...)

  If no exprs are provided, then result is #t.
  If a single expr is provided, then it is in tail position, so the results of the and expression are the results of the expr.
  Otherwise, the first expr is evaluated. If it produces #f, the result of the and expression is #f.
  Otherwise, the result is the same as an and expression with the remaining exprs in tail position with respect to the original and form.
      */
      assert(p.primitive_name == "and");

      if (p.arguments.empty())
        {
        e = _make_true();
        }
      else if (p.arguments.size() == 1)
        {
        Expression expr = p.arguments[0];
        e = expr;
        }
      else
        {
        e = _make_if(p.arguments.front(), _make_primitivecall("and", p.arguments.begin() + 1, p.arguments.end()), _make_false());
        visitor<Expression, simplify_to_core_visitor>::visit(e, this);
        }
      }

    void _convert_or(Expression& e, PrimitiveCall& p)
      {
      /*
      (or expr ...)

  If no exprs are provided, then result is #f.
  If a single expr is provided, then it is in tail position, so the results of the or expression are the results of the expr.
  Otherwise, the first expr is evaluated. If it produces a value other than #f, that result is the result of the or expression. Otherwise, the result is the same as an or expression with the remaining exprs in tail position with respect to the original or form.
      */
      assert(p.primitive_name == "or");

      if (p.arguments.empty())
        {
        e = _make_false();
        }
      else if (p.arguments.size() == 1)
        {
        Expression expr = p.arguments[0];
        e = expr;
        }
      else
        {
        Let l;
        l.bindings.emplace_back("#%x", p.arguments.front());
        Begin letbody;
        letbody.arguments.push_back(_make_if(_make_var("#%x"), _make_var("#%x"), _make_primitivecall("or", p.arguments.begin() + 1, p.arguments.end())));
        l.body.push_back(letbody);
        e = l;
        visitor<Expression, simplify_to_core_visitor>::visit(e, this);
        }
      }

    void _convert_unless(Expression& e, PrimitiveCall& p)
      {
      assert(p.primitive_name == "unless");
      If i;
      i.line_nr = p.line_nr;
      i.column_nr = p.column_nr;
      if (p.arguments.empty())
        throw_error(p.line_nr, p.column_nr, p.filename, invalid_number_of_arguments, "unless");
      PrimitiveCall np;
      np.primitive_name = "not";
      np.arguments.push_back(p.arguments.front());
      i.arguments.push_back(np);
      Begin new_begin;
      new_begin.arguments.insert(new_begin.arguments.end(), p.arguments.begin() + 1, p.arguments.end());
      i.arguments.emplace_back(new_begin);
      i.arguments.push_back(Nop());
      e = i;
      }

    void _convert_when(Expression& e, PrimitiveCall& p)
      {
      assert(p.primitive_name == "when");
      If i;
      i.line_nr = p.line_nr;
      i.column_nr = p.column_nr;
      if (p.arguments.empty())
        throw_error(p.line_nr, p.column_nr, p.filename, invalid_number_of_arguments, "when");
      i.arguments.push_back(p.arguments.front());
      Begin new_begin;
      new_begin.arguments.insert(new_begin.arguments.end(), p.arguments.begin() + 1, p.arguments.end());
      i.arguments.emplace_back(new_begin);
      i.arguments.push_back(Nop());
      e = i;
      }

    void _convert_delay(Expression& e, PrimitiveCall& p)
      {
      assert(p.primitive_name == "delay"); 
      if (p.arguments.size() != 1)
        throw_error(p.line_nr, p.column_nr, p.filename, invalid_number_of_arguments, "delay");
      Lambda lam;
      Begin lam_begin;
      lam_begin.arguments.push_back(p.arguments.front());
      lam.body.push_back(lam_begin);
      //p.primitive_name = "make-promise";
      //p.as_object = true;
      Variable v;
      v.name = "make-promise";
      FunCall f;
      f.fun.push_back(v);
      f.arguments.push_back(lam);
      e = f;
      }

    virtual void _postvisit(Expression& e)
      {
      if (std::holds_alternative<PrimitiveCall>(e))
        {
        PrimitiveCall& p = std::get<PrimitiveCall>(e);
        if (p.primitive_name == "and")
          {
          _convert_and(e, p);
          _revisit_expr(e);
          }
        else if (p.primitive_name == "or")
          {
          _convert_or(e, p);
          _revisit_expr(e);
          }
        else if (p.primitive_name == "unless")
          {
          _convert_unless(e, p);
          _revisit_expr(e);
          }
        else if (p.primitive_name == "when")
          {
          _convert_when(e, p);
          _revisit_expr(e);
          }
        else if (p.primitive_name == "delay")
          {
          _convert_delay(e, p);
          _revisit_expr(e);
          }
        }
      else if (std::holds_alternative<Let>(e))
        {
        Let& l = std::get<Let>(e);
        if (l.bt == bt_letrec)
          {
          _convert_letrec(l);
          _revisit_expr(e);
          }
        else if (l.bt == bt_let_star)
          {
          _convert_let_star(l);
          _revisit_expr(e);
          }
        else if (l.named_let)
          {
          _convert_named_let(e, l);
          _revisit_expr(e);
          }
        }
      else if (std::holds_alternative<Cond>(e))
        {
        _convert_cond(e);
        _revisit_expr(e);
        }
      else if (std::holds_alternative<Case>(e))
        {
        _convert_case(e);
        _revisit_expr(e);
        }
      else if (std::holds_alternative<Do>(e))
        {
        _convert_do(e);
        _revisit_expr(e);
        }
      }
    };

  }

void simplify_to_core_forms(Program& prog)
  {
  simplify_to_core_visitor stcv;
  stcv.letrec_index = 0;
  visitor<Program, simplify_to_core_visitor>::visit(prog, &stcv);
  prog.simplified_to_core_forms = true;
  }

SKIWI_END
