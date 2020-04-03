#include "format.h"
#include "tokenize.h"

#include <cassert>
#include <sstream>

SKIWI_BEGIN

format_options::format_options()
  {
  indent_offset = 2;
  max_width = 10;
  min_width = 5;
  }

namespace
  {
  std::vector<token>::const_iterator find_corresponding_parenthesis(std::vector<token>::const_iterator first, std::vector<token>::const_iterator last)
    {
    assert(first->type == token::T_LEFT_ROUND_BRACKET || first->type == token::T_LEFT_SQUARE_BRACKET);
    token::e_type left_t = first->type;
    token::e_type right_t = token::T_RIGHT_ROUND_BRACKET;
    if (left_t == token::T_LEFT_SQUARE_BRACKET)
      right_t = token::T_RIGHT_SQUARE_BRACKET;
    int cnt = 0;
    while (++first != last)
      {
      if (first->type == left_t)
        ++cnt;
      else if (first->type == right_t)
        {
        if (cnt)
          --cnt;
        else
          return first;
        }
      }
    return last;
    }

  bool insert_space(std::stringstream& ss, token::e_type current_type, token::e_type prev_type)
    {
    if (current_type != token::T_RIGHT_SQUARE_BRACKET && current_type != token::T_RIGHT_ROUND_BRACKET)
      {
      if (prev_type >= token::T_RIGHT_SQUARE_BRACKET || prev_type == token::T_RIGHT_ROUND_BRACKET)
        {
        ss << " ";
        return true;
        }
      }
    return false;
    }

  std::string dump(std::vector<token>::const_iterator first, std::vector<token>::const_iterator last, const format_options&)
    {
    std::stringstream ss;
    token::e_type prev_t = token::T_LEFT_ROUND_BRACKET;
    while (first != last)
      {
      const auto& t = *first;
      switch (t.type)
        {
        case token::T_LEFT_ROUND_BRACKET: insert_space(ss, t.type, prev_t); ss << "("; break;
        case token::T_LEFT_SQUARE_BRACKET: insert_space(ss, t.type, prev_t); ss << "["; break;
        case token::T_RIGHT_ROUND_BRACKET: insert_space(ss, t.type, prev_t); ss << ")"; break;
        case token::T_RIGHT_SQUARE_BRACKET: insert_space(ss, t.type, prev_t); ss << "]"; break;
        case token::T_BAD: insert_space(ss, t.type, prev_t); ss << t.value; break;
        case token::T_FIXNUM: insert_space(ss, t.type, prev_t); ss << t.value; break;
        case token::T_FLONUM: insert_space(ss, t.type, prev_t); ss << t.value; break;
        case token::T_STRING: insert_space(ss, t.type, prev_t); ss << t.value; break;
        case token::T_SYMBOL: insert_space(ss, t.type, prev_t); ss << t.value; break;
        case token::T_ID: insert_space(ss, t.type, prev_t); ss << t.value; break;
        }
      prev_t = t.type;
      ++first;
      }
    return ss.str();
    }

  std::string format_expression(std::vector<token>::const_iterator first, std::vector<token>::const_iterator last, std::vector<size_t>& depth_stack, const format_options& ops)
    {
    assert(ops.max_width > ops.min_width);
    size_t popped_depth = depth_stack.back();
    std::stringstream ss;
    for (int i = 0; i < depth_stack.back(); ++i)
      ss << " ";
    auto dmp = dump(first, last, ops);
    if ((dmp.length() + depth_stack.back()) <= ops.max_width)
      {
      ss << dmp;
      depth_stack.pop_back();
      }
    else
      {
      size_t length = 1;
      token::e_type prev_t = token::T_LEFT_ROUND_BRACKET;
      ss << first->value;
      ++first;
      while (first != last)
        {
        const auto& t = *first;
        if (insert_space(ss, t.type, prev_t))
          length += 1;

        if (t.type == token::T_LEFT_ROUND_BRACKET || t.type == token::T_LEFT_SQUARE_BRACKET)
          {
          size_t bracket_pos = depth_stack.back() + length;

          bool new_line = (bracket_pos > ops.max_width && prev_t != token::T_LEFT_ROUND_BRACKET && prev_t != token::T_LEFT_SQUARE_BRACKET);

          if (new_line && (length < ops.min_width))
            new_line = false;

          if (!new_line && (prev_t == token::T_RIGHT_ROUND_BRACKET || prev_t == token::T_RIGHT_SQUARE_BRACKET))
            {
            auto next_par = find_corresponding_parenthesis(first, last);
            size_t le = std::distance(first, next_par);
            if (bracket_pos + le > ops.max_width)
              new_line = true;
            }
          if (new_line)
            {
            if (prev_t == token::T_QUOTE)
              {              
              ss.seekp(-1, ss.cur);              
              }
            ss << "\n";
            auto next_par = find_corresponding_parenthesis(first, last);
            if (prev_t == token::T_QUOTE)
              --first;
            if (next_par == last)
              {
              ss << dump(first, last, ops);
              break;
              }
            else
              {
              if (popped_depth == depth_stack.back())
                {
                popped_depth += ops.indent_offset;
                depth_stack.push_back(popped_depth);
                }
              else
                depth_stack.push_back(popped_depth);
              auto s = format_expression(first, next_par + 1, depth_stack, ops);
              ss << s;
              length = s.length();
              first = next_par + 1;
              continue;
              }
            }
          else
            {
            popped_depth = bracket_pos;
            depth_stack.push_back(bracket_pos);
            length = 0;
            }
          }
        else if (t.type == token::T_RIGHT_ROUND_BRACKET || t.type == token::T_RIGHT_SQUARE_BRACKET)
          {
          size_t l = depth_stack.back();
          popped_depth = l;
          depth_stack.pop_back();
          l -= depth_stack.empty() ? 0 : depth_stack.back();          
          length += l;
          }
        else if (prev_t == token::T_RIGHT_ROUND_BRACKET || prev_t == token::T_RIGHT_SQUARE_BRACKET)
          {
          size_t bracket_pos = depth_stack.back() + length;

          bool new_line = (bracket_pos > ops.max_width);

          if (new_line && (length < ops.min_width))
            new_line = false;

          if (new_line)
            {
            ss << "\n";
            for (int i = 0; i < popped_depth; ++i)
              ss << " ";
            length = popped_depth - depth_stack.back();
            }
          }
        ss << t.value;
        length += t.value.length();
        prev_t = t.type;
        ++first;
        }
      }
    return ss.str();
    }

  }

