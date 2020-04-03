#include "tokenize.h"


#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <algorithm>
#include <sstream>
#include <set>

SKIWI_BEGIN

double to_double(const char* value)
  {
  return atof(value);
  }

int is_number(int* is_real, int* is_scientific, const char* value)
  {
  if (value[0] == '\0')
    return 0;
  int i = 0;
  if (value[0] == 'e' || value[0] == 'E')
    return 0;
  if (value[0] == '-' || value[0] == '+')
    {
    ++i;
    if (value[1] == '\0')
      return 0;
    }
  *is_real = 0;
  *is_scientific = 0;
  const char* s = value + i;
  while (*s != '\0')
    {
    if (isdigit((unsigned char)(*s)) == 0)
      {
      if ((*s == '.') && (*is_real == 0) && (*is_scientific == 0))
        *is_real = 1;
      else if ((*s == 'e' || *s == 'E') && (*is_scientific == 0))
        {
        *is_scientific = 1;        
        *is_real = 1;
        if (*(s+1) == '\0')
          return 0;
        if (*(s + 1) == '-' || *(s + 1) == '+')
          {          
          ++s;
          }
        if (*(s + 1) == '\0')
          return 0;
        }
      else
        return 0;
      }
    ++s;
    }
  return 1;
  }

namespace
  {
  std::string replace_escape_chars(std::string s)
    {
    auto pos = s.find_first_of('\\');
    if (pos == std::string::npos)
      return s;

    std::stringstream str;    
    while (pos != std::string::npos)
      {
      str << s.substr(0, pos);
      switch (s[pos + 1])
        {
        case 'a': str << '\a'; break;
        case 'b': str << '\b'; break;
        case 'n': str << '\n'; break;
        case 'r': str << '\r'; break;
        case 't': str << '\t'; break;
        default: str << s[pos + 1]; break;
        }      
      s = s.substr(pos+2);
      pos = s.find_first_of('\\');      
      }
    str << s;
    return str.str();
    }

  void _treat_buffer(std::string& buff, std::vector<token>& tokens, int line_nr, int column_nr, bool& is_a_symbol)
    {
    if (is_a_symbol)
      {
      is_a_symbol = false;
      if (buff.empty())
        tokens.emplace_back(token::T_BAD, buff, line_nr, column_nr);
      else
        {
        if (buff[0] == '#') // special case. some primitives (inlined ones) can start with two ##
          {
          tokens.emplace_back(token::T_ID, "#"+buff, line_nr, column_nr - (int)buff.length());
          }
        else
          tokens.emplace_back(token::T_SYMBOL, "#" + buff, line_nr, column_nr - (int)buff.length() - 1);
        }
      }
    else if (!buff.empty() && buff[0] != '\0')
      {
      int is_real;
      int is_scientific;
      if (is_number(&is_real, &is_scientific, buff.c_str()))
        {
        if (is_real)
          tokens.emplace_back(token::T_FLONUM, buff, line_nr, column_nr - (int)buff.length());
        else
          tokens.emplace_back(token::T_FIXNUM, buff, line_nr, column_nr - (int)buff.length());
        }
      else
        {
        tokens.emplace_back(token::T_ID, buff, line_nr, column_nr - (int)buff.length());
        }
      }
    buff.clear();
    }

  bool ignore_character(const char& ch)
    {
    return (ch == ' ' || ch == '\n' || ch == '\t');
    }
  }

