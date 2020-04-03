#pragma once

#include <asm/namespace.h>
#include <variant>
#include <cassert>

#include "parse.h"

SKIWI_BEGIN


template <class U, class T>
struct visitor
  {
  static void visit(U&, T*)
    {
    assert(0);
    }
  };

template <class T>
struct visitor<Fixnum, T>
  {
  static void visit(Fixnum& f, T* v)
    {
    ((T::base*)v)->_previsit(f);
    ((T::base*)v)->_postvisit(f);
    }
  };

template <class T>
struct visitor<Flonum, T>
  {
  static void visit(Flonum& f, T* v)
    {
    ((T::base*)v)->_previsit(f);
    ((T::base*)v)->_postvisit(f);
    }
  };

template <class T>
struct visitor<Nil, T>
  {
  static void visit(Nil& n, T* v)
    {
    ((T::base*)v)->_previsit(n);
    ((T::base*)v)->_postvisit(n);
    }
  };

template <class T>
struct visitor<String, T>
  {
  static void visit(String& s, T* v)
    {
    ((T::base*)v)->_previsit(s);
    ((T::base*)v)->_postvisit(s);
    }
  };

template <class T>
struct visitor<Symbol, T>
  {
  static void visit(Symbol& s, T* v)
    {
    ((T::base*)v)->_previsit(s);
    ((T::base*)v)->_postvisit(s);
    }

  };
template <class T>
struct visitor<True, T>
  {
  static void visit(True& t, T* v)
    {
    ((T::base*)v)->_previsit(t);
    ((T::base*)v)->_postvisit(t);
    }
  };

template <class T>
struct visitor<False, T>
  {
  static void visit(False& f, T* v)
    {
    ((T::base*)v)->_previsit(f);
    ((T::base*)v)->_postvisit(f);
    }
  };

template <class T>
struct visitor<Character, T>
  {
  static void visit(Character& c, T* v)
    {
    ((T::base*)v)->_previsit(c);
    ((T::base*)v)->_postvisit(c);
    }
  };

template <class T>
struct visitor<Nop, T>
  {
  static void visit(Nop& c, T* v)
    {
    ((T::base*)v)->_previsit(c);
    ((T::base*)v)->_postvisit(c);
    }
  };

template <class T>
struct visitor<Variable, T>
  {
  static void visit(Variable& va, T* v)
    {
    ((T::base*)v)->_previsit(va);
    ((T::base*)v)->_postvisit(va);
    }
  };

template <class T>
struct visitor<Set, T>
  {
  static void visit(Set& s, T* v)
    {
    if (((T::base*)v)->_previsit(s))
      visitor<Expression, T>::visit(s.value.front(), v);
    ((T::base*)v)->_postvisit(s);
    }
  };

template <class T>
struct visitor<PrimitiveCall, T>
  {
  static void visit(PrimitiveCall& p, T* v)
    {
    if (((T::base*)v)->_previsit(p))
      for (auto& arg : p.arguments)
        visitor<Expression, T>::visit(arg, v);
    ((T::base*)v)->_postvisit(p);
    }
  };

template <class T>
struct visitor<ForeignCall, T>
  {
  static void visit(ForeignCall& p, T* v)
    {
    if (((T::base*)v)->_previsit(p))
      for (auto& arg : p.arguments)
        visitor<Expression, T>::visit(arg, v);
    ((T::base*)v)->_postvisit(p);
    }
  };

template <class T>
struct visitor<Literal, T>
  {
  static void visit(Literal& l, T* v)
    {
    if (((T::base*)v)->_previsit(l))
      std::visit(*v, l);
    ((T::base*)v)->_postvisit(l);
    }
  };

template <class T>
struct visitor<Let, T>
  {
  static void visit(Let& l, T* v)
    {
    if (((T::base*)v)->_previsit(l))
      {
      for (auto& arg : l.bindings)
        visitor<Expression, T>::visit(arg.second, v);
      visitor<Expression, T>::visit(l.body.front(), v);
      }
    ((T::base*)v)->_postvisit(l);
    }
  };

template <class T>
struct visitor<Lambda, T>
  {
  static void visit(Lambda& l, T* v)
    {
    if (((T::base*)v)->_previsit(l))
      visitor<Expression, T>::visit(l.body.front(), v);
    ((T::base*)v)->_postvisit(l);
    }
  };

template <class T>
struct visitor<If, T>
  {
  static void visit(If& i, T* v)
    {
    if (((T::base*)v)->_previsit(i))
      for (auto& arg : i.arguments)
        visitor<Expression, T>::visit(arg, v);
    ((T::base*)v)->_postvisit(i);
    }
  };

template <class T>
struct visitor<Cond, T>
  {
  static void visit(Cond& c, T* v)
    {
    if (((T::base*)v)->_previsit(c))
      {
      for (auto& arg_v : c.arguments)
        for (auto& arg : arg_v)
          visitor<Expression, T>::visit(arg, v);
      }
    ((T::base*)v)->_postvisit(c);
    }
  };

template <class T>
struct visitor<Do, T>
  {
  static void visit(Do& d, T* v)
    {
    if (((T::base*)v)->_previsit(d))
      {
      for (auto& binding : d.bindings)
        for (auto& arg : binding)
          visitor<Expression, T>::visit(arg, v);
      for (auto& tst : d.test)
        visitor<Expression, T>::visit(tst, v);
      for (auto& command : d.commands)
        visitor<Expression, T>::visit(command, v);
      }
    ((T::base*)v)->_postvisit(d);
    }
  };


