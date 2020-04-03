#include "compile_error.h"
#include <sstream>

SKIWI_BEGIN

void throw_error(int line_nr, int column_nr, const std::string& filename, error_type t, std::string extra)
  {
  std::stringstream str;
  if (!filename.empty())
    str << "error in " << filename << ":";
  else
    str << "error:";
  if (line_nr >= 0)
    str << line_nr << ":";
  if (column_nr >= 0)
    str << column_nr << ":";
  str << " ";
  switch (t)
    {
    case bad_syntax:
      str << "Bad syntax";
      break;
    case no_tokens:
      str << "I expect more tokens at the end";
      break;
    case invalid_argument:
      str << "Invalid argument";
      break;
    case invalid_number_of_arguments:
      str << "Invalid number of arguments";
      break;
    case invalid_variable_name:
      str << "Invalid variable name";
      break;
    case expected_keyword:
      str << "Keyword expected";
      break;
    case not_implemented:
      str << "Not implemented";
      break;
    case variable_unknown:
      str << "Variable unknown";
      break;
    case primitive_unknown:
      str << "Primitive unknown";
      break;
    case foreign_call_unknown:
        str << "Foreign call unknown";
      break;
    case define_invalid_place:
      str << "Define is only allowed at the top of the program or the top of a body";
      break;
    case too_many_locals:
      str << "Too many locals";
      break;
    case too_many_globals:
      str << "Too many globals";
      break;
    case division_by_zero:
      str << "Division by zero";
      break;
    case macro_invalid_pattern:
      str << "No matching case for calling pattern in macro";
      break;
    case unsupported_number_syntax:
      str << "Unsupported number syntax";
      break;
    }
  if (!extra.empty())
    str << ": " << extra;
  throw std::logic_error(str.str());
  }

void throw_error(error_type t)
  {
  throw_error(-1, -1, "", t);
  }

SKIWI_END