std::vector<token> tokenize(const std::string& str)
  {
  std::vector<token> tokens;
  std::string buff;

  const char* s = str.c_str();
  const char* s_end = str.c_str() + str.length();

  bool is_a_symbol = false;
  bool is_a_character = false;

  int line_nr = 1;
  int column_nr = 1;

  while (s < s_end)
    {
    //if (*s == ' ' || *s == '\n')
    if (ignore_character(*s))
      {
      _treat_buffer(buff, tokens, line_nr, column_nr, is_a_symbol);
      is_a_character = false;
      //while (*s == ' ' || *s == '\n')
      while (ignore_character(*s))
        {
        if (*s == '\n')
          {
          ++line_nr;
          column_nr = 0;
          }
        ++s;
        ++column_nr;
        }
      }

    const char* s_copy = s;
    if (!is_a_character) // any special sign can appear as a character, e.g. #\(
      {
      switch (*s)
        {
        case '(':
        {
        if (is_a_symbol && buff.empty()) // #(
          {
          is_a_symbol = false;
          tokens.emplace_back(token::T_LEFT_ROUND_BRACKET, "#(", line_nr, column_nr);
          ++s;
          ++column_nr;
          break;
          }
        _treat_buffer(buff, tokens, line_nr, column_nr, is_a_symbol);
        tokens.emplace_back(token::T_LEFT_ROUND_BRACKET, "(", line_nr, column_nr);
        ++s;
        ++column_nr;
        break;
        }
        case ')':
        {
        _treat_buffer(buff, tokens, line_nr, column_nr, is_a_symbol);
        tokens.emplace_back(token::T_RIGHT_ROUND_BRACKET, ")", line_nr, column_nr);
        ++s;
        ++column_nr;
        break;
        }
        case '[':
        {
        _treat_buffer(buff, tokens, line_nr, column_nr, is_a_symbol);
        tokens.emplace_back(token::T_LEFT_SQUARE_BRACKET, "[", line_nr, column_nr);
        ++s;
        ++column_nr;
        break;
        }
        case ']':
        {
        _treat_buffer(buff, tokens, line_nr, column_nr, is_a_symbol);
        tokens.emplace_back(token::T_RIGHT_SQUARE_BRACKET, "]", line_nr, column_nr);
        ++s;
        ++column_nr;
        break;
        }
        case '#':
        {
        if (is_a_symbol)
          break;
        _treat_buffer(buff, tokens, line_nr, column_nr, is_a_symbol);
        const char* t = s;
        ++t;
        if (t && *t == ';') //treat as comment
          {
          while (*s && *s != '\n') // comment, so skip till end of the line
            ++s;
          ++s;
          ++line_nr;
          column_nr = 1;
          }
        else if (t && *t == '|') //treat as multiline comment
          {
          s = t;
          ++s; ++column_nr;
          bool end_of_comment_found = false;
          while (!end_of_comment_found)
            {
            while (*s && *s != '|')
              {
              if (*s == '\n')
                {
                ++line_nr;
                column_nr = 0;
                }
              ++s;
              ++column_nr;
              }
            if (!(*s) || (*(++s) == '#'))
              end_of_comment_found = true;
            }
          ++s; ++column_nr;
          }
        else
          {
          is_a_symbol = true;
          ++s;
          ++column_nr;
          }
        break;
        }
        case ';':
        {
        _treat_buffer(buff, tokens, line_nr, column_nr, is_a_symbol);
        while (*s && *s != '\n') // comment, so skip till end of the line
          ++s;
        ++s;
        ++line_nr;
        column_nr = 1;
        break;
        }
        case '\'':
        {
        _treat_buffer(buff, tokens, line_nr, column_nr, is_a_symbol);
        tokens.emplace_back(token::T_QUOTE, "'", line_nr, column_nr);
        ++s;
        ++column_nr;
        break;
        }
        case '`':
        {
        _treat_buffer(buff, tokens, line_nr, column_nr, is_a_symbol);
        tokens.emplace_back(token::T_BACKQUOTE, "`", line_nr, column_nr);
        ++s;
        ++column_nr;
        break;
        }
        case ',':
        {
        _treat_buffer(buff, tokens, line_nr, column_nr, is_a_symbol);
        const char* t = s;
        ++t;
        if (*t && *t == '@')
          {
          tokens.emplace_back(token::T_UNQUOTE_SPLICING, ",@", line_nr, column_nr);
          ++s;
          ++column_nr;
          }
        else
          {
          tokens.emplace_back(token::T_UNQUOTE, ",", line_nr, column_nr);
          }
        ++s;
        ++column_nr;
        break;
        }
        case '"':
        {
        _treat_buffer(buff, tokens, line_nr, column_nr, is_a_symbol);
        int temp_column_nr = column_nr;
        int temp_line_nr = line_nr;
        const char* t = s;
        ++t;
        ++column_nr;
        while (*t && *t != '"')
          {
          if (*t == '\n')
            {
            ++line_nr;
            column_nr = 0;
            }
          if (*t == '\\') // escape syntax
            {
            ++t;
            if (!*t)
              {
              tokens.emplace_back(token::T_BAD, std::string(s, t), temp_line_nr, temp_column_nr);
              break;
              }            
            }
          ++t;
          ++column_nr;
          }
        if (*t)
          {
          ++t;
          ++column_nr;
          }
        tokens.emplace_back(token::T_STRING, replace_escape_chars(std::string(s, t)), temp_line_nr, temp_column_nr);
        s = t;
        break;
        }
        }
      }

    if (s_copy == s)
      {
      buff += *s;
      ++s;
      ++column_nr;
      }

    is_a_character = (is_a_symbol && buff.size() == 1 && buff[0] == '\\');
    }

  _treat_buffer(buff, tokens, line_nr, column_nr, is_a_symbol);

  return tokens;
  }

SKIWI_END