std::string format(const std::string& in, const format_options& ops)
  {
  std::stringstream ss;
  std::vector<token> tokens = tokenize(in);

  // simple valid input checks
  if (tokens.size() < 2)
    return in;
  if (tokens.front().type != token::T_LEFT_ROUND_BRACKET)
    return in;
  if (tokens.back().type != token::T_RIGHT_ROUND_BRACKET)
    return in;

  auto next = tokens.cbegin();

  while (next != tokens.end())
    {
    next = find_corresponding_parenthesis(tokens.begin(), tokens.end());
    if (next == tokens.end())
      {
      ss << dump(tokens.begin(), tokens.end(), ops);
      tokens.clear();
      next = tokens.end();
      }
    else
      {
      auto first = tokens.begin();
      auto last = next + 1;
      std::vector<size_t> depth_stack;
      depth_stack.push_back(0);
      ss << format_expression(first, last, depth_stack, ops);
      tokens.erase(first, last);
      next = tokens.end();
      }    
    if (!tokens.empty() && tokens.front().type == token::T_LEFT_ROUND_BRACKET)
      {
      next = tokens.cbegin();
      }
    }
  if (!tokens.empty())
    ss << dump(tokens.begin(), tokens.end(), ops);
  return ss.str();
  }

SKIWI_END
