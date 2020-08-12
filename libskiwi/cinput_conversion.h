#pragma once

#include "namespace.h"

#include "compiler.h"
#include "parse.h"

SKIWI_BEGIN

struct cinput_data;

void cinput_conversion(cinput_data& cinput, Program& prog, environment_map& env, repl_data& rd, context& ctxt);

SKIWI_END