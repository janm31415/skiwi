#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <ostream>
#include "namespace.h"
#include "libskiwi_api.h"

SKIWI_BEGIN

double to_double(const char* value);
int is_number(int* is_real, const char* value);

struct token
  {
  enum e_type
    {
    T_BAD,    
    T_QUOTE, // (')
    T_BACKQUOTE, // (`)
    T_UNQUOTE, // comma (,)
    T_UNQUOTE_SPLICING, // comma at (,@)
    T_LEFT_ROUND_BRACKET,
    T_RIGHT_ROUND_BRACKET,    
    T_LEFT_SQUARE_BRACKET,
    T_RIGHT_SQUARE_BRACKET,
    T_FIXNUM,
    T_FLONUM,    
    T_STRING,
    T_SYMBOL,
    T_ID
    };

  e_type type;
  std::string value;
  int line_nr;
  int column_nr;

  token(e_type i_type, const std::string& v, int i_line_nr, int i_column_nr) : type(i_type), value(v), line_nr(i_line_nr), column_nr(i_column_nr) {}
  };

SKIWI_SCHEME_API std::vector<token> tokenize(const std::string& str);

SKIWI_END
