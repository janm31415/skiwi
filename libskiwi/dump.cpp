#include "dump.h"
#include "visitor.h"
#include <sstream>
#include <iomanip>

SKIWI_BEGIN

namespace
  {
  struct dump_visitor : public base_visitor<dump_visitor>
    {
    std::stringstream str;
    char sbl, sbr;

    dump_visitor() : sbl('['), sbr(']') {}

    virtual bool _previsit(Fixnum& f) { str << f.value << " "; return true; }
    virtual bool _previsit(Flonum& f) { str << f.value << " "; return true; }
    virtual bool _previsit(Nil&) { str << "() " << " "; return true; }
    virtual bool _previsit(String& s) { str << "\"" << s.value << "\" "; return true; }
    virtual bool _previsit(Symbol& s) { str << s.value << " "; return true; }
    virtual bool _previsit(True&) { str << "#t "; return true; }
    virtual bool _previsit(False&) { str << "#f "; return true; }
    virtual bool _previsit(Nop&) { str << "#undefined "; return true; }
    virtual bool _previsit(Character& cha)
      {
      unsigned char ch = (unsigned char)cha.value;
      str << "#\\" << int(ch) << " ";
      return true;
      }
    virtual bool _previsit(Variable& v) { str << v.name << " "; return true; }
    virtual bool _previsit(Begin&) { str << "( begin "; return true; }
    virtual bool _previsit(FunCall& f)
      {
      str << "( ";
      visitor<Expression, dump_visitor>::visit(f.fun.front(), this);
      for (auto& arg : f.arguments)
        visitor<Expression, dump_visitor>::visit(arg, this);
      return false;
      }
    virtual bool _previsit(If&) { str << "( if ";  return true; }
    virtual bool _previsit(Lambda& l)
      {
      str << "( lambda ( ";
      for (const auto& v : l.variables)
        str << v << " ";
      str << ") ";
      return true;
      }
    virtual bool _previsit(Case& c)
      {
      str << "( case ";
      visitor<Expression, dump_visitor>::visit(c.val_expr.front(), this);
      assert(c.datum_args.size() == c.then_bodies.size());
      for (size_t i = 0; i < c.datum_args.size(); ++i)
        {
        str << sbl << " " << c.datum_args[i] << " ";       
        for (auto& arg : c.then_bodies[i])
          visitor<Expression, dump_visitor>::visit(arg, this);
        str << sbr << " ";
        }
      str << sbl << " else ";
      for (auto& arg : c.else_body)
        visitor<Expression, dump_visitor>::visit(arg, this);
      str << sbr << " ) ";
      return false;
      }
    virtual bool _previsit(Cond& c)
      {
      str << "( cond ";
      for (auto& arg_v : c.arguments)
        {
        str << sbl << " ";
        for (auto& arg : arg_v)
          visitor<Expression, dump_visitor>::visit(arg, this);
        str << sbr << " ";
        }
      str << ") ";
      return false;
      }
    virtual bool _previsit(Do& d)
      {
      str << "( do ";
      for (auto& b : d.bindings)
        {
        str << sbl << " ";
        for (auto& arg : b)
          visitor<Expression, dump_visitor>::visit(arg, this);
        str << sbr << " ";
        }
      str << ") ( ";
      for (auto& tst : d.test)
        visitor<Expression, dump_visitor>::visit(tst, this);
      str << ") ";
      for (auto& c : d.commands)
        visitor<Expression, dump_visitor>::visit(c, this);
      str << ") ";
      return false;
      }
    virtual bool _previsit(Let& l)
      {
      switch (l.bt)
        {
        case bt_let: str << "( let ( "; break;
        case bt_let_star: str << "( let* ( "; break;
        case bt_letrec: str << "( letrec ( "; break;
        }
      for (auto& arg : l.bindings)
        {
        str << sbl << " " << arg.first << " ";
        visitor<Expression, dump_visitor>::visit(arg.second, this);
        str << sbr << " ";
        }
      str << ") ";
      visitor<Expression, dump_visitor>::visit(l.body.front(), this);
      return false;
      }
    virtual bool _previsit(Quote& q)
      {
      switch (q.type)
        {
        case Quote::qt_quote: str << "( quote "; break;
        case Quote::qt_backquote: str << "( quasiquote "; break;
        case Quote::qt_unquote: str << "( unquote "; break;
        case Quote::qt_unquote_splicing: str << "( unquote-splicing "; break;
        }      
      str << q.arg << " ) ";
      return true;
      }
    virtual bool _previsit(PrimitiveCall& p)
      {
      if (p.as_object)
        {
        str << p.primitive_name << " ";
        return false;
        }
      str << "( ";
      str << p.primitive_name;
      str << " ";
      return true;
      }
    virtual bool _previsit(ForeignCall& p)
      {
      str << "( foreign-call ";
      str << p.foreign_name;
      str << " ";
      return true;
      }
    virtual bool _previsit(Set& s)
      {
      str << "( set! " << s.name << " ";
      return true;
      }

    virtual void _postvisit(Begin&) { str << ") "; }
    virtual void _postvisit(FunCall&) { str << ") "; }
    virtual void _postvisit(If&) { str << ") "; }
    virtual void _postvisit(Lambda&) { str << ") "; }
    virtual void _postvisit(Let&) { str << ") "; }
    virtual void _postvisit(ForeignCall&) { str << ") "; }
    virtual void _postvisit(PrimitiveCall& p) { if (!p.as_object) str << ") "; }
    virtual void _postvisit(Set&) { str << ") "; }
    };
  }

void dump(std::ostream& out, Program& prog, bool use_square_brackets)
  {
  dump_visitor dv;
  if (!use_square_brackets)
    {
    dv.sbl = '(';
    dv.sbr = ')';
    }
  visitor<Program, dump_visitor>::visit(prog, &dv);
  out << dv.str.str();
  }

void dump(std::ostream& out, Expression& expr, bool use_square_brackets)
  {
  dump_visitor dv;
  if (!use_square_brackets)
    {
    dv.sbl = '(';
    dv.sbr = ')';
    }
  visitor<Expression, dump_visitor>::visit(expr, &dv);
  out << dv.str.str();
  }

SKIWI_END
