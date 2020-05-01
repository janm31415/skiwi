#pragma once

#include "libskiwi_api.h"

#include "namespace.h"
#include <string>

#include <ostream>
#include "parse.h"

SKIWI_BEGIN

SKIWI_SCHEME_API void dump(std::ostream& out, Program& prog, bool use_square_brackets = true);

void dump(std::ostream& out, Expression& expr, bool use_square_brackets = true);

SKIWI_END
