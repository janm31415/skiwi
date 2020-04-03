#pragma once

#include "libskiwi_api.h"

#include <asm/namespace.h>

#include "parse.h"

SKIWI_BEGIN

struct compiler_options;
/*
Assumes that alpha conversion has been done and that all variable names are unique
*/
SKIWI_SCHEME_API void assignable_variable_conversion(Program& prog, const compiler_options& ops);

SKIWI_END
