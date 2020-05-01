#pragma once

#include "libskiwi_api.h"

#include "namespace.h"
#include <stdint.h>

#include "parse.h"

SKIWI_BEGIN

struct compiler_options;

SKIWI_SCHEME_API void closure_conversion(Program& prog, const compiler_options& ops);

SKIWI_END
