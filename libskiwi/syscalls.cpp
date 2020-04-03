#include "syscalls.h"

#include <stdint.h>
#include <io.h>

SKIWI_BEGIN

void add_system_calls(std::map<std::string, external_function>& externals)
  {
  external_function ef;
  ef.name = "_write";
  ef.address = (uint64_t)&_write;
  ef.return_type = external_function::T_INT64;
  ef.arguments.push_back(external_function::T_INT64);
  ef.arguments.push_back(external_function::T_CHAR_POINTER);
  ef.arguments.push_back(external_function::T_INT64);
  externals[ef.name] = ef;
  }

SKIWI_END
