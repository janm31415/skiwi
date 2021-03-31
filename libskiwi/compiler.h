#pragma once

#include <asm/asmcode.h>
#include <asm/assembler.h>
#include "namespace.h"
#include "libskiwi_api.h"
#include "context.h"
#include "compile_data.h"
#include "compiler_options.h"
#include "environment.h"
#include "parse.h"
#include "repl_data.h"
#include <memory>
#include <map>

SKIWI_BEGIN

struct primitive_entry
  {
  std::string label_name;
  uint64_t address;
  };

typedef std::map<std::string, primitive_entry> primitive_map;

typedef void(*fun_ptr)(ASM::asmcode&, const compiler_options&);

typedef std::map<std::string, fun_ptr> function_map;

struct external_function
  {
  enum argtype
    {
    T_BOOL,
    T_CHAR_POINTER,
    T_DOUBLE,
    T_INT64,        
    T_VOID,
    T_SCM
    };
  std::string name;
  uint64_t address;
  std::vector<argtype> arguments;
  argtype return_type;
  };

struct environment_entry
  {
  environment_entry() {}

  enum storage_type
    {
    st_register,
    st_local,
    st_global
    };

  storage_type st;
  uint64_t pos;
  liveness_range live_range;

  bool operator < (const environment_entry& other) const
    {
    if (st == other.st)
      return pos < other.pos;
    return st < other.st;
    }
  };

typedef std::shared_ptr<environment<environment_entry>> environment_map;

struct macro_data;

function_map generate_function_map();
function_map generate_simplified_function_map();

SKIWI_SCHEME_API void compile(environment_map& env, repl_data& rd, macro_data& md, context& ctxt, ASM::asmcode& code, Program& prog, const primitive_map& pm, const std::map<std::string, external_function>& external_functions, const compiler_options& options);


SKIWI_END
