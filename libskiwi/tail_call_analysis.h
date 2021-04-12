#pragma once

#include "libskiwi_api.h"

#include "namespace.h"
#include "parse.h"

SKIWI_BEGIN

SKIWI_SCHEME_API void tail_call_analysis(Expression& e);
SKIWI_SCHEME_API void tail_call_analysis(Program& prog);

SKIWI_END
