#pragma once

#include "libskiwi_api.h"

#include <asm/namespace.h>
#include "parse.h"

SKIWI_BEGIN

SKIWI_SCHEME_API void constant_propagation(Program& prog);

SKIWI_END
