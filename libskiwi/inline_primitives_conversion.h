#pragma once

#include "libskiwi_api.h"

#include "namespace.h"
#include "compiler.h"
#include "parse.h"
#include "repl_data.h"

SKIWI_BEGIN

SKIWI_SCHEME_API void inline_primitives(Program& prog, uint64_t& alpha_conversion_index, bool safe_primitives, bool standard_bindings);

SKIWI_END
