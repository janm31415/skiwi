#include "reader.h"
#include "tokenize.h"

#include <sstream>
#include <vector>

SKIWI_BEGIN

cell::cell()
  {

  }

cell::cell(cell_type t) : type(t)
  {

  }

cell::cell(cell_type t, const std::string& val) : type(t), value(val)
  {

  }


bool operator == (const cell& left, const cell& right)
  {
  if (left.type != right.type)
    return false;
  if (left.type == ct_symbol || left.type == ct_string || left.type == ct_fixnum || left.type == ct_flonum)
    return (left.value == right.value ? true : false);
  if (left.type == ct_pair)
    {
    if (left.pair.size() != right.pair.size())
      return false;
    if (left.pair.empty() && right.pair.empty())
      return true;
    if (left.pair[0] != right.pair[0])
      return false;
    if (left.pair[1] != right.pair[1])
      return false;
    return true;
    }
  if (left.type == ct_vector)
    {
    if (left.vec.size() != right.vec.size())
      return false;
    for (size_t j = 0; j < left.vec.size(); ++j)
      if (left.vec[j] != right.vec[j])
        return false;
    return true;
    }
  return false;
  }

bool operator != (const cell& left, const cell& right)
  {
  return !(left == right);
  }

std::ostream& operator << (std::ostream& os, const cell& c)
  {
  switch (c.type)
    {
    case ct_fixnum:
    case ct_flonum:
    case ct_string:
    case ct_symbol: os << c.value; break;
    case ct_pair:
    {
    if (c.pair.empty())
      {
      os << "()";
      break;
      }
    os << "(" << c.pair[0];
    cell p2 = c.pair[1];
    if (p2.type == ct_pair)
      {
      while (p2.type == ct_pair && (p2 != nil_sym) && (p2.pair[1].type == ct_pair))
        {
        os << " " << p2.pair[0];
        cell tmp = p2.pair[1];
        p2 = tmp;
        }
      if (p2 != nil_sym)
        {
        os << " " << p2.pair[0] << " . " << p2.pair[1];
        }
      os << ")";
      }
    else
      {
      os << " . " << c.pair[1] << ")";
      }
    break;
    }
    case ct_vector:
    {
    os << "#(";
    for (size_t j = 0; j < c.vec.size(); ++j)
      {
      os << c.vec[j];
      if (j < c.vec.size() - 1)
        os << " ";
      }
    os << ")";
    break;
    }
    }
  return os;
  }

namespace
  {
  bool isdig(char c) { return isdigit(static_cast<unsigned char>(c)) != 0; }

  cell atom(const token& toke)
    {
    switch (toke.type)
      {
      case token::T_FIXNUM: return cell(ct_fixnum, toke.value);
      case token::T_FLONUM: return cell(ct_flonum, toke.value);
      case token::T_STRING: return cell(ct_string, toke.value);
      default: return cell(ct_symbol, toke.value);
      }
    }

  cell get_pair(std::vector<cell>::iterator first, std::vector<cell>::iterator last)
    {
    bool recurse = true;
    int iter = 0;
    cell global_out;
    cell* last_cell = nullptr;
    while (recurse)
      {
      recurse = false;
      cell out;
      size_t sz = std::distance(first, last);
      if (sz == 0)
        out = nil_sym;
      else if (sz == 1)
        {
        cell c(ct_pair);
        if (first->value == ".")
          c.pair.push_back(nil_sym);
        else
          c.pair.push_back(*first);
        c.pair.push_back(nil_sym);
        out = c;
        }
      else
        {
        cell c(ct_pair);
        if (first->value == ".")
          c.pair.push_back(nil_sym);
        else
          c.pair.push_back(*first);
        ++first;
        auto next_first = first;
        if (first->value == ".")
          {
          auto sz2 = std::distance(first + 1, last);
          if (sz2 == 0)
            c.pair.push_back(nil_sym);
          else if (sz2 == 1)
            c.pair.push_back(*(first + 1));
          else
            {
            recurse = true;
            next_first = first + 1;
            }
          }
        else
          recurse = true;
        first = next_first;
        out = c;
        }
      if (iter)
        {
        last_cell->pair.push_back(out);
        last_cell = &last_cell->pair.back();
        }
      else
        {
        global_out = out;
        last_cell = &global_out;
        }
      ++iter;
      }
    return global_out;
    }

  /*
  this is the recursive version of the upper method, which is iterative (and thus better because avoids stack overflow)
  cell get_pair(std::vector<cell>::iterator first, std::vector<cell>::iterator last)
    {
    if (first == last)
      return nil_sym;
    size_t sz = std::distance(first, last);
    if (sz == 1)
      {
      cell c(ct_pair);
      if (first->value == ".")
        c.pair.push_back(nil_sym);
      else
        c.pair.push_back(*first);
      c.pair.push_back(nil_sym);
      return c;
      }
    cell c(ct_pair);
    if (first->value == ".")
      c.pair.push_back(nil_sym);
    else
      c.pair.push_back(*first);
    ++first;
    if (first->value == ".")
      {
      auto sz2 = std::distance(first + 1, last);
      if (sz2 == 0)
        c.pair.push_back(nil_sym);
      else if (sz2 == 1)
        c.pair.push_back(*(first + 1));
      else
        c.pair.push_back(get_pair(first + 1, last));
      }
    else
      c.pair.push_back(get_pair(first, last));
    return c;
    }
    */

  cell get_vector(std::vector<cell>::iterator first, std::vector<cell>::iterator last)
    {
    cell c(ct_vector);
    for (; first != last; ++first)
      c.vec.push_back(*first);
    return c;
    }

  }

