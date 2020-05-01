#pragma once

#include <string>

#include "namespace.h"
#include "libskiwi_api.h"
#include "environment.h"

#include <ostream>
#include <memory>


SKIWI_BEGIN

struct context;
struct environment_entry;
struct repl_data;

SKIWI_SCHEME_API void print_last_global_variable_used(std::ostream& out, std::shared_ptr<SKIWI::environment<SKIWI::environment_entry>> env, const repl_data& rd, const context* p_ctxt);

SKIWI_SCHEME_API void scheme_runtime(uint64_t rax, std::ostream& out, std::shared_ptr<SKIWI::environment<SKIWI::environment_entry>> env, const repl_data& rd, const context* p_ctxt);

SKIWI_END
