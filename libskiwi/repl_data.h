#pragma once

#include <asm/namespace.h>
#include <stdint.h>
#include "environment.h"
#include "libskiwi_api.h"
#include <memory>
#include <string>
#include <map>
#include "alpha_conversion.h"
#include "reader.h"

SKIWI_BEGIN

struct repl_data
  {
  SKIWI_SCHEME_API repl_data();

  std::shared_ptr < environment<alpha_conversion_data>> alpha_conversion_env;
  uint64_t alpha_conversion_index;
  std::map<std::string, uint64_t> quote_to_index;
  uint64_t global_index;
  };

SKIWI_SCHEME_API repl_data make_deep_copy(const repl_data& rd);

SKIWI_END
