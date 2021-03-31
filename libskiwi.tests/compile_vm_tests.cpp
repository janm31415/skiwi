#include "compile_tests.h"
#include "test_assert.h"

#include "asm/vm.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <cassert>
#include <map>
#include <stdint.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <fcntl.h>

#include "libskiwi/alpha_conversion.h"
#include "libskiwi/cinput_data.h"
#include "libskiwi/compiler_options.h"
#include "libskiwi/compile_data.h"
#include "libskiwi/compiler.h"
#include "libskiwi/context.h"
#include "libskiwi/define_conversion.h"
#include "libskiwi/dump.h"
#include "libskiwi/format.h"
#include "libskiwi/globals.h"
#include "libskiwi/load_lib.h"
#include "libskiwi/macro_data.h"
#include "libskiwi/parse.h"
#include "libskiwi/preprocess.h"
#include "libskiwi/primitives.h"
#include "libskiwi/primitives_lib.h"
#include "libskiwi/repl_data.h"
#include "libskiwi/runtime.h"
#include "libskiwi/simplify_to_core.h"
#include "libskiwi/syscalls.h"
#include "libskiwi/tokenize.h"
#include "libskiwi/types.h"
#include "libskiwi/debug_find.h"

#include "libskiwi/libskiwi.h"

#include <chrono>

SKIWI_BEGIN

using namespace ASM;

namespace 
  {

  struct compile_fixture
    {
    compiler_options ops;
    context ctxt;
    repl_data rd;
    macro_data md;
    std::shared_ptr<environment<environment_entry>> env;
    bool stream_out;
    std::vector<std::pair<void*, uint64_t>> compiled_bytecode;
    primitive_map pm;
    std::map<std::string, external_function> externals;
    registers reg;

    compile_fixture()
      {      
      stream_out = false;
      ctxt = create_context(1024 * 1024, 1024, 1024, 1024);
      env = std::make_shared<environment<environment_entry>>(nullptr);    

      asmcode code;
      try
        {
        compile_simplified_primitives_library(pm, rd, env, ctxt, code, ops);
        uint64_t size;
        first_pass_data d;
        uint8_t* f = (uint8_t*)vm_bytecode(size, d, code);
        reg.rcx = (uint64_t)(&ctxt);
        run_bytecode(f, size, reg);       
        compiled_bytecode.emplace_back(f, size);
        assign_primitive_addresses(pm, d, (uint64_t)f);
        }
      catch (std::logic_error e)
        {
        std::cout << e.what() << " while compiling primitives library\n\n";
        }
      }

    virtual ~compile_fixture()
      {
      TEST_ASSERT(ctxt.stack == ctxt.stack_top);
      for (auto& f : compiled_bytecode)
        free_bytecode((void*)f.first, f.second);
      destroy_context(ctxt);
      }

    asmcode get_asmcode(const std::string& script)
      {
      asmcode code;
      auto tokens = tokenize(script);
      std::reverse(tokens.begin(), tokens.end());
      Program prog;
      try
        {
        prog = make_program(tokens);
        }
      catch (std::logic_error e)
        {
        code.clear();
        throw e;
        }
      try
        {
        compile(env, rd, md, ctxt, code, prog, pm, externals, ops);
        }
      catch (std::logic_error e)
        {
        code.clear();
        throw e;
        }
      return code;
      }

    void to_file(const std::string& filename, const std::string& script)
      {
      std::ofstream my_file(filename.c_str());
      dump(my_file, script);
      my_file.close();
      }

    void dump(std::ostream& out, const std::string& script, bool dump_primitives_lib = false)
      {
      // has side effects. todo: make deep copies of env and rd and use those
      if (dump_primitives_lib)
        {
        asmcode code2;
        compile_primitives_library(pm, rd, env, ctxt, code2, ops);
        code2.stream(out);
        }
      asmcode code;
      try
        {
        code = get_asmcode(script);
        }
      catch (std::logic_error e)
        {
        out << e.what();
        return;
        }

      code.stream(out);
      }

    std::string run(const std::string& script, bool no_stack_info = true, int precision = 6)
      {
      asmcode code;
      try
        {
        code = get_asmcode(script);        
        }
      catch (std::logic_error e)
        {
        return e.what();
        }
      if (stream_out)
        code.stream(std::cout);
      uint64_t size;
      first_pass_data d;
      uint8_t* f = (uint8_t*)vm_bytecode(size, d, code);
      std::stringstream str;
      str << std::setprecision(precision);
      reg.rcx = (uint64_t)(&ctxt);
      try {
        run_bytecode(f, size, reg);
        }
      catch (std::logic_error e)
        {
        std::cout << e.what() << "\n";
        }
      uint64_t res = reg.rax;
      if (no_stack_info)
        scheme_runtime(res, str, env, rd, nullptr);
      else
        scheme_runtime(res, str, env, rd, &ctxt);
      compiled_bytecode.emplace_back(f, size);     
      return str.str();
      }    

    };

  struct fixnums : public compile_fixture
    {
    void test()
      {
      ops.garbage_collection = false;
      TEST_EQ("45", run("(45)"));
      TEST_EQ("-1", run("(-1)"));
      TEST_EQ("100", run("100"));
      TEST_EQ("4611686018427387903", run("4611686018427387903"));
      TEST_EQ("-4611686018427387904", run("-4611686018427387904"));
      TEST_EQ("-1", run("18446744073709551615"));
      TEST_EQ("-1", run("-1"));
      TEST_EQ("-123", run("-123"));
      TEST_EQ("-1", run("(18446744073709551615)"));
      TEST_EQ("-1", run("(-1)"));
      TEST_EQ("-123", run("(-123)"));
      }
    };

  }

SKIWI_END

void run_all_compile_vm_tests()
  {
  using namespace SKIWI;
  fixnums().test();
  }