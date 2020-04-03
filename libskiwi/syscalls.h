#pragma once

#include <asm/namespace.h>
#include "compiler.h"
#include "libskiwi_api.h"

SKIWI_BEGIN

SKIWI_SCHEME_API void add_system_calls(std::map<std::string, external_function>& externals);

SKIWI_END
