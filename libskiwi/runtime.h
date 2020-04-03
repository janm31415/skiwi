#pragma once

#include <string>

#include <asm/namespace.h>
#include "libskiwi_api.h"
#include "environment.h"

#include <ostream>
#include <memory>


SKIWI_BEGIN

struct context;
struct environment_entry;

SKIWI_SCHEME_API void scheme_runtime(uint64_t rax, std::ostream& out, std::shared_ptr<SKIWI::environment<SKIWI::environment_entry>> env, const context* p_ctxt);

SKIWI_END
