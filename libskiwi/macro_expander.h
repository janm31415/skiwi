#pragma once

#include "libskiwi_api.h"

#include <asm/namespace.h>
#include <stdint.h>

#include "macro_data.h"
#include "compiler.h"
#include "parse.h"

SKIWI_BEGIN

struct compiler_options;

SKIWI_SCHEME_API void expand_macros(Program& prog, environment_map& env, repl_data& rd, macro_data& md, context& ctxt, const primitive_map& pm, const compiler_options& ops);

SKIWI_END
