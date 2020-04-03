#include "quasiquote_conversion.h"

#include <cassert>
#include <sstream>
#include <inttypes.h>
#include <stdio.h>
#include <iostream>

#include "compile_error.h"
#include "visitor.h"
#include "tokenize.h"
#include "parse.h"
#include "dump.h"

SKIWI_BEGIN

namespace
  {

  struct quasiquote_conversion_visitor : public base_visitor<quasiquote_conversion_visitor>
    {
    bool tag_is_backquote(const cell& c)
      {
      if (c.type == ct_pair && c.pair.size() == 2)
        {
        return (c.pair[0].type == ct_symbol && ((c.pair[0].value == "`") || (c.pair[0].value == "quasiquote")));
        }
      return false;
      }

    bool tag_is_quote(const cell& c)
      {
      if (c.type == ct_pair && c.pair.size() == 2)
        {
        return (c.pair[0].type == ct_symbol && ((c.pair[0].value == "'") || (c.pair[0].value == "quote")));
        }
      return false;
      }

    bool tag_is_comma(const cell& c)
      {
      if (c.type == ct_pair && c.pair.size() == 2)
        {
        return (c.pair[0].type == ct_symbol && ((c.pair[0].value == ",") || (c.pair[0].value == "unquote")));
        }
      return false;
      }

    bool tag_is_comma_atsign(const cell& c)
      {
      if (c.type == ct_pair && c.pair.size() == 2)
        {
        return (c.pair[0].type == ct_symbol && ((c.pair[0].value == ",@") || (c.pair[0].value == "unquote-splicing")));
        }
      return false;
      }

    bool tag_is_special(const cell& c)
      {
      return tag_is_backquote(c) || tag_is_quote(c) || tag_is_comma(c) || tag_is_comma_atsign(c);
      }

    cell tag_data(const cell& c)
      {
      assert(tag_is_backquote(c) || tag_is_comma(c) || tag_is_comma_atsign(c));
      if (c.pair[0].value.length() > 3) // quasiquote, unquote, or unquote-splicing
        {
        if (c.pair[1].pair[1] == nil_sym)
          return c.pair[1].pair[0];
        else
          assert(0);
        }
      return c.pair[1];
      }

    bool is_pair(const cell& c)
      {
      return (c.type == ct_pair && c.pair.size() == 2);
      }

    bool is_nil(const cell& c)
      {
      return (c.type == ct_pair && c.pair.size() == 0);
      }

    bool is_simple(const cell& in)
      {
      std::vector<cell> todo;
      todo.push_back(in);
      while (!todo.empty())
        {
        cell c = todo.back();
        todo.pop_back();
        if (tag_is_special(c))
          return false;
        if (is_pair(c))
          {
          todo.push_back(c.pair[0]);
          todo.push_back(c.pair[1]);
          }
        else if (c.type == ct_vector)
          {
          for (const auto& v : c.vec)
            todo.push_back(v);
          }
        }
      return true;
      }

    bool has_splicing(const cell& in)
      {
      std::vector<cell> todo;
      todo.push_back(in);
      while (!todo.empty())
        {
        cell c = todo.back();
        todo.pop_back();
        if (tag_is_comma_atsign(c))
          return true;
        if (is_pair(c))
          {
          todo.push_back(c.pair[0]);
          todo.push_back(c.pair[1]);
          }
        else if (c.type == ct_vector)
          {
          for (const auto& v : c.vec)
            todo.push_back(v);
          }
        }
      return false;
      }

    cell expression_to_cell(Expression& expr)
      {
      std::stringstream str;
      dump(str, expr);
      auto tokens = tokenize(str.str());
      std::reverse(tokens.begin(), tokens.end());
      return read_from(tokens, true);
      }

    Expression make_list(const Expression& expr)
      {
      PrimitiveCall p;
      p.primitive_name = "cons";
      Quote q;
      q.arg.type = ct_pair;
      p.arguments.push_back(expr);
      p.arguments.push_back(q);
      return p;
      }

    Expression cell_to_expression(const cell& c, bool as_list)
      {
      std::stringstream str;
      str << c;
      auto tokens = tokenize(str.str());
      std::reverse(tokens.begin(), tokens.end());
      auto expr = make_expression(tokens);
      if (!as_list)
        return expr;
      return make_list(expr);
      }

    Expression make_quote(const std::string& quote_type, const cell& c, bool list, int depth)
      {
      Quote q;
      q.arg.type = ct_symbol;
      q.arg.value = quote_type;
      auto td = tag_data(c);
      int target_depth = quote_type == "quasiquote" ? depth + 1 : depth - 1;
      Expression e;
      e = qq_expand(td, target_depth);
      PrimitiveCall p;
      p.primitive_name = "cons";
      p.arguments.push_back(q);
      PrimitiveCall p2;
      p2.primitive_name = "cons";
      p2.arguments.push_back(e);
      Quote q2;
      q2.arg.type = ct_pair;
      p2.arguments.push_back(q2);
      p.arguments.push_back(p2);
      if (!list)
        return p;
      else
        {
        return make_list(p);
        }      
      }

    Expression qq_expand_list(const cell& c, int depth)
      {
      if (is_simple(c))
        {
        Quote q;
        q.arg.type = ct_pair;
        q.arg.pair.push_back(c);
        q.arg.pair.push_back(nil_sym);
        return q;
        }
      else if (tag_is_comma(c))
        {
        if (depth == 0)
          {
          auto expr = cell_to_expression(tag_data(c), true);
          return expr;
          }
        else
          {
          return make_quote("unquote", c, true, depth);
          }
        }
      else if (tag_is_comma_atsign(c))
        {
        if (depth == 0)
          {
          auto td = tag_data(c);
          return cell_to_expression(td, false);
          /*
          if (is_pair(td))
            {
            return cell_to_expression(td, false);
            }
          else
            {
            throw_error(-1, -1, "", bad_syntax, ",@");
            }
            */
          }
        else
          return make_quote("unquote-splicing", c, true, depth);
        }
      else if (tag_is_backquote(c))
        {
        return make_quote("quasiquote", c, true, depth);     
        }
      else if (is_pair(c))
        {
        if (!has_splicing(c.pair[0]))
          {
          auto expr1 = qq_expand(c.pair[0], depth);
          auto expr2 = qq_expand(c.pair[1], depth);
          PrimitiveCall prim;
          prim.primitive_name = "cons";
          prim.arguments.push_back(expr1);
          prim.arguments.push_back(expr2);
          return make_list(prim);
          }
        else if (is_nil(c.pair[1]))
          {
          auto expr1 = qq_expand_list(c.pair[0], depth);
          return make_list(expr1);
          }
        else
          {
          auto expr1 = qq_expand_list(c.pair[0], depth);
          auto expr2 = qq_expand(c.pair[1], depth);
          Expression expr;
          FunCall f;
          Variable v;
          v.name = "append";
          f.fun.push_back(v);
          f.arguments.push_back(expr1);
          f.arguments.push_back(expr2);
          return make_list(f);
          }
        }
      else if (c.type == ct_vector)
        {
        cell list_cell;
        list_cell.type = ct_pair;
        cell* parent = &list_cell;
        for (const auto& cv : c.vec)
          {
          assert(parent->pair.size() == 0);
          parent->pair.push_back(cv);
          parent->pair.emplace_back();
          parent->pair.back().type = ct_pair;
          parent = &parent->pair.back();
          }
        auto expr = qq_expand(list_cell, depth);
        FunCall f2;
        Variable v2;
        v2.name = "list->vector";
        f2.fun.push_back(v2);
        f2.arguments.push_back(expr);
        return make_list(f2);
        }
      else
        {
        Quote q;
        q.arg.type = ct_pair;
        q.arg.pair.push_back(c);
        q.arg.pair.push_back(nil_sym);
        return q;
        }


      return Nop();
      }

    Expression qq_expand(const cell& c, int depth)
      {
      if (is_simple(c))
        {
        Quote q;
        q.arg = c;
        return q;
        }
      else if (tag_is_comma(c))
        {
        if (depth == 0)
          {
          auto expr = cell_to_expression(tag_data(c), false);
          return expr;
          }
        else
          return make_quote("unquote", c, false, depth);
        }
      else if (tag_is_comma_atsign(c))
        {
        if (depth == 0)
          throw_error(-1, -1, "", bad_syntax, ",@");
        else
          return make_quote("unquote-splicing", c, false, depth);
        }
      else if (tag_is_backquote(c))
        {
        return make_quote("quasiquote", c, false, depth);        
        }
      else if (is_pair(c))
        {
        if (!has_splicing(c.pair[0]))
          {
          auto expr1 = qq_expand(c.pair[0], depth);
          auto expr2 = qq_expand(c.pair[1], depth);
          PrimitiveCall prim;
          prim.primitive_name = "cons";
          prim.arguments.push_back(expr1);
          prim.arguments.push_back(expr2);
          return prim;
          }
        else if (is_nil(c.pair[1]))
          {
          return qq_expand_list(c.pair[0], depth);
          }
        else
          {
          auto expr1 = qq_expand_list(c.pair[0], depth);
          auto expr2 = qq_expand(c.pair[1], depth);
          Expression expr;
          FunCall f;
          Variable v;
          v.name = "append";
          f.fun.push_back(v);
          f.arguments.push_back(expr1);
          f.arguments.push_back(expr2);
          return f;
          }
        }
      else if (c.type == ct_vector)
        {
        cell list_cell;
        list_cell.type = ct_pair;
        cell* parent = &list_cell;
        for (const auto& cv : c.vec)
          {
          assert(parent->pair.size() == 0);
          parent->pair.push_back(cv);
          parent->pair.emplace_back();
          parent->pair.back().type = ct_pair;
          parent = &parent->pair.back();
          }
        auto expr = qq_expand(list_cell, depth);
        FunCall f2;
        Variable v2;
        v2.name = "list->vector";
        f2.fun.push_back(v2);
        f2.arguments.push_back(expr);
        return f2;
        }
      else
        {
        Quote q;
        q.arg = c;
        return q;
        }

      return Nop();
      }

    void _convert(Expression& e)
      {
      assert(std::holds_alternative<Quote>(e));
      Quote& q = std::get<Quote>(e);
      assert(q.type == Quote::qt_backquote);
      e = qq_expand(q.arg, 0);
      }

    virtual bool _previsit(Expression& e)
      {
      if (std::holds_alternative<Quote>(e))
        {
        Quote& q = std::get<Quote>(e);
        if (q.type == Quote::qt_backquote)
          _convert(e);
        }
      return true;
      }

    };
  }

void quasiquote_conversion(Program& prog)
  {
  quasiquote_conversion_visitor qcv;
  visitor<Program, quasiquote_conversion_visitor>::visit(prog, &qcv);

  prog.quasiquotes_converted = true;
  }

SKIWI_END
