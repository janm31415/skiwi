#pragma once

#include "libskiwi_api.h"

#include <asm/namespace.h>
#include <stdint.h>
#include <memory>
#include <string>

#include "parse.h"
#include "environment.h"

SKIWI_BEGIN

struct alpha_conversion_data
  {
  SKIWI_SCHEME_API alpha_conversion_data() : forward_declaration(false) {}
  SKIWI_SCHEME_API alpha_conversion_data(const std::string& str) : forward_declaration(false), name(str) {}
  std::string name;
  bool forward_declaration;
  };

/*
Looks for variable names in Program and changes their names by their original name + _index, where index is always
incremented.

The environment env contains the mapping of a variable name to their alpha conversioned name.

The index is in and output via the variable alpha_conversion_index.
*/
SKIWI_SCHEME_API void alpha_conversion(Program& prog, uint64_t& alpha_conversion_index, std::shared_ptr<environment<alpha_conversion_data>>& env, bool modify_names = true);

std::string get_variable_name_before_alpha(const std::string& variable_name_after_alpha);

SKIWI_END
