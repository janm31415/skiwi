#pragma once

#include <stdint.h>
#include <memory>
#include <string>

#include "repl_data.h"
#include "compiler.h"
#include "compiler_options.h"
#include "macro_data.h"
#include "parse.h"
#include "environment.h"

SKIWI_BEGIN

struct cinput_data;

SKIWI_SCHEME_API void preprocess(environment_map& env, repl_data& data, macro_data& md, context& ctxt, cinput_data& cinput, Program& prog, const primitive_map& pm, const compiler_options& options);

SKIWI_END
