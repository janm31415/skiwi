#pragma once

#include "libskiwi_api.h"

#include "namespace.h"
#include <stdint.h>
#include <memory>
#include <string>

#include "parse.h"
#include "compiler.h"
#include "repl_data.h"

SKIWI_BEGIN

/*
For each global define, allocates a variable in the environment.
*/
SKIWI_SCHEME_API void global_define_environment_allocation(Program& prog, environment_map& env, repl_data& data, context& ctxt);

SKIWI_END
