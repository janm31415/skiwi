#pragma once

#include <asm/namespace.h>
#include "libskiwi_api.h"
#include <string>

SKIWI_BEGIN

enum error_type
  {
  bad_syntax,
  expected_keyword,
  invalid_argument,
  invalid_number_of_arguments,
  invalid_variable_name,
  no_tokens,
  not_implemented,
  variable_unknown,
  primitive_unknown,
  foreign_call_unknown,
  define_invalid_place,
  too_many_locals,
  too_many_globals,
  division_by_zero,
  macro_invalid_pattern,
  unsupported_number_syntax
  };

void throw_error(int line_nr, int column_nr, const std::string& filename, error_type t, std::string extra = std::string(""));
void throw_error(error_type t);

SKIWI_END
