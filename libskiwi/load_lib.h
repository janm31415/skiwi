#pragma once

#include "namespace.h"

#include <map>
#include <string>
#include <vector>

#include "compiler_options.h"
#include "compiler.h"
#include "libskiwi_api.h"

SKIWI_BEGIN

SKIWI_SCHEME_API bool load_lib(const std::string& libname, environment_map& env, repl_data& rd, macro_data& md, context& ctxt, ASM::asmcode& code, const primitive_map& pm, const compiler_options& options);

SKIWI_SCHEME_API bool load_apply(environment_map& env, repl_data& rd, macro_data& md, context& ctxt, ASM::asmcode& code, const primitive_map& pm, const compiler_options& options);
SKIWI_SCHEME_API bool load_modules(environment_map& env, repl_data& rd, macro_data& md, context& ctxt, ASM::asmcode& code, const primitive_map& pm, const compiler_options& options);
SKIWI_SCHEME_API bool load_string_to_symbol(environment_map& env, repl_data& rd, macro_data& md, context& ctxt, ASM::asmcode& code, const primitive_map& pm, const compiler_options& options);
SKIWI_SCHEME_API bool load_r5rs(environment_map& env, repl_data& rd, macro_data& md, context& ctxt, ASM::asmcode& code, const primitive_map& pm, const compiler_options& options);
SKIWI_SCHEME_API bool load_callcc(environment_map& env, repl_data& rd, macro_data& md, context& ctxt, ASM::asmcode& code, const primitive_map& pm, const compiler_options& options);


SKIWI_END