cell read_from(std::vector<token>& tokens, bool quasiquote)
  {
  const token toke(tokens.back());
  tokens.pop_back();
  if (toke.type == token::T_LEFT_ROUND_BRACKET)
    {
    std::vector<cell> items;
    while (!tokens.empty() && tokens.back().type != token::T_RIGHT_ROUND_BRACKET)
      items.push_back(read_from(tokens, quasiquote));
    cell c;
    if (toke.value == "(")
      c = get_pair(items.begin(), items.end());
    else
      c = get_vector(items.begin(), items.end());
    if (!tokens.empty())
      tokens.pop_back();
    return c;
    }
  else
    {
    if (quasiquote && (toke.type == token::T_QUOTE || toke.type == token::T_BACKQUOTE || toke.type == token::T_UNQUOTE || toke.type == token::T_UNQUOTE_SPLICING))
      {
      cell c;
      c.type = ct_pair;
      cell p1;
      p1.type = ct_symbol;
      switch (toke.type)
        {
        case token::T_QUOTE: p1.value = "quote"; break;
        case token::T_BACKQUOTE: p1.value = "quasiquote"; break;
        case token::T_UNQUOTE: p1.value = "unquote"; break;
        case token::T_UNQUOTE_SPLICING: p1.value = "unquote-splicing"; break;
        }
      c.pair.push_back(p1);
      cell p2;
      p2.type = ct_pair;
      p2.pair.push_back(read_from(tokens, true));
      p2.pair.push_back(nil_sym);
      c.pair.push_back(p2);
      return c;
      }
    else
      {
      if (toke.type == token::T_QUOTE || toke.type == token::T_BACKQUOTE || toke.type == token::T_UNQUOTE || toke.type == token::T_UNQUOTE_SPLICING)
        {
        cell c = read_from(tokens, quasiquote);
        /*
        cell out;
        out.type = ct_symbol;
        std::stringstream str;
        str << toke.value << c;
        out.value = str.str();
        return out;
        */
        
        cell c2;
        c2.type = ct_pair;
        c2.pair.push_back(c);
        c2.pair.push_back(nil_sym);
        cell out;
        out.type = ct_pair;
        out.pair.emplace_back();
        out.pair.push_back(c2);
        out.pair[0].type = ct_symbol;
        switch (toke.type)
          {
          case token::T_QUOTE: out.pair[0].value = "quote"; break;
          case token::T_BACKQUOTE: out.pair[0].value = "quasiquote"; break;
          case token::T_UNQUOTE: out.pair[0].value = "unquote"; break;
          case token::T_UNQUOTE_SPLICING: out.pair[0].value = "unquote-splicing"; break;
          }
          
        return out;
        }
      else
        return atom(toke);
      }
    }
  }

cell read(const std::string& s)
  {
  std::vector<token> tokens = tokenize(s);
  std::reverse(tokens.begin(), tokens.end());
  return read_from(tokens);
  }

SKIWI_END
