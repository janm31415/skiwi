#pragma once

#include "libskiwi_api.h"

#include <asm/namespace.h>
#include "parse.h"

SKIWI_BEGIN

SKIWI_SCHEME_API void single_begin_conversion(Program& prog);

void remove_nested_begin_expressions(Program& p);
void remove_nested_begin_expressions(Lambda& lam);
void remove_nested_begin_expressions(Let& let);

SKIWI_END
