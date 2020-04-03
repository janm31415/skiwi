#pragma once

#include <asm/namespace.h>
#include "libskiwi_api.h"

#include <string>

SKIWI_BEGIN

struct format_options
  {
  SKIWI_SCHEME_API format_options();

  int indent_offset;
  int max_width;
  int min_width;
  };

SKIWI_SCHEME_API std::string format(const std::string& in, const format_options& ops);

SKIWI_END
