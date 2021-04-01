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
#endif    
  }