#pragma once

#include "libskiwi_api.h"

#include <asm/namespace.h>
#include <stdint.h>

#include "parse.h"

SKIWI_BEGIN

struct compiler_options;

enum linear_scan_algorithm
  {
  lsa_naive,
  lsa_detailed
  };

SKIWI_SCHEME_API void linear_scan(Program& prog, linear_scan_algorithm lsa, const compiler_options& ops);

SKIWI_END