template <class T>
struct visitor<Case, T>
  {
  static void visit(Case& c, T* v)
    {
    if (((T::base*)v)->_previsit(c))
      {
      visitor<Expression, T>::visit(c.val_expr.front(), v);
      assert(c.datum_args.size() == c.then_bodies.size());
      for (size_t i = 0; i < c.datum_args.size(); ++i)
        {
        for (auto& arg : c.then_bodies[i])
          visitor<Expression, T>::visit(arg, v);
        }
      for (auto& arg : c.else_body)
        visitor<Expression, T>::visit(arg, v);
      }
    ((T::base*)v)->_postvisit(c);
    }
  };

template <class T>
struct visitor<Quote, T>
  {
  static void visit(Quote& q, T* v)
    {
    ((T::base*)v)->_previsit(q);
    ((T::base*)v)->_postvisit(q);
    }
  };

template <class T>
struct visitor<FunCall, T>
  {
  static void visit(FunCall& f, T* v)
    {
    if (((T::base*)v)->_previsit(f))
      {
      for (auto& arg : f.arguments)
        visitor<Expression, T>::visit(arg, v);
      visitor<Expression, T>::visit(f.fun.front(), v);
      }
    ((T::base*)v)->_postvisit(f);
    }
  };

template <class T>
struct visitor<Begin, T>
  {
  static void visit(Begin& b, T* v)
    {
    if (((T::base*)v)->_previsit(b))
      for (auto& arg : b.arguments)
        visitor<Expression, T>::visit(arg, v);
    ((T::base*)v)->_postvisit(b);
    }
  };

template <class T>
struct visitor<Expression, T>
  {
  static void visit(Expression& exp, T* v)
    {
    if (((T::base*)v)->_previsit(exp))
      std::visit(*v, exp);
    ((T::base*)v)->_postvisit(exp);
    }
  };

template <class T>
struct visitor<Expressions, T>
  {
  static void visit(Expressions& exprs, T* v)
    {
    for (auto& exp : exprs)
      visitor<Expression, T>::visit(exp, v);
    }
  };

template <class T>
struct visitor<Program, T>
  {
  static void visit(Program& prog, T* v)
    {
    if (((T::base*)v)->_previsit(prog))
      visitor<Expressions, T>::visit(prog.expressions, v);
    ((T::base*)v)->_postvisit(prog);
    }
  };


template <class T>
struct base_visitor
  {
  typedef typename base_visitor<T> base;

  template <class F>
  void operator()(F& i)
    {
    visitor<F, T>::visit(i, (T*)this);
    }

  virtual bool _previsit(Fixnum&) { return true; }
  virtual bool _previsit(Flonum&) { return true; }
  virtual bool _previsit(Nil&) { return true; }
  virtual bool _previsit(String&) { return true; }
  virtual bool _previsit(Symbol&) { return true; }
  virtual bool _previsit(True&) { return true; }
  virtual bool _previsit(False&) { return true; }
  virtual bool _previsit(Character&) { return true; }
  virtual bool _previsit(Nop&) { return true; }
  virtual bool _previsit(Variable&) { return true; }
  virtual bool _previsit(Expression&) { return true; }
  virtual bool _previsit(Begin&) { return true; }
  virtual bool _previsit(FunCall&) { return true; }
  virtual bool _previsit(If&) { return true; }
  virtual bool _previsit(Case&) { return true; }
  virtual bool _previsit(Cond&) { return true; }
  virtual bool _previsit(Do&) { return true; }
  virtual bool _previsit(Lambda&) { return true; }
  virtual bool _previsit(Let&) { return true; }
  virtual bool _previsit(Literal&) { return true; }
  virtual bool _previsit(PrimitiveCall&) { return true; }
  virtual bool _previsit(ForeignCall&) { return true; }
  virtual bool _previsit(Set&) { return true; }
  virtual bool _previsit(Quote&) { return true; }
  virtual bool _previsit(Program&) { return true; }

  virtual void _postvisit(Fixnum&) {}
  virtual void _postvisit(Flonum&) {}
  virtual void _postvisit(Nil&) {}
  virtual void _postvisit(String&) {}
  virtual void _postvisit(Symbol&) {}
  virtual void _postvisit(True&) {}
  virtual void _postvisit(False&) {}
  virtual void _postvisit(Character&) {}
  virtual void _postvisit(Nop&) {}
  virtual void _postvisit(Variable&) {}
  virtual void _postvisit(Expression&) {}
  virtual void _postvisit(Begin&) {}
  virtual void _postvisit(FunCall&) {}
  virtual void _postvisit(If&) {}
  virtual void _postvisit(Case&) {}
  virtual void _postvisit(Cond&) {}
  virtual void _postvisit(Do&) {}
  virtual void _postvisit(Lambda&) {}
  virtual void _postvisit(Let&) {}
  virtual void _postvisit(Literal&) {}
  virtual void _postvisit(PrimitiveCall&) {}
  virtual void _postvisit(ForeignCall&) {}
  virtual void _postvisit(Set&) {}
  virtual void _postvisit(Quote&) {}
  virtual void _postvisit(Program&) {}
  };

SKIWI_END
