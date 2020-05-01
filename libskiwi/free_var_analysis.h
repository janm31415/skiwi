#pragma once

#include "libskiwi_api.h"

#include "namespace.h"
#include <stdint.h>

#include "compiler.h"
#include "parse.h"

SKIWI_BEGIN

SKIWI_SCHEME_API void free_variable_analysis(Program& prog, environment_map& env);
void free_variable_analysis(Lambda& lam, environment_map& env);

SKIWI_END
