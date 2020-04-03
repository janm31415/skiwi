#pragma once

#include "libskiwi_api.h"

#include <asm/namespace.h>
#include "parse.h"
#include "repl_data.h"

SKIWI_BEGIN

SKIWI_SCHEME_API void collect_quotes(Program& prog, repl_data& data);

SKIWI_END
