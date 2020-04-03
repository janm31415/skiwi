#pragma once

#include <asm/namespace.h>
#include "libskiwi_api.h"
#include "compiler.h"
#include "parse.h"
#include <string>
#include <map>
#include <vector>

SKIWI_BEGIN

struct macro_entry
  {
  std::string name;
  std::vector<std::string> variables;
  bool variable_arity;
  };

typedef std::map<std::string, macro_entry> macro_map;

struct macro_data
  {
  macro_map m;  

  std::vector<std::pair<void*, uint64_t>> compiled_macros;
  };

SKIWI_SCHEME_API macro_data create_macro_data();
SKIWI_SCHEME_API void destroy_macro_data(macro_data& md);

SKIWI_END
