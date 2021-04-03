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
    std::map<std::string, SKIWI::external_function> externals;
    std::vector<ASM::external_function> externals_for_vm;
    registers reg;

    compile_fixture()
      {
      stream_out = false;
      ctxt = create_context(1024 * 1024, 1024, 1024, 1024);
      env = std::make_shared<environment<environment_entry>>(nullptr);

      asmcode code;
      try
        {
        compile_primitives_library(pm, rd, env, ctxt, code, ops);
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
      destroy_macro_data(md);
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
        run_bytecode(f, size, reg, externals_for_vm);
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

    void build_callcc()
      {
      asmcode code;
      try
        {
        if (load_callcc(env, rd, md, ctxt, code, pm, ops))
          {
          first_pass_data d;
          uint64_t size;
          uint8_t* f = (uint8_t*)vm_bytecode(size, d, code);
          reg.rcx = (uint64_t)(&ctxt);
          try {
            run_bytecode(f, size, reg);
            }
          catch (std::logic_error e)
            {
            std::cout << e.what() << "\n";
            }
          compiled_bytecode.emplace_back(f, size);
          }
        else
          std::cout << "Cannot load callcc\n";
        }
      catch (std::logic_error e)
        {
        std::cout << e.what() << " while compiling callcc library\n\n";
        }
      }

    void build_apply()
      {
      asmcode code;
      try
        {
        if (load_apply(env, rd, md, ctxt, code, pm, ops))
          {
          first_pass_data d;
          uint64_t size;
          uint8_t* f = (uint8_t*)vm_bytecode(size, d, code);
          reg.rcx = (uint64_t)(&ctxt);
          try {
            run_bytecode(f, size, reg);
            }
          catch (std::logic_error e)
            {
            std::cout << e.what() << "\n";
            }
          compiled_bytecode.emplace_back(f, size);
          }
        else
          std::cout << "Cannot load apply\n";
        }
      catch (std::logic_error e)
        {
        std::cout << e.what() << " while compiling apply library\n\n";
        }
      }

    void build_r5rs()
      {
      asmcode code;
      try
        {
        if (load_r5rs(env, rd, md, ctxt, code, pm, ops))
          {
          first_pass_data d;
          uint64_t size;
          uint8_t* f = (uint8_t*)vm_bytecode(size, d, code);
          reg.rcx = (uint64_t)(&ctxt);
          try {
            run_bytecode(f, size, reg);
            }
          catch (std::logic_error e)
            {
            std::cout << e.what() << "\n";
            }
          compiled_bytecode.emplace_back(f, size);
          }
        else
          std::cout << "Cannot load r5rs library\n";
        }
      catch (std::logic_error e)
        {
        std::cout << e.what() << " while compiling r5rs library\n\n";
        }
      }

    void build_string_to_symbol()
      {
      asmcode code;
      try
        {
        if (load_string_to_symbol(env, rd, md, ctxt, code, pm, ops))
          {
          first_pass_data d;
          uint64_t size;
          uint8_t* f = (uint8_t*)vm_bytecode(size, d, code);
          reg.rcx = (uint64_t)(&ctxt);
          try {
            run_bytecode(f, size, reg);
            }
          catch (std::logic_error e)
            {
            std::cout << e.what() << "\n";
            }
          compiled_bytecode.emplace_back(f, size);
          }
        else
          std::cout << "Cannot load string-to-symbol library\n";
        }
      catch (std::logic_error e)
        {
        std::cout << e.what() << " while compiling string to symbol library\n\n";
        }
      }

    void build_libs()
      {
      build_string_to_symbol();
      build_apply();
      build_callcc();
      build_r5rs();
      }

    void init_macro_data()
      {
      md = create_macro_data();
      }

    void make_new_context(uint64_t heap_size, uint64_t global_stack, uint16_t local_stack, uint64_t scheme_stack)
      {
      for (auto& f : compiled_bytecode)
        free_bytecode((void*)f.first, f.second);
      compiled_bytecode.clear();
      destroy_context(ctxt);
      ctxt = create_context(heap_size, global_stack, local_stack, scheme_stack);
      env = std::make_shared<environment<environment_entry>>(nullptr);
      rd = repl_data();
      asmcode code;
      try
        {
        compile_primitives_library(pm, rd, env, ctxt, code, ops);
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

    ASM::external_function::argtype convert(SKIWI::external_function::argtype arg)
      {
      switch (arg)
        {
        case SKIWI::external_function::T_BOOL: return ASM::external_function::T_BOOL;
        case SKIWI::external_function::T_CHAR_POINTER: return ASM::external_function::T_CHAR_POINTER;
        case SKIWI::external_function::T_DOUBLE: return ASM::external_function::T_DOUBLE;
        case SKIWI::external_function::T_INT64: return ASM::external_function::T_INT64;
        case SKIWI::external_function::T_VOID: return ASM::external_function::T_VOID;
        case SKIWI::external_function::T_SCM: return ASM::external_function::T_INT64;
        default: return ASM::external_function::T_VOID;
        }
      }

    void convert_externals_to_vm()
      {
      externals_for_vm.clear();
      for (const auto& e : externals)
        {
        ASM::external_function ef;
        ef.name = e.second.name;
        ef.address = e.second.address;
        ef.return_type = convert(e.second.return_type);
        for (auto arg : e.second.arguments)
          {
          ef.arguments.push_back(convert(arg));
          }
        externals_for_vm.push_back(ef);
        }
      }
    };

  struct fixnums : public compile_fixture
    {
    void test()
      {
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

  struct bools : public compile_fixture {
    void test()
      {
      TEST_EQ("#t", run("#t"));
      TEST_EQ("#f", run("#f"));
      TEST_EQ("#t", run("(#t)"));
      TEST_EQ("#f", run("(#f)"));
      }
    };

  struct test_for_nil : public compile_fixture {
    void test()
      {
      TEST_EQ("()", run("()"));
      TEST_EQ("()", run("(())"));
      TEST_EQ("()", run("((()))"));
      TEST_EQ("()", run("(((())))"));
      }
    };

  struct chars : public compile_fixture {
    void test()
      {
      TEST_EQ(R"(#\*)", run(R"(#\*)"));
      TEST_EQ(R"(#\a)", run(R"(#\97)"));

      TEST_EQ(R"(#\*)", run(R"(#\*)"));
      TEST_EQ(R"(#\!)", run(R"(#\!)"));
      TEST_EQ(R"(#\@)", run(R"(#\@)"));
      TEST_EQ(R"(#\#)", run(R"(#\#)"));
      TEST_EQ(R"(#\$)", run(R"(#\$)"));
      TEST_EQ(R"(#\%)", run(R"(#\%)"));
      TEST_EQ(R"(#\^)", run(R"(#\^)"));
      TEST_EQ(R"(#\&)", run(R"(#\&)"));
      TEST_EQ(R"(#\()", run(R"(#\()"));
      TEST_EQ(R"(#\))", run(R"(#\))"));
      TEST_EQ(R"(#\")", run(R"(#\")"));
      TEST_EQ(R"(#\+)", run(R"(#\+)"));
      TEST_EQ(R"(#\-)", run(R"(#\-)"));
      TEST_EQ(R"(#\/)", run(R"(#\/)"));
      TEST_EQ(R"(#\')", run(R"(#\')"));
      TEST_EQ(R"(#\;)", run(R"(#\;)"));
      TEST_EQ(R"(#\:)", run(R"(#\:)"));
      TEST_EQ(R"(#\<)", run(R"(#\<)"));
      TEST_EQ(R"(#\>)", run(R"(#\>)"));
      TEST_EQ(R"(#\.)", run(R"(#\.)"));
      TEST_EQ(R"(#\=)", run(R"(#\=)"));
      TEST_EQ(R"(#\?)", run(R"(#\?)"));

      TEST_EQ(R"(#\a)", run(R"(#\a)"));
      TEST_EQ(R"(#\b)", run(R"(#\b)"));
      TEST_EQ(R"(#\c)", run(R"(#\c)"));
      TEST_EQ(R"(#\d)", run(R"(#\d)"));
      TEST_EQ(R"(#\e)", run(R"(#\e)"));
      TEST_EQ(R"(#\f)", run(R"(#\f)"));
      TEST_EQ(R"(#\g)", run(R"(#\g)"));
      TEST_EQ(R"(#\h)", run(R"(#\h)"));
      TEST_EQ(R"(#\i)", run(R"(#\i)"));
      TEST_EQ(R"(#\j)", run(R"(#\j)"));
      TEST_EQ(R"(#\k)", run(R"(#\k)"));
      TEST_EQ(R"(#\l)", run(R"(#\l)"));
      TEST_EQ(R"(#\m)", run(R"(#\m)"));
      TEST_EQ(R"(#\n)", run(R"(#\n)"));
      TEST_EQ(R"(#\o)", run(R"(#\o)"));
      TEST_EQ(R"(#\p)", run(R"(#\p)"));
      TEST_EQ(R"(#\q)", run(R"(#\q)"));
      TEST_EQ(R"(#\r)", run(R"(#\r)"));
      TEST_EQ(R"(#\s)", run(R"(#\s)"));
      TEST_EQ(R"(#\t)", run(R"(#\t)"));
      TEST_EQ(R"(#\u)", run(R"(#\u)"));
      TEST_EQ(R"(#\v)", run(R"(#\v)"));
      TEST_EQ(R"(#\w)", run(R"(#\w)"));
      TEST_EQ(R"(#\x)", run(R"(#\x)"));
      TEST_EQ(R"(#\y)", run(R"(#\y)"));
      TEST_EQ(R"(#\z)", run(R"(#\z)"));

      TEST_EQ(R"(#\A)", run(R"(#\A)"));
      TEST_EQ(R"(#\B)", run(R"(#\B)"));
      TEST_EQ(R"(#\C)", run(R"(#\C)"));
      TEST_EQ(R"(#\D)", run(R"(#\D)"));
      TEST_EQ(R"(#\E)", run(R"(#\E)"));
      TEST_EQ(R"(#\F)", run(R"(#\F)"));
      TEST_EQ(R"(#\G)", run(R"(#\G)"));
      TEST_EQ(R"(#\H)", run(R"(#\H)"));
      TEST_EQ(R"(#\I)", run(R"(#\I)"));
      TEST_EQ(R"(#\J)", run(R"(#\J)"));
      TEST_EQ(R"(#\K)", run(R"(#\K)"));
      TEST_EQ(R"(#\L)", run(R"(#\L)"));
      TEST_EQ(R"(#\M)", run(R"(#\M)"));
      TEST_EQ(R"(#\N)", run(R"(#\N)"));
      TEST_EQ(R"(#\O)", run(R"(#\O)"));
      TEST_EQ(R"(#\P)", run(R"(#\P)"));
      TEST_EQ(R"(#\Q)", run(R"(#\Q)"));
      TEST_EQ(R"(#\R)", run(R"(#\R)"));
      TEST_EQ(R"(#\S)", run(R"(#\S)"));
      TEST_EQ(R"(#\T)", run(R"(#\T)"));
      TEST_EQ(R"(#\U)", run(R"(#\U)"));
      TEST_EQ(R"(#\V)", run(R"(#\V)"));
      TEST_EQ(R"(#\W)", run(R"(#\W)"));
      TEST_EQ(R"(#\X)", run(R"(#\X)"));
      TEST_EQ(R"(#\Y)", run(R"(#\Y)"));
      TEST_EQ(R"(#\Z)", run(R"(#\Z)"));

      TEST_EQ(R"(#\000)", run(R"(#\000)"));
      TEST_EQ(R"(#\001)", run(R"(#\001)"));
      TEST_EQ(R"(#\002)", run(R"(#\002)"));
      TEST_EQ(R"(#\003)", run(R"(#\003)"));
      TEST_EQ(R"(#\004)", run(R"(#\004)"));
      TEST_EQ(R"(#\005)", run(R"(#\005)"));
      TEST_EQ(R"(#\006)", run(R"(#\006)"));
      TEST_EQ(R"(#\007)", run(R"(#\007)"));
      TEST_EQ(R"(#\008)", run(R"(#\008)"));
      TEST_EQ(R"(#\009)", run(R"(#\009)"));
      TEST_EQ(R"(#\010)", run(R"(#\010)"));
      TEST_EQ(R"(#\011)", run(R"(#\011)"));
      TEST_EQ(R"(#\012)", run(R"(#\012)"));
      TEST_EQ(R"(#\013)", run(R"(#\013)"));
      TEST_EQ(R"(#\014)", run(R"(#\014)"));
      TEST_EQ(R"(#\015)", run(R"(#\015)"));
      TEST_EQ(R"(#\016)", run(R"(#\016)"));
      TEST_EQ(R"(#\017)", run(R"(#\017)"));
      TEST_EQ(R"(#\018)", run(R"(#\018)"));
      TEST_EQ(R"(#\019)", run(R"(#\019)"));

      TEST_EQ(R"(#\*)", run(R"((#\*))"));
      TEST_EQ(R"(#\a)", run(R"((#\97))"));

      TEST_EQ(R"(#\*)", run(R"((#\*))"));
      TEST_EQ(R"(#\!)", run(R"((#\!))"));
      TEST_EQ(R"(#\@)", run(R"((#\@))"));
      TEST_EQ(R"(#\#)", run(R"((#\#))"));
      TEST_EQ(R"(#\$)", run(R"((#\$))"));
      TEST_EQ(R"(#\%)", run(R"((#\%))"));
      TEST_EQ(R"(#\^)", run(R"((#\^))"));
      TEST_EQ(R"(#\&)", run(R"((#\&))"));
      TEST_EQ(R"(#\()", run(R"((#\())"));
      TEST_EQ(R"(#\))", run(R"((#\)))"));
      TEST_EQ(R"(#\")", run(R"((#\"))"));
      TEST_EQ(R"(#\+)", run(R"((#\+))"));
      TEST_EQ(R"(#\-)", run(R"((#\-))"));
      TEST_EQ(R"(#\/)", run(R"((#\/))"));
      TEST_EQ(R"(#\')", run(R"((#\'))"));
      TEST_EQ(R"(#\;)", run(R"((#\;))"));
      TEST_EQ(R"(#\:)", run(R"((#\:))"));
      TEST_EQ(R"(#\<)", run(R"((#\<))"));
      TEST_EQ(R"(#\>)", run(R"((#\>))"));
      TEST_EQ(R"(#\.)", run(R"((#\.))"));
      TEST_EQ(R"(#\=)", run(R"((#\=))"));
      TEST_EQ(R"(#\?)", run(R"((#\?))"));

      TEST_EQ(R"(#\a)", run(R"((#\a))"));
      TEST_EQ(R"(#\b)", run(R"((#\b))"));
      TEST_EQ(R"(#\c)", run(R"((#\c))"));
      TEST_EQ(R"(#\d)", run(R"((#\d))"));
      TEST_EQ(R"(#\e)", run(R"((#\e))"));
      TEST_EQ(R"(#\f)", run(R"((#\f))"));
      TEST_EQ(R"(#\g)", run(R"((#\g))"));
      TEST_EQ(R"(#\h)", run(R"((#\h))"));
      TEST_EQ(R"(#\i)", run(R"((#\i))"));
      TEST_EQ(R"(#\j)", run(R"((#\j))"));
      TEST_EQ(R"(#\k)", run(R"((#\k))"));
      TEST_EQ(R"(#\l)", run(R"((#\l))"));
      TEST_EQ(R"(#\m)", run(R"((#\m))"));
      TEST_EQ(R"(#\n)", run(R"((#\n))"));
      TEST_EQ(R"(#\o)", run(R"((#\o))"));
      TEST_EQ(R"(#\p)", run(R"((#\p))"));
      TEST_EQ(R"(#\q)", run(R"((#\q))"));
      TEST_EQ(R"(#\r)", run(R"((#\r))"));
      TEST_EQ(R"(#\s)", run(R"((#\s))"));
      TEST_EQ(R"(#\t)", run(R"((#\t))"));
      TEST_EQ(R"(#\u)", run(R"((#\u))"));
      TEST_EQ(R"(#\v)", run(R"((#\v))"));
      TEST_EQ(R"(#\w)", run(R"((#\w))"));
      TEST_EQ(R"(#\x)", run(R"((#\x))"));
      TEST_EQ(R"(#\y)", run(R"((#\y))"));
      TEST_EQ(R"(#\z)", run(R"((#\z))"));

      TEST_EQ(R"(#\A)", run(R"((#\A))"));
      TEST_EQ(R"(#\B)", run(R"((#\B))"));
      TEST_EQ(R"(#\C)", run(R"((#\C))"));
      TEST_EQ(R"(#\D)", run(R"((#\D))"));
      TEST_EQ(R"(#\E)", run(R"((#\E))"));
      TEST_EQ(R"(#\F)", run(R"((#\F))"));
      TEST_EQ(R"(#\G)", run(R"((#\G))"));
      TEST_EQ(R"(#\H)", run(R"((#\H))"));
      TEST_EQ(R"(#\I)", run(R"((#\I))"));
      TEST_EQ(R"(#\J)", run(R"((#\J))"));
      TEST_EQ(R"(#\K)", run(R"((#\K))"));
      TEST_EQ(R"(#\L)", run(R"((#\L))"));
      TEST_EQ(R"(#\M)", run(R"((#\M))"));
      TEST_EQ(R"(#\N)", run(R"((#\N))"));
      TEST_EQ(R"(#\O)", run(R"((#\O))"));
      TEST_EQ(R"(#\P)", run(R"((#\P))"));
      TEST_EQ(R"(#\Q)", run(R"((#\Q))"));
      TEST_EQ(R"(#\R)", run(R"((#\R))"));
      TEST_EQ(R"(#\S)", run(R"((#\S))"));
      TEST_EQ(R"(#\T)", run(R"((#\T))"));
      TEST_EQ(R"(#\U)", run(R"((#\U))"));
      TEST_EQ(R"(#\V)", run(R"((#\V))"));
      TEST_EQ(R"(#\W)", run(R"((#\W))"));
      TEST_EQ(R"(#\X)", run(R"((#\X))"));
      TEST_EQ(R"(#\Y)", run(R"((#\Y))"));
      TEST_EQ(R"(#\Z)", run(R"((#\Z))"));

      TEST_EQ(R"(#\000)", run(R"((#\000))"));
      TEST_EQ(R"(#\001)", run(R"((#\001))"));
      TEST_EQ(R"(#\002)", run(R"((#\002))"));
      TEST_EQ(R"(#\003)", run(R"((#\003))"));
      TEST_EQ(R"(#\004)", run(R"((#\004))"));
      TEST_EQ(R"(#\005)", run(R"((#\005))"));
      TEST_EQ(R"(#\006)", run(R"((#\006))"));
      TEST_EQ(R"(#\007)", run(R"((#\007))"));
      TEST_EQ(R"(#\008)", run(R"((#\008))"));
      TEST_EQ(R"(#\009)", run(R"((#\009))"));
      TEST_EQ(R"(#\010)", run(R"((#\010))"));
      TEST_EQ(R"(#\011)", run(R"((#\011))"));
      TEST_EQ(R"(#\012)", run(R"((#\012))"));
      TEST_EQ(R"(#\013)", run(R"((#\013))"));
      TEST_EQ(R"(#\014)", run(R"((#\014))"));
      TEST_EQ(R"(#\015)", run(R"((#\015))"));
      TEST_EQ(R"(#\016)", run(R"((#\016))"));
      TEST_EQ(R"(#\017)", run(R"((#\017))"));
      TEST_EQ(R"(#\018)", run(R"((#\018))"));
      TEST_EQ(R"(#\019)", run(R"((#\019))"));

      TEST_EQ(R"(#\000)", run(R"(#\nul)"));
      TEST_EQ(R"(#\000)", run(R"(#\null)"));
      TEST_EQ(R"(#\008)", run(R"(#\backspace)"));
      TEST_EQ(R"(#\009)", run(R"(#\tab)"));
      TEST_EQ(R"(#\010)", run(R"(#\newline)"));
      TEST_EQ(R"(#\010)", run(R"(#\linefeed)"));
      TEST_EQ(R"(#\011)", run(R"(#\vtab)"));
      TEST_EQ(R"(#\012)", run(R"(#\page)"));
      TEST_EQ(R"(#\013)", run(R"(#\return)"));
      TEST_EQ(R"(#\ )", run(R"(#\space)"));
      TEST_EQ(R"(#\127)", run(R"(#\rubout)"));
      }
    };

  struct doubles : public compile_fixture {
    void test()
      {
      TEST_EQ("3.14", run("3.14"));
      TEST_EQ("3.14", run("(3.14)"));
      TEST_EQ("3.14", run("((3.14))"));
      TEST_EQ("3.14159", run("3.14159265359"));
      TEST_EQ("3.1415926535900001", run("(3.14159265359)", true, 17));
      }
    };

  struct add1 : public compile_fixture {
    void test()
      {
      TEST_EQ("0", run("(add1)"));      
      TEST_EQ("1", run("(add1 0)"));
      
      TEST_EQ("0", run("(add1 -1)"));
      TEST_EQ("6", run("(add1 5)"));
      TEST_EQ("-999", run("(add1 -1000)"));
      
      TEST_EQ("536870911", run("(add1 536870910)"));
      TEST_EQ("-536870911", run("(add1 -536870912)"));
      TEST_EQ("2", run("(add1 (add1 0))"));
      TEST_EQ("18", run("(add1 (add1 (add1 (add1 (add1 (add1 12))))))"));
      TEST_EQ("53687091001", run("(add1 53687091000)"));
      TEST_EQ("-53687091000", run("(add1 -53687091001)"));      
      TEST_EQ("1.5", run("(add1 0.5)"));
      TEST_EQ("0.4", run("(add1 -0.6)"));      
      }
    };

  struct sub1 : public compile_fixture {
    void test()
      {
      TEST_EQ("0", run("(sub1 1)"));
      TEST_EQ("-1", run("(sub1 0)"));
      TEST_EQ("-2", run("(sub1 -1)"));
      TEST_EQ("4", run("(sub1 5)"));
      TEST_EQ("-1001", run(R"((sub1 -1000))"));

      TEST_EQ("0.5", run("(sub1 1.5)"));
      TEST_EQ("-0.5", run("(sub1 0.5)"));
      TEST_EQ("-1.6", run("(sub1 -0.6)"));
      }
    };

  struct add_fixnums : public compile_fixture {
    void test()
      {
      TEST_EQ("3", run("(+ 1 2)"));
      TEST_EQ("6", run("(+ 1 2 3)"));
      TEST_EQ("10", run("(+ 1 2 3 4)"));
      TEST_EQ("15", run("(+ 1 2 3 4 5)"));
      TEST_EQ("21", run("(+ 1 2 3 4 5 6)"));
      TEST_EQ("28", run("(+ 1 2 3 4 5 6 7)"));
      TEST_EQ("36", run("(+ 1 2 3 4 5 6 7 8)"));
      TEST_EQ("45", run("(+ 1 2 3 4 5 6 7 8 9)"));
      TEST_EQ("55", run("(+ 1 2 3 4 5 6 7 8 9 10)"));
      TEST_EQ("66", run("(+ 1 2 3 4 5 6 7 8 9 10 11)"));
      }
    };

  struct add_flonums : public compile_fixture {
    void test()
      {
      TEST_EQ("3", run("(+ 1.0 2.0)"));
      TEST_EQ("6", run("(+ 1.0 2.0 3.0)"));
      TEST_EQ("10", run("(+ 1.0 2.0 3.0 4.0)"));
      TEST_EQ("15", run("(+ 1.0 2.0 3.0 4.0 5.0)"));
      TEST_EQ("21", run("(+ 1.0 2.0 3.0 4.0 5.0 6.0)"));
      TEST_EQ("28", run("(+ 1.0 2.0 3.0 4.0 5.0 6.0 7.0)"));
      TEST_EQ("36", run("(+ 1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0)"));
      TEST_EQ("45", run("(+ 1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0)"));
      TEST_EQ("55", run("(+ 1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0)"));
      TEST_EQ("66", run("(+ 1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0 11.0)"));
      }
    };

  struct add_flonums_and_fixnums : public compile_fixture {
    void test()
      {
      TEST_EQ("3", run("(+ 1 2.0)"));
      TEST_EQ("3", run("(+ 1.0 2)"));
      }
    };

  struct sub : public compile_fixture {
    void test()
      {
      TEST_EQ("-1", run("(- 1 2)"));
      TEST_EQ("-4", run("(- 1 2 3)"));
      TEST_EQ("-3", run("(- 1 2 3 -1)"));

      TEST_EQ("0.1", run("(- 0.5 0.4)"));
      TEST_EQ("-2.77556e-17", run("(- 0.5 0.4 0.1)"));

      TEST_EQ("5.8", run("(- 7 0.5 0.4 0.1 0.2)"));
      }
    };

  struct mul : public compile_fixture {
    void test()
      {
      TEST_EQ("2", run("(* 1 2)"));
      TEST_EQ("6", run("(* 1 2 3)"));
      TEST_EQ("-6", run("(* 1 2 3 -1)"));

      TEST_EQ("1.25", run("(* 0.5 2.5)"));
      TEST_EQ("0.125", run("(* 0.5 2.5 0.1)"));
      }
    };

  struct divtest : public compile_fixture {
    void test()
      {
      TEST_EQ("2", run("(/ 4 2)"));
      TEST_EQ("2", run("(/ 8 2 2)"));
      TEST_EQ("-1", run("(/ 16 4 -4)"));
      TEST_EQ("-1", run("(/ 16 -4 4)"));

      TEST_EQ("1.25", run("(/ 2.5 2)"));
      TEST_EQ("-12.5", run("(/ 2.5 2 -0.1)"));
      }
    };

  struct add_incorrect_argument : public compile_fixture {
    void test()
      {
      TEST_EQ("runtime error: add1: contract violation", run("(add1 #t)"));
      TEST_EQ("runtime error: sub1: contract violation", run("(sub1 ())"));

      TEST_EQ("runtime error: +: contract violation", run("(+ #t #t)"));
      TEST_EQ("runtime error: +: contract violation", run("(+ 3 #t)"));
      TEST_EQ("runtime error: +: contract violation", run("(+ #t 5)"));
      TEST_EQ("runtime error: +: contract violation", run("(+ 3.1 #t)"));
      TEST_EQ("runtime error: +: contract violation", run("(+ #t 5.1)"));
      TEST_EQ("runtime error: +: contract violation", run("(+ 1 2 3 4 5 6 7 8 3.1 #t)"));
      TEST_EQ("runtime error: +: contract violation", run("(+ 1 2 3 4 5 6 7 8 #t 5.1)"));

      TEST_EQ("runtime error: -: contract violation", run("(- #t #t)"));
      TEST_EQ("runtime error: -: contract violation", run("(- 3 #t)"));
      TEST_EQ("runtime error: -: contract violation", run("(- #t 5)"));
      TEST_EQ("runtime error: -: contract violation", run("(- 3.1 #t)"));
      TEST_EQ("runtime error: -: contract violation", run("(- #t 5.1)"));
      TEST_EQ("runtime error: -: contract violation", run("(- 1 2 3 4 5 6 7 8 3.1 #t)"));
      TEST_EQ("runtime error: -: contract violation", run("(- 1 2 3 4 5 6 7 8 #t 5.1)"));

      TEST_EQ("runtime error: *: contract violation", run("(* #t #t)"));
      TEST_EQ("runtime error: *: contract violation", run("(* 3 #t)"));
      TEST_EQ("runtime error: *: contract violation", run("(* #t 5)"));
      TEST_EQ("runtime error: *: contract violation", run("(* 3.1 #t)"));
      TEST_EQ("runtime error: *: contract violation", run("(* #t 5.1)"));
      TEST_EQ("runtime error: *: contract violation", run("(* 1 2 3 4 5 6 7 8 3.1 #t)"));
      TEST_EQ("runtime error: *: contract violation", run("(* 1 2 3 4 5 6 7 8 #t 5.1)"));

      TEST_EQ("runtime error: /: contract violation", run("(/ #t #t)"));
      TEST_EQ("runtime error: /: contract violation", run("(/ 3 #t)"));
      TEST_EQ("runtime error: /: contract violation", run("(/ #t 5)"));
      TEST_EQ("runtime error: /: contract violation", run("(/ 3.1 #t)"));
      TEST_EQ("runtime error: /: contract violation", run("(/ #t 5.1)"));
      TEST_EQ("runtime error: /: contract violation", run("(/ 1 2 3 4 5 6 7 8 3.1 #t)"));
      TEST_EQ("runtime error: /: contract violation", run("(/ 1 2 3 4 5 6 7 8 #t 5.1)"));
      }
    };

  struct combination_of_math_ops : public compile_fixture {
    void test()
      {
      TEST_EQ("27", run("(/ (* 3 (- (+ 23 9) 20.0) 1.5) 2)")); // 3*12*1.5 / 2 = 3*6*1.5 = 3*9 = 27
      }
    };

  struct equal : public compile_fixture {
    void test()
      {
      TEST_EQ("#f", run("(= 12 13)"));
      TEST_EQ("#t", run("(= 12 12)"));
      TEST_EQ("#f", run("(= 12.1 13.1)"));
      TEST_EQ("#t", run("(= 12.1 12.1)"));
      TEST_EQ("#f", run("(= 12 13.1)"));
      TEST_EQ("#t", run("(= 12 12.0)"));
      TEST_EQ("#f", run("(= 12.0 13)"));
      TEST_EQ("#t", run("(= 12.0 12)"));

      TEST_EQ("#t", run("(= 12 12)"));
      TEST_EQ("#f", run("(= 13 12)"));
      TEST_EQ("#f", run("(= 16 (+ 13 1)) "));
      TEST_EQ("#t", run("(= 16 (+ 13 3))"));
      TEST_EQ("#f", run("(= 16 (+ 13 13))"));
      TEST_EQ("#f", run("(= (+ 13 1) 16) "));
      TEST_EQ("#t", run("(= (+ 13 3) 16) "));
      TEST_EQ("#f", run("(= (+ 13 13) 16)"));

      TEST_EQ("#f", run("(= 12.0 13)"));
      TEST_EQ("#t", run("(= 12.0 12)"));
      TEST_EQ("#f", run("(= 13.0 12)"));
      TEST_EQ("#f", run("(= 16.0 (+ 13 1)) "));
      TEST_EQ("#t", run("(= 16.0 (+ 13 3))"));
      TEST_EQ("#f", run("(= 16.0 (+ 13 13))"));
      TEST_EQ("#f", run("(= (+ 13.0 1) 16) "));
      TEST_EQ("#t", run("(= (+ 13.0 3) 16.0) "));
      TEST_EQ("#f", run("(= (+ 13.0 13.0) 16.0)"));

      TEST_EQ("#t", run("(= 12 12 12)"));
      TEST_EQ("#t", run("(= 12 12 12 12)"));
      TEST_EQ("#t", run("(= 12 12 12 12 12)"));
      TEST_EQ("#t", run("(= 12 12 12 12 12 12)"));
      TEST_EQ("#t", run("(= 12 12 12 12 12 12 12)"));
      TEST_EQ("#t", run("(= 12 12 12 12 12 12 12 12)"));
      TEST_EQ("#t", run("(= 12 12 12 12 12 12 12 12 12)"));
      TEST_EQ("#t", run("(= 12 12 12 12 12 12 12 12 12 12)"));

      TEST_EQ("#f", run("(= 13 12 12)"));
      TEST_EQ("#f", run("(= 13 12 12 12)"));
      TEST_EQ("#f", run("(= 13 12 12 12 12)"));
      TEST_EQ("#f", run("(= 13 12 12 12 12 12)"));
      TEST_EQ("#f", run("(= 13 12 12 12 12 12 12)"));
      TEST_EQ("#f", run("(= 13 12 12 12 12 12 12 12)"));
      TEST_EQ("#f", run("(= 13 12 12 12 12 12 12 12 12)"));
      TEST_EQ("#f", run("(= 13 12 12 12 12 12 12 12 12 12)"));

      TEST_EQ("#f", run("(= 12 12 13)"));
      TEST_EQ("#f", run("(= 12 12 12 13)"));
      TEST_EQ("#f", run("(= 12 12 12 12 13)"));
      TEST_EQ("#f", run("(= 12 12 12 12 12 13)"));
      TEST_EQ("#f", run("(= 12 12 12 12 12 12 13)"));
      TEST_EQ("#f", run("(= 12 12 12 12 12 12 12 13)"));
      TEST_EQ("#f", run("(= 12 12 12 12 12 12 12 12 13)"));
      TEST_EQ("#f", run("(= 12 12 12 12 12 12 12 12 12 13)"));
      }
    };

  struct not_equal : public compile_fixture {
    void test()
      {
      TEST_EQ("#t", run("(!= 12 13)"));
      TEST_EQ("#f", run("(!= 12 12)"));
      TEST_EQ("#t", run("(!= 12.1 13.1)"));
      TEST_EQ("#f", run("(!= 12.1 12.1)"));
      TEST_EQ("#t", run("(!= 12 13.1)"));
      TEST_EQ("#f", run("(!= 12 12.0)"));
      TEST_EQ("#t", run("(!= 12.0 13)"));
      TEST_EQ("#f", run("(!= 12.0 12)"));

      TEST_EQ("#f", run("(!= 12 12)"));
      TEST_EQ("#t", run("(!= 13 12)"));
      TEST_EQ("#t", run("(!= 16 (+ 13 1)) "));
      TEST_EQ("#f", run("(!= 16 (+ 13 3))"));
      TEST_EQ("#t", run("(!= 16 (+ 13 13))"));
      TEST_EQ("#t", run("(!= (+ 13 1) 16) "));
      TEST_EQ("#f", run("(!= (+ 13 3) 16) "));
      TEST_EQ("#t", run("(!= (+ 13 13) 16)"));

      TEST_EQ("#t", run("(!= 12.0 13)"));
      TEST_EQ("#f", run("(!= 12.0 12)"));
      TEST_EQ("#t", run("(!= 13.0 12)"));
      TEST_EQ("#t", run("(!= 16.0 (+ 13 1)) "));
      TEST_EQ("#f", run("(!= 16.0 (+ 13 3))"));
      TEST_EQ("#t", run("(!= 16.0 (+ 13 13))"));
      TEST_EQ("#t", run("(!= (+ 13.0 1) 16) "));
      TEST_EQ("#f", run("(!= (+ 13.0 3) 16.0) "));
      TEST_EQ("#t", run("(!= (+ 13.0 13.0) 16.0)"));

      TEST_EQ("#t", run("(!= 12 13 14 15 16 17 18 19 20 21 22 23)"));
      TEST_EQ("#t", run("(!= 12 13 14 15 16 17 18 19 20 21 22 12)"));
      }
    };

  struct less : public compile_fixture {
    void test()
      {
      TEST_EQ("#f", run("(< 4 2)"));
      TEST_EQ("#t", run("(< 2 4)"));
      TEST_EQ("#f", run("(< 4 2 3)"));
      TEST_EQ("#t", run("(< 2 4 5)"));
      TEST_EQ("#f", run("(< 2 4 3)"));

      TEST_EQ("#f", run("(< 4.1 2)"));
      TEST_EQ("#t", run("(< 2.1 4)"));
      TEST_EQ("#f", run("(< 4.1 2 3)"));
      TEST_EQ("#t", run("(< 2.1 4 5)"));
      TEST_EQ("#f", run("(< 2.1 4 3)"));

      TEST_EQ("#t", run("(< 12 13)"));
      TEST_EQ("#f", run("(< 12 12)"));
      TEST_EQ("#f", run("(< 13 12)"));
      TEST_EQ("#f", run("(< 16 (+ 13 1)) "));
      TEST_EQ("#f", run("(< 16 (+ 13 3))"));
      TEST_EQ("#t", run("(< 16 (+ 13 13))"));
      TEST_EQ("#t", run("(< (+ 13 1) 16) "));
      TEST_EQ("#f", run("(< (+ 13 3) 16) "));
      TEST_EQ("#f", run("(< (+ 13 13) 16)"));

      TEST_EQ("#t", run("(< 12.0 13)"));
      TEST_EQ("#f", run("(< 12.0 12)"));
      TEST_EQ("#f", run("(< 13.0 12)"));
      TEST_EQ("#f", run("(< 16.0 (+ 13 1.0)) "));
      TEST_EQ("#f", run("(< 16.0 (+ 13.0 3.0))"));
      TEST_EQ("#t", run("(< 16.0 (+ 13.0 13.0))"));
      TEST_EQ("#t", run("(< (+ 13.0 1) 16.0) "));
      TEST_EQ("#f", run("(< (+ 13.0 3.000000001) 16.0)"));
      TEST_EQ("#f", run("(< (+ 13 13.0) 16)"));
      }
    };

  struct leq : public compile_fixture {
    void test()
      {
      TEST_EQ("#t", run("(<= 12 13)"));
      TEST_EQ("#t", run("(<= 12 12)"));
      TEST_EQ("#f", run("(<= 13 12)"));
      TEST_EQ("#f", run("(<= 16 (+ 13 1)) "));
      TEST_EQ("#t", run("(<= 16 (+ 13 3))"));
      TEST_EQ("#t", run("(<= 16 (+ 13 13))"));
      TEST_EQ("#t", run("(<= (+ 13 1) 16) "));
      TEST_EQ("#t", run("(<= (+ 13 3) 16) "));
      TEST_EQ("#f", run("(<= (+ 13 13) 16)"));

      TEST_EQ("#t", run("(<= 12.0 13.0)"));
      TEST_EQ("#t", run("(<= 12.0 12.0)"));
      TEST_EQ("#f", run("(<= 13.0 12)"));
      TEST_EQ("#f", run("(<= 16 (+ 13.0 1)) "));
      TEST_EQ("#t", run("(<= 16 (+ 13 3.0))"));
      TEST_EQ("#t", run("(<= 16.0 (+ 13.0 13.0))"));
      TEST_EQ("#t", run("(<= (+ 13.0 1) 16) "));
      TEST_EQ("#t", run("(<= (+ 13 3.0) 16.0) "));
      TEST_EQ("#f", run("(<= (+ 13.0 13) 16.0)"));
      }
    };

  struct greater : public compile_fixture {
    void test()
      {
      TEST_EQ("#f", run("(> 12 13)"));
      TEST_EQ("#f", run("(> 12 12)"));
      TEST_EQ("#t", run("(> 13 12)"));
      TEST_EQ("#t", run("(> 16 (+ 13 1)) "));
      TEST_EQ("#f", run("(> 16 (+ 13 3))"));
      TEST_EQ("#f", run("(> 16 (+ 13 13))"));
      TEST_EQ("#f", run("(> (+ 13 1) 16) "));
      TEST_EQ("#f", run("(> (+ 13 3) 16) "));
      TEST_EQ("#t", run("(> (+ 13 13) 16)"));

      TEST_EQ("#f", run("(> 12.0 13)"));
      TEST_EQ("#f", run("(> 12.0 12)"));
      TEST_EQ("#t", run("(> 13.0 12)"));
      TEST_EQ("#t", run("(> 16.0 (+ 13 1)) "));
      TEST_EQ("#f", run("(> 16.0 (+ 13 3))"));
      TEST_EQ("#f", run("(> 16.0 (+ 13 13))"));
      TEST_EQ("#f", run("(> (+ 13.0 1) 16) "));
      TEST_EQ("#f", run("(> (+ 13.0 3) 16) "));
      TEST_EQ("#t", run("(> (+ 13.0 13) 16)"));
      }
    };

  struct geq : public compile_fixture {
    void test()
      {
      TEST_EQ("#f", run("(>= 12 13)"));
      TEST_EQ("#t", run("(>= 12 12)"));
      TEST_EQ("#t", run("(>= 13 12)"));
      TEST_EQ("#t", run("(>= 16 (+ 13 1)) "));
      TEST_EQ("#t", run("(>= 16 (+ 13 3))"));
      TEST_EQ("#f", run("(>= 16 (+ 13 13))"));
      TEST_EQ("#f", run("(>= (+ 13 1) 16) "));
      TEST_EQ("#t", run("(>= (+ 13 3) 16) "));
      TEST_EQ("#t", run("(>= (+ 13 13) 16)"));

      TEST_EQ("#f", run("(>= 12.0 13)"));
      TEST_EQ("#t", run("(>= 12.0 12)"));
      TEST_EQ("#t", run("(>= 13.0 12)"));
      TEST_EQ("#t", run("(>= 16.0 (+ 13 1)) "));
      TEST_EQ("#t", run("(>= 16.0 (+ 13 3))"));
      TEST_EQ("#f", run("(>= 16.0 (+ 13 13))"));
      TEST_EQ("#f", run("(>= (+ 13.0 1) 16) "));
      TEST_EQ("#t", run("(>= (+ 13.0 3) 16) "));
      TEST_EQ("#t", run("(>= (+ 13.0 13) 16)"));
      }
    };

  struct compare_incorrect_argument : public compile_fixture {
    void test()
      {
      TEST_EQ("runtime error: =: contract violation", run("(= 3 #t)"));
      TEST_EQ("runtime error: !=: contract violation", run("(!= 3 ())"));
      TEST_EQ("runtime error: <: contract violation", run("(< 3 #t)"));
      TEST_EQ("runtime error: <=: contract violation", run("(<= 3 ())"));
      TEST_EQ("runtime error: >: contract violation", run("(> 3 #t)"));
      TEST_EQ("runtime error: >=: contract violation", run("(>= 3 ())"));
      }
    };

  struct iftest : public compile_fixture {
    void test()
      {
      TEST_EQ("2", run("(if (< 2 3) (2) (3))"));
      TEST_EQ("3", run("(if (< 3 2) (2) (3))"));
      TEST_EQ("12", run("(if #t 12 13)"));
      TEST_EQ("13", run("(if #f 12 13)"));
      TEST_EQ("12", run("(if 0 12 13)"));
      TEST_EQ("43", run("(if () 43 ())"));
      TEST_EQ("13", run("(if #t (if 12 13 4) 17)"));
      TEST_EQ("4", run("(if #f 12 (if #f 13 4))"));
      TEST_EQ("2", run(R"((if #\X (if 1 2 3) (if 4 5 6)))"));
      TEST_EQ("#t", run("(if (not (boolean? #t)) 15 (boolean? #f))"));
      TEST_EQ("-23", run(R"((if (if (char? #\a) (boolean? #\b) (fixnum? #\c)) 119 -23))"));
      TEST_EQ("6", run(R"((if (if (if (not 1) (not 2) (not 3)) 4 5) 6 7))"));
      TEST_EQ("7", run(R"((if (not (if (if (not 1) (not 2) (not 3)) 4 5)) 6 7))"));
      TEST_EQ("#f", run(R"((not (if (not (if (if (not 1) (not 2) (not 3)) 4 5)) 6 7)) )"));
      TEST_EQ("14", run(R"((if (char? 12) 13 14) )"));
      TEST_EQ("13", run(R"((if (char? #\a) 13 14) )"));
      TEST_EQ("13", run(R"((add1 (if (sub1 1) (sub1 13) 14)))"));

      TEST_EQ("13", run("(if (= 12 13) 12 13) "));
      TEST_EQ("13", run("(if (= 12 12) 13 14) "));
      TEST_EQ("12", run("(if (< 12 13) 12 13) "));
      TEST_EQ("14", run("(if (< 12 12) 13 14) "));
      TEST_EQ("14", run("(if (< 13 12) 13 14) "));
      TEST_EQ("12", run("(if (<= 12 13) 12 13) "));
      TEST_EQ("12", run("(if (<= 12 12) 12 13) "));
      TEST_EQ("14", run("(if (<= 13 12) 13 14) "));
      TEST_EQ("13", run("(if (> 12 13) 12 13) "));
      TEST_EQ("13", run("(if (> 12 12) 12 13) "));
      TEST_EQ("13", run("(if (> 13 12) 13 14) "));
      TEST_EQ("13", run("(if (>= 12 13) 12 13) "));
      TEST_EQ("12", run("(if (>= 12 12) 12 13) "));
      TEST_EQ("13", run("(if (>= 13 12) 13 14) "));
      }
    };
  struct andtest : public compile_fixture {
    void test()
      {
      TEST_EQ("#t", run("(and #t)"));
      TEST_EQ("#f", run("(and #f)"));
      TEST_EQ("#t", run("(and #t #t)"));
      TEST_EQ("#f", run("(and #f #f)"));
      TEST_EQ("#f", run("(and #f #t)"));
      TEST_EQ("#f", run("(and #t #f)"));
      TEST_EQ("#t", run("(and #t #t #t)"));
      TEST_EQ("#f", run("(and #f #t #t)"));
      TEST_EQ("#f", run("(and #t #t #f)"));
      TEST_EQ("#f", run("(and #t #f #t)"));
      TEST_EQ("#f", run("(and #f #f #f)"));
      }
    };

  struct ortest : public compile_fixture {
    void test()
      {
      TEST_EQ("#t", run("(or #t)"));
      TEST_EQ("#f", run("(or #f)"));
      TEST_EQ("#t", run("(or #t #t)"));
      TEST_EQ("#f", run("(or #f #f)"));
      TEST_EQ("#t", run("(or #f #t)"));
      TEST_EQ("#t", run("(or #t #f)"));
      TEST_EQ("#t", run("(or #t #t #t)"));
      TEST_EQ("#t", run("(or #f #t #t)"));
      TEST_EQ("#t", run("(or #t #t #f)"));
      TEST_EQ("#t", run("(or #t #f #t)"));
      TEST_EQ("#f", run("(or #f #f #f)"));
      }
    };

  struct let : public compile_fixture {
    void test()
      {
      TEST_EQ("5", run("(let ([x 5]) x)"));
      TEST_EQ("5", run("(let ([x 5][y 6]) x)"));
      TEST_EQ("6", run("(let ([x 5][y 6]) y)"));

      TEST_EQ("3", run("(let ([x (+ 1 2)]) x)"));
      TEST_EQ("10", run("(let ([x (+ 1 2)]) (let([y(+ 3 4)])(+ x y))) "));
      TEST_EQ("4", run("(let ([x (+ 1 2)])  (let([y(+ 3 4)])(- y x)))"));
      TEST_EQ("4", run("(let ([x (+ 1 2)] [y(+ 3 4)])  (- y x))"));
      TEST_EQ("18", run("(let ([x (let ([y (+ 1 2)]) (* y y))]) (+ x x))"));
      TEST_EQ("7", run("(let ([x (+ 1 2)]) (let([x(+ 3 4)]) x))"));
      TEST_EQ("7", run("(let ([x (+ 1 2)]) (let([x(+ x 4)]) x)) "));
      TEST_EQ("3", run("(let ([t (let ([t (let ([t (let ([t (+ 1 2)]) t)]) t)]) t)]) t)"));
      TEST_EQ("192", run("(let ([x 12])  (let([x(+ x x)]) (let([x(+ x x)]) (let([x(+ x x)]) (+ x x)))))"));

      TEST_EQ("45", run("(let ([a 0] [b 1] [c 2] [d 3] [e 4] [f 5] [g 6] [h 7] [i 8] [j 9]) (+ a b c d e f g h i j) )"));
      TEST_EQ("runtime error: closure expected", run("(let ([x 5]) (x))"));
      }
    };

  struct let_star : public compile_fixture {
    void test()
      {
      TEST_EQ("5", run("(let* ([x 5]) x)"));
      TEST_EQ("3", run("(let* ([x (+ 1 2)]) x)"));
      TEST_EQ("10", run("(let* ([x (+ 1 2)] [y(+ 3 4)])(+ x y))"));
      TEST_EQ("4", run("(let* ([x (+ 1 2)] [y(+ 3 4)]) (- y x))"));
      TEST_EQ("18", run("(let* ([x (let* ([y (+ 1 2)]) (* y y))])(+ x x))"));
      TEST_EQ("7", run("(let* ([x (+ 1 2)] [x(+ 3 4)]) x)"));
      TEST_EQ("7", run("(let* ([x (+ 1 2)] [x(+ x 4)]) x)"));
      TEST_EQ("3", run("(let* ([t (let* ([t (let* ([t (let* ([t (+ 1 2)]) t)]) t)]) t)]) t)"));
      TEST_EQ("192", run("(let* ([x 12] [x(+ x x)] [x(+ x x)] [x(+ x x)])  (+ x x))"));
      }
    };

  struct arithmetic : public compile_fixture {
    void test()
      {
      TEST_EQ("45", run("(+ (+ (+ (+ (+ (+ (+ (+ 1 2) 3) 4) 5) 6) 7) 8) 9)"));
      TEST_EQ("45", run("(+ 1 (+ 2 (+ 3 (+ 4 (+ 5 (+ 6 (+ 7 (+ 8 9))))))))"));

      TEST_EQ("-43", run("(- (- (- (- (- (- (- (- 1 2) 3) 4) 5) 6) 7) 8) 9)"));
      TEST_EQ("5", run("(- 1 (- 2 (- 3 (- 4 (- 5 (- 6 (- 7 (- 8 9))))))))"));

      TEST_EQ("5040", run("(* (* (* (* (* 2 3) 4) 5) 6) 7)"));
      TEST_EQ("5040", run("(* 2 (* 3 (* 4 (* 5 (* 6 7)))))"));
      }
    };

  struct globals : public compile_fixture {
    void test()
      {
      TEST_EQ("3.14", run("(define x 3.14) 50 x"));
      TEST_EQ("3.14", run("x"));
      TEST_EQ("7", run("(define x 7) 51 x"));
      TEST_EQ("7", run("x"));
      }
    };

  struct vector : public compile_fixture {
    void test()
      {
      TEST_EQ("#(1 2)", run("(vector 1 2)"));
      TEST_EQ("#(1 2 3 4 5 6 7 8)", run("(vector 1 2 3 4 5 6 7 8)"));
      TEST_EQ("#(1 2 3 4 5 6 7 8 9)", run("(vector 1 2 3 4 5 6 7 8 9)"));
      TEST_EQ("#(1 2 3 4 5 6 7 8 9 10)", run("(vector 1 2 3 4 5 6 7 8 9 10)"));
      TEST_EQ("#(1 2 3 4 5 6 7 8 9 10 11 12 13 14 15)", run("(vector 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15)"));
      TEST_EQ("2", run("(let ([x (vector 1 2)]) (vector-ref x 1))"));
      TEST_EQ("2", run("(let ([x (vector 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15)]) (vector-ref x 1))"));
      TEST_EQ("10", run("(let ([x (vector 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15)]) (vector-ref x 9))"));
      TEST_EQ("1", run("(let ([x (vector 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15)]) (vector-ref x 0))"));
      TEST_EQ("15", run("(let ([x (vector 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15)]) (vector-ref x 14))"));

      TEST_EQ("runtime error: vector-ref: contract violation", run("(let ([x (vector 1 2)] [y 3]) (vector-ref y 1))"));
      TEST_EQ("runtime error: vector-ref: contract violation", run("(let ([x (vector 1 2)] [y 3]) (vector-ref x #t))"));
      TEST_EQ("runtime error: vector-ref: out of bounds", run("(let ([x (vector 1 2)] [y 3]) (vector-ref x 2))"));
      TEST_EQ("runtime error: vector-ref: out of bounds", run("(let ([x (vector 1 2)] [y 3]) (vector-ref x -1))"));

      TEST_EQ("100", run("(let ([x (vector 1 2)]) (begin (vector-set! x 1 100) (vector-ref x 1)))"));
      TEST_EQ("#(1 100)", run("(let ([x (vector 1 2)]) (begin (vector-set! x 1 100) x))"));
      TEST_EQ("#(3.14 2)", run("(let ([x (vector 1 2)]) (begin (vector-set! x 0 3.14) x))"));

      TEST_EQ("runtime error: vector-set!: contract violation", run("(let ([x (vector 1 2)]) (begin (vector-set! x 0.0 3.14) x))"));
      TEST_EQ("runtime error: vector-set!: contract violation", run("(let ([x (vector 1 2)] [y 3]) (begin (vector-set! y 0.0 3.14) x))"));

      TEST_EQ("runtime error: vector-set!: out of bounds", run("(let ([x (vector 1 2)]) (begin (vector-set! x -1 3.14) x))"));
      TEST_EQ("runtime error: vector-set!: out of bounds", run("(let ([x (vector 1 2)]) (begin (vector-set! x 100 3.14) x))"));

      TEST_EQ("#t", run("(vector? (vector 0)) "));
      TEST_EQ("#f", run("(vector? 1287) "));
      TEST_EQ("#f", run("(vector? ()) "));
      TEST_EQ("#f", run("(vector? #t) "));
      TEST_EQ("#f", run("(vector? #f) "));
      }
    };

  struct letrec : public compile_fixture {
    void test()
      {
      TEST_EQ("12", run("(letrec () 12)"));
      TEST_EQ("10", run("(letrec () (let ([x 5]) (+ x x)))"));
      TEST_EQ("7", run("(letrec ([f (lambda () 5)]) 7)"));
      TEST_EQ("12", run("(letrec () 12)"));
      TEST_EQ("10", run("(letrec () (let ([x 5]) (+ x x)))"));
      TEST_EQ("7", run("(letrec ([f (lambda () 5)]) 7)"));
      TEST_EQ("12", run("(letrec ([f (lambda () 5)]) (let ([x 12]) x))"));
      TEST_EQ("5", run("(letrec ([f (lambda () 5)]) (f))"));
      TEST_EQ("5", run("(letrec ([f (lambda () 5)]) (let ([x (f)]) x))"));
      TEST_EQ("11", run("(letrec ([f (lambda () 5)]) (+ (f) 6))"));
      TEST_EQ("11", run("(letrec ([f (lambda () 5)]) (+ 6 (f)))"));
      TEST_EQ("15", run("(letrec ([f (lambda () 5)]) (- 20 (f)))"));
      TEST_EQ("10", run("(letrec ([f (lambda () 5)]) (+ (f) (f)))"));
      TEST_EQ("12", run("(letrec ([f (lambda () (+ 5 7))])(f))"));
      TEST_EQ("25", run("(letrec ([f (lambda (x) (+ x 12))]) (f 13))"));
      TEST_EQ("12", run("(letrec ([f (lambda (x) (+ x 12))]) (f 0))"));
      TEST_EQ("24", run("(letrec ([f (lambda (x) (+ x 12))]) (f (f 0)))"));
      TEST_EQ("36", run("(letrec ([f (lambda (x) (+ x 12))]) (f (f (f 0))))"));
      TEST_EQ("41", run("(letrec ([f (lambda (x y) (+ x y))] [g (lambda(x) (+ x 12))])(f 16 (f (g 0) (+ 1 (g 0)))))"));
      TEST_EQ("24", run("(letrec ([f (lambda (x) (g x x))][g(lambda(x y) (+ x y))])(f 12))"));
      TEST_EQ("34", run("(letrec ([f (lambda (x) (+ x 12))]) (f (f 10)))"));
      TEST_EQ("36", run("(letrec ([f (lambda (x) (+ x 12))]) (f (f (f 0))))"));
      TEST_EQ("25", run("(let ([f (lambda () 12)][g(lambda() 13)])(+ (f)(g)))"));
      TEST_EQ("120", run("(letrec ([f (lambda (x) (if (zero? x) 1 (* x(f(sub1 x)))))]) (f 5))"));
      TEST_EQ("120", run("(letrec ([f (lambda (x acc) (if (zero? x) acc (f(sub1 x) (* acc x))))]) (f 5 1))"));
      TEST_EQ("200", run("(letrec ([f (lambda (x) (if (zero? x) 0 (+ 1 (f(sub1 x)))))]) (f 200))"));
      TEST_EQ("500", run("(letrec ([f (lambda (x) (if (zero? x) 0 (+ 1 (f(sub1 x)))))]) (f 500))"));
      }
    };

  struct lambdas : public compile_fixture {
    void test()
      {
      TEST_EQ("<lambda>", run("(lambda (x) (+ x x))"));
      TEST_EQ("3", run("((lambda(x) x) 3)"));
      TEST_EQ("10", run("((lambda(x y) (+ x y)) 3 7)"));
      TEST_EQ("8", run("(let ([x 5]) ((lambda (y) (+ 3 y)) x ))"));
      TEST_EQ("5", run("( (lambda () (+ 3 2)) () )"));
      TEST_EQ("<lambda>", run("(let ([f (lambda () 5)]) f)"));
      TEST_EQ("8", run("(let ([f (lambda (n) (+ n 5))]) (f 3))"));
      TEST_EQ("5", run("(let ([f (lambda () 5)]) (f))"));

      TEST_EQ("8", run("(let ([x 5]) ((lambda (y) (+ x y)) 3) )"));
      }
    };

  struct is_fixnum : public compile_fixture {
    void test()
      {
      TEST_EQ("#f", run(R"((fixnum? #\019))"));
      TEST_EQ("#f", run(R"((fixnum? #\a))"));
      TEST_EQ("#f", run(R"((fixnum? #t))"));
      TEST_EQ("#f", run(R"((fixnum? #f))"));
      TEST_EQ("#f", run(R"((fixnum? 0.3))"));
      TEST_EQ("#f", run(R"((fixnum? 0.0))"));
      TEST_EQ("#f", run(R"((fixnum? ()))"));
      TEST_EQ("#t", run(R"((fixnum? 0))"));
      TEST_EQ("#t", run(R"((fixnum? -1))"));
      TEST_EQ("#t", run(R"((fixnum? 1))"));
      TEST_EQ("#t", run(R"((fixnum? -1000))"));
      TEST_EQ("#t", run(R"((fixnum? 1000))"));
      TEST_EQ("#t", run(R"((fixnum? ((1000))))"));
      }
    };

  struct nottest : public compile_fixture {
    void test()
      {
      TEST_EQ("#f", run("(not #t)"));
      TEST_EQ("#t", run("(not #f)"));
      TEST_EQ("#t", run("(not (not #t))"));
      TEST_EQ("#f", run(R"((not #\a))"));
      TEST_EQ("#f", run("(not 0)"));
      TEST_EQ("#f", run("(not 15)"));
      TEST_EQ("#f", run("(not ())"));
      TEST_EQ("#t", run("(not (not 15))"));
      TEST_EQ("#f", run("(not (fixnum? 15))"));
      TEST_EQ("#t", run("(not (fixnum? #f))"));
      }
    };

  struct is_null : public compile_fixture {
    void test()
      {
      TEST_EQ("#t", run("(null? ())"));
      TEST_EQ("#f", run("(null? #f)"));
      TEST_EQ("#f", run("(null? #t)"));
      TEST_EQ("#f", run("(null? (null? ()))"));
      TEST_EQ("#f", run(R"((null? #\a))"));
      TEST_EQ("#f", run("(null? 0)"));
      TEST_EQ("#f", run("(null? -10)"));
      TEST_EQ("#f", run("(null? 10)"));
      }
    };


  struct is_flonum : public compile_fixture {
    void test()
      {
      TEST_EQ("#f", run(R"((flonum? #\019))"));
      TEST_EQ("#f", run(R"((flonum? #\a))"));
      TEST_EQ("#f", run(R"((flonum? #t))"));
      TEST_EQ("#f", run(R"((flonum? #f))"));
      TEST_EQ("#t", run(R"((flonum? 0.3))"));
      TEST_EQ("#t", run(R"((flonum? 0.0))"));
      TEST_EQ("#f", run(R"((flonum? ()))"));
      TEST_EQ("#t", run(R"((flonum? 0.0))"));
      TEST_EQ("#t", run(R"((flonum? -1.50))"));
      TEST_EQ("#t", run(R"((flonum? 1.02))"));
      TEST_EQ("#f", run(R"((flonum? -1000))"));
      TEST_EQ("#f", run(R"((flonum? 1000))"));
      TEST_EQ("#t", run(R"((flonum? ((1000.0))))"));
      }
    };
  struct is_zero : public compile_fixture {
    void test()
      {
      TEST_EQ("#t", run("(zero? 0)"));
      TEST_EQ("#f", run("(zero? 1)"));
      TEST_EQ("#f", run("(zero? -1)"));
      TEST_EQ("#f", run("(zero? 64)"));
      TEST_EQ("#f", run("(zero? 960)"));
      TEST_EQ("#f", run("(zero? 53687091158)"));

      TEST_EQ("#t", run("(zero? 0.0)"));
      TEST_EQ("#f", run("(zero? 0.0001)"));
      TEST_EQ("#f", run("(zero? 1.1)"));
      TEST_EQ("#f", run("(zero? -1.1)"));
      TEST_EQ("#f", run("(zero? 64.1)"));
      TEST_EQ("#f", run("(zero? 960.1)"));
      TEST_EQ("#f", run("(zero? 53687091158.1)"));

      TEST_EQ("runtime error: zero?: contract violation", run("(zero? #t)"));
      }
    };

  struct is_boolean : public compile_fixture {
    void test()
      {
      TEST_EQ("#t", run("(boolean? #t)"));
      TEST_EQ("#t", run("(boolean? #f)"));
      TEST_EQ("#f", run("(boolean? 0)"));
      TEST_EQ("#f", run("(boolean? 1)"));
      TEST_EQ("#f", run("(boolean? -1)"));
      TEST_EQ("#f", run("(boolean? ())"));
      TEST_EQ("#f", run(R"((boolean? #\a))"));
      TEST_EQ("#t", run("(boolean? (boolean? 0))"));
      TEST_EQ("#t", run("(boolean? (fixnum? (boolean? 0)))"));
      }
    };

  struct is_char : public compile_fixture {
    void test()
      {
      TEST_EQ("#t", run(R"((char? #\a))"));
      TEST_EQ("#t", run(R"((char? #\Z))"));
      TEST_EQ("#t", run(R"((char? #\000))"));
      TEST_EQ("#f", run(R"((char? #t))"));
      TEST_EQ("#f", run(R"((char? #f))"));
      TEST_EQ("#f", run(R"((char? (char? #t)))"));
      TEST_EQ("#f", run(R"((char? 0))"));
      TEST_EQ("#f", run(R"((char? 10))"));
      TEST_EQ("#f", run(R"((char? 0.3))"));
      }
    };
  struct cons : public compile_fixture {
    void test()
      {
      TEST_EQ("#t", run("(pair? (cons 1 2))"));
      TEST_EQ("#f", run("(pair? 12)"));
      TEST_EQ("#f", run("(pair? #t)"));
      TEST_EQ("#f", run("(pair? #f)"));
      TEST_EQ("#f", run("(pair? ())"));

      TEST_EQ("#f", run("(fixnum? (cons 12 43))"));
      TEST_EQ("#f", run("(boolean? (cons 12 43))"));
      TEST_EQ("#f", run("(null? (cons 12 43))"));
      TEST_EQ("#f", run("(not (cons 12 43))"));

      TEST_EQ("32", run("(if (cons 12 43) 32 43)"));
      TEST_EQ("1", run("(car(cons 1 23))"));
      TEST_EQ("123", run("(cdr(cons 43 123))"));
      TEST_EQ("#t", run("(let([x(cons 1 2)] [y(cons 3 4)]) (pair? x))"));
      TEST_EQ("#t", run("(pair? (cons(cons 12 3) #f))"));
      TEST_EQ("#t", run("(pair? (cons(cons 12 3) (cons #t #f)))"));
      TEST_EQ("12", run("(car(car(cons(cons 12 3) (cons #t #f))))"));
      TEST_EQ("3", run("(cdr(car(cons(cons 12 3) (cons #t #f))))"));
      TEST_EQ("#t", run("(car(cdr(cons(cons 12 3) (cons #t #f))))"));
      TEST_EQ("#f", run("(cdr(cdr(cons(cons 12 3) (cons #t #f))))"));
      TEST_EQ("#t", run("(pair? (cons(* 1 1) 1))"));

      TEST_EQ("((1 . 4) 3 . 2)", run("(let ([t0 (cons 1 2)] [t1 (cons 3 4)]) (let([a0(car t0)][a1(car t1)][d0(cdr t0)][d1(cdr t1)])(let([t0(cons a0 d1)][t1(cons a1 d0)]) (cons t0 t1))))"));
      TEST_EQ("(1 . 2)", run("(let ([t (cons 1 2)])(let([t t])(let([t t])(let([t t]) t))))"));
      TEST_EQ("(1 . 2)", run("(let ([t (let ([t (let ([t (let ([t (cons 1 2)]) t)]) t)]) t)]) t)"));
      TEST_EQ("((((()) ()) (()) ()) ((()) ()) (()) ())", run("(let ([x ()])(let([x(cons x x)])(let([x(cons x x)])(let([x(cons x x)])(cons x x)))))"));
      TEST_EQ("((#t #t . #t) ((#f . #f) . #f))", run("(cons (let ([x #t]) (let ([y (cons x x)]) (cons x y))) (cons(let([x #f]) (let([y(cons x x)]) (cons y x))) ())) "));

      TEST_EQ("(1 2 3 4 5 6 7 8 9 10)", run("(letrec ([f (lambda (i lst) (if (= i 0) lst (f (sub1 i) (cons i lst))))])(f 10 ())) "));

      TEST_EQ("runtime error: cons: contract violation", run("(cons 2)"));
      TEST_EQ("runtime error: cons: contract violation", run("(cons 2 3 4)"));

      TEST_EQ("runtime error: car: contract violation", run("(car 2)"));
      TEST_EQ("runtime error: cdr: contract violation", run("(cdr 2 3 4)"));
      }
    };
  struct tailcall : public compile_fixture {
    void test()
      {
      TEST_EQ("#f", run("(letrec ([e (lambda (x) (if (zero? x) #t (o (sub1 x))))][o(lambda(x) (if (zero? x) #f(e(sub1 x))))])(e 1))"));
      TEST_EQ("0", run("(letrec ([countdown (lambda (n) (if (zero? n) n (countdown(sub1 n))))])(countdown 5050))"));
      TEST_EQ("50005000", run("(letrec ([sum (lambda (n ac)(if (zero? n) ac (sum(sub1 n) (+ n ac))))]) (sum 10000 0))"));
      TEST_EQ("#t", run("(letrec ([e (lambda (x) (if (zero? x) #t (o (sub1 x))))][o(lambda(x) (if (zero? x) #f(e(sub1 x))))])(e 500))"));
      }
    };

  struct begin : public compile_fixture {
    void test()
      {
      TEST_EQ("12", run("(begin 12)"));
      TEST_EQ("122", run("(begin 13 122)"));
      TEST_EQ("#t", run("(begin 123 2343 #t)"));
      TEST_EQ("(1 . 2)", run("(let ([t (begin 12 (cons 1 2))]) (begin t t))"));
      TEST_EQ("(1 . 2)", run("(let ([t (begin 13 (cons 1 2))])(begin (cons 1 t)t ))"));
      TEST_EQ("(1 . 2)", run("(let ([t (cons 1 2)])(if (pair? t)(begin t) 12))"));
      }
    };

  struct implicit_begin : public compile_fixture {
    void test()
      {
      TEST_EQ("(1 . 2)", run("(let ([t (begin 13 (cons 1 2))])(cons 1 t)t)"));
      }
    };

  struct closures : public compile_fixture {
    void test()
      {
      TEST_EQ("12", run("(let ([n 12])(let([f(lambda() n)])(f)))"));
      TEST_EQ("112", run("(let ([n 12])(let([f(lambda(m) (+ n m))])(f 100)))"));
      TEST_EQ("120", run("(let ([f (lambda (f n m)(if (zero? n)  m (f(sub1 n) (* n m))))]) (let([g(lambda(g n m) (f(lambda(n m) (g g n m)) n m))])  (g g 5 1)))"));
      TEST_EQ("120", run("(let ([f (lambda (f n) (if (zero? n) 1 (* n(f(sub1 n)))))]) (let([g(lambda(g n) (f(lambda(n) (g g n)) n))]) (g g 5)))"));
      }
    };

  struct set : public compile_fixture {
    void test()
      {
      TEST_EQ("13", run("(let ([x 12])(set! x 13) x)"));
      TEST_EQ("13", run("(let([x 12]) (set! x(add1 x)) x) "));
      TEST_EQ("12", run("(let([x 12]) (let([x #f]) (set! x 14)) x) "));
      TEST_EQ("12", run("(let([x 12]) (let([y(let([x #f]) (set! x 14))])  x)) "));
      TEST_EQ("10", run("(let([f #f])(let([g(lambda() f)]) (set! f 10) (g))) "));
      TEST_EQ("13", run("(let([f(lambda(x) (set! x (add1 x)) x)]) (f 12)) "));
      TEST_EQ("(10 . 11)", run("(let([x 10]) (let([f(lambda(x) (set! x(add1 x)) x)]) (cons x(f x)))) "));
      TEST_EQ("17", run("(let([t #f]) (let([locative (cons (lambda() t) (lambda(n) (set! t n)))]) ((cdr locative) 17) ((car locative))))"));
      TEST_EQ("17", run("(let([locative (let([t #f]) (cons (lambda() t)  (lambda(n) (set! t n))))]) ((cdr locative) 17) ((car locative))) "));
      TEST_EQ("(1 . 0)", run("(let([make-counter (lambda() (let([counter -1]) (lambda()  (set! counter(add1 counter)) counter)))]) (let([c0(make-counter)] [c1(make-counter)])(c0) (cons(c0) (c1)))) "));
      TEST_EQ("120", run("(let([fact #f]) (set! fact(lambda(n) (if (zero? n) 1 (* n(fact(sub1 n)))))) (fact 5)) "));
      TEST_EQ("120", run("(let([fact #f]) ((begin (set! fact(lambda(n) (if (zero? n)  1 (* n(fact(sub1 n)))))) fact) 5)) "));
      }
    };

  struct letrec2 : public compile_fixture {
    void test()
      {
      TEST_EQ("12", run("(letrec() 12) "));
      TEST_EQ("12", run("(letrec([f 12]) f) "));
      TEST_EQ("25", run("(letrec([f 12][g 13]) (+ f g)) "));
      TEST_EQ("120", run("(letrec([fact (lambda(n) (if (zero? n)  1  (* n(fact(sub1 n)))))]) (fact 5)) "));
      TEST_EQ("12", run("(letrec([f 12][g(lambda() f)]) (g)) "));
      TEST_EQ("130", run("(letrec([f 12][g(lambda(n) (set! f n))])(g 130) f) "));
      TEST_EQ("12", run("(letrec([f (lambda(g) (set! f g) (f))]) (f(lambda() 12))) "));
      TEST_EQ("100", run("(letrec([f (cons (lambda() f) (lambda(x) (set! f x)))]) (let([g(car f)]) ((cdr f) 100) (g))) "));
      TEST_EQ("1", run("(let([v (vector 1 2 3)])(vector-ref v 0))"));
      TEST_EQ("#(5 2 3)", run("(let([v (vector 1 2 3)])(vector-set! v 0 5) v)"));
      TEST_EQ("48", run("(letrec([f(letrec([g(lambda(x) (* x 2))]) (lambda(n) (g(* n 2))))]) (f 12)) "));
      TEST_EQ("120", run("(letrec([f(lambda(f n) (if (zero? n)  1 (* n(f f(sub1 n)))))]) (f f 5)) "));
      TEST_EQ("120", run("(let([f(lambda(f) (lambda(n) (if (zero? n)  1 (* n(f(sub1 n))))))]) (letrec([fix (lambda(f) (f(lambda(n) ((fix f) n))))])((fix f) 5))) "));
      }
    };


  struct inner_define : public compile_fixture {
    void test()
      {
      TEST_EQ("7", run("(let()(define x 3)(define y 4)(+ x y))"));
      //TEST_EQ("6", run("(let()(letrec ([x 3][y x]) (+ x y)))")); // this violates r5rs letrec definition
      //TEST_EQ("6", run("(letrec ([x 3][y x]) (+ x y))")); // this violates r5rs letrec definition  
      //TEST_EQ("6", run("(let()(define x 3)(define y x)(+ x y))"));  // this violates r5rs letrec / define definition : it must be possible to evaluate each (expression) of every internal definition in a body without assigning or referring to the value of any variable being defined
      //TEST_EQ("8", run("(let()(define x 3)(set! x 4)(define y x)(+ x y))"));  // this violates r5rs letrec / define definition    
      TEST_EQ("45", run("(let([x 5])(define foo(lambda(y) (bar x y))) (define bar(lambda(a b) (+(* a b) a))) (foo(+ x 3)))"));
      }
    };

  struct global_define : public compile_fixture {
    void test()
      {
      TEST_EQ("3", run("(define x 3) x"));
      TEST_EQ("4", run("(define x 3) (set! x 4) x"));
      TEST_EQ("4", run("(begin (define x 3) (set! x 4) x)"));
      }
    };
  struct list : public compile_fixture {
    void test()
      {
      TEST_EQ("()", run("(list)"));
      TEST_EQ("(3)", run("(list 3)"));
      TEST_EQ("(3 6)", run("(list 3 6)"));
      TEST_EQ("(1 2 3)", run("(list 1 2 3)"));
      TEST_EQ("(1 2 3 4)", run("(list 1 2 3 4)"));
      TEST_EQ("(1 2 3 4 5)", run("(list 1 2 3 4 5)"));
      TEST_EQ("(1 2 3 4 5 6)", run("(list 1 2 3 4 5 6)"));
      TEST_EQ("(1 2 3 4 5 6 7)", run("(list 1 2 3 4 5 6 7)"));
      TEST_EQ("(1 2 3 4 5 6 7 8)", run("(list 1 2 3 4 5 6 7 8)"));
      TEST_EQ("(1 2 3 4 5 6 7 8 9)", run("(list 1 2 3 4 5 6 7 8 9)"));
      TEST_EQ("(1 2 3 4 5 6 7 8 9 10)", run("(list 1 2 3 4 5 6 7 8 9 10)"));
      TEST_EQ("(1 2 3 4 5 6 7 8 9 10 11)", run("(list 1 2 3 4 5 6 7 8 9 10 11)"));
      TEST_EQ("(1 2 3 4 5 6 7 8 9 10 11 12)", run("(list 1 2 3 4 5 6 7 8 9 10 11 12)"));
      }
    };
  
  struct scheme_tests : public compile_fixture {
    void test()
      {
      build_string_to_symbol();
      TEST_EQ("(testing 1 (2) -3.14e+159)", run("(quote (testing 1 (2.0) -3.14e159))"));
      TEST_EQ("4", run("(+ 2 2)"));
      TEST_EQ("210", run("(+ (* 2 100) (* 1 10))"));
      TEST_EQ("2", run("(if (> 6 5) (+ 1 1) (+ 2 2))"));
      TEST_EQ("4", run("(if (< 6 5) (+ 1 1) (+ 2 2))"));
      TEST_EQ("3", run("(define x 3)"));
      TEST_EQ("3", run("x"));
      TEST_EQ("6", run("(+ x x)"));
      TEST_EQ("3", run("(begin (define x 1) (set! x (+ x 1)) (+ x 1))"));
      TEST_EQ("10", run("((lambda (x) (+ x x)) 5)"));
      TEST_EQ("<lambda>", run("(define twice (lambda (x) (* 2 x)))"));
      TEST_EQ("10", run("(twice 5)"));
      TEST_EQ("<lambda>", run("(define cube (lambda (x) (* x x x)))"));
      TEST_EQ("125", run("(cube 5)"));
      TEST_EQ("<lambda>", run("(define compose (lambda (f g) (lambda (x) (f (g x)))))"));
      TEST_EQ("<lambda>", run("(define repeat (lambda (f) (compose f f)))"));
      TEST_EQ("20", run("((repeat twice) 5)"));
      TEST_EQ("80", run("((repeat (repeat twice)) 5)"));
      TEST_EQ("120", run("(let ([x 5]) (define fact (lambda (n) (if (<= n 1) 1 (* n (fact (- n 1)))))) (fact x))"));
      TEST_EQ("<lambda>", run("(define fact (lambda (n) (if (<= n 1) 1 (* n (fact (- n 1))))))"));
      TEST_EQ("6", run("(fact 3)"));
      TEST_EQ("479001600", run("(fact 12)"));
      TEST_EQ("<lambda>", run("(define make_list (lambda (n) (list n)))"));
      TEST_EQ("(10)", run("((compose make_list twice) 5)"));
      TEST_EQ("<lambda>", run("(define abs (lambda (n) (if (> n 0) n (- 0 n))))"));
      TEST_EQ("(3 0 3)", run("(list (abs -3) (abs 0) (abs 3))"));
      TEST_EQ("<lambda>", run("(define make_cons (lambda (m n) (cons m n)))"));
      TEST_EQ("(1 . 2)", run("(make_cons 1 2)"));

      TEST_EQ("<lambda>", run("(define combine (lambda (f) (lambda (x y) (if (null? x) (quote ()) (f (list (car x) (car y)) ((combine f) (cdr x) (cdr y)))))))"));
      TEST_EQ("<lambda>", run("(define zip (combine make_cons))"));
      TEST_EQ("((1 5) (2 6) (3 7) (4 8))", run("(zip (list 1 2 3 4) (list 5 6 7 8))"));
      }
    };
    
  struct scheme_tests_part_b : public compile_fixture {
    void test()
      {
      run("(define append (lambda (l m)(if (null? l) m (cons(car l) (append(cdr l) m)))))");
      TEST_EQ("<lambda>", run("(define combine (lambda (f) (lambda (x y) (if (null? x) (quote ()) (f (list (car x) (car y)) ((combine f) (cdr x) (cdr y)))))))"));
      TEST_EQ("<lambda>", run("(define riff-shuffle (lambda (deck) (begin (define take (lambda (n seq) (if (<= n 0) (quote ()) (cons (car seq) (take (- n 1) (cdr seq)))))) (define drop (lambda (n seq) (if (<= n 0) seq (drop (- n 1) (cdr seq))))) (define mid (lambda (seq) (/ (length seq) 2))) ((combine append) (take (mid deck) deck) (drop (mid deck) deck)))))"));
      TEST_EQ("(1 2 3 4 5 6 7 8)", run("(cons 1 (cons 2 (cons 3 (cons 4 (cons 5 (cons 6 (cons 7 (cons 8 ()))))))))"));
      TEST_EQ("(1 5 2 6 3 7 4 8)", run("(riff-shuffle (list 1 2 3 4 5 6 7 8))"));
      TEST_EQ("(1 2 3 4 5 6 7 8)", run("(riff-shuffle (riff-shuffle (riff-shuffle (list 1 2 3 4 5 6 7 8))))"));
      }
    };

  struct scheme_tests_part_c : public compile_fixture {
    void test()
      {
      run("(define append (lambda (l m)(if (null? l) m (cons(car l) (append(cdr l) m)))))");
      TEST_EQ("<lambda>", run("(define combine (lambda (f) (lambda (x y) (if (null? x) (quote ()) (f (list (car x) (car y)) ((combine f) (cdr x) (cdr y)))))))"));
      TEST_EQ("<lambda>", run("(define riff-shuffle (lambda (deck) (begin (define take (lambda (n seq) (if (<= n 0) (quote ()) (cons (car seq) (take (- n 1) (cdr seq)))))) (define drop (lambda (n seq) (if (<= n 0) seq (drop (- n 1) (cdr seq))))) (define mid (lambda (seq) (/ (length seq) 2))) ((combine append) (take (mid deck) deck) (drop (mid deck) deck)))))"));
      TEST_EQ("<lambda>", run("(define compose (lambda (f g) (lambda (x) (f (g x)))))"));
      TEST_EQ("<lambda>", run("(define repeat (lambda (f) (compose f f)))"));
      TEST_EQ("(1 3 5 7 2 4 6 8)", run("((repeat riff-shuffle) (list 1 2 3 4 5 6 7 8))"));
      }
    };

  struct scheme_tests_2 : public compile_fixture {
    void test()
      {
      TEST_EQ("<lambda>", run("(define multiply-by (lambda (n) (lambda (y) (* y n))))"));
      TEST_EQ("<lambda>", run("(define doubler (multiply-by 2))"));
      TEST_EQ("<lambda>", run("(define tripler (multiply-by 3))"));
      TEST_EQ("8", run("(doubler 4)"));
      TEST_EQ("12", run("(tripler 4)"));
      TEST_EQ("9", run("(doubler 4.5)"));
      TEST_EQ("13.5", run("(tripler 4.5)"));
      }
    };

  struct scheme_tests_3 : public compile_fixture {
    void test()
      {
      TEST_EQ("<lambda>", run("(define count-down-from (lambda (n) (lambda () (set! n (- n 1)))))"));
      TEST_EQ("<lambda>", run("(define count-down-from-3  (count-down-from 3))"));
      TEST_EQ("<lambda>", run("(define count-down-from-4  (count-down-from 4))"));
      TEST_EQ("2", run("(count-down-from-3)"));
      TEST_EQ("3", run("(count-down-from-4)"));
      TEST_EQ("1", run("(count-down-from-3)"));
      TEST_EQ("0", run("(count-down-from-3)"));
      TEST_EQ("2", run("(count-down-from-4)"));
      TEST_EQ("1", run("(count-down-from-4)"));
      TEST_EQ("0", run("(count-down-from-4)"));
      TEST_EQ("-1", run("(count-down-from-4)"));
      TEST_EQ("-1", run("(count-down-from-3)"));
      }
    };

  struct scheme_tests_4 : public compile_fixture {
    void test()
      {
      TEST_EQ("0", run("(define set-hidden 0)"));
      TEST_EQ("0", run("(define get-hidden 0)"));
      TEST_EQ("<lambda>", run("((lambda () (begin (define hidden 0) (set! set-hidden (lambda (n) (set! hidden n))) (set! get-hidden (lambda () hidden)))))"));
      /*
      std::string script = "((lambda () (begin (define hidden 0) (set! set-hidden (lambda (n) (set! hidden n))) (set! get-hidden (lambda () hidden)))))";

      ops.do_cps_conversion = true;
      ops.do_assignable_variables_conversion = true;
      ops.do_global_define_env_allocation = false;
      ops.do_free_variables_analysis = false;
      ops.do_closure_conversion = false;
      ops.do_tail_call_analysis = false;
      ops.do_linear_scan_indices_computation = false;
      ops.do_linear_scan = false;
      ops.do_alpha_conversion = true;
      auto prog = get_program(script);
      SKIWI::dump(std::cout, prog);
      */
      TEST_EQ("0", run("(get-hidden)"));
      TEST_EQ("1234", run("(set-hidden 1234)"));
      TEST_EQ("1234", run("(get-hidden)"));
      }
    };

  struct and_or : public compile_fixture {
    void test()
      {
      TEST_EQ("#t", run("(and) "));
      TEST_EQ("5", run("(and 5) "));
      TEST_EQ("#f", run("(and #f) "));
      TEST_EQ("6", run("(and 5 6) "));
      TEST_EQ("#f", run("(and #f ((lambda(x) (x x)) (lambda(x) (x x)))) "));
      TEST_EQ("#f", run("(or ) "));
      TEST_EQ("#t", run("(or #t) "));
      TEST_EQ("5", run("(or 5) "));
      TEST_EQ("1", run("(or 1 2 3) "));
      TEST_EQ("(1 . 2)", run("(or (cons 1 2) ((lambda(x) (x x)) (lambda(x) (x x)))) "));
      TEST_EQ("12", run("(let([i 12]) (or i 17)) "));
      TEST_EQ("17", run("(let([i 12]) (and i 17)) "));
      TEST_EQ("8", run("(let([l 8]) (or l 18)) "));
      TEST_EQ("18", run("(let([l 8]) (and l 18)) "));
      TEST_EQ("2", run("(let([t 1]) (and (begin(set! t (add1 t)) t) t)) "));
      TEST_EQ("2", run("(let([t 1]) (or (begin(set! t (add1 t)) t) t)) "));
      }
    };

  struct vectors : public compile_fixture {
    void test()
      {
      TEST_EQ("#t", run("(vector? (make-vector 0)) "));
      TEST_EQ("12", run("(vector-length(make-vector 12)) "));
      TEST_EQ("#f", run("(vector? (cons 1 2)) "));
      TEST_EQ("#f", run("(vector? 1287) "));
      TEST_EQ("#f", run("(vector? ()) "));
      TEST_EQ("#f", run("(vector? #t) "));
      TEST_EQ("#f", run("(vector? #f) "));
      TEST_EQ("#f", run("(pair? (make-vector 12)) "));
      TEST_EQ("#f", run("(null? (make-vector 12)) "));
      TEST_EQ("#f", run("(boolean? (make-vector 12)) "));
      TEST_EQ("#()", run("(make-vector 0) "));
      TEST_EQ("#(3.14 3.14 3.14 3.14 3.14)", run("(make-vector 5 3.14) "));
      TEST_EQ("12", run("(vector-ref (make-vector 5 12) 0)"));
      TEST_EQ("12", run("(vector-ref (make-vector 5 12) 1)"));
      TEST_EQ("12", run("(vector-ref (make-vector 5 12) 2)"));
      TEST_EQ("12", run("(vector-ref (make-vector 5 12) 3)"));
      TEST_EQ("12", run("(vector-ref (make-vector 5 12) 4)"));

      TEST_EQ("#(5 3.14 (3 . 2))", run("(vector 5 3.14 (cons 3 2)) "));

      TEST_EQ("#(#t #f)", run("(let([v(make-vector 2)]) (vector-set! v 0 #t)(vector-set! v 1 #f) v) "));

      TEST_EQ("(#(100 200) . #(300 400))", run("(let([v0(make-vector 2)])(let([v1(make-vector 2)]) (vector-set! v0 0 100) (vector-set! v0 1 200)  (vector-set! v1 0 300)  (vector-set! v1 1 400) (cons v0 v1))) "));
      TEST_EQ("(#(100 200 150) . #(300 400 350))", run("(let([v0(make-vector 3)]) (let([v1(make-vector 3)]) (vector-set! v0 0 100)  (vector-set! v0 1 200)  (vector-set! v0 2 150) (vector-set! v1 0 300)  (vector-set! v1 1 400)  (vector-set! v1 2 350)  (cons v0 v1))) "));
      TEST_EQ("(#(100 200) . #(300 400))", run("(let([n 2])  (let([v0(make-vector n)]) (let([v1(make-vector n)])  (vector-set! v0 0 100) (vector-set! v0 1 200)  (vector-set! v1 0 300) (vector-set! v1 1 400)  (cons v0 v1)))) "));
      TEST_EQ("(#(100 200 150) . #(300 400 350))", run("(let([n 3]) (let([v0(make-vector n)]) (let([v1(make-vector(vector-length v0))]) (vector-set! v0(- (vector-length v0) 3) 100) (vector-set! v0(- (vector-length v1) 2) 200) (vector-set! v0(- (vector-length v0) 1) 150) (vector-set! v1(- (vector-length v1) 3) 300)  (vector-set! v1(- (vector-length v0) 2) 400)  (vector-set! v1(- (vector-length v1) 1) 350)  (cons v0 v1)))) "));
      TEST_EQ("1", run("(let([n 1]) (vector-set! (make-vector n) (sub1 n) (* n n)) n) "));
      TEST_EQ("1", run("(let([n 1])(let([v(make-vector 1)]) (vector-set! v(sub1 n) n) (vector-ref  v(sub1 n)))) "));
      TEST_EQ("(#(2) . #(13))", run("(let([v0(make-vector 1)]) (vector-set! v0 0 1) (let([v1(make-vector 1)]) (vector-set! v1 0 13)  (vector-set! (if (vector? v0) v0 v1) (sub1(vector-length(if (vector? v0) v0 v1)))  (add1(vector-ref  (if (vector? v0) v0 v1)  (sub1(vector-length(if (vector? v0) v0 v1))))))  (cons v0 v1))) "));
      TEST_EQ("100", run("(letrec([f (lambda(v i) (if (>= i 0) (begin (vector-set! v i i) (f v (sub1 i))) v))])(let([ v (make-vector 100) ]) (vector-length (f v (sub1 100)))))"));

      TEST_EQ("#t", run("(let([v(make-vector 2)]) (vector-set! v 0 v) (vector-set! v 1 v) (eq? (vector-ref  v 0) (vector-ref  v 1))) "));
      TEST_EQ("((1 . 2) . #t)", run("(let([v(make-vector 1)][y(cons 1 2)]) (vector-set! v 0 y) (cons y(eq? y(vector-ref  v 0)))) "));
      TEST_EQ("(3 . 3)", run("(letrec([f (lambda(v i) (if (>= i 0) (f v (sub1 i)) v))])(let([ v (cons 3 3) ]) (f v 3)))"));

      TEST_EQ("#(#undefined #undefined #undefined #undefined #undefined #undefined #undefined #undefined #undefined #undefined)", run("(let ([a (make-vector 10)] [b (make-vector 10)] [c (make-vector 10)]) a)"));
      }
    };

  struct strings : public compile_fixture {
    void test()
      {
      TEST_EQ("#t", run("(string? (make-string 3)) "));
      TEST_EQ("#f", run("(string? (make-vector 3)) "));

      TEST_EQ("\"aaaaa\"", run(R"((make-string 5 #\a))"));
      TEST_EQ("\"bbbbbb\"", run(R"((make-string 6 #\b))"));
      TEST_EQ("\"ccccccc\"", run(R"((make-string 7 #\c))"));
      TEST_EQ("\"dddddddd\"", run(R"((make-string 8 #\d))"));
      TEST_EQ("\"eeeeeeeee\"", run(R"((make-string 9 #\e))"));
      TEST_EQ("\"ffffffffff\"", run(R"((make-string 10 #\f))"));
      TEST_EQ("\"ggggggggggg\"", run(R"((make-string 11 #\g))"));
      TEST_EQ("\"hhhhhhhhhhhh\"", run(R"((make-string 12 #\h))"));
      TEST_EQ("\"iiiiiiiiiiiii\"", run(R"((make-string 13 #\i))"));
      TEST_EQ("\"jjjjjjjjjjjjjj\"", run(R"((make-string 14 #\j))"));
      TEST_EQ("\"kkkkkkkkkkkkkkk\"", run(R"((make-string 15 #\k))"));
      TEST_EQ("\"llllllllllllllll\"", run(R"((make-string 16 #\l))"));
      TEST_EQ("\"mmmmmmmmmmmmmmmmm\"", run(R"((make-string 17 #\m))"));

      TEST_EQ("\"jan\"", run(R"((string #\j #\a #\n))"));
      TEST_EQ("\"janneman\"", run(R"((string #\j #\a #\n #\n #\e #\m #\a #\n))"));
      TEST_EQ("\"jannemanneke\"", run(R"((string #\j #\a #\n #\n #\e #\m #\a #\n #\n #\e #\k #\e))"));
      TEST_EQ("\"jannemannekejannemanneke\"", run(R"((string #\j #\a #\n #\n #\e #\m #\a #\n #\n #\e #\k #\e #\j #\a #\n #\n #\e #\m #\a #\n #\n #\e #\k #\e))"));

      TEST_EQ("0", run("(string-length (make-string 0 #\\a))"));
      TEST_EQ("1", run("(string-length (make-string 1 #\\a))"));
      TEST_EQ("2", run("(string-length (make-string 2 #\\a))"));
      TEST_EQ("3", run("(string-length (make-string 3 #\\a))"));
      TEST_EQ("4", run("(string-length (make-string 4 #\\a))"));
      TEST_EQ("5", run("(string-length (make-string 5 #\\a))"));
      TEST_EQ("6", run("(string-length (make-string 6 #\\a))"));
      TEST_EQ("7", run("(string-length (make-string 7 #\\a))"));
      TEST_EQ("8", run("(string-length (make-string 8 #\\a))"));
      TEST_EQ("9", run("(string-length (make-string 9 #\\a))"));
      TEST_EQ("10", run("(string-length (make-string 10 #\\a))"));

      TEST_EQ("0", run("(string-length (make-string 0))"));
      TEST_EQ("1", run("(string-length (make-string 1))"));
      TEST_EQ("2", run("(string-length (make-string 2))"));
      TEST_EQ("3", run("(string-length (make-string 3))"));
      TEST_EQ("4", run("(string-length (make-string 4))"));
      TEST_EQ("5", run("(string-length (make-string 5))"));
      TEST_EQ("6", run("(string-length (make-string 6))"));
      TEST_EQ("7", run("(string-length (make-string 7))"));
      TEST_EQ("8", run("(string-length (make-string 8))"));
      TEST_EQ("9", run("(string-length (make-string 9))"));
      TEST_EQ("10", run("(string-length (make-string 10))"));


      TEST_EQ("#\\j", run(R"((string-ref (string #\j #\a #\n #\n #\e #\m #\a #\n) 0))"));
      TEST_EQ("#\\a", run(R"((string-ref (string #\j #\a #\n #\n #\e #\m #\a #\n) 1))"));
      TEST_EQ("#\\n", run(R"((string-ref (string #\j #\a #\n #\n #\e #\m #\a #\n) 2))"));
      TEST_EQ("#\\000", run(R"((string-ref (string #\j #\a #\n #\n #\e #\m #\a #\n) 9))"));

      TEST_EQ("#\\j", run(R"((string-ref (string #\j #\a #\n) 0))"));
      TEST_EQ("#\\a", run(R"((string-ref (string #\j #\a #\n) 1))"));
      TEST_EQ("#\\n", run(R"((string-ref (string #\j #\a #\n) 2))"));
      TEST_EQ("#\\000", run(R"((string-ref (string #\j #\a #\n) 4))"));
      TEST_EQ("runtime error: string-ref: out of bounds", run(R"((string-ref (string #\j #\a #\n) 9))"));

      TEST_EQ("\"abcde\"", run(R"((let([s (make-string 5 #\a)]) (string-set! s 1 #\b)  (string-set! s 2 #\c) (string-set! s 3 #\d) (string-set! s 4 #\e) s))"));
      TEST_EQ(R"(#\c)", run(R"((let([s (make-string 5 #\a)]) (string-set! s 1 #\b)  (string-set! s 2 #\c) (string-set! s 3 #\d) (string-set! s 4 #\e) (string-ref s 2)))"));
      TEST_EQ("\"\"", run(R"((make-string 0) )"));
      TEST_EQ("#\\a", run(R"((let([s(make-string 1)])(string-set! s 0 #\a) (string-ref s 0)) )"));
      TEST_EQ("(#\\a . #\\b)", run(R"((let([s(make-string 2)]) (string-set! s 0 #\a) (string-set! s 1 #\b) (cons(string-ref s 0) (string-ref s 1))) )"));
      TEST_EQ("#\\a", run(R"((let([i 0])(let([s(make-string 1)])(string-set! s i #\a) (string-ref s i))) )"));
      TEST_EQ("(#\\a . #\\b)", run(R"((let([i 0][j 1])(let([s(make-string 2)])(string-set! s i #\a)  (string-set! s j #\b)  (cons(string-ref s i) (string-ref s j)))) )"));
      TEST_EQ("#\\a", run(R"((let([i 0][c #\a])(let([s(make-string 1)])(string-set! s i c) (string-ref s i))) )"));

      TEST_EQ("\"jan\"", run(R"((string #\j #\a #\n))"));
      TEST_EQ("\"janneman\"", run(R"((string #\j #\a #\n #\n #\e #\m #\a #\n))"));

      TEST_EQ("\"jan\"", run(R"(("jan"))"));
      TEST_EQ("\"janneman\"", run(R"(("janneman"))"));

      TEST_EQ("12", run(R"((string-length(make-string 12)) )"));
      TEST_EQ("#f", run(R"((string? (make-vector 12)) )"));
      TEST_EQ("#f", run(R"((string? (cons 1 2)) )"));
      TEST_EQ("#f", run(R"((string? 1287) )"));
      TEST_EQ("#f", run(R"((string? ()) )"));
      TEST_EQ("#f", run(R"((string? #t) )"));
      TEST_EQ("#f", run(R"((string? #f) )"));
      TEST_EQ("#f", run(R"((pair? (make-string 12)) )"));
      TEST_EQ("#f", run(R"((null? (make-string 12)) )"));
      TEST_EQ("#f", run(R"((boolean? (make-string 12)) )"));
      TEST_EQ("#f", run(R"((vector? (make-string 12)) )"));
      TEST_EQ("\"\"", run(R"((make-string 0) )"));

      TEST_EQ("\"tf\"", run(R"((let([v(make-string 2)])(string-set! v 0 #\t)(string-set! v 1 #\f) v) )"));
      TEST_EQ("#t", run(R"((let([v(make-string 2)]) (string-set! v 0 #\x) (string-set! v 1 #\x) (char=? (string-ref v 0) (string-ref v 1))) )"));
      TEST_EQ("(\"abc\" . \"def\")", run(R"((let([v0(make-string 3)]) (let([v1(make-string 3)]) (string-set! v0 0 #\a) (string-set! v0 1 #\b)(string-set! v0 2 #\c) (string-set! v1 0 #\d) (string-set! v1 1 #\e) (string-set! v1 2 #\f) (cons v0 v1))) )"));
      TEST_EQ("(\"ab\" . \"cd\")", run(R"((let([n 2])(let([v0(make-string n)])(let([v1(make-string n)])(string-set! v0 0 #\a)(string-set! v0 1 #\b)(string-set! v1 0 #\c)(string-set! v1 1 #\d)(cons v0 v1)))) )"));
      TEST_EQ("(\"abc\" . \"ZYX\")", run(R"((let([n 3])(let([v0(make-string n)])(let([v1(make-string(string-length v0))])(string-set! v0(- (string-length v0) 3) #\a)(string-set! v0(- (string-length v1) 2) #\b)(string-set! v0(- (string-length v0) 1) #\c)(string-set! v1(- (string-length v1) 3) #\Z)(string-set! v1(- (string-length v0) 2) #\Y)(string-set! v1(- (string-length v1) 1) #\X)(cons v0 v1)))) )"));
      TEST_EQ("1", run(R"((let([n 1])(string-set! (make-string n) (sub1 n) (fixnum->char 34)) n) )"));
      TEST_EQ("1", run(R"((let([n 1]) (let([v(make-string 1)]) (string-set! v(sub1 n) (fixnum->char n)) (char->fixnum(string-ref v(sub1 n))))) )"));
      TEST_EQ("(\"b\" . \"A\")", run(R"((let([v0(make-string 1)]) (string-set! v0 0 #\a) (let([v1(make-string 1)]) (string-set! v1 0 #\A) (string-set! (if (string? v0) v0 v1) (sub1(string-length(if (string? v0) v0 v1))) (fixnum->char (add1 (char->fixnum (string-ref (if (string? v0) v0 v1)(sub1(string-length(if (string? v0) v0 v1)))))))) (cons v0 v1))) )"));
      TEST_EQ(R"(""")", run(R"((let([s(make-string 1)]) (string-set! s 0 #\") s) )"));
      TEST_EQ("\"\\\"", run(R"((let([s(make-string 1)]) (string-set! s 0 #\\) s) )"));
      }
    };

  struct length : public compile_fixture {
    void test()
      {
      TEST_EQ("0", run("(length ())"));
      TEST_EQ("1", run("(length (list 8))"));
      TEST_EQ("2", run("(length (list 8 9))"));
      TEST_EQ("3", run("(length (list 8 9 10))"));
      TEST_EQ("8", run("(length (list 8 9 10 11 12 13 14 15))"));
      }
    };
  struct append : public compile_fixture {
    void test()
      {
      run("(define append (lambda (l m)(if (null? l) m (cons(car l) (append(cdr l) m)))))");
      TEST_EQ("(1 2 3 4)", run("(append (list 1 2) (list 3 4))"));
      TEST_EQ("(1 2 3 4)", run("(append '(1 2) '(3 4))"));
      }
    };

  struct quote_bug : public compile_fixture {
    void test()
      {
      // bug: quote was assigned but never executed fix: move quote construction to begin of program with quote_conversion
      TEST_EQ("3", run("(if #f (quote ()) 3)"));
      TEST_EQ("()", run("(quote ())"));
      }
    };

  struct quote : public compile_fixture {
    void test()
      {
      build_string_to_symbol();
      TEST_EQ("1", run("(quote 1)"));
      TEST_EQ("1", run("(quote 1)"));
      TEST_EQ("1.3", run("(quote 1.3)"));
      TEST_EQ("\"Jan\"", run("(quote \"Jan\")"));
      TEST_EQ("#t", run("(quote #t)"));
      TEST_EQ("#f", run("(quote #f)"));
      TEST_EQ("a", run("(begin (quote a) (quote ()) (quote a))"));
      TEST_EQ("a", run("(begin (quote a) (quote ()) (quote a))"));
      TEST_EQ("(1 2)", run("(quote (1 2))"));
      TEST_EQ("#(1 2 3.14 #t)", run("(quote #(1 2 3.14 #t))"));

      TEST_EQ("42", run("quote 42 "));
      TEST_EQ("(1 . 2)", run("quote (1 . 2) "));
      TEST_EQ("(1 2 3)", run("quote (1 2 3) "));
      TEST_EQ("(1 2 3)", run("quote (1 2 3) "));
      TEST_EQ("(1 2 3)", run("(let([x quote (1 2 3)]) x) "));
      TEST_EQ("(1 2 3)", run("(let([f(lambda() quote (1 2 3))]) (f)) "));
      TEST_EQ("#t", run("(let([f(lambda() quote (1 2 3))]) (eq? (f)(f))) "));
      TEST_EQ("(1 2 3)", run("(let([f(lambda() (lambda()  quote (1 2 3)))]) ((f))) "));
      TEST_EQ("(#(1 2 3) . 1)", run("(let([x quote #(1 2 3)]) (cons x(vector-ref x 0))) "));
      TEST_EQ("\"Hello World\"", run("\"Hello World\" "));
      TEST_EQ("(\"Hello\" \"World\")", run("quote (\"Hello\" \"World\") "));

      TEST_EQ("42", run("'42 "));
      TEST_EQ("(1 . 2)", run("'(1 . 2) "));
      TEST_EQ("(1 2 3)", run("'(1 2 3) "));
      TEST_EQ("(1 2 3)", run("(let([x '(1 2 3)]) x) "));
      TEST_EQ("(1 2 3)", run("(let([f(lambda() '(1 2 3))]) (f)) "));
      TEST_EQ("#t", run("(let([f(lambda() '(1 2 3))]) (eq? (f)(f))) "));
      TEST_EQ("(1 2 3)", run("(let([f(lambda() (lambda()  '(1 2 3)))]) ((f))) "));
      TEST_EQ("(#(1 2 3) . 1)", run("(let([x '#(1 2 3)]) (cons x(vector-ref x 0))) "));
      TEST_EQ("\"Hello World\"", run("\"Hello World\" "));
      TEST_EQ("(\"Hello\" \"World\")", run("'(\"Hello\" \"World\") "));

      TEST_EQ("#\\a", run("(quote #\\a)"));
      TEST_EQ("(a b c)", run("(quote (a b c))"));
      }
    };


  struct fibonacci : public compile_fixture {
    void test()
      {
      ops.garbage_collection = false;
      run("(define fib (lambda (n) (cond [(< n 2) 1]  [else (+ (fib (- n 2)) (fib(- n 1)))])))");
      TEST_EQ("34", run("(fib 8)"));
      TEST_EQ("55", run("(fib 9)"));
      TEST_EQ("89", run("(fib 10)"));
      //TEST_EQ("165580141", run("(fib 40)"));
      TEST_EQ("runtime error: closure: heap overflow", run("(fib 40)"));
      }
    };

  struct set_car_cdr : public compile_fixture {
    void test()
      {
      TEST_EQ("(1)", run("(let ([x (cons 1 2)])(begin(set-cdr! x ()) x))"));
      TEST_EQ("(1)", run("(let ([x (cons 1 2)]) (set-cdr! x ()) x)"));
      TEST_EQ("(12 14 . 15)", run("(let ([x (cons 12 13)] [y (cons 14 15)])  (set-cdr! x y) x)"));
      TEST_EQ("(14 12 . 13)", run("(let ([x (cons 12 13)] [y (cons 14 15)]) (set-cdr! y x) y)"));
      TEST_EQ("(12 . 13)", run("(let ([x (cons 12 13)] [y (cons 14 15)])(set-cdr! y x)x)"));
      TEST_EQ("(14 . 15)", run("(let ([x (cons 12 13)] [y (cons 14 15)]) (set-cdr! x y) y)"));
      TEST_EQ("(#t . #f)", run("(let ([x (let ([x (cons 1 2)]) (set-car! x #t) (set-cdr! x #f) x)]) (cons x x) x)"));
      TEST_EQ("(#t . #t)", run("(let ([x (cons 1 2)]) (set-cdr! x x)  (set-car! (cdr x) x) (cons(eq? x(car x)) (eq? x(cdr x))))"));
      TEST_EQ("#f", run("(let ([x #f])(if (pair? x) (set-car! x 12) #f)  x)"));
      }
    };

  struct when_unless : public compile_fixture {
    void test()
      {
      TEST_EQ("(3 . 2)", run("(let([x(cons 1 2)]) (when(pair? x) (set-car! x(+ (car x) (cdr x)))) x) "));
      TEST_EQ("(5 . 2)", run("(let([x(cons 1 2)]) (when(pair? x) (set-car! x(+ (car x) (cdr x))) (set-car! x(+ (car x) (cdr x)))) x) "));
      TEST_EQ("(3 . 2)", run("(let([x(cons 1 2)]) (unless(fixnum? x) (set-car! x(+ (car x) (cdr x)))) x) "));
      TEST_EQ("(5 . 2)", run("(let([x(cons 1 2)]) (unless(fixnum? x) (set-car! x(+ (car x) (cdr x))) (set-car! x(+ (car x) (cdr x)))) x) "));
      }
    };

  struct fixnum_to_char : public compile_fixture {
    void test()
      {
      TEST_EQ(R"(#\019)", run("(fixnum->char 19)"));
      TEST_EQ(R"(#\a)", run("(fixnum->char 97)"));

      TEST_EQ(R"(#\A)", run("(fixnum->char 65)"));
      TEST_EQ(R"(#\z)", run("(fixnum->char 122)"));
      TEST_EQ(R"(#\Z)", run("(fixnum->char 90)"));
      TEST_EQ(R"(#\0)", run("(fixnum->char 48)"));
      TEST_EQ(R"(#\9)", run("(fixnum->char 57)"));
      }
    };

  struct char_to_fixnum : public compile_fixture {
    void test()
      {
      TEST_EQ("19", run(R"((char->fixnum #\019))"));
      TEST_EQ("97", run(R"((char->fixnum #\a))"));

      TEST_EQ("65", run(R"((char->fixnum #\A))"));
      TEST_EQ("122", run(R"((char->fixnum #\z))"));
      TEST_EQ("90", run(R"((char->fixnum #\Z))"));
      TEST_EQ("48", run(R"((char->fixnum #\0))"));
      TEST_EQ("57", run(R"((char->fixnum #\9))"));

      TEST_EQ("12", run(R"((char->fixnum (fixnum->char 12)))"));
      TEST_EQ(R"(#\x)", run(R"((fixnum->char (char->fixnum #\x)))"));
      }
    };

  struct fxlog : public compile_fixture {
    void test()
      {
      TEST_EQ("6", run("(bitwise-not -7)"));
      TEST_EQ("6", run("(bitwise-not (bitwise-or (bitwise-not 7) 1))"));
      TEST_EQ("2", run("(bitwise-not (bitwise-or (bitwise-not 7) (bitwise-not 2)))"));
      TEST_EQ("12", run("(bitwise-and (bitwise-not (bitwise-not 12)) (bitwise-not (bitwise-not 12)))"));
      TEST_EQ("19", run("(bitwise-or 3 16)"));
      TEST_EQ("7", run("(bitwise-or 3 5)"));
      TEST_EQ("7", run("(bitwise-or 3 7)"));
      TEST_EQ("6", run("(bitwise-not (bitwise-or (bitwise-not 7) 1))"));
      TEST_EQ("6", run("(bitwise-not (bitwise-or 1 (bitwise-not 7)))"));
      TEST_EQ("3", run("(bitwise-and 3 7)"));
      TEST_EQ("1", run("(bitwise-and 3 5)"));

      TEST_EQ("0", run("(bitwise-and 2346 (bitwise-not 2346))"));
      TEST_EQ("0", run("(bitwise-and (bitwise-not 2346) 2346)"));
      TEST_EQ("2376", run("(bitwise-and 2376 2376)"));
      }
    };

  struct symbols : public compile_fixture {
    void test()
      {
      build_string_to_symbol();
      TEST_EQ("#t", run("(symbol? 'foo) "));
      TEST_EQ("#f", run("(symbol? '()) "));
      TEST_EQ("#f", run("(symbol? "") "));
      TEST_EQ("#f", run("(symbol? '(1 2)) "));
      TEST_EQ("#f", run("(symbol? '#()) "));
      TEST_EQ("#f", run("(symbol? (lambda(x) x)) "));
      TEST_EQ("#t", run("(symbol? 'foo) "));
      TEST_EQ("#f", run("(string? 'foo) "));
      TEST_EQ("#f", run("(pair? 'foo) "));
      TEST_EQ("#f", run("(vector? 'foo) "));
      TEST_EQ("#f", run("(null? 'foo) "));
      TEST_EQ("#f", run("(boolean? 'foo) "));
      TEST_EQ("#f", run("(procedure? 'foo) "));
      TEST_EQ("#f", run("(eq? 'foo 'bar) "));
      TEST_EQ("#t", run("(eq? 'foo 'foo) "));
      TEST_EQ("foo", run("'foo "));
      TEST_EQ("(foo bar baz)", run("'(foo bar baz) "));
      TEST_EQ("(foo foo foo foo foo foo foo foo foo foo foo)", run("'(foo foo foo foo foo foo foo foo foo foo foo)  "));
      }
    };

  struct applying_thunks : public compile_fixture {
    void test()
      {
      TEST_EQ("12", run("(let([f(lambda() 12)]) (f)) "));
      TEST_EQ("25", run("(let([f(lambda() (+ 12 13))]) (f)) "));
      TEST_EQ("26", run("(let([f(lambda() 13)]) (+ (f)(f))) "));
      TEST_EQ("50", run("(let([f(lambda()(let([g(lambda() (+ 2 3))])(* (g)(g))))])(+ (f)(f))) "));
      TEST_EQ("50", run("(let([f(lambda()(let([f(lambda() (+ 2 3))])(* (f)(f))))]) (+ (f)(f))) "));
      TEST_EQ("14", run("(let([f(if (boolean? (lambda() 12))(lambda() 13)(lambda() 14))])(f)) "));
      }
    };
  struct parameter_passing : public compile_fixture {
    void test()
      {
      TEST_EQ("12", run("(let([f(lambda(x) x)]) (f 12)) "));
      TEST_EQ("25", run("(let([f(lambda(x y) (+ x y))]) (f 12 13)) "));
      TEST_EQ("1100", run("(let([f(lambda(x)(let([g(lambda(x y) (+ x y))])(g x 100)))])(f 1000)) "));
      TEST_EQ("26", run("(let([f(lambda(g) (g 2 13))])(f(lambda(n m) (* n m)))) "));
      TEST_EQ("10100", run("(let([f(lambda(g) (+ (g 10) (g 100)))])(f(lambda(x) (* x x)))) "));
      TEST_EQ("120", run("(let([f(lambda(f n m)(if (zero? n) m (f f(sub1 n) (* n m))))]) (f f 5 1)) "));
      TEST_EQ("120", run("(let([f(lambda(f n)(if (zero? n) 1 (* n(f f(sub1 n)))))]) (f f 5)) "));
      }
    };
  struct scheme_tests_with_primitives_as_object : public compile_fixture {
    void test()
      {
      run("(define fib (lambda (n) (cond [(< n 2) 1]  [else (+ (fib (- n 2)) (fib(- n 1)))])))");
      TEST_EQ("3", run(R"((define op (lambda (a b c) (a b c)))   (op + 1 2))"));
      TEST_EQ("7", run(R"((define op (lambda (a b c) (a b c)))  (define three (lambda () 3)) (define four (lambda () 4)) (op + (three) (four)))"));
      TEST_EQ("7", run(R"((define op (lambda (a b c) (a (b) (c))))  (define three (lambda () 3)) (define four (lambda () 4)) (op + three four))"));
      TEST_EQ("8", run(R"((define op (lambda (a b c) (a b c)))  (op + (fib 3) (fib 4)))"));
      TEST_EQ("<lambda>", run("(define twice (lambda (x) (* 2 x)))"));
      TEST_EQ("10", run("(twice 5)"));
      TEST_EQ("<lambda>", run("(define compose (lambda (f g) (lambda (x) (f (g x)))))"));
      TEST_EQ("(10)", run("((compose list twice) 5)"));
      TEST_EQ("<lambda>", run("(define abs (lambda (n) ((if (> n 0) + -) 0 n)))"));
      TEST_EQ("(3 0 3)", run("(list (abs -3) (abs 0) (abs 3))"));
      }
    };
  struct cond : public compile_fixture {
    void test()
      {
      build_string_to_symbol();
      TEST_EQ("2", run("(cond[1 2][else 3]) "));
      TEST_EQ("1", run("(cond[1][else 13]) "));
      TEST_EQ("#f", run("(cond[#f #t][#t #f]) "));
      TEST_EQ("17", run("(cond[else 17]) "));
      TEST_EQ("13", run("(cond[#f][#f 12][12 13]) "));
      TEST_EQ("1287", run("(let([b #t])(cond [else 1287])) "));
      TEST_EQ("2", run("(cond[(cons 1 2) => (lambda(x) (cdr x))]) "));

      TEST_EQ("yes", run("(if (> 3 2) 'yes 'no)"));
      TEST_EQ("no", run("(if (> 2 3) 'yes 'no)"));
      TEST_EQ("1", run("(if (> 3 2) (- 3 2)(+ 3 2))"));
      TEST_EQ("greater", run("(cond [(> 3 2) 'greater] [(< 3 2) 'less])"));
      TEST_EQ("less", run("(cond [(> 2 3) 'greater] [(< 2 3) 'less])"));
      TEST_EQ("equal", run("(cond [(> 3 3) 'greater] [(< 3 3) 'less] [else 'equal])"));
      }
    };

  struct newton : public compile_fixture {
    void test()
      {
      TEST_EQ("<lambda>", run("(define abs (lambda (n) ((if (> n 0) + -) 0 n)))"));
      TEST_EQ("<lambda>", run("(define newton lambda(guess function derivative epsilon) (define guess2 (- guess (/ (function guess) (derivative guess)))) (if (< (abs(- guess guess2)) epsilon) guess2 (newton guess2 function derivative epsilon)))"));
      TEST_EQ("<lambda>", run("(define square-root lambda(a) (newton 1 (lambda(x) (-(* x x) a)) (lambda(x) (* 2 x)) 1e-8))"));
      TEST_EQ("14.1421", run("(square-root 200.)"));
      }
    };

  struct callcc : public compile_fixture {
    void test()
      {
      ops.do_cps_conversion = false;
      //TEST_EQ("runtime error: invalid program termination", run("(define call/cc (lambda(k f) (f k (lambda(dummy-k result) (k result)))))"));
      TEST_EQ("<lambda>", run("(define call/cc (lambda(k f) (f k (lambda(dummy-k result) (k result)))))"));
      ops.do_cps_conversion = true;
      TEST_EQ("1", run("(call/cc (lambda(throw) (+ 5 (* 10 (throw 1)))))"));
      TEST_EQ("15", run("(call/cc (lambda(throw) (+ 5 (* 10 1))))"));

      TEST_EQ("35", run("(call/cc(lambda(throw) (+ 5 (* 10 (call/cc(lambda(escape) (* 100 (escape 3))))))))"));
      TEST_EQ("3", run("(call/cc(lambda(throw) (+ 5 (* 10 (call/cc(lambda(escape) (* 100 (throw 3))))))))"));
      TEST_EQ("1005", run("(call/cc(lambda(throw) (+ 5 (* 10 (call/cc(lambda(escape) (* 100 1)))))))"));
      }
    };

  struct heap_overflow : public compile_fixture {
    void test()
      {
      ops.garbage_collection = false;
      ops.safe_cons = true;
      ops.safe_flonums = true;
      ops.primitives_inlined = false;
      ops.do_constant_propagation = false;
      uint64_t global_stack_space = 512;
      make_new_context(64, global_stack_space, 64, 128);
      TEST_EQ("#(#undefined)", run("(make-vector 1)"));
      TEST_EQ("runtime error: make-vector: heap overflow", run("(make-vector 1000000000)"));
      for (int i = 0; i < 3; ++i)
        TEST_EQ("15", run("(let ([a 1] [b 2] [c 3] [d 4] [e 5]) ((lambda () (+ a b c d e))))"));
      TEST_EQ("runtime error: closure: heap overflow", run("(let ([a 1] [b 2] [c 3] [d 4] [e 5]) ((lambda () (+ a b c d e))))"));
      make_new_context(64, global_stack_space, 64, 128);
      for (int i = 0; i < 5; ++i)
        TEST_EQ("#(1 2 3 4 5)", run("(vector 1 2 3 4 5)"));
      TEST_EQ("runtime error: vector: heap overflow", run("(vector 1 2 3 4 5)"));
      make_new_context(64, global_stack_space, 64, 128);
      for (int i = 0; i < 5; ++i)
        TEST_EQ(R"("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")", run("(make-string (+ (* 8 4) 1) #\\a)"));
      TEST_EQ("runtime error: make-string: heap overflow", run("(make-string (+ (* 8 4) 1) #\\a)"));

      make_new_context(64, global_stack_space, 64, 128);
      for (int i = 0; i < 5; ++i)
        TEST_EQ(R"("abcdefghabcdefghabcdefghabcdefgha")", run("(string #\\a #\\b #\\c #\\d #\\e #\\f #\\g #\\h #\\a #\\b #\\c #\\d #\\e #\\f #\\g #\\h #\\a #\\b #\\c #\\d #\\e #\\f #\\g #\\h #\\a #\\b #\\c #\\d #\\e #\\f #\\g #\\h #\\a)"));
      TEST_EQ("runtime error: string: heap overflow", run("(string #\\a #\\b #\\c #\\d #\\e #\\f #\\g #\\h #\\a #\\b #\\c #\\d #\\e #\\f #\\g #\\h #\\a #\\b #\\c #\\d #\\e #\\f #\\g #\\h #\\a #\\b #\\c #\\d #\\e #\\f #\\g #\\h #\\a)"));
      make_new_context(64, global_stack_space, 64, 128);
      for (int i = 0; i < 5; ++i)
        TEST_EQ(R"("abcdefghabcdefghabcdefghabcdefgha")", run("\"abcdefghabcdefghabcdefghabcdefgha\""));
      TEST_EQ("runtime error: string: heap overflow", run("\"abcdefghabcdefghabcdefghabcdefgha\""));

      make_new_context(64, global_stack_space, 64, 128);
      TEST_EQ("(1 2 3 4 5 6 7 8 9 10)", run("(list 1 2 3 4 5 6 7 8 9 10)"));
      TEST_EQ("runtime error: list: heap overflow", run("(list 1 2)"));

      make_new_context(64, global_stack_space, 64, 128);
      TEST_EQ("(1 2 3 4 5 6 7 8 9 10)", run("(cons 1 (cons 2 (cons 3 (cons 4 (cons 5 (cons 6 (cons 7 (cons 8 (cons 9 (cons 10 ()))))))))))"));
      TEST_EQ("runtime error: cons: heap overflow", run("(cons 1 2)"));

      make_new_context((256 + 32) * 2, global_stack_space, 64, 128); // at least 256 because of the symbol table
      build_string_to_symbol();
      TEST_EQ("abcdefghabcdefghabcdefghabcdefgh1", run("(quote abcdefghabcdefghabcdefghabcdefgh1)"));
      TEST_EQ("runtime error: string: heap overflow", run("(quote abcdefghabcdefghabcdefghabcdefgh2)"));
      TEST_EQ("runtime error: closure: heap overflow", run("(quote abcdefghabcdefghabcdefghabcdefgh3)"));
      TEST_EQ("runtime error: closure: heap overflow", run("(quote abcdefghabcdefghabcdefghabcdefgh4)"));
      TEST_EQ("runtime error: closure: heap overflow", run("(quote abcdefghabcdefghabcdefghabcdefgh5)"));
      TEST_EQ("runtime error: closure: heap overflow", run("(quote abcdefghabcdefghabcdefghabcdefgh6)"));

      make_new_context(8, global_stack_space, 64, 128);
      TEST_EQ("2.5", run("(2.5)"));
      TEST_EQ("runtime error: flonum: heap overflow", run("(2.5)"));
      }
    };

  struct no_heap_overflow_because_gc : public compile_fixture {
    void test()
      {
      ops.garbage_collection = true;
      ops.safe_cons = true;
      ops.safe_flonums = true;
      make_new_context(64, 512, 64, 128);
      TEST_EQ("#(#undefined)", run("(make-vector 1)"));
      TEST_EQ("runtime error: make-vector: heap overflow", run("(make-vector 1000000000)"));
      for (int i = 0; i < 3; ++i)
        TEST_EQ("15", run("(let ([a 1] [b 2] [c 3] [d 4] [e 5]) ((lambda () (+ a b c d e))))"));
      TEST_EQ("15", run("(let ([a 1] [b 2] [c 3] [d 4] [e 5]) ((lambda () (+ a b c d e))))"));
      make_new_context(64, 512, 64, 128);
      for (int i = 0; i < 5; ++i)
        TEST_EQ("#(1 2 3 4 5)", run("(vector 1 2 3 4 5)"));
      TEST_EQ("#(1 2 3 4 5)", run("(vector 1 2 3 4 5)"));
      make_new_context(64, 512, 64, 128);
      for (int i = 0; i < 5; ++i)
        TEST_EQ(R"("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")", run("(make-string (+ (* 8 4) 1) #\\a)"));
      TEST_EQ(R"("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")", run("(make-string (+ (* 8 4) 1) #\\a)"));

      make_new_context(64, 512, 64, 128);
      for (int i = 0; i < 5; ++i)
        TEST_EQ(R"("abcdefghabcdefghabcdefghabcdefgha")", run("(string #\\a #\\b #\\c #\\d #\\e #\\f #\\g #\\h #\\a #\\b #\\c #\\d #\\e #\\f #\\g #\\h #\\a #\\b #\\c #\\d #\\e #\\f #\\g #\\h #\\a #\\b #\\c #\\d #\\e #\\f #\\g #\\h #\\a)"));
      TEST_EQ(R"("abcdefghabcdefghabcdefghabcdefgha")", run("(string #\\a #\\b #\\c #\\d #\\e #\\f #\\g #\\h #\\a #\\b #\\c #\\d #\\e #\\f #\\g #\\h #\\a #\\b #\\c #\\d #\\e #\\f #\\g #\\h #\\a #\\b #\\c #\\d #\\e #\\f #\\g #\\h #\\a)"));
      make_new_context(64, 512, 64, 128);
      for (int i = 0; i < 5; ++i)
        TEST_EQ(R"("abcdefghabcdefghabcdefghabcdefgha")", run("\"abcdefghabcdefghabcdefghabcdefgha\""));
      TEST_EQ(R"("abcdefghabcdefghabcdefghabcdefgha")", run("\"abcdefghabcdefghabcdefghabcdefgha\""));
      }
    };

  struct gctest : public compile_fixture {
    void test()
      {
      make_new_context(64, 512, 64, 128);


      for (int i = 0; i < 5; ++i)
        run("(vector 1 2 3 4 5)");
      run("(reclaim)");
      for (int i = 0; i < 5; ++i)
        run("(vector 1 2 3 4 5)");
      run("(reclaim)");
      for (int i = 0; i < 5; ++i)
        run("(vector 1 2 3 4 5)");
      run("(reclaim)");

      make_new_context(64, 512, 64, 128);
      TEST_EQ("#(1 1 1 1 1)", run("(define a (vector 1 1 1 1 1))"));
      TEST_EQ("#(2 2 2 2 2)", run("(define b (vector 2 2 2 2 2))"));
      TEST_EQ("#(1 2 3 4 5)", run("(vector 1 2 3 4 5)"));
      TEST_EQ("#(1 2 3 4 5)", run("(vector 1 2 3 4 5)"));
      TEST_EQ("#(1 2 3 4 5)", run("(vector 1 2 3 4 5)"));
      run("(reclaim)");
      TEST_EQ("#(1 1 1 1 1)", run("a"));
      TEST_EQ("#(2 2 2 2 2)", run("b"));

      make_new_context(64 * 2, 512, 64, 128);
      TEST_ASSERT(ctxt.stack == ctxt.stack_top);
      for (int i = 0; i < 30; ++i)
        TEST_EQ("15", run("(let ([a 1] [b 2] [c 3] [d 4] [e 5]) ((lambda () (+ a b c d e))))"));
      TEST_ASSERT(ctxt.stack == ctxt.stack_top);
      }
    };

  struct gc_fib : public compile_fixture {
    void test()
      {
      make_new_context(64 * 8, 512, 64, 128);
      //run("(define fib (lambda (n) (cond [(< n 2) 1]  [else (+ (fib (- n 2)) (fib(- n 1)))])))");
      run("(define fib (lambda (n) (if (< n 2) 1  (+ (fib (- n 2)) (fib(- n 1))))))");
      TEST_EQ("34", run("(fib 8)"));
      TEST_EQ("55", run("(fib 9)"));
      TEST_EQ("89", run("(fib 10)"));
      //TEST_EQ("165580141", run("(fib 40)"));
      }
    };

  struct gc_overflow : public compile_fixture {
    void test()
      {
      make_new_context(1024 * 2, 1024, 1024, 1024);
      TEST_EQ("100", run("(letrec([f (lambda(i) (when (<= i 1000) (let([x (make-vector 1000)]) (f(add1 i)))))]) (f 0) 100)"));
      TEST_EQ("100", run("(letrec([f (lambda(i)  (when (<= i 100000) (let([x (list 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)]) (f(add1 i)))))]) (f 0) 100)"));
      }
    };

  struct fib_perf_test : public compile_fixture {
    void test()
      {
      ops.primitives_inlined = true;
      ops.safe_primitives = false;
      ops.standard_bindings = true;

      run("(define fib (lambda (n) (cond [(fx<? n 2) 1]  [else (fx+ (fib (fx- n 2)) (fib(fx- n 1)))]))) ");
      //run("(define fib (lambda (n) (cond [(< n 2) 1]  [else (+ (fib (- n 2)) (fib(- n 1)))]))) ");
      auto tic = std::clock();
      TEST_EQ("121393", run("(fib 25)"));
      //TEST_EQ("165580141", run("(define fib (lambda (n) (cond [(< n 2) 1]  [else (+ (fib (- n 2)) (fib(- n 1)))]))) (fib 40)"));
      auto toc = std::clock();
      std::cout << "fibonacci: " << (toc - tic) << "ms\n";

      //ops.safe_primitives = true;
      //ops.standard_bindings = false;
      //dump("(define FIB (lambda (n) (cond [(fx<? n 2) 1]  [else (fx+ (FIB (fx- n 2)) (FIB(fx- n 1)))])))");
      //expand_and_format("(define fib (lambda (n) (cond [(< n 2) 1]  [else (+ (fib (- n 2)) (fib(- n 1)))]))) ");

      //expand_and_format("(define FIB (lambda (n) (cond [(fx<? n 2) 1]  [else (fx+ (FIB (fx- n 2)) (FIB(fx- n 1)))]))) ");
      }
    };
  struct primitive_of_2_args_inlined : public compile_fixture {
    void test()
      {
      build_string_to_symbol();
      ops.primitives_inlined = true;
      TEST_EQ("5", run("(+ 3 2)"));
      TEST_EQ("5.2", run("(+ 3.2 2)"));
      TEST_EQ("5.5", run("(+ 3.2 2.3)"));
      TEST_EQ("5.1", run("(+ 3 2.1)"));

      TEST_EQ("1", run("(- 3 2)"));
      TEST_EQ("1.2", run("(- 3.2 2)"));
      TEST_EQ("0.9", run("(- 3.2 2.3)"));
      TEST_EQ("0.9", run("(- 3 2.1)"));

      TEST_EQ("6", run("(* 3 2)"));
      TEST_EQ("6.4", run("(* 3.2 2)"));
      TEST_EQ("6.9", run("(* 3.0 2.3)"));
      TEST_EQ("6.3", run("(* 3 2.1)"));

      TEST_EQ("4", run("(/ 8 2)"));
      TEST_EQ("1.6", run("(/ 3.2 2)"));
      TEST_EQ("1.5", run("(/ 3.0 2.0)"));
      TEST_EQ("1.5", run("(/ 3 2.0)"));

      TEST_EQ("#f", run("(< 3 2.1)"));
      TEST_EQ("#t", run("(< 3.1 8)"));
      TEST_EQ("#f", run("(< 3.2 2.1)"));
      TEST_EQ("#t", run("(< 1 21)"));

      TEST_EQ("#f", run("(<= 3 2.1)"));
      TEST_EQ("#t", run("(<= 3.1 8)"));
      TEST_EQ("#f", run("(<= 3.2 2.1)"));
      TEST_EQ("#t", run("(<= 1 21)"));
      TEST_EQ("#t", run("(<= 2 2.0)"));
      TEST_EQ("#t", run("(<= 3.0 3)"));
      TEST_EQ("#t", run("(<= 3.2 3.2)"));
      TEST_EQ("#t", run("(<= 1 1)"));

      TEST_EQ("#t", run("(> 3 2.1)"));
      TEST_EQ("#f", run("(> 3.1 8)"));
      TEST_EQ("#t", run("(> 3.2 2.1)"));
      TEST_EQ("#f", run("(> 1 21)"));

      TEST_EQ("#t", run("(>= 3 2.1)"));
      TEST_EQ("#f", run("(>= 3.1 8)"));
      TEST_EQ("#t", run("(>= 3.2 2.1)"));
      TEST_EQ("#f", run("(>= 1 21)"));
      TEST_EQ("#t", run("(>= 2 2.0)"));
      TEST_EQ("#t", run("(>= 3.0 3)"));
      TEST_EQ("#t", run("(>= 3.2 3.2)"));
      TEST_EQ("#t", run("(>= 1 1)"));

      TEST_EQ("#f", run("(= 12 13)"));
      TEST_EQ("#t", run("(= 12 12)"));
      TEST_EQ("#f", run("(= 12.1 13.1)"));
      TEST_EQ("#t", run("(= 12.1 12.1)"));
      TEST_EQ("#f", run("(= 12 13.1)"));
      TEST_EQ("#t", run("(= 12 12.0)"));
      TEST_EQ("#f", run("(= 12.0 13)"));
      TEST_EQ("#t", run("(= 12.0 12)"));

      TEST_EQ("#t", run("(!= 12 13)"));
      TEST_EQ("#f", run("(!= 12 12)"));
      TEST_EQ("#t", run("(!= 12.1 13.1)"));
      TEST_EQ("#f", run("(!= 12.1 12.1)"));
      TEST_EQ("#t", run("(!= 12 13.1)"));
      TEST_EQ("#f", run("(!= 12 12.0)"));
      TEST_EQ("#t", run("(!= 12.0 13)"));
      TEST_EQ("#f", run("(!= 12.0 12)"));

      TEST_EQ("#f", run("(eq? 'foo 'bar) "));
      TEST_EQ("#t", run("(eq? 'foo 'foo) "));

      TEST_EQ("0", run("(+)"));
      TEST_EQ("1", run("(*)"));
      TEST_EQ("0", run("(-)"));
      TEST_EQ("1", run("(/)"));
      TEST_EQ("3", run("(+ 3)"));
      TEST_EQ("-3", run("(- 3)"));
      TEST_EQ("5", run("(* 5)"));
      TEST_EQ("0.2", run("(/ 1 5)"));
      TEST_EQ("0.1", run("(/ 1 5 2)"));
      TEST_EQ("0.2", run("(/ 5)"));
      TEST_EQ("0.2", run("(/ 5.0)"));
      }
    };

  struct lambda_variable_arity_not_using_rest_arg : public compile_fixture {
    void test()
      {
      TEST_EQ("12", run("(let([f(lambda args 12)]) (f)) "));
      TEST_EQ("12", run("(let([f(lambda args 12)]) (f 10)) "));
      TEST_EQ("12", run("(let([f(lambda args 12)]) (f 10 20)) "));
      TEST_EQ("12", run("(let([f(lambda args 12)]) (f 10 20 30)) "));
      TEST_EQ("12", run("(let([f(lambda args 12)]) (f 10 20 30 40)) "));
      TEST_EQ("12", run("(let([f(lambda args 12)]) (f 10 20 30 40 50)) "));
      TEST_EQ("12", run("(let([f(lambda args 12)]) (f 10 20 30 40 50 60 70 80 90)) "));
      TEST_EQ("12", run("(let([f(lambda (a0 . args) 12)]) (f 10)) "));
      TEST_EQ("10", run("(let([f(lambda (a0 . args) a0)]) (f 10)) "));
      TEST_EQ("12", run("(let([f(lambda (a0 . args) 12)]) (f 10 20)) "));
      TEST_EQ("10", run("(let([f(lambda (a0 . args) a0)]) (f 10 20)) "));
      TEST_EQ("12", run("(let([f(lambda (a0 . args) 12)]) (f 10 20 30)) "));
      TEST_EQ("10", run("(let([f(lambda (a0 . args) a0)]) (f 10 20 30)) "));
      TEST_EQ("12", run("(let([f(lambda (a0 . args) 12)]) (f 10 20 30 40)) "));
      TEST_EQ("10", run("(let([f(lambda (a0 . args) a0)]) (f 10 20 30 40)) "));
      TEST_EQ("#(10 20)", run("(let([f(lambda(a0 a1 . args) (vector a0 a1))]) (f 10 20 30 40 50 60 70 80 90 100)) "));
      TEST_EQ("#(10 20 30)", run("(let([f(lambda(a0 a1 a2 . args) (vector a0 a1 a2))]) (f 10 20 30 40 50 60 70 80 90 100)) "));
      TEST_EQ("#(10 20 30 40)", run("(let([f(lambda(a0 a1 a2 a3 . args) (vector a0 a1 a2 a3))]) (f 10 20 30 40 50 60 70 80 90 100)) "));
      TEST_EQ("#(10 20 30 40 50)", run("(let([f(lambda(a0 a1 a2 a3 a4 . args) (vector a0 a1 a2 a3 a4))]) (f 10 20 30 40 50 60 70 80 90 100)) "));
      TEST_EQ("#(10 20 30 40 50 60)", run("(let([f(lambda(a0 a1 a2 a3 a4 a5 . args) (vector a0 a1 a2 a3 a4 a5))]) (f 10 20 30 40 50 60 70 80 90 100)) "));
      }
    };
  struct lambda_variable_arity_while_using_rest_arg : public compile_fixture {
    void test()
      {
      TEST_EQ("()", run("(let([f(lambda args args)]) (f)) "));
      TEST_EQ("(10)", run("(let([f(lambda args args)]) (f 10)) "));
      TEST_EQ("(10 20)", run("(let([f(lambda args args)]) (f 10 20)) "));
      TEST_EQ("(10 20 30)", run("(let([f(lambda args args)]) (f 10 20 30)) "));
      TEST_EQ("(10 20 30 40)", run("(let([f(lambda args args)]) (f 10 20 30 40)) "));
      TEST_EQ("(10 20 30 40 50)", run("(let([f(lambda args args)]) (f 10 20 30 40 50)) "));
      TEST_EQ("(10 20 30 40 50 60)", run("(let([f(lambda args args)]) (f 10 20 30 40 50 60)) "));
      TEST_EQ("(10 20 30 40 50 60 70)", run("(let([f(lambda args args)]) (f 10 20 30 40 50 60 70)) "));
      TEST_EQ("#(10 ())", run("(let([f(lambda(a0 . args) (vector a0 args))]) (f 10)) "));
      TEST_EQ("#(10 (20))", run("(let([f(lambda(a0 . args) (vector a0 args))]) (f 10 20)) "));
      TEST_EQ("#(10 (20 30))", run("(let([f(lambda(a0 . args) (vector a0 args))]) (f 10 20 30)) "));
      TEST_EQ("#(10 (20 30 40))", run("(let([f(lambda(a0 . args) (vector a0 args))]) (f 10 20 30 40)) "));
      TEST_EQ("#(10 20 (30 40 50 60 70 80 90))", run("(let([f(lambda(a0 a1 . args) (vector a0 a1 args))]) (f 10 20 30 40 50 60 70 80 90)) "));
      TEST_EQ("#(10 20 30 (40 50 60 70 80 90))", run("(let([f(lambda(a0 a1 a2 . args) (vector a0 a1 a2 args))]) (f 10 20 30 40 50 60 70 80 90)) "));
      TEST_EQ("#(10 20 30 40 (50 60 70 80 90))", run("(let([f(lambda(a0 a1 a2 a3 . args) (vector a0 a1 a2 a3 args))]) (f 10 20 30 40 50 60 70 80 90)) "));
      TEST_EQ("#(10 20 30 40 50 (60 70 80 90))", run("(let([f(lambda(a0 a1 a2 a3 a4 . args) (vector a0 a1 a2 a3 a4 args))]) (f 10 20 30 40 50 60 70 80 90)) "));
      TEST_EQ("#(10 20 30 40 50 60 (70 80 90))", run("(let([f(lambda(a0 a1 a2 a3 a4 a5 . args)(vector a0 a1 a2 a3 a4 a5 args))]) (f 10 20 30 40 50 60 70 80 90)) "));
      TEST_EQ("#(10 20 30 40 50 60 70 (80 90))", run("(let([f(lambda(a0 a1 a2 a3 a4 a5 a6 . args)(vector a0 a1 a2 a3 a4 a5 a6 args))]) (f 10 20 30 40 50 60 70 80 90)) "));
      TEST_EQ("#(10 20 30 40 50 60 70 80 (90))", run("(let([f(lambda(a0 a1 a2 a3 a4 a5 a6 a7 . args)(vector a0 a1 a2 a3 a4 a5 a6 a7 args))]) (f 10 20 30 40 50 60 70 80 90)) "));
      }
    };

  struct lambda_bug : public compile_fixture {
    void test()
      {
      TEST_EQ("#(10 20 30 40 50 60 70 80 (90))", run("(let([f(lambda(a0 a1 a2 a3 a4 a5 a6 a7 a8)(vector a0 a1 a2 a3 a4 a5 a6 a7 (list a8)))]) (f 10 20 30 40 50 60 70 80 90)) "));
      TEST_EQ("#(10 20 30 40 50 60 70 80 90)", run("((lambda(a0 a1 a2 a3 a4 a5 a6 a7 a8)(vector a0 a1 a2 a3 a4 a5 a6 a7 a8)) 10 20 30 40 50 60 70 80 90) "));
      TEST_EQ("450", run("((lambda(a0 a1 a2 a3 a4 a5 a6 a7 a8) (+ a0 a1 a2 a3 a4 a5 a6 a7 a8)) 10 20 30 40 50 60 70 80 90) "));
      }
    };

  struct lambda_variable_arity_while_using_rest_arg_and_closure : public compile_fixture {
    void test()
      {
      TEST_EQ("112", run("(let ([n 12])(let([f(lambda(m . args) (+ n m))])(f 100 5 2 8)))"));
      TEST_EQ("112", run("(let ([n 12])(let([f(lambda(m . args) (+ n m))])(f 100 5)))"));
      TEST_EQ("112", run("(let ([n 12])(let([f(lambda(m . args) (+ n m))])(f 100 ())))"));
      TEST_EQ("112", run("(let ([n 12])(let([f(lambda(m . args) (+ n m))])(f 100)))"));
      TEST_EQ("115", run("(let ([n 12][m 100])(let([f (lambda(a b . args) (+ a b n m))])(f 1 2 5)))"));
      TEST_EQ("115", run("(let ([n 12][m 100])(let([f (lambda(a b . args) (+ a b n m))])(f 1 2)))"));

      TEST_EQ("3", run("(let([f (lambda(a b . args) (+ a b))])(f 1 2 3 4))"));
      TEST_EQ("6", run("(let([f (lambda(a b . args) (+ a b (car args)))])(f 1 2 3 4))"));
      TEST_EQ("10", run("(let([f (lambda(a b . args) (+ a b (car args) (car (cdr args))))])(f 1 2 3 4))"));

      TEST_EQ("322", run("(let ([n 12][m 100][z 200])(let([f (lambda(a b . args) (+ a b z n m (car args) (car (cdr args))))])(f 1 2 3 4)))"));
      TEST_EQ("315", run("(let ([n 12][m 100][z 200])(let([f (lambda(a b . args) (+ a b z n m))])(f 1 2 3 4)))"));
      }
    };

  uint64_t seventeen()
    {
    return 17;
    }


  struct foreign_call_1 : public compile_fixture {
    void test()
      {
      SKIWI::external_function ef;
      ef.name = "seventeen";
      ef.address = (uint64_t)&seventeen;
      ef.return_type = external_function::T_INT64;
      externals[ef.name] = ef;

      convert_externals_to_vm();
  
      TEST_EQ("17", run("(foreign-call seventeen)"));
      }
    };


  int64_t add_two_integers(int64_t a, int64_t b)
    {
    return a + b;
    }


  struct foreign_call_2 : public compile_fixture {
    void test()
      {
      external_function ef;
      ef.name = "add_two_integers";
      ef.address = (uint64_t)&add_two_integers;
      ef.return_type = external_function::T_INT64;
      ef.arguments.push_back(external_function::T_INT64);
      ef.arguments.push_back(external_function::T_INT64);
      externals[ef.name] = ef;

      convert_externals_to_vm();

      TEST_EQ("15", run("(foreign-call add_two_integers 7 8)"));
      }
    };


  double add_two_doubles(double a, double b)
    {
    return a + b;
    }


  struct foreign_call_3 : public compile_fixture {
    void test()
      {
      external_function ef;
      ef.name = "add_two_doubles";
      ef.address = (uint64_t)&add_two_doubles;
      ef.return_type = external_function::T_DOUBLE;
      ef.arguments.push_back(external_function::T_DOUBLE);
      ef.arguments.push_back(external_function::T_DOUBLE);
      externals[ef.name] = ef;

      convert_externals_to_vm();

      TEST_EQ("15", run("(foreign-call add_two_doubles 7.0 8.0)"));
      }
    };

  double add_two_doubles_and_two_integers(double a, double b, int64_t c, int64_t d)
    {
    return a + b + c + d;
    }


  struct foreign_call_4 : public compile_fixture {
    void test()
      {
      external_function ef;
      ef.name = "add_two_doubles_and_two_integers";
      ef.address = (uint64_t)&add_two_doubles_and_two_integers;
      ef.return_type = external_function::T_DOUBLE;
      ef.arguments.push_back(external_function::T_DOUBLE);
      ef.arguments.push_back(external_function::T_DOUBLE);
      ef.arguments.push_back(external_function::T_INT64);
      ef.arguments.push_back(external_function::T_INT64);
      externals[ef.name] = ef;

      convert_externals_to_vm();

      TEST_EQ("18.6", run("(foreign-call add_two_doubles_and_two_integers 7.5 8.1 1 2)"));
      }
    };


  void print_a_text(const char* txt)
    {
    printf("%s\n", txt);
    }



  struct foreign_call_5 : public compile_fixture {
    void test()
      {
      external_function ef;
      ef.name = "print_a_text";
      ef.address = (uint64_t)&print_a_text;
      ef.return_type = external_function::T_VOID;
      ef.arguments.push_back(external_function::T_CHAR_POINTER);
      externals[ef.name] = ef;

      convert_externals_to_vm();

      TEST_EQ("0", run("(foreign-call print_a_text \"Hello world!\")"));
      }
    };

  const char* get_a_text()
    {
    return "This is a text";
    }



  struct foreign_call_6 : public compile_fixture {
    void test()
      {
      external_function ef;
      ef.name = "get_a_text";
      ef.address = (uint64_t)&get_a_text;
      ef.return_type = external_function::T_CHAR_POINTER;
      externals[ef.name] = ef;

      convert_externals_to_vm();

      TEST_EQ("\"This is a text\"", run("(foreign-call get_a_text)"));
      }
    };

  struct foreign_call_7 : public compile_fixture {
    void test()
      {
      external_function ef;
      ef.name = "printf";
      ef.address = (uint64_t)&printf;
      ef.return_type = external_function::T_VOID;
      ef.arguments.push_back(external_function::T_CHAR_POINTER);
      externals[ef.name] = ef;

      convert_externals_to_vm();

      TEST_EQ("0", run("(foreign-call printf \"Hello world!\n\")"));
      }
    };


  int64_t get_a_boolean(bool b)
    {
    if (b)
      return 100;
    else
      return 200;
    }


  struct foreign_call_8 : public compile_fixture {
    void test()
      {
      external_function ef;
      ef.name = "get_a_boolean";
      ef.address = (uint64_t)&get_a_boolean;
      ef.return_type = external_function::T_INT64;
      ef.arguments.push_back(external_function::T_BOOL);
      externals[ef.name] = ef;

      convert_externals_to_vm();

      TEST_EQ("100", run("(foreign-call get_a_boolean #t)"));
      TEST_EQ("200", run("(foreign-call get_a_boolean #f)"));
      TEST_EQ("100", run("(foreign-call get_a_boolean 34859)"));
      }
    };



  bool is_even(int64_t i)
    {
    return (i % 2 == 0);
    }


  struct foreign_call_9 : public compile_fixture {
    void test()
      {
      external_function ef;
      ef.name = "is_even";
      ef.address = (uint64_t)&is_even;
      ef.return_type = external_function::T_BOOL;
      ef.arguments.push_back(external_function::T_INT64);
      externals[ef.name] = ef;

      convert_externals_to_vm();

      TEST_EQ("#f", run("(foreign-call is_even 1)"));
      TEST_EQ("#t", run("(foreign-call is_even 2)"));
      TEST_EQ("#f", run("(foreign-call is_even 3)"));
      TEST_EQ("#t", run("(foreign-call is_even 4)"));
      }
    };

  struct foreign_call_10 : public compile_fixture {
    void test()
      {
      external_function ef;
      ef.name = "add-two-integers";
      ef.address = (uint64_t)&add_two_integers;
      ef.return_type = external_function::T_INT64;
      ef.arguments.push_back(external_function::T_INT64);
      ef.arguments.push_back(external_function::T_INT64);
      externals[ef.name] = ef;

      convert_externals_to_vm();

      run("(define (add-two-integers a b) (foreign-call add-two-integers a b))");
      TEST_EQ("15", run("(add-two-integers 7 8)"));
      }
    };

  void print_scheme_variable(uint64_t i)
    {
    std::shared_ptr<SKIWI::environment<SKIWI::environment_entry>> env;
    repl_data rd;
    scheme_runtime(i, std::cout, env, rd, nullptr);
    }

  struct foreign_call_11 : public compile_fixture {
    void test()
      {
      external_function ef;
      ef.name = "print-scheme-variable";
      ef.address = (uint64_t)&print_scheme_variable;
      ef.return_type = external_function::T_VOID;
      ef.arguments.push_back(external_function::T_SCM);
      externals[ef.name] = ef;

      convert_externals_to_vm();

      run("(define a (vector 1 0 0 0 0 1 0 0 0 0 1 0 20 30 40 1))");
      run("(foreign-call print-scheme-variable a)");
      std::cout << std::endl;
      }
    };

  struct case_examples : public compile_fixture {
    void test()
      {
      build_string_to_symbol();
      TEST_EQ("big", run("(case (+ 7 5) [(1 2 3) 'small] [(10 11 12) 'big])"));
      TEST_EQ("75", run("(case (+ 7 3) [(10 11 12) 75])"));
      TEST_EQ("small", run("(case (- 7 5) [(1 2 3) 'small] [(10 11 12) 'big])"));

      TEST_EQ("composite", run("(case (* 2 3) [(2 3 5 7) 'prime][(1 4 6 8 9) 'composite])"));
      TEST_EQ("#undefined", run("(case (car '(c d)) [(a) 'a] [(b) 'b])"));
      TEST_EQ("consonant", run("(case (car '(c d)) [(a e i o u) 'vowel][(w y) 'semivowel][else 'consonant])"));
      TEST_EQ("vowel", run("(case (car '(e d)) [(a e i o u) 'vowel][(w y) 'semivowel][else 'consonant])"));

      //TEST_EQ("backwards", run("(case (list 'y 'x) [((a b) (x y)) 'forwards]  [((b a) (y x)) 'backwards])")); // this only works if case is rewritten with member instead of memv
      TEST_EQ("5", run("(let ([x 3]) (case x [else 5]))"));

      TEST_EQ("5", run("(let ([x #\\a]) (case x [(#\\newline) 7] [else 5]))"));
      TEST_EQ("5", run("(let ([x #\\a]) (case x [(#\\newline) (char->fixnum x)] [else 5]))"));
      TEST_EQ("10", run("(let ([x #\\newline]) (case x [(#\\newline) (char->fixnum x)] [else 5]))"));
      TEST_EQ("#t", run("(let ([x #\\newline]) (eqv? x #\\newline))"));
      TEST_EQ("(#\\010)", run("(let ([x #\\newline]) (memv x '(#\\newline)))"));
      }
    };

  struct memv : public compile_fixture {
    void test()
      {
      build_string_to_symbol();
      TEST_EQ("(2 3 4)", run("(memv 2 (list 1 2 3 4))"));
      TEST_EQ("#f", run("(memv 9 (list 1 2 3 4))"));
      TEST_EQ("(101 102)", run("(memv 101 '(100 101 102))"));

      TEST_EQ("#f", run("(memv 101 '((b a) (y x)))"));
      TEST_EQ("(a c)", run("(memv 'a '(b a c))"));
      TEST_EQ("#f", run("(memv '(y x) '((b a) (y x)))"));
      }
    };

  struct memq : public compile_fixture {
    void test()
      {
      build_string_to_symbol();
      TEST_EQ("(2 3 4)", run("(memq 2 (list 1 2 3 4))"));
      TEST_EQ("#f", run("(memq 9 (list 1 2 3 4))"));
      TEST_EQ("(101 102)", run("(memq 101 '(100 101 102))"));

      TEST_EQ("#f", run("(memq 101 '((b a) (y x)))"));
      TEST_EQ("(a c)", run("(memq 'a '(b a c))"));
      TEST_EQ("#f", run("(memq '(y x) '((b a) (y x)))"));
      }
    };

  struct member : public compile_fixture {
    void test()
      {
      build_string_to_symbol();
      TEST_EQ("(2 3 4)", run("(member 2 (list 1 2 3 4))"));
      TEST_EQ("#f", run("(member 9 (list 1 2 3 4))"));
      TEST_EQ("(101 102)", run("(member 101 '(100 101 102))"));

      TEST_EQ("#f", run("(member 101 '((b a) (y x)))"));
      TEST_EQ("(a c)", run("(member 'a '(b a c))"));
      TEST_EQ("((y x))", run("(member '(y x) '((b a) (y x)))"));
      }
    };

  struct equality : public compile_fixture {
    void test()
      {
      build_string_to_symbol();
      TEST_EQ("#t", run("(equal? 'yes 'yes)"));
      TEST_EQ("#f", run("(equal? 'yes 'no)"));
      TEST_EQ("#t", run("(equal? (* 6 7) 42)"));
      TEST_EQ("#f", run("(equal? 2 2.0)"));
      TEST_EQ("#t", run("(let ([v (cons 1 2)]) (equal? v v))"));
      TEST_EQ("#t", run("(equal? (cons 1 2) (cons 1 2))"));
      TEST_EQ("#t", run("(equal? (cons (cons 3 4) 2) (cons (cons 3 4) 2))"));
      TEST_EQ("#t", run("(equal? (fixnum->char 955) (fixnum->char 955))"));
      TEST_EQ("#t", run("(equal? (make-string 3 #\\z) (make-string 3 #\\z))"));
      TEST_EQ("#t", run("(equal? #t #t)"));

      TEST_EQ("#t", run("(%eqv? 'yes 'yes)"));
      TEST_EQ("#f", run("(%eqv? 'yes 'no)"));
      TEST_EQ("#t", run("(%eqv? (* 6 7) 42)"));
      TEST_EQ("#f", run("(%eqv? 2 2.0)"));
      TEST_EQ("#t", run("(let ([v (cons 1 2)]) (%eqv? v v))"));
      TEST_EQ("#t", run("(%eqv? (cons 1 2) (cons 1 2))"));
      TEST_EQ("#f", run("(%eqv? (cons (cons 3 4) 2) (cons (cons 3 4) 2))"));
      TEST_EQ("#t", run("(%eqv? (fixnum->char 955) (fixnum->char 955))"));
      TEST_EQ("#t", run("(%eqv? (make-string 3 #\\z) (make-string 3 #\\z))"));
      TEST_EQ("#t", run("(%eqv? #t #t)"));

      TEST_EQ("#t", run("(eq? 'yes 'yes)"));
      TEST_EQ("#f", run("(eq? 'yes 'no)"));
      TEST_EQ("#t", run("(eq? (* 6 7) 42)"));
      TEST_EQ("#f", run("(eq? 2 2.0)"));
      TEST_EQ("#t", run("(let ([v (cons 1 2)]) (eq? v v))"));
      TEST_EQ("#f", run("(eq? (cons 1 2) (cons 1 2))"));
      TEST_EQ("#f", run("(eq? (cons (cons 3 4) 2) (cons (cons 3 4) 2))"));
      TEST_EQ("#t", run("(eq? (fixnum->char 955) (fixnum->char 955))"));
      TEST_EQ("#f", run("(eq? (make-string 3 #\\z) (make-string 3 #\\z))"));
      TEST_EQ("#t", run("(eq? #t #t)"));
      }
    };

  struct equality_inlined : public compile_fixture {
    void test()
      {
      build_string_to_symbol();
      ops.primitives_inlined = true;
      TEST_EQ("#t", run("(equal? 'yes 'yes)"));
      TEST_EQ("#f", run("(equal? 'yes 'no)"));
      TEST_EQ("#t", run("(equal? (* 6 7) 42)"));
      TEST_EQ("#f", run("(equal? 2 2.0)"));
      TEST_EQ("#t", run("(let ([v (cons 1 2)]) (equal? v v))"));
      TEST_EQ("#t", run("(equal? (cons 1 2) (cons 1 2))"));
      TEST_EQ("#t", run("(equal? (cons (cons 3 4) 2) (cons (cons 3 4) 2))"));
      TEST_EQ("#t", run("(equal? (fixnum->char 955) (fixnum->char 955))"));
      TEST_EQ("#t", run("(equal? (make-string 3 #\\z) (make-string 3 #\\z))"));
      TEST_EQ("#t", run("(equal? #t #t)"));

      TEST_EQ("#t", run("(%eqv? 'yes 'yes)"));
      TEST_EQ("#f", run("(%eqv? 'yes 'no)"));
      TEST_EQ("#t", run("(%eqv? (* 6 7) 42)"));
      TEST_EQ("#f", run("(%eqv? 2 2.0)"));
      TEST_EQ("#t", run("(let ([v (cons 1 2)]) (eqv? v v))"));
      TEST_EQ("#t", run("(%eqv? (cons 1 2) (cons 1 2))"));
      TEST_EQ("#f", run("(%eqv? (cons (cons 3 4) 2) (cons (cons 3 4) 2))"));
      TEST_EQ("#t", run("(%eqv? (fixnum->char 955) (fixnum->char 955))"));
      TEST_EQ("#t", run("(%eqv? (make-string 3 #\\z) (make-string 3 #\\z))"));
      TEST_EQ("#t", run("(%eqv? #t #t)"));

      TEST_EQ("#t", run("(eq? 'yes 'yes)"));
      TEST_EQ("#f", run("(eq? 'yes 'no)"));
      TEST_EQ("#t", run("(eq? (* 6 7) 42)"));
      TEST_EQ("#f", run("(eq? 2 2.0)"));
      TEST_EQ("#t", run("(let ([v (cons 1 2)]) (eq? v v))"));
      TEST_EQ("#f", run("(eq? (cons 1 2) (cons 1 2))"));
      TEST_EQ("#f", run("(eq? (cons (cons 3 4) 2) (cons (cons 3 4) 2))"));
      TEST_EQ("#t", run("(eq? (fixnum->char 955) (fixnum->char 955))"));
      TEST_EQ("#f", run("(eq? (make-string 3 #\\z) (make-string 3 #\\z))"));
      TEST_EQ("#t", run("(eq? #t #t)"));
      }
    };

  struct apply : public compile_fixture {
    void test()
      {
      build_string_to_symbol();
      build_apply();
      TEST_EQ("0", run("(apply + ())"));
      TEST_EQ("7", run("(apply + (list 3 4))"));
      TEST_EQ("10", run("(apply + (list 1 2 3 4))"));
      TEST_EQ("10", run("(apply + 1 2 (list 3 4))"));
      TEST_EQ("36", run("(apply + (list 1 2 3 4 5 6 7 8))"));
      TEST_EQ("45", run("(apply + (list 1 2 3 4 5 6 7 8 9))"));
      TEST_EQ("55", run("(apply + (list 1 2 3 4 5 6 7 8 9 10))"));
      TEST_EQ("55", run("(apply + 1 2 3 4 5 6 7 8 9 (list 10))"));

      TEST_EQ("(#(1 2 3 4 5 6 7 8))", run("(cons(apply vector '(1 2 3 4 5 6 7 8)) '())"));
      TEST_EQ("(#(1 2 3 4 5 6 7 8))", run("(cons(apply vector 1 '(2 3 4 5 6 7 8)) '())"));
      TEST_EQ("(#(1 2 3 4 5 6 7 8))", run("(cons(apply vector 1 2 '(3 4 5 6 7 8)) '())"));
      TEST_EQ("(#(1 2 3 4 5 6 7 8))", run("(cons(apply vector 1 2 3 '(4 5 6 7 8)) '())"));
      TEST_EQ("(#(1 2 3 4 5 6 7 8))", run("(cons(apply vector 1 2 3 4 '(5 6 7 8)) '())"));
      TEST_EQ("(#(1 2 3 4 5 6 7 8))", run("(cons(apply vector 1 2 3 4 5 '(6 7 8)) '())"));
      TEST_EQ("(#(1 2 3 4 5 6 7 8))", run("(cons(apply vector 1 2 3 4 5 6 '(7 8)) '())"));
      TEST_EQ("(#(1 2 3 4 5 6 7 8))", run("(cons(apply vector 1 2 3 4 5 6 7 '(8)) '())"));
      TEST_EQ("(#(1 2 3 4 5 6 7 8))", run("(cons(apply vector 1 2 3 4 5 6 7 8 ()) '())"));

      TEST_EQ("13", run("(let([f(lambda() 12)])( + (apply f '()) 1))"));
      TEST_EQ("26", run("(let([f(lambda(x) ( + x 12))]) ( + (apply f 13 '()) 1))"));
      TEST_EQ("26", run("(let([f(lambda(x) ( + x 12))]) ( + (apply f(cons 13 '())) 1))"));
      TEST_EQ("27", run("(let([f(lambda(x y z) ( + x(* y z)))])( + (apply f 12 '(7 2)) 1))"));

      TEST_EQ("12", run("(let([f(lambda() 12)])(apply f '()))"));
      TEST_EQ("25", run("(let([f(lambda(x) ( + x 12))])(apply f 13 '()))"));
      TEST_EQ("25", run("(let([f(lambda(x) ( + x 12))])(apply f(cons 13 '())))"));
      TEST_EQ("26", run("(let([f(lambda(x y z) ( + x(* y z)))])(apply f 12 '(7 2)))"));
      TEST_EQ("#(1 2 3 4 5 6 7 8)", run("(apply vector '(1 2 3 4 5 6 7 8))"));
      TEST_EQ("#(1 2 3 4 5 6 7 8)", run("(apply vector 1 '(2 3 4 5 6 7 8))"));
      TEST_EQ("#(1 2 3 4 5 6 7 8)", run("(apply vector 1 2 '(3 4 5 6 7 8)) "));
      TEST_EQ("#(1 2 3 4 5 6 7 8)", run("(apply vector 1 2 3 '(4 5 6 7 8)) "));
      TEST_EQ("#(1 2 3 4 5 6 7 8)", run("(apply vector 1 2 3 4 '(5 6 7 8)) "));
      TEST_EQ("#(1 2 3 4 5 6 7 8)", run("(apply vector 1 2 3 4 5 '(6 7 8)) "));
      TEST_EQ("#(1 2 3 4 5 6 7 8)", run("(apply vector 1 2 3 4 5 6 '(7 8)) "));
      TEST_EQ("#(1 2 3 4 5 6 7 8)", run("(apply vector 1 2 3 4 5 6 7 '(8)) "));
      TEST_EQ("#(1 2 3 4 5 6 7 8)", run("(apply vector 1 2 3 4 5 6 7 8 ())"));


      TEST_EQ("<lambda>", run("(define compose (lambda(f g)(lambda args(f(apply g args)))))"));
      TEST_EQ("<lambda>", run("(define twice (lambda (x) (* 2 x)))"));
      TEST_EQ("1800", run("((compose twice *) 12 75)"));
      }
    };

  struct fib_iterative_perf_test : public compile_fixture
    {
    void test()
      {
      TEST_EQ("<lambda>", run("(define fib (lambda (n) (define iter (lambda (a b c) (cond [(= c 0) b]  [else (iter (+ a b) a (- c 1))]) ) ) (iter 1 0 n) ))"));
      TEST_EQ("21", run("(fib 8)"));
      TEST_EQ("165580141", run("(fib 41)"));
      TEST_EQ("-4249520595888827205", run("(fib 1000000)"));
      }
    };

  struct make_port_test : public compile_fixture
    {
    void test()
      {
      build_string_to_symbol();
      build_apply();
      build_r5rs();
      TEST_EQ("#t", run("(define default-port (make-port #f \"stdout\" 1 (make-string 1024) 0 1024)) (port? default-port)"));
      TEST_EQ("#f", run("(port? (vector #f \"stdout\" 1 (make-string 1024) 0 1024))"));
      TEST_EQ("", run("(write-char #\\j default-port)"));
      TEST_EQ("", run("(write-char #\\a default-port)"));
      TEST_EQ("", run("(write-char #\\n default-port)"));
      TEST_EQ("36", run("(foreign-call _write 1 \"This is a test of '_write' function\n\" 36)"));
      TEST_EQ("", run("(flush-output-port default-port)"));
      TEST_EQ("", run("(write-char #\\J default-port)"));
      TEST_EQ("", run("(write-char #\\A default-port)"));
      TEST_EQ("", run("(write-char #\\N default-port)"));
      TEST_EQ("", run("(write-char #\\010 default-port)"));
      TEST_EQ("", run("(flush-output-port default-port)"));
      }
    };

  struct make_port2_test : public compile_fixture
    {
    void test()
      {
      build_string_to_symbol();
      build_apply();
      build_r5rs();
      TEST_EQ("#t", run("(define default-port (make-port #f \"stdout\" 1 (make-string 8) 0 8)) (port? default-port)"));
      TEST_EQ("", run("(write-char #\\j default-port)"));
      TEST_EQ("", run("(write-char #\\a default-port)"));
      TEST_EQ("", run("(write-char #\\n default-port)"));
      TEST_EQ("", run("(write-char #\\m default-port)"));
      TEST_EQ("", run("(write-char #\\a default-port)"));
      TEST_EQ("", run("(write-char #\\e default-port)"));
      TEST_EQ("", run("(write-char #\\s default-port)"));
      TEST_EQ("", run("(write-char #\\010 default-port)"));
      TEST_EQ("", run("(write-char #\\! default-port)"));
      }
    };

  struct r5rs_test : public compile_fixture
    {
    void test()
      {
      build_libs();

      TEST_EQ("(1 2 3 4)", run("(append (list 1 2) (list 3 4))"));
      TEST_EQ("(1 2 3 4)", run("(append '(1 2) '(3 4))"));
      TEST_EQ("#t", run("(exact? 3)"));
      TEST_EQ("#f", run("(exact? 3.14)"));
      TEST_EQ("#f", run("(inexact? 3)"));
      TEST_EQ("#t", run("(inexact? 3.14)"));
      TEST_EQ("#t", run("(number? 3)"));
      TEST_EQ("#t", run("(number? 3.14)"));
      TEST_EQ("#f", run("(number? 'a)"));

      TEST_EQ("#t", run("(rational? 5)"));
      TEST_EQ("#t", run("(rational? 5.2)"));
      TEST_EQ("#f", run("(rational? \"test\")"));

      TEST_EQ("#t", run("(positive? 5)"));
      TEST_EQ("#f", run("(negative? 5)"));
      TEST_EQ("#t", run("(positive? 5.2)"));
      TEST_EQ("#f", run("(negative? 5.2)"));

      TEST_EQ("#f", run("(positive? -5)"));
      TEST_EQ("#t", run("(negative? -5)"));
      TEST_EQ("#f", run("(positive? -5.2)"));
      TEST_EQ("#t", run("(negative? -5.2)"));

      TEST_EQ("#f", run("(even? -5)"));
      TEST_EQ("#t", run("(even? -6)"));

      TEST_EQ("#t", run("(odd? -5)"));
      TEST_EQ("#f", run("(odd? -6)"));
      }
    };

  typedef union {
    double d;
    struct {
      uint64_t mantissa : 52;
      uint64_t exponent : 11;
      uint64_t sign : 1;
      } parts;
    struct {
      uint64_t notsign : 63;
      uint64_t sign : 1;
      } parts2;
    } double_cast;

  std::string to_string(int64_t val)
    {
    std::stringstream ss;
    ss << val;
    return ss.str();
    }

  struct ieee745_test : public compile_fixture {
    void test()
      {
      build_libs();
      double_cast d1;
      d1.d = 3.14;

      TEST_EQ(to_string(d1.parts.sign), run("(ieee754-sign 3.14)"));
      TEST_EQ(to_string(d1.parts.exponent), run("(ieee754-exponent 3.14)"));
      TEST_EQ(to_string(d1.parts.mantissa), run("(ieee754-mantissa 3.14)"));
      
      d1.d = -3.14;

      TEST_EQ(to_string(d1.parts.sign), run("(ieee754-sign -3.14)"));
      TEST_EQ(to_string(d1.parts.exponent), run("(ieee754-exponent -3.14)"));
      TEST_EQ(to_string(d1.parts.mantissa), run("(ieee754-mantissa -3.14)"));

      d1.d = 458972348798345098345.0;

      TEST_EQ(to_string(d1.parts.sign), run("(ieee754-sign 458972348798345098345.0)"));
      TEST_EQ(to_string(d1.parts.exponent), run("(ieee754-exponent 458972348798345098345.0)"));
      TEST_EQ(to_string(d1.parts.mantissa), run("(ieee754-mantissa 458972348798345098345.0)"));

      d1.d = 0.15625;

      TEST_EQ(to_string(d1.parts.sign), run("(ieee754-sign 0.15625)"));
      TEST_EQ(to_string(d1.parts.exponent), run("(ieee754-exponent 0.15625)"));
      TEST_EQ(to_string(d1.parts.mantissa), run("(ieee754-mantissa 0.15625)"));

      d1.d = -0.15625;

      TEST_EQ(to_string(d1.parts.sign), run("(ieee754-sign -0.15625)"));
      TEST_EQ(to_string(d1.parts.exponent), run("(ieee754-exponent -0.15625)"));
      TEST_EQ(to_string(d1.parts.mantissa), run("(ieee754-mantissa -0.15625)"));

      TEST_EQ("3.14159", run("(ieee754-pi)"));

      TEST_EQ("3", run("(ieee754-sqrt 9)"));
      TEST_EQ("1.41421", run("(ieee754-sqrt 2.0)"));
      
      TEST_EQ("0.841471", run("(ieee754-sin 1.0)"));
      TEST_EQ("0.841471", run("(ieee754-sin 1)"));
      TEST_EQ("0.909297", run("(ieee754-sin 2.0)"));
      TEST_EQ("0.909297", run("(ieee754-sin 2)"));
      TEST_EQ("1.22465e-16", run("(ieee754-sin (ieee754-pi))"));
      TEST_EQ("1", run("(ieee754-sin (/ (ieee754-pi) 2))"));
      
      TEST_EQ("0.540302", run("(ieee754-cos 1.0)"));
      TEST_EQ("0.540302", run("(ieee754-cos 1)"));

      TEST_EQ("1.55741", run("(ieee754-tan 1.0)"));
      TEST_EQ("1.55741", run("(ieee754-tan 1)"));

      TEST_EQ("1.5708", run("(ieee754-asin 1.0)"));
      TEST_EQ("1.5708", run("(ieee754-asin 1)"));
      
      TEST_EQ("1.0472", run("(ieee754-acos 0.5)"));
      TEST_EQ("0", run("(ieee754-acos 1)"));

      TEST_EQ("0.785398", run("(ieee754-atan1 1.0)"));
      TEST_EQ("0.785398", run("(ieee754-atan1 1)"));

      TEST_EQ("2.07944", run("(ieee754-log 8.0)"));
      TEST_EQ("2.07944", run("(ieee754-log 8)"));

      TEST_EQ("8", run("(ieee754-round 8.0)"));
      TEST_EQ("8", run("(ieee754-round 8.4)"));
      TEST_EQ("8", run("(ieee754-round 7.5)"));
      TEST_EQ("8", run("(ieee754-round 8)"));

      TEST_EQ("8", run("(ieee754-truncate 8.0)"));
      TEST_EQ("8", run("(ieee754-truncate 8.4)"));
      TEST_EQ("7", run("(ieee754-truncate 7.5)"));
      TEST_EQ("8", run("(ieee754-truncate 8)"));
      
      TEST_EQ("8", run("(fixnum-expt 2 3)"));
      TEST_EQ("1", run("(fixnum-expt 2 0)"));
      TEST_EQ("1", run("(fixnum-expt 0 0)"));
      TEST_EQ("1024", run("(fixnum-expt 2 10)"));
      TEST_EQ("16", run("(fixnum-expt 4 2)"));
      
      TEST_EQ("16", run("(flonum-expt 4.0 2.0)"));
      TEST_EQ("2", run("(flonum-expt 4.0 0.5)"));
      TEST_EQ("1.41421", run("(flonum-expt 2.0 0.5)"));
      TEST_EQ("343", run("(flonum-expt 7 3)"));
      TEST_EQ("907.493", run("(flonum-expt 7 3.5)"));
      
      TEST_EQ("1.41421", run("(expt 2.0 0.5)"));
      TEST_EQ("8", run("(expt 2 3)"));
      TEST_EQ("1", run("(expt 2 0)"));
      TEST_EQ("1", run("(expt 0 0)"));
      TEST_EQ("1024", run("(expt 2 10)"));
      TEST_EQ("16", run("(expt 4 2)"));
      TEST_EQ("16", run("(expt 4.0 2.0)"));
      TEST_EQ("2", run("(expt 4.0 0.5)"));
      TEST_EQ("0.5", run("(expt 4.0 -0.5)"));
      TEST_EQ("0.125", run("(expt 2 -3)"));

      TEST_EQ("343", run("(expt 7 3)"));
      TEST_EQ("907.493", run("(expt 7 3.5)"));

      TEST_EQ("2.71828", run("(exp 1)"));
      TEST_EQ("2.71828", run("(exp 1.0)"));
      TEST_EQ("1", run("(exp 0.0)"));
      TEST_EQ("1", run("(exp 0)"));


      TEST_EQ("3", run("(sqrt 9)"));
      TEST_EQ("1.41421", run("(sqrt 2.0)"));

      TEST_EQ("0.841471", run("(sin 1.0)"));
      TEST_EQ("0.841471", run("(sin 1)"));
      TEST_EQ("0.909297", run("(sin 2.0)"));
      TEST_EQ("0.909297", run("(sin 2)"));
      TEST_EQ("1.22461e-16", run("(sin (ieee754-pi))"));
      TEST_EQ("1", run("(sin (/ (ieee754-pi) 2))"));

      TEST_EQ("0.540302", run("(cos 1.0)"));
      TEST_EQ("0.540302", run("(cos 1)"));

      TEST_EQ("1.55741", run("(tan 1.0)"));
      TEST_EQ("1.55741", run("(tan 1)"));

      TEST_EQ("1.5708", run("(asin 1.0)"));
      TEST_EQ("1.5708", run("(asin 1)"));

      TEST_EQ("1.0472", run("(acos 0.5)"));
      TEST_EQ("0", run("(acos 1)"));

      TEST_EQ("0.785398", run("(atan 1.0)"));
      TEST_EQ("0.785398", run("(atan 1)"));

      TEST_EQ("2.07944", run("(log 8.0)"));
      TEST_EQ("2.07944", run("(log 8)"));

      TEST_EQ("8", run("(round 8.0)"));
      TEST_EQ("8", run("(round 8.4)"));
      TEST_EQ("8", run("(round 7.5)"));
      TEST_EQ("8", run("(round 8)"));

      TEST_EQ("0.554307", run("(atan 1.3 2.1)"));
      TEST_EQ("-0.554307", run("(atan -1.3 2.1)"));
      TEST_EQ("2.58729", run("(atan 1.3 -2.1)"));
      TEST_EQ("-2.58729", run("(atan -1.3 -2.1)"));

      TEST_EQ("8", run("(truncate 8.0)"));
      TEST_EQ("8", run("(truncate 8.4)"));
      TEST_EQ("7", run("(truncate 7.5)"));
      TEST_EQ("8", run("(truncate 8)"));
      
      }
    };

  struct minmax_test : public compile_fixture {
    void test()
      {
      build_libs();

      TEST_EQ("1", run("(max 1)"));
      TEST_EQ("2", run("(min 2)"));
      TEST_EQ("1.5", run("(max 1.5)"));
      TEST_EQ("2.2", run("(min 2.2)"));
      TEST_EQ("2", run("(max 1.0 2.0)"));
      TEST_EQ("7.2", run("(max 1.0 2.0 -0.4 7.2 2.3 7.1 -11.0)"));
      TEST_EQ("2", run("(max 1 2.0)"));
      TEST_EQ("2", run("(max 1.0 2)"));
      TEST_EQ("2", run("(max 1 2)"));
      TEST_EQ("7", run("(max 1 2 7 -3 -11 6 3)"));

      TEST_EQ("1", run("(min 1.0 2.0)"));
      TEST_EQ("-11", run("(min 1.0 2.0 -0.4 7.2 2.3 7.1 -11.0)"));
      TEST_EQ("1", run("(min 1 2.0)"));
      TEST_EQ("1", run("(min 1.0 2)"));
      TEST_EQ("1", run("(min 1 2)"));
      TEST_EQ("-11", run("(min 1 2 7 -3 -11 6 3)"));
      TEST_EQ("1024", run("(arithmetic-shift 1 10)"));
      TEST_EQ("31", run("(arithmetic-shift 255 -3)"));

      TEST_EQ("3", run("(quotient 10 3)"));
      TEST_EQ("-3", run("(quotient -10 3)"));
      TEST_EQ("3", run("(quotient 10.0 3)"));
      TEST_EQ("-3", run("(quotient -10.0 3)"));
      TEST_EQ("3", run("(quotient 10.0 3.0)"));
      TEST_EQ("-3", run("(quotient -10.0 3.0)"));
      TEST_EQ("3", run("(quotient 10 3.0)"));
      TEST_EQ("-3", run("(quotient -10 3.0)"));

      TEST_EQ("1", run("(remainder 10 3)"));
      TEST_EQ("-1", run("(remainder -10 3)"));
      TEST_EQ("1", run("(remainder 10.0 3)"));
      TEST_EQ("-1", run("(remainder -10.0 3)"));
      TEST_EQ("1", run("(remainder 10.0 3.0)"));
      TEST_EQ("-1", run("(remainder -10.0 3.0)"));
      TEST_EQ("1", run("(remainder 10 3.0)"));
      TEST_EQ("-1", run("(remainder -10 3.0)"));


      TEST_EQ("#t", run("(let ([a 1324] [b 324]) (= (+ (* (quotient a b) b)  (remainder a b) )  a)  )"));
      TEST_EQ("#t", run("(let ([a 1324] [b -324.0]) (= (+ (* (quotient a b) b)  (remainder a b) )  a)  )"));
      TEST_EQ("-4", run("(let ([a 1324] [b -324.0]) (quotient a b ))"));
      TEST_EQ("28", run("(let ([a 1324] [b -324.0]) (remainder a b ))"));
      TEST_EQ("1324", run("(let ([a 1324] [b -324.0]) (+ (* (quotient a b) b)  (remainder a b) ))"));

      TEST_EQ("1", run("(modulo 10 3)"));
      TEST_EQ("2", run("(modulo -10 3)"));
      TEST_EQ("1", run("(modulo 10.0 3)"));
      TEST_EQ("-1", run("(modulo -10.0 -3)"));
      TEST_EQ("1", run("(modulo 10.0 3.0)"));
      TEST_EQ("2", run("(modulo -10.0 3.0)"));
      TEST_EQ("-2", run("(modulo 10 -3.0)"));
      TEST_EQ("2", run("(modulo -10 3.0)"));

      TEST_EQ("3", run("(abs 3)"));
      TEST_EQ("3.2", run("(abs 3.2)"));
      TEST_EQ("3", run("(abs -3)"));
      TEST_EQ("3.2", run("(abs -3.2)"));
      TEST_EQ("0", run("(abs 0)"));
      TEST_EQ("0", run("(abs 0.0)"));

      TEST_EQ("3", run("(flonum->fixnum 3.143)"));
      TEST_EQ("-3", run("(flonum->fixnum -3.143)"));

      TEST_EQ("3", run("(fixnum->flonum 3)"));
      TEST_EQ("#t", run("(flonum? (fixnum->flonum 3))"));     

      TEST_EQ("3", run("(inexact->exact 3.14)"));
      TEST_EQ("3", run("(inexact->exact 3)"));

      TEST_EQ("3.14", run("(exact->inexact 3.14)"));
      TEST_EQ("3", run("(exact->inexact 3)"));
      TEST_EQ("#t", run("(flonum? (exact->inexact 3))"));
      TEST_EQ("#t", run("(inexact? (exact->inexact 3))"));

      TEST_EQ("#t", run("(finite? 3.14)"));
      TEST_EQ("#t", run("(finite? 2)"));
      TEST_EQ("#f", run("(finite? (/ 1.0 0.0))"));
      TEST_EQ("#f", run("(finite? (/ 0.0 0.0))"));

      TEST_EQ("#f", run("(nan? 3.14)"));
      TEST_EQ("#f", run("(nan? 2)"));
      TEST_EQ("#f", run("(nan? (/ 1.0 0.0))"));
      TEST_EQ("#t", run("(nan? (/ 0.0 0.0))"));

      TEST_EQ("#f", run("(inf? 3.14)"));
      TEST_EQ("#f", run("(inf? 2)"));
      TEST_EQ("#t", run("(inf? (/ 1.0 0.0))"));
      TEST_EQ("#f", run("(inf? (/ 0.0 0.0))"));

#ifdef _WIN32
      TEST_EQ("-nan(ind)", run("(/ 0.0 0.0)"));
#elif defined(unix)
      TEST_EQ("-nan", run("(/ 0.0 0.0)"));
#else
      TEST_EQ("nan", run("(/ 0.0 0.0)"));
#endif

      TEST_EQ("#t", run("(integer? 3)"));
      TEST_EQ("#f", run("(integer? 3.2)"));
      TEST_EQ("#t", run("(integer? 3.0)"));

      TEST_EQ("3", run("(ceiling 3)"));
      TEST_EQ("3", run("(ceiling 3.0)"));
      TEST_EQ("4", run("(ceiling 3.0000000001)"));
      TEST_EQ("3", run("(ceiling 2.9)"));
      TEST_EQ("-3", run("(ceiling -3)"));
      TEST_EQ("-3", run("(ceiling -3.1)"));
      TEST_EQ("-2", run("(ceiling -2.9)"));
      TEST_EQ("-2", run("(ceiling -2.5)"));
      TEST_EQ("-2", run("(ceiling -2.1)"));

      TEST_EQ("3", run("(floor 3)"));
      TEST_EQ("3", run("(floor 3.0)"));
      TEST_EQ("3", run("(floor 3.0000000001)"));
      TEST_EQ("2", run("(floor 2.9)"));
      TEST_EQ("-3", run("(floor -3)"));
      TEST_EQ("-4", run("(floor -3.1)"));
      TEST_EQ("-3", run("(floor -2.9)"));
      TEST_EQ("-3", run("(floor -2.5)"));
      TEST_EQ("-3", run("(floor -2.1)"));
      }
    };

  }

SKIWI_END

#define ONLY_LAST

void run_all_compile_vm_tests()
  {
  using namespace SKIWI;
#ifndef ONLY_LAST  
  fixnums().test();
  bools().test();
  test_for_nil().test();
  chars().test();
  doubles().test();
  add1().test();
  sub1().test();
  add_fixnums().test();
  add_flonums().test();
  add_flonums_and_fixnums().test();
  sub().test();
  mul().test();
  divtest().test(); 
  add_incorrect_argument().test();
  combination_of_math_ops().test();
  equal().test();
  not_equal().test();
  less().test();
  leq().test();
  greater().test();
  geq().test();
  compare_incorrect_argument().test();
  iftest().test();
  andtest().test();
  ortest().test();
  let().test();
  let_star().test();
  arithmetic().test();
  globals().test();
  vector().test();
  letrec().test();
  lambdas().test();
  is_fixnum().test();
  nottest().test();
  is_null().test();
  is_flonum().test();
  is_zero().test();
  is_boolean().test();
  is_char().test();
  cons().test();
  tailcall().test();
  begin().test();
  implicit_begin().test();
  closures().test();
  set().test();
  letrec2().test();
  inner_define().test();
  global_define().test();
  list().test();
  scheme_tests().test();
  scheme_tests_part_b().test();
  scheme_tests_part_c().test();
  scheme_tests_2().test();
  scheme_tests_3().test();
  scheme_tests_4().test();
  and_or().test();
  vectors().test();
  strings().test();
  length().test();
  append().test();
  quote_bug().test();
  quote().test();
  fibonacci().test();
  set_car_cdr().test();
  when_unless().test();
  fixnum_to_char().test();
  char_to_fixnum().test();
  fxlog().test();
  symbols().test();
  applying_thunks().test();
  parameter_passing().test();
  scheme_tests_with_primitives_as_object().test();
  cond().test();
  newton().test();
  callcc().test();
  heap_overflow().test();
  no_heap_overflow_because_gc().test();
  gctest().test();
  gc_fib().test();
  gc_overflow().test();
  primitive_of_2_args_inlined().test();
  lambda_variable_arity_not_using_rest_arg().test();
  lambda_variable_arity_while_using_rest_arg().test();
  lambda_bug().test();
  lambda_variable_arity_while_using_rest_arg_and_closure().test();
  foreign_call_1().test();
  foreign_call_2().test();
  foreign_call_3().test();
  foreign_call_4().test();
  foreign_call_5().test();
  foreign_call_6().test();
  foreign_call_7().test();
  foreign_call_8().test();
  foreign_call_9().test();
  foreign_call_10().test();
  foreign_call_11().test();
  case_examples().test();
  memv().test();
  memq().test();
  member().test();
  equality().test();
  equality_inlined().test();
  apply().test();
  fib_iterative_perf_test().test();
  fib_perf_test().test();
  r5rs_test().test();
#endif     

  
  //make_port_test().test();
  //make_port2_test().test();

  ieee745_test().test();
  //minmax_test().test();
  }