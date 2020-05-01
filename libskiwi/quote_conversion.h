#pragma once

#include "libskiwi_api.h"

#include "namespace.h"
#include "compiler.h"
#include "parse.h"
#include "repl_data.h"

SKIWI_BEGIN

SKIWI_SCHEME_API void quote_conversion(Program& prog, repl_data& data, environment_map& env, context& ctxt);

SKIWI_END
