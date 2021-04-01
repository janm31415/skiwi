#pragma once

#include <asm/assembler.h>
#include "namespace.h"

#include <map>
#include <string>
#include <vector>

#include "compiler_options.h"
#include "compiler.h"
#include "context.h"
#include "libskiwi_api.h"

SKIWI_BEGIN

SKIWI_SCHEME_API void compile_primitives_library(primitive_map& pm, repl_data& rd, environment_map& env, context& ctxt, ASM::asmcode& code, const compiler_options& options);
SKIWI_SCHEME_API void assign_primitive_addresses(primitive_map& pm, const ASM::first_pass_data& d, uint64_t address_start);

SKIWI_END
