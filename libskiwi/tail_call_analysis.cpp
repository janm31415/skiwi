#include "tail_call_analysis.h"
#include "visitor.h"
#include <variant>

SKIWI_BEGIN

namespace
  {
  struct unset_tail_pos
    {
    template <class F>
    void operator()(F& i)
      {
      i.tail_position = false;
      }

    void operator()(Literal& i)
      {
      unset_tail_pos stp;
      std::visit(stp, i);
      }
    };

  struct tail_call_init : public base_visitor<tail_call_init>
    {

    virtual bool _previsit(Expression& e)
      {
      unset_tail_pos stp;
      std::visit(stp, e);
      return true;
      }
    };

  /*
  A procedure call is called a tail call if it occurs in a tail position.
  A tail position is defined recursively as follows:

  - the body of a procedure is in tail position
  - if a let expression is in tail position, then the body
    of the let is in tail position
  - if the condition expression (if test conseq altern) is in tail
    position, then the conseq and altern branches are also in tail position.
  - all other expressions are not in tail position.
  */

  struct set_tail_pos
    {
    template <class F>
    void operator()(F& i)
      {
      i.tail_position = true;
      }

    void operator()(Literal& i)
      {
      set_tail_pos stp;
      std::visit(stp, i);
      }
    };

  struct tail_call_analysis_visitor : public base_visitor<tail_call_analysis_visitor>
    {
    bool in_tail_position;
    tail_call_analysis_visitor() : in_tail_position(false) {}

    void set_tail_position(Expression& e)
      {
      set_tail_pos stp;
      std::visit(stp, e);
      }

    virtual bool _previsit(Variable&)
      {
      return true;
      }

    virtual bool _previsit(Begin& b)
      {
      if (b.tail_position && !b.arguments.empty())
        {
        set_tail_position(b.arguments.back());
        }
      return true;
      }

    virtual bool _previsit(FunCall&)
      {
      return true;
      }

    virtual bool _previsit(If& i)
      {
      if (i.tail_position)
        {
        if (i.arguments.size() >= 2)
          set_tail_position(i.arguments[1]);
        if (i.arguments.size() >= 3)
          set_tail_position(i.arguments[2]);
        }
      return true;
      }

    virtual bool _previsit(Lambda& l)
      {
      set_tail_position(l.body.front());
      return true;
      }

    virtual bool _previsit(Let& l)
      {
      if (l.tail_position)
        {
        set_tail_position(l.body.front());
        }
      return true;
      }

    virtual bool _previsit(Literal&)
      {
      return true;
      }

    virtual bool _previsit(ForeignCall&)
      {
      return true;
      }

    virtual bool _previsit(PrimitiveCall&)
      {
      return true;
      }

    virtual bool _previsit(Set&)
      {
      return true;
      }

    virtual bool _previsit(Program& p)
      {
      if (!p.expressions.empty())
        {
        if (p.expressions.size() == 1 && std::holds_alternative<Begin>(p.expressions.front()))
          {
          for (auto& expr : std::get<Begin>(p.expressions.front()).arguments)
            set_tail_position(expr);
          }
        else
          set_tail_position(p.expressions.back());
        }
      return true;
      }
    };


  struct tail_calls_set_helper
    {
    std::vector<Expression*> expressions;
    bool tail_position;

    tail_calls_set_helper(bool target_tail_position) : tail_position(target_tail_position)
      {
      }

    void treat_expressions()
      {
      while (!expressions.empty())
        {
        Expression* p_expr = expressions.back();
        expressions.pop_back();
        Expression& e = *p_expr;
        if (std::holds_alternative<Literal>(e))
          {
          if (tail_position)
            {
            set_tail_pos stp;
            std::visit(stp, e);
            }
          else
            {
            unset_tail_pos utp;
            std::visit(utp, e);
            }
          }
        else if (std::holds_alternative<Variable>(e))
          {
          std::get<Variable>(e).tail_position = tail_position;
          }
        else if (std::holds_alternative<Nop>(e))
          {
          std::get<Nop>(e).tail_position = tail_position;
          }
        else if (std::holds_alternative<Quote>(e))
          {
          std::get<Quote>(e).tail_position = tail_position;
          }
        else if (std::holds_alternative<Set>(e))
          {
          std::get<Set>(e).tail_position = tail_position;
          //Set& s = std::get<Set>(e);
          expressions.push_back(&std::get<Set>(e).value.front());
          }
        else if (std::holds_alternative<If>(e))
          {
          std::get<If>(e).tail_position = tail_position;
          for (auto rit = std::get<If>(e).arguments.rbegin(); rit != std::get<If>(e).arguments.rend(); ++rit)
            expressions.push_back(&(*rit));
          }
        else if (std::holds_alternative<Begin>(e))
          {
          std::get<Begin>(e).tail_position = tail_position;
          for (auto rit = std::get<Begin>(e).arguments.rbegin(); rit != std::get<Begin>(e).arguments.rend(); ++rit)
            expressions.push_back(&(*rit));
          }
        else if (std::holds_alternative<PrimitiveCall>(e))
          {
          std::get<PrimitiveCall>(e).tail_position = tail_position;
          for (auto rit = std::get<PrimitiveCall>(e).arguments.rbegin(); rit != std::get<PrimitiveCall>(e).arguments.rend(); ++rit)
            expressions.push_back(&(*rit));
          }
        else if (std::holds_alternative<ForeignCall>(e))
          {
          std::get<ForeignCall>(e).tail_position = tail_position;
          for (auto rit = std::get<ForeignCall>(e).arguments.rbegin(); rit != std::get<ForeignCall>(e).arguments.rend(); ++rit)
            expressions.push_back(&(*rit));
          }
        else if (std::holds_alternative<Lambda>(e))
          {
          std::get<Lambda>(e).tail_position = tail_position;
          Lambda& l = std::get<Lambda>(e);
          expressions.push_back(&l.body.front());
          }
        else if (std::holds_alternative<FunCall>(e))
          {
          std::get<FunCall>(e).tail_position = tail_position;
          expressions.push_back(&std::get<FunCall>(e).fun.front());
          for (auto rit = std::get<FunCall>(e).arguments.rbegin(); rit != std::get<FunCall>(e).arguments.rend(); ++rit)
            expressions.push_back(&(*rit));
          }
        else if (std::holds_alternative<Let>(e))
          {
          std::get<Let>(e).tail_position = tail_position;
          Let& l = std::get<Let>(e);
          expressions.push_back(&l.body.front());
          for (auto rit = l.bindings.rbegin(); rit != l.bindings.rend(); ++rit)
            expressions.push_back(&(*rit).second);
          }
        else
          throw std::runtime_error("Compiler error!: Tail call analysis: not implemented");
        }
      }
    };

  struct tail_call_analysis_helper
    {
    std::vector<Expression*> expressions;

    void set_tail_position(Expression& e)
      {
      set_tail_pos stp;
      std::visit(stp, e);
      }

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
          If& i = std::get<If>(e);
          if (i.tail_position)
            {
            if (i.arguments.size() >= 2)
              set_tail_position(i.arguments[1]);
            if (i.arguments.size() >= 3)
              set_tail_position(i.arguments[2]);
            }
          for (auto rit = std::get<If>(e).arguments.rbegin(); rit != std::get<If>(e).arguments.rend(); ++rit)
            expressions.push_back(&(*rit));
          }
        else if (std::holds_alternative<Begin>(e))
          {
          Begin& b = std::get<Begin>(e);
          if (b.tail_position && !b.arguments.empty())
            {
            set_tail_position(b.arguments.back());
            }
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
          set_tail_position(l.body.front());
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
          if (l.tail_position)
            {
            set_tail_position(l.body.front());
            }
          expressions.push_back(&l.body.front());
          for (auto rit = l.bindings.rbegin(); rit != l.bindings.rend(); ++rit)
            expressions.push_back(&(*rit).second);
          }
        else
          throw std::runtime_error("Compiler error!: Tail call analysis: not implemented");
        }
      }

    void add_program(Program& p)
      {
      if (!p.expressions.empty())
        {
        if (p.expressions.size() == 1 && std::holds_alternative<Begin>(p.expressions.front()))
          {
          for (auto& expr : std::get<Begin>(p.expressions.front()).arguments)
            set_tail_position(expr);
          }
        else
          set_tail_position(p.expressions.back());
        }
      for (auto& expr : p.expressions)
        expressions.push_back(&expr);
      std::reverse(expressions.begin(), expressions.end());
      }

    void add_expression(Expression& e)
      {
      if (std::holds_alternative<Begin>(e))
        {
        for (auto& expr : std::get<Begin>(e).arguments)
          set_tail_position(expr);
        }
      else
        set_tail_position(e);
      expressions.push_back(&e);
      }
    };
  }

void tail_call_analysis(Program& prog)
  {
  if (prog.tail_call_analysis)
    {
    //tail_call_init tci;
    //visitor<Program, tail_call_init>::visit(prog, &tci);
    tail_calls_set_helper tcsh(false);
    for (auto& e : prog.expressions)
      tcsh.expressions.push_back(&e);
    tcsh.treat_expressions();
    }
  //tail_call_analysis_visitor tcav;
  //visitor<Program, tail_call_analysis_visitor>::visit(prog, &tcav);

  tail_call_analysis_helper tcah;
  tcah.add_program(prog);
  tcah.treat_expressions();

  prog.tail_call_analysis = true;
  }

void tail_call_analysis(Expression& e)
  {
  tail_calls_set_helper tcsh(false);
  tcsh.expressions.push_back(&e);
  tcsh.treat_expressions();

  tail_call_analysis_helper tcah;
  tcah.add_expression(e);
  tcah.treat_expressions();
  }

SKIWI_END
