///////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////

//#define ONLY_LAST

#include "compile_tests.h"
#include "test_assert.h"

#include <asm/assembler.h>

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

#include <libskiwi/alpha_conversion.h>
#include <libskiwi/compiler_options.h>
#include <libskiwi/compile_data.h>
#include <libskiwi/compiler.h>
#include <libskiwi/context.h>
#include <libskiwi/define_conversion.h>
#include <libskiwi/dump.h>
#include <libskiwi/format.h>
#include <libskiwi/load_lib.h>
#include <libskiwi/macro_data.h>
#include <libskiwi/parse.h>
#include <libskiwi/preprocess.h>
#include <libskiwi/primitives_lib.h>
#include <libskiwi/repl_data.h>
#include <libskiwi/runtime.h>
#include <libskiwi/simplify_to_core.h>
#include <libskiwi/syscalls.h>
#include <libskiwi/tokenize.h>
#include <libskiwi/types.h>
#include <libskiwi/debug_find.h>

#include <libskiwi/libskiwi.h>

SKIWI_BEGIN

namespace
  {

  struct compile_fixture
    {
    compiler_options ops;
    typedef uint64_t(*fun_ptr)(void*);
    context ctxt;
    repl_data rd;
    macro_data md;
    std::shared_ptr<environment<environment_entry>> env;
    bool stream_out;
    std::vector<std::pair<fun_ptr, uint64_t>> compiled_functions;
    primitive_map pm;
    std::map<std::string, external_function> externals;

    compile_fixture()
      {
      add_system_calls(externals);
      stream_out = false;
      ctxt = create_context(1024 * 1024, 1024, 1024, 1024);
      env = std::make_shared<environment<environment_entry>>(nullptr);
      //rd = repl_data();

      asmcode code;
      try
        {
        compile_primitives_library(pm, rd, env, ctxt, code, ops);
        first_pass_data d;
        uint64_t size;
        fun_ptr f = (fun_ptr)assemble(size, d, code);
        f(&ctxt);
        compiled_functions.emplace_back(f, size);
        assign_primitive_addresses(pm, d, (uint64_t)f);
        }
      catch (std::logic_error e)
        {
        std::cout << e.what() << " while compiling primitives library\n\n";
        }
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
          fun_ptr f = (fun_ptr)assemble(size, d, code);
          f(&ctxt);
          compiled_functions.emplace_back(f, size);
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
          fun_ptr f = (fun_ptr)assemble(size, d, code);
          f(&ctxt);
          compiled_functions.emplace_back(f, size);
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
          fun_ptr f = (fun_ptr)assemble(size, d, code);
          f(&ctxt);
          compiled_functions.emplace_back(f, size);
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
          fun_ptr f = (fun_ptr)assemble(size, d, code);
          f(&ctxt);
          compiled_functions.emplace_back(f, size);
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
      for (auto& f : compiled_functions)
        free_assembled_function((void*)f.first, f.second);
      destroy_context(ctxt);
      ctxt = create_context(heap_size, global_stack, local_stack, scheme_stack);
      env = std::make_shared<environment<environment_entry>>(nullptr);
      rd = repl_data();
      asmcode code;
      try
        {
        compile_primitives_library(pm, rd, env, ctxt, code, ops);
        first_pass_data d;
        uint64_t size;
        fun_ptr f = (fun_ptr)assemble(size, d, code);
        f(&ctxt);
        compiled_functions.emplace_back(f, size);
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
      for (auto& f : compiled_functions)
        free_assembled_function((void*)f.first, f.second);
      destroy_macro_data(md);
      destroy_context(ctxt);
      }

    Program get_program(const std::string& script)
      {
      auto tokens = tokenize(script);
      std::reverse(tokens.begin(), tokens.end());
      auto prog = make_program(tokens);
      preprocess(env, rd, md, ctxt, prog, pm, ops);
      return prog;
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

    void dump(const std::string& script, bool dump_primitives_lib = false)
      {
      dump(std::cout, script, dump_primitives_lib);
      }

    void expand(std::ostream& out, const std::string& script)
      {
      // has side effects. todo: make deep copies of env and rd and use those
      auto tokens = tokenize(script);
      std::reverse(tokens.begin(), tokens.end());
      auto prog = make_program(tokens);
      preprocess(env, rd, md, ctxt, prog, pm, ops);
      SKIWI::dump(out, prog);
      out << "\n";
      }

    void expand(const std::string& script)
      {
      expand(std::cout, script);
      }

    void expand_and_format(const std::string& script)
      {
      std::stringstream str;
      expand(str, script);
      format_options fops;
      fops.max_width = 10;
      fops.min_width = 5;
      std::cout << format(str.str(), fops) << "\n";
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
      first_pass_data d;
      uint64_t size;
      fun_ptr f = (fun_ptr)assemble(size, d, code);
      std::stringstream str;
      str << std::setprecision(precision);
      if (f)
        {
        uint64_t res = f(&ctxt);
        if (no_stack_info)
          scheme_runtime(res, str, env, rd, nullptr);
        else
          scheme_runtime(res, str, env, rd, &ctxt);
        compiled_functions.emplace_back(f, size);
        }
      return str.str();
      }

    std::string run_optimized(const std::string& script, int precision = 6)
      {
      bool save_check = ops.safe_primitives;
      ops.safe_primitives = false;
      ops.primitives_inlined = true;
      std::string res = run(script, precision);
      ops.safe_primitives = save_check;
      return res;
      }

    };

  struct compile_fixture_skiwi
    {
    compile_fixture_skiwi()
      {
      using namespace skiwi;
      skiwi_parameters params;
      params.trace = nullptr;
      params.stderror = &std::cout;
      params.stdoutput = nullptr;
      scheme_with_skiwi(nullptr, nullptr, params);
      }

    ~compile_fixture_skiwi()
      {
      skiwi::skiwi_quit();
      }

    std::string run(const std::string& script)
      {
      return skiwi::skiwi_raw_to_string(skiwi::skiwi_run_raw(script));
      }

    void build_values()
      {
      skiwi::skiwi_run_raw("(import 'values)");
      }

    void build_dynamic_wind()
      {
      skiwi::skiwi_run_raw("(import 'dynamic-wind)");
      }

    void build_srfi6()
      {
      skiwi::skiwi_run_raw("(import 'srfi-6)");
      }

    void build_srfi1()
      {
      skiwi::skiwi_run_raw("(import 'srfi-1)");
      }

    void build_srfi28()
      {
      skiwi::skiwi_run_raw("(import 'srfi-28)");
      }

    void build_mbe()
      {
      skiwi::skiwi_run_raw("(import 'mbe)");
      }

    void build_csv()
      {
      skiwi::skiwi_run_raw("(import 'csv)");
      }

    void build_eval()
      {
      skiwi::skiwi_run_raw("(import 'eval)");
      }
    };

  std::string first_line(const std::string& long_string)
    {
    auto pos = long_string.find_first_of('\n');
    if (pos == std::string::npos)
      return long_string;
    else
      return long_string.substr(0, pos-1);
    }

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

  struct add1_optimized : public compile_fixture {
    void test()
      {
      TEST_EQ("0", run_optimized("(add1)"));
      TEST_EQ("1", run_optimized("(add1 0)"));
      TEST_EQ("0", run_optimized("(add1 -1)"));
      TEST_EQ("6", run_optimized("(add1 5)"));
      TEST_EQ("-999", run_optimized("(add1 -1000)"));

      TEST_EQ("536870911", run_optimized("(add1 536870910)"));
      TEST_EQ("-536870911", run_optimized("(add1 -536870912)"));
      TEST_EQ("2", run_optimized("(add1 (add1 0))"));
      TEST_EQ("18", run_optimized("(add1 (add1 (add1 (add1 (add1 (add1 12))))))"));
      TEST_EQ("53687091001", run_optimized("(add1 53687091000)"));
      TEST_EQ("-53687091000", run_optimized("(add1 -53687091001)"));

      TEST_EQ("1.5", run_optimized("(add1 0.5)"));
      TEST_EQ("0.4", run_optimized("(add1 -0.6)"));
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


  struct sub1_optimized : public compile_fixture {
    void test()
      {
      TEST_EQ("0", run_optimized("(sub1 1)"));
      TEST_EQ("-1", run_optimized("(sub1 0)"));
      TEST_EQ("-2", run_optimized("(sub1 -1)"));
      TEST_EQ("4", run_optimized("(sub1 5)"));
      TEST_EQ("-1001", run_optimized(R"((sub1 -1000))"));

      TEST_EQ("0.5", run_optimized("(sub1 1.5)"));
      TEST_EQ("-0.5", run_optimized("(sub1 0.5)"));
      TEST_EQ("-1.6", run_optimized("(sub1 -0.6)"));
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

  struct add_fixnums_opt : public compile_fixture {
    void test()
      {
      TEST_EQ("3", run_optimized("(+ 1 2)"));
      TEST_EQ("6", run_optimized("(+ 1 2 3)"));
      TEST_EQ("10", run_optimized("(+ 1 2 3 4)"));
      TEST_EQ("15", run_optimized("(+ 1 2 3 4 5)"));
      TEST_EQ("21", run_optimized("(+ 1 2 3 4 5 6)"));
      TEST_EQ("28", run_optimized("(+ 1 2 3 4 5 6 7)"));
      TEST_EQ("36", run_optimized("(+ 1 2 3 4 5 6 7 8)"));
      TEST_EQ("45", run_optimized("(+ 1 2 3 4 5 6 7 8 9)"));
      TEST_EQ("55", run_optimized("(+ 1 2 3 4 5 6 7 8 9 10)"));
      TEST_EQ("66", run_optimized("(+ 1 2 3 4 5 6 7 8 9 10 11)"));
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

  struct add_flonums_optimized : public compile_fixture {
    void test()
      {
      TEST_EQ("3", run_optimized("(+ 1.0 2.0)"));
      TEST_EQ("6", run_optimized("(+ 1.0 2.0 3.0)"));
      TEST_EQ("10", run_optimized("(+ 1.0 2.0 3.0 4.0)"));
      TEST_EQ("15", run_optimized("(+ 1.0 2.0 3.0 4.0 5.0)"));
      TEST_EQ("21", run_optimized("(+ 1.0 2.0 3.0 4.0 5.0 6.0)"));
      TEST_EQ("28", run_optimized("(+ 1.0 2.0 3.0 4.0 5.0 6.0 7.0)"));
      TEST_EQ("36", run_optimized("(+ 1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0)"));
      TEST_EQ("45", run_optimized("(+ 1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0)"));
      TEST_EQ("55", run_optimized("(+ 1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0)"));
      TEST_EQ("66", run_optimized("(+ 1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0 11.0)"));
      }
    };

  struct add_flonums_and_fixnums : public compile_fixture {
    void test()
      {
      TEST_EQ("3", run("(+ 1 2.0)"));
      TEST_EQ("3", run("(+ 1.0 2)"));
      }
    };

  struct add_flonums_and_fixnums_optimized : public compile_fixture {
    void test()
      {
      TEST_EQ("3", run_optimized("(+ 1 2.0)"));
      TEST_EQ("3", run_optimized("(+ 1.0 2)"));
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

  struct sub_optimized : public compile_fixture {
    void test()
      {
      TEST_EQ("-1", run_optimized("(- 1 2)"));
      TEST_EQ("-4", run_optimized("(- 1 2 3)"));
      TEST_EQ("-3", run_optimized("(- 1 2 3 -1)"));

      TEST_EQ("0.1", run_optimized("(- 0.5 0.4)"));
      TEST_EQ("-2.77556e-17", run_optimized("(- 0.5 0.4 0.1)"));

      TEST_EQ(std::string("5.8"), run_optimized("(- 7 0.5 0.4 0.1 0.2)"));
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

  struct mul_optimized : public compile_fixture {
    void test()
      {
      TEST_EQ("2", run_optimized("(* 1 2)"));
      TEST_EQ("6", run_optimized("(* 1 2 3)"));
      TEST_EQ("-6", run_optimized("(* 1 2 3 -1)"));

      TEST_EQ("1.25", run_optimized("(* 0.5 2.5)"));
      TEST_EQ("0.125", run_optimized("(* 0.5 2.5 0.1)"));
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

  struct div_optimized : public compile_fixture {
    void test()
      {
      TEST_EQ("2", run_optimized("(/ 4 2)"));
      TEST_EQ("2", run_optimized("(/ 8 2 2)"));
      TEST_EQ("-1", run_optimized("(/ 16 4 -4)"));
      TEST_EQ("-1", run_optimized("(/ 16 -4 4)"));

      TEST_EQ("1.25", run_optimized("(/ 2.5 2)"));
      TEST_EQ("-12.5", run_optimized("(/ 2.5 2 -0.1)"));
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
  struct combination_of_math_ops_optimized : public compile_fixture {
    void test()
      {
      TEST_EQ("27", run_optimized("(/ (* 3 (- (+ 23 9) 20.0) 1.5) 2)")); // 3*12*1.5 / 2 = 3*6*1.5 = 3*9 = 27
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

  struct equal_optimized : public compile_fixture {
    void test()
      {
      TEST_EQ("#f", run_optimized("(= 12 13)"));
      TEST_EQ("#t", run_optimized("(= 12 12)"));
      TEST_EQ("#f", run_optimized("(= 12.1 13.1)"));
      TEST_EQ("#t", run_optimized("(= 12.1 12.1)"));
      TEST_EQ("#f", run_optimized("(= 12 13.1)"));
      TEST_EQ("#t", run_optimized("(= 12 12.0)"));
      TEST_EQ("#f", run_optimized("(= 12.0 13)"));
      TEST_EQ("#t", run_optimized("(= 12.0 12)"));

      TEST_EQ("#t", run_optimized("(= 12 12)"));
      TEST_EQ("#f", run_optimized("(= 13 12)"));
      TEST_EQ("#f", run_optimized("(= 16 (+ 13 1)) "));
      TEST_EQ("#t", run_optimized("(= 16 (+ 13 3))"));
      TEST_EQ("#f", run_optimized("(= 16 (+ 13 13))"));
      TEST_EQ("#f", run_optimized("(= (+ 13 1) 16) "));
      TEST_EQ("#t", run_optimized("(= (+ 13 3) 16) "));
      TEST_EQ("#f", run_optimized("(= (+ 13 13) 16)"));

      TEST_EQ("#f", run_optimized("(= 12.0 13)"));
      TEST_EQ("#t", run_optimized("(= 12.0 12)"));
      TEST_EQ("#f", run_optimized("(= 13.0 12)"));
      TEST_EQ("#f", run_optimized("(= 16.0 (+ 13 1)) "));
      TEST_EQ("#t", run_optimized("(= 16.0 (+ 13 3))"));
      TEST_EQ("#f", run_optimized("(= 16.0 (+ 13 13))"));
      TEST_EQ("#f", run_optimized("(= (+ 13.0 1) 16) "));
      TEST_EQ("#t", run_optimized("(= (+ 13.0 3) 16.0) "));
      TEST_EQ("#f", run_optimized("(= (+ 13.0 13.0) 16.0)"));

      TEST_EQ("#t", run_optimized("(= 12 12 12)"));
      TEST_EQ("#t", run_optimized("(= 12 12 12 12)"));
      TEST_EQ("#t", run_optimized("(= 12 12 12 12 12)"));
      TEST_EQ("#t", run_optimized("(= 12 12 12 12 12 12)"));
      TEST_EQ("#t", run_optimized("(= 12 12 12 12 12 12 12)"));
      TEST_EQ("#t", run_optimized("(= 12 12 12 12 12 12 12 12)"));
      TEST_EQ("#t", run_optimized("(= 12 12 12 12 12 12 12 12 12)"));
      TEST_EQ("#t", run_optimized("(= 12 12 12 12 12 12 12 12 12 12)"));

      TEST_EQ("#f", run_optimized("(= 13 12 12)"));
      TEST_EQ("#f", run_optimized("(= 13 12 12 12)"));
      TEST_EQ("#f", run_optimized("(= 13 12 12 12 12)"));
      TEST_EQ("#f", run_optimized("(= 13 12 12 12 12 12)"));
      TEST_EQ("#f", run_optimized("(= 13 12 12 12 12 12 12)"));
      TEST_EQ("#f", run_optimized("(= 13 12 12 12 12 12 12 12)"));
      TEST_EQ("#f", run_optimized("(= 13 12 12 12 12 12 12 12 12)"));
      TEST_EQ("#f", run_optimized("(= 13 12 12 12 12 12 12 12 12 12)"));

      TEST_EQ("#f", run_optimized("(= 12 12 13)"));
      TEST_EQ("#f", run_optimized("(= 12 12 12 13)"));
      TEST_EQ("#f", run_optimized("(= 12 12 12 12 13)"));
      TEST_EQ("#f", run_optimized("(= 12 12 12 12 12 13)"));
      TEST_EQ("#f", run_optimized("(= 12 12 12 12 12 12 13)"));
      TEST_EQ("#f", run_optimized("(= 12 12 12 12 12 12 12 13)"));
      TEST_EQ("#f", run_optimized("(= 12 12 12 12 12 12 12 12 13)"));
      TEST_EQ("#f", run_optimized("(= 12 12 12 12 12 12 12 12 12 13)"));
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

  struct not_equal_optimized : public compile_fixture {
    void test()
      {
      TEST_EQ("#t", run_optimized("(!= 12 13)"));
      TEST_EQ("#f", run_optimized("(!= 12 12)"));
      TEST_EQ("#t", run_optimized("(!= 12.1 13.1)"));
      TEST_EQ("#f", run_optimized("(!= 12.1 12.1)"));
      TEST_EQ("#t", run_optimized("(!= 12 13.1)"));
      TEST_EQ("#f", run_optimized("(!= 12 12.0)"));
      TEST_EQ("#t", run_optimized("(!= 12.0 13)"));
      TEST_EQ("#f", run_optimized("(!= 12.0 12)"));

      TEST_EQ("#f", run_optimized("(!= 12 12)"));
      TEST_EQ("#t", run_optimized("(!= 13 12)"));
      TEST_EQ("#t", run_optimized("(!= 16 (+ 13 1)) "));
      TEST_EQ("#f", run_optimized("(!= 16 (+ 13 3))"));
      TEST_EQ("#t", run_optimized("(!= 16 (+ 13 13))"));
      TEST_EQ("#t", run_optimized("(!= (+ 13 1) 16) "));
      TEST_EQ("#f", run_optimized("(!= (+ 13 3) 16) "));
      TEST_EQ("#t", run_optimized("(!= (+ 13 13) 16)"));

      TEST_EQ("#t", run_optimized("(!= 12.0 13)"));
      TEST_EQ("#f", run_optimized("(!= 12.0 12)"));
      TEST_EQ("#t", run_optimized("(!= 13.0 12)"));
      TEST_EQ("#t", run_optimized("(!= 16.0 (+ 13 1)) "));
      TEST_EQ("#f", run_optimized("(!= 16.0 (+ 13 3))"));
      TEST_EQ("#t", run_optimized("(!= 16.0 (+ 13 13))"));
      TEST_EQ("#t", run_optimized("(!= (+ 13.0 1) 16) "));
      TEST_EQ("#f", run_optimized("(!= (+ 13.0 3) 16.0) "));
      TEST_EQ("#t", run_optimized("(!= (+ 13.0 13.0) 16.0)"));

      TEST_EQ("#t", run_optimized("(!= 12 13 14 15 16 17 18 19 20 21 22 23)"));
      TEST_EQ("#t", run_optimized("(!= 12 13 14 15 16 17 18 19 20 21 22 12)"));
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

  struct less_optimized : public compile_fixture {
    void test()
      {
      TEST_EQ("#f", run_optimized("(< 4 2)"));
      TEST_EQ("#t", run_optimized("(< 2 4)"));
      TEST_EQ("#f", run_optimized("(< 4 2 3)"));
      TEST_EQ("#t", run_optimized("(< 2 4 5)"));
      TEST_EQ("#f", run_optimized("(< 2 4 3)"));

      TEST_EQ("#f", run_optimized("(< 4.1 2)"));
      TEST_EQ("#t", run_optimized("(< 2.1 4)"));
      TEST_EQ("#f", run_optimized("(< 4.1 2 3)"));
      TEST_EQ("#t", run_optimized("(< 2.1 4 5)"));
      TEST_EQ("#f", run_optimized("(< 2.1 4 3)"));

      TEST_EQ("#t", run_optimized("(< 12 13)"));
      TEST_EQ("#f", run_optimized("(< 12 12)"));
      TEST_EQ("#f", run_optimized("(< 13 12)"));
      TEST_EQ("#f", run_optimized("(< 16 (+ 13 1)) "));
      TEST_EQ("#f", run_optimized("(< 16 (+ 13 3))"));
      TEST_EQ("#t", run_optimized("(< 16 (+ 13 13))"));
      TEST_EQ("#t", run_optimized("(< (+ 13 1) 16) "));
      TEST_EQ("#f", run_optimized("(< (+ 13 3) 16) "));
      TEST_EQ("#f", run_optimized("(< (+ 13 13) 16)"));

      TEST_EQ("#t", run_optimized("(< 12.0 13)"));
      TEST_EQ("#f", run_optimized("(< 12.0 12)"));
      TEST_EQ("#f", run_optimized("(< 13.0 12)"));
      TEST_EQ("#f", run_optimized("(< 16.0 (+ 13 1.0)) "));
      TEST_EQ("#f", run_optimized("(< 16.0 (+ 13.0 3.0))"));
      TEST_EQ("#t", run_optimized("(< 16.0 (+ 13.0 13.0))"));
      TEST_EQ("#t", run_optimized("(< (+ 13.0 1) 16.0) "));
      TEST_EQ("#f", run_optimized("(< (+ 13.0 3.000000001) 16.0)"));
      TEST_EQ("#f", run_optimized("(< (+ 13 13.0) 16)"));
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

  struct leq_optimized : public compile_fixture {
    void test()
      {
      TEST_EQ("#t", run_optimized("(<= 12 13)"));
      TEST_EQ("#t", run_optimized("(<= 12 12)"));
      TEST_EQ("#f", run_optimized("(<= 13 12)"));
      TEST_EQ("#f", run_optimized("(<= 16 (+ 13 1)) "));
      TEST_EQ("#t", run_optimized("(<= 16 (+ 13 3))"));
      TEST_EQ("#t", run_optimized("(<= 16 (+ 13 13))"));
      TEST_EQ("#t", run_optimized("(<= (+ 13 1) 16) "));
      TEST_EQ("#t", run_optimized("(<= (+ 13 3) 16) "));
      TEST_EQ("#f", run_optimized("(<= (+ 13 13) 16)"));

      TEST_EQ("#t", run_optimized("(<= 12.0 13.0)"));
      TEST_EQ("#t", run_optimized("(<= 12.0 12.0)"));
      TEST_EQ("#f", run_optimized("(<= 13.0 12)"));
      TEST_EQ("#f", run_optimized("(<= 16 (+ 13.0 1)) "));
      TEST_EQ("#t", run_optimized("(<= 16 (+ 13 3.0))"));
      TEST_EQ("#t", run_optimized("(<= 16.0 (+ 13.0 13.0))"));
      TEST_EQ("#t", run_optimized("(<= (+ 13.0 1) 16) "));
      TEST_EQ("#t", run_optimized("(<= (+ 13 3.0) 16.0) "));
      TEST_EQ("#f", run_optimized("(<= (+ 13.0 13) 16.0)"));
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

  struct greater_optimized : public compile_fixture {
    void test()
      {
      TEST_EQ("#f", run_optimized("(> 12 13)"));
      TEST_EQ("#f", run_optimized("(> 12 12)"));
      TEST_EQ("#t", run_optimized("(> 13 12)"));
      TEST_EQ("#t", run_optimized("(> 16 (+ 13 1)) "));
      TEST_EQ("#f", run_optimized("(> 16 (+ 13 3))"));
      TEST_EQ("#f", run_optimized("(> 16 (+ 13 13))"));
      TEST_EQ("#f", run_optimized("(> (+ 13 1) 16) "));
      TEST_EQ("#f", run_optimized("(> (+ 13 3) 16) "));
      TEST_EQ("#t", run_optimized("(> (+ 13 13) 16)"));

      TEST_EQ("#f", run_optimized("(> 12.0 13)"));
      TEST_EQ("#f", run_optimized("(> 12.0 12)"));
      TEST_EQ("#t", run_optimized("(> 13.0 12)"));
      TEST_EQ("#t", run_optimized("(> 16.0 (+ 13 1)) "));
      TEST_EQ("#f", run_optimized("(> 16.0 (+ 13 3))"));
      TEST_EQ("#f", run_optimized("(> 16.0 (+ 13 13))"));
      TEST_EQ("#f", run_optimized("(> (+ 13.0 1) 16) "));
      TEST_EQ("#f", run_optimized("(> (+ 13.0 3) 16) "));
      TEST_EQ("#t", run_optimized("(> (+ 13.0 13) 16)"));
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

  struct geq_optimized : public compile_fixture {
    void test()
      {
      TEST_EQ("#f", run_optimized("(>= 12 13)"));
      TEST_EQ("#t", run_optimized("(>= 12 12)"));
      TEST_EQ("#t", run_optimized("(>= 13 12)"));
      TEST_EQ("#t", run_optimized("(>= 16 (+ 13 1)) "));
      TEST_EQ("#t", run_optimized("(>= 16 (+ 13 3))"));
      TEST_EQ("#f", run_optimized("(>= 16 (+ 13 13))"));
      TEST_EQ("#f", run_optimized("(>= (+ 13 1) 16) "));
      TEST_EQ("#t", run_optimized("(>= (+ 13 3) 16) "));
      TEST_EQ("#t", run_optimized("(>= (+ 13 13) 16)"));

      TEST_EQ("#f", run_optimized("(>= 12.0 13)"));
      TEST_EQ("#t", run_optimized("(>= 12.0 12)"));
      TEST_EQ("#t", run_optimized("(>= 13.0 12)"));
      TEST_EQ("#t", run_optimized("(>= 16.0 (+ 13 1)) "));
      TEST_EQ("#t", run_optimized("(>= 16.0 (+ 13 3))"));
      TEST_EQ("#f", run_optimized("(>= 16.0 (+ 13 13))"));
      TEST_EQ("#f", run_optimized("(>= (+ 13.0 1) 16) "));
      TEST_EQ("#t", run_optimized("(>= (+ 13.0 3) 16) "));
      TEST_EQ("#t", run_optimized("(>= (+ 13.0 13) 16)"));
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

  struct and_optimized : public compile_fixture {
    void test()
      {
      TEST_EQ("#t", run_optimized("(and #t)"));
      TEST_EQ("#f", run_optimized("(and #f)"));
      TEST_EQ("#t", run_optimized("(and #t #t)"));
      TEST_EQ("#f", run_optimized("(and #f #f)"));
      TEST_EQ("#f", run_optimized("(and #f #t)"));
      TEST_EQ("#f", run_optimized("(and #t #f)"));
      TEST_EQ("#t", run_optimized("(and #t #t #t)"));
      TEST_EQ("#f", run_optimized("(and #f #t #t)"));
      TEST_EQ("#f", run_optimized("(and #t #t #f)"));
      TEST_EQ("#f", run_optimized("(and #t #f #t)"));
      TEST_EQ("#f", run_optimized("(and #f #f #f)"));
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

  struct or_optimized : public compile_fixture {
    void test()
      {
      TEST_EQ("#t", run_optimized("(or #t)"));
      TEST_EQ("#f", run_optimized("(or #f)"));
      TEST_EQ("#t", run_optimized("(or #t #t)"));
      TEST_EQ("#f", run_optimized("(or #f #f)"));
      TEST_EQ("#t", run_optimized("(or #f #t)"));
      TEST_EQ("#t", run_optimized("(or #t #f)"));
      TEST_EQ("#t", run_optimized("(or #t #t #t)"));
      TEST_EQ("#t", run_optimized("(or #f #t #t)"));
      TEST_EQ("#t", run_optimized("(or #t #t #f)"));
      TEST_EQ("#t", run_optimized("(or #t #f #t)"));
      TEST_EQ("#f", run_optimized("(or #f #f #f)"));
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
  struct let_optimized : public compile_fixture {
    void test()
      {
      TEST_EQ("5", run_optimized("(let ([x 5]) x)"));
      TEST_EQ("5", run_optimized("(let ([x 5][y 6]) x)"));
      TEST_EQ("6", run_optimized("(let ([x 5][y 6]) y)"));

      TEST_EQ("3", run_optimized("(let ([x (+ 1 2)]) x)"));
      TEST_EQ("10", run_optimized("(let ([x (+ 1 2)]) (let([y(+ 3 4)])(+ x y))) "));
      TEST_EQ("4", run_optimized("(let ([x (+ 1 2)])  (let([y(+ 3 4)])(- y x)))"));
      TEST_EQ("4", run_optimized("(let ([x (+ 1 2)] [y(+ 3 4)])  (- y x))"));
      TEST_EQ("18", run_optimized("(let ([x (let ([y (+ 1 2)]) (* y y))]) (+ x x))"));
      TEST_EQ("7", run_optimized("(let ([x (+ 1 2)]) (let([x(+ 3 4)]) x))"));
      TEST_EQ("7", run_optimized("(let ([x (+ 1 2)]) (let([x(+ x 4)]) x)) "));
      TEST_EQ("3", run_optimized("(let ([t (let ([t (let ([t (let ([t (+ 1 2)]) t)]) t)]) t)]) t)"));
      TEST_EQ("192", run_optimized("(let ([x 12])  (let([x(+ x x)]) (let([x(+ x x)]) (let([x(+ x x)]) (+ x x)))))"));

      TEST_EQ("45", run_optimized("(let ([a 0] [b 1] [c 2] [d 3] [e 4] [f 5] [g 6] [h 7] [i 8] [j 9]) (+ a b c d e f g h i j) )"));
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

  struct arithmetic_optimized : public compile_fixture {
    void test()
      {
      TEST_EQ("45", run_optimized("(+ (+ (+ (+ (+ (+ (+ (+ 1 2) 3) 4) 5) 6) 7) 8) 9)"));
      TEST_EQ("45", run_optimized("(+ 1 (+ 2 (+ 3 (+ 4 (+ 5 (+ 6 (+ 7 (+ 8 9))))))))"));

      TEST_EQ("-43", run_optimized("(- (- (- (- (- (- (- (- 1 2) 3) 4) 5) 6) 7) 8) 9)"));
      TEST_EQ("5", run_optimized("(- 1 (- 2 (- 3 (- 4 (- 5 (- 6 (- 7 (- 8 9))))))))"));

      TEST_EQ("5040", run_optimized("(* (* (* (* (* 2 3) 4) 5) 6) 7)"));
      TEST_EQ("5040", run_optimized("(* 2 (* 3 (* 4 (* 5 (* 6 7)))))"));
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

  struct globals_optimized : public compile_fixture {
    void test()
      {
      TEST_EQ("3.14", run_optimized("(define x 3.14) 50 x"));
      TEST_EQ("3.14", run_optimized("x"));
      TEST_EQ("7", run_optimized("(define x 7) 51 x"));
      TEST_EQ("7", run_optimized("x"));
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
      TEST_EQ("0", run("(letrec ([countdown (lambda (n) (if (zero? n) n (countdown(sub1 n))))])(countdown 50005000))"));
      TEST_EQ("50005000", run("(letrec ([sum (lambda (n ac)(if (zero? n) ac (sum(sub1 n) (+ n ac))))]) (sum 10000 0))"));
      TEST_EQ("#t", run("(letrec ([e (lambda (x) (if (zero? x) #t (o (sub1 x))))][o(lambda(x) (if (zero? x) #f(e(sub1 x))))])(e 5000000))"));
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
  struct is_procedure : public compile_fixture_skiwi {
    void test()
      {
      build_values();

      TEST_EQ("#t", run("(procedure? (lambda (x) x)) "));
      TEST_EQ("#t", run("(let ([f (lambda (x) x)]) (procedure? f))"));
      TEST_EQ("#f", run(R"((procedure? (make-vector 0)))"));
      TEST_EQ("#f", run(R"((procedure? (make-string 0)))"));
      TEST_EQ("#f", run(R"((procedure? (cons 1 2)))"));
      TEST_EQ("#f", run(R"((procedure? #\S))"));
      TEST_EQ("#f", run(R"((procedure? ()))"));
      TEST_EQ("#f", run(R"((procedure? #t))"));
      TEST_EQ("#f", run(R"((procedure? #f))"));
      TEST_EQ("#f", run(R"((string? (lambda (x) x)))"));
      TEST_EQ("#f", run(R"((vector? (lambda (x) x)))"));
      TEST_EQ("#f", run(R"((boolean? (lambda (x) x)))"));
      TEST_EQ("#f", run(R"((null? (lambda (x) x)))"));
      TEST_EQ("#f", run(R"((not (lambda (x) x)))"));
      TEST_EQ("#t", run("(procedure? +)"));
      TEST_EQ("#t", run("(procedure? car)"));
      TEST_EQ("#t", run("(procedure? apply)"));
      TEST_EQ("#t", run("(procedure? (lambda (x) (* x x)))"));
      TEST_EQ("#f", run("(procedure? '(lambda (x) (* x x)))"));
      //TEST_EQ("#t", run("(call-with-current-continuation procedure?)"));
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
      TEST_EQ("165580141", run("(fib 40)"));
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
      external_function ef;
      ef.name = "seventeen";
      ef.address = (uint64_t)&seventeen;
      ef.return_type = external_function::T_INT64;
      externals[ef.name] = ef;
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

  struct callcc_test : public compile_fixture_skiwi {
    void test()
      {
      build_values();
      build_dynamic_wind();

      TEST_EQ("1", run("(call/cc (lambda(throw) (+ 5 (* 10 (throw 1)))))"));
      TEST_EQ("15", run("(call/cc (lambda(throw) (+ 5 (* 10 1))))"));
      TEST_EQ("35", run("(call/cc(lambda(throw) (+ 5 (* 10 (call/cc(lambda(escape) (* 100 (escape 3))))))))"));
      TEST_EQ("3", run("(call/cc(lambda(throw) (+ 5 (* 10 (call/cc(lambda(escape) (* 100 (throw 3))))))))"));
      TEST_EQ("1005", run("(call/cc(lambda(throw) (+ 5 (* 10 (call/cc(lambda(escape) (* 100 1)))))))"));

      TEST_EQ("1", run("(call-with-current-continuation (lambda(throw) (+ 5 (* 10 (throw 1)))))"));
      TEST_EQ("15", run("(call-with-current-continuation (lambda(throw) (+ 5 (* 10 1))))"));
      TEST_EQ("35", run("(call-with-current-continuation(lambda(throw) (+ 5 (* 10 (call/cc(lambda(escape) (* 100 (escape 3))))))))"));
      TEST_EQ("3", run("(call-with-current-continuation(lambda(throw) (+ 5 (* 10 (call/cc(lambda(escape) (* 100 (throw 3))))))))"));
      TEST_EQ("1005", run("(call-with-current-continuation(lambda(throw) (+ 5 (* 10 (call/cc(lambda(escape) (* 100 1)))))))"));

      TEST_EQ("12", run("(call/cc (lambda(k) 12))"));
      TEST_EQ("12", run("(call/cc (lambda(k)  (k 12)))"));
      TEST_EQ("12", run("(call/cc (lambda(k) (fx+ 1 (k 12))))"));

      TEST_EQ("25", run("(fx+ (call/cc(lambda(k) (k 12)))(call/cc(lambda(k) 13)))"));
      TEST_EQ("#f", run("(fxzero? 1)"));
      TEST_EQ("#f", run("(fxzero? -1)"));
      TEST_EQ("#t", run("(fxzero? 0)"));
      TEST_EQ("4", run("(fxsub1 5)"));
      TEST_EQ("-1", run("(fxsub1 0)"));
      TEST_EQ("1", run("(fxadd1 0)"));
      TEST_EQ("0", run("(fxadd1 -1)"));
      TEST_EQ("5", run("(fxadd1 4)"));
      TEST_EQ("1", run("(letrec([fact (lambda(n k) (cond [(fxzero? n) (k 1)][else (fx* n (fact (fxsub1 n) k))]))]) (call/cc (lambda(k)(fact 5 k)))) "));
      TEST_EQ("1", run("(call/cc   (lambda(k)  (letrec([fact    (lambda(n)    (cond  [(fxzero? n) (k 1)][else (fx* n(fact(fxsub1 n)))]))])  (fact 5))))"));
      TEST_EQ("120", run("(let([k #f])  (letrec([fact (lambda(n)  (cond [(fxzero? n)  (call/cc (lambda(nk)    (set! k nk)   (k 1)))][else (fx* n(fact(fxsub1 n)))]))]) (let([v(fact 5)])  v)))"));
      TEST_EQ("(120 . 14400)", run("(let([k #f])(letrec([fact (lambda(n) (cond [(fxzero? n) (call/cc  (lambda(nk)  (set! k nk) (k 1)))] [else (fx* n(fact(fxsub1 n)))]))]) (let([v(fact 5)])(let([nk k])  (set! k(lambda(x) (cons v x))) (nk v))))) "));
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
      TEST_EQ("1.22461e-16", run("(ieee754-sin (ieee754-pi))"));
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

      TEST_EQ("1", run_optimized("(max 1)"));
      TEST_EQ("2", run_optimized("(min 2)"));
      TEST_EQ("1.5", run_optimized("(max 1.5)"));
      TEST_EQ("2.2", run_optimized("(min 2.2)"));
      TEST_EQ("2", run_optimized("(max 1.0 2.0)"));
      TEST_EQ("7.2", run_optimized("(max 1.0 2.0 -0.4 7.2 2.3 7.1 -11.0)"));
      TEST_EQ("2", run_optimized("(max 1 2.0)"));
      TEST_EQ("2", run_optimized("(max 1.0 2)"));
      TEST_EQ("2", run_optimized("(max 1 2)"));
      TEST_EQ("7", run_optimized("(max 1 2 7 -3 -11 6 3)"));

      TEST_EQ("1", run_optimized("(min 1.0 2.0)"));
      TEST_EQ("-11", run_optimized("(min 1.0 2.0 -0.4 7.2 2.3 7.1 -11.0)"));
      TEST_EQ("1", run_optimized("(min 1 2.0)"));
      TEST_EQ("1", run_optimized("(min 1.0 2)"));
      TEST_EQ("1", run_optimized("(min 1 2)"));
      TEST_EQ("-11", run_optimized("(min 1 2 7 -3 -11 6 3)"));
      TEST_EQ("1024", run_optimized("(arithmetic-shift 1 10)"));
      TEST_EQ("31", run_optimized("(arithmetic-shift 255 -3)"));

      TEST_EQ("3", run_optimized("(quotient 10 3)"));
      TEST_EQ("-3", run_optimized("(quotient -10 3)"));
      TEST_EQ("3", run_optimized("(quotient 10.0 3)"));
      TEST_EQ("-3", run_optimized("(quotient -10.0 3)"));
      TEST_EQ("3", run_optimized("(quotient 10.0 3.0)"));
      TEST_EQ("-3", run_optimized("(quotient -10.0 3.0)"));
      TEST_EQ("3", run_optimized("(quotient 10 3.0)"));
      TEST_EQ("-3", run_optimized("(quotient -10 3.0)"));

      TEST_EQ("1", run_optimized("(remainder 10 3)"));
      TEST_EQ("-1", run_optimized("(remainder -10 3)"));
      TEST_EQ("1", run_optimized("(remainder 10.0 3)"));
      TEST_EQ("-1", run_optimized("(remainder -10.0 3)"));
      TEST_EQ("1", run_optimized("(remainder 10.0 3.0)"));
      TEST_EQ("-1", run_optimized("(remainder -10.0 3.0)"));
      TEST_EQ("1", run_optimized("(remainder 10 3.0)"));
      TEST_EQ("-1", run_optimized("(remainder -10 3.0)"));

      TEST_EQ("#t", run_optimized("(let ([a 1324] [b 324]) (= (+ (* (quotient a b) b)  (remainder a b) )  a)  )"));
      TEST_EQ("#t", run_optimized("(let ([a 1324] [b -324.0]) (= (+ (* (quotient a b) b)  (remainder a b) )  a)  )"));
      TEST_EQ("-4", run_optimized("(let ([a 1324] [b -324.0]) (quotient a b ))"));
      TEST_EQ("28", run_optimized("(let ([a 1324] [b -324.0]) (remainder a b ))"));
      TEST_EQ("1324", run_optimized("(let ([a 1324] [b -324.0]) (+ (* (quotient a b) b)  (remainder a b) ))"));

      TEST_EQ("1", run_optimized("(modulo 10 3)"));
      TEST_EQ("2", run_optimized("(modulo -10 3)"));
      TEST_EQ("1", run_optimized("(modulo 10.0 3)"));
      TEST_EQ("-1", run_optimized("(modulo -10.0 -3)"));
      TEST_EQ("1", run_optimized("(modulo 10.0 3.0)"));
      TEST_EQ("2", run_optimized("(modulo -10.0 3.0)"));
      TEST_EQ("-2", run_optimized("(modulo 10 -3.0)"));
      TEST_EQ("2", run_optimized("(modulo -10 3.0)"));

      TEST_EQ("3", run_optimized("(abs 3)"));
      TEST_EQ("3.2", run_optimized("(abs 3.2)"));
      TEST_EQ("3", run_optimized("(abs -3)"));
      TEST_EQ("3.2", run_optimized("(abs -3.2)"));
      TEST_EQ("0", run_optimized("(abs 0)"));
      TEST_EQ("0", run_optimized("(abs 0.0)"));

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
#else
      TEST_EQ("-nan", run("(/ 0.0 0.0)"));
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

  struct make_port3_test : public compile_fixture
    {
    void test()
      {
      build_libs();
      TEST_EQ("#t", run("(port? standard-input-port)"));
      TEST_EQ("#t", run("(input-port? standard-input-port)"));
      TEST_EQ("#f", run("(output-port? standard-input-port)"));
      TEST_EQ("#t", run("(port? standard-output-port)"));
      TEST_EQ("#f", run("(input-port? standard-output-port)"));
      TEST_EQ("#t", run("(output-port? standard-output-port)"));
      TEST_EQ("#t", run("(port? standard-error-port)"));
      TEST_EQ("#f", run("(input-port? standard-error-port)"));
      TEST_EQ("#t", run("(output-port? standard-error-port)"));

      TEST_EQ("<port>: \"altout\"", run("(define alternative-output-port (make-port #f \"altout\" 1 (make-string 1024) 0 1024))"));

      TEST_EQ("<port>: \"stdout\"", run("(current-output-port)"));
      TEST_EQ("<port>: \"altout\"", run("(current-output-port alternative-output-port)"));
      TEST_EQ("<port>: \"altout\"", run("(current-output-port)"));
      TEST_EQ("<port>: \"stdin\"", run("(current-input-port)"));
      TEST_EQ("<port>: \"stderr\"", run("(current-error-port)"));

      TEST_EQ("", run("(write-char #\\f)(write-char #\\l)(write-char #\\u)(write-char #\\s)(write-char #\\h)(write-char #\\010)"));
      TEST_EQ("", run("(flush-output-port)"));

      TEST_EQ("3", run("(open-file \"out.txt\" #f)"));
      TEST_EQ("0", run("(close-file 3)"));

      TEST_EQ("<port>: \"out.txt\"", run("(define my_file (open-output-file \"out.txt\"))"));
      TEST_EQ("3", run("(%slot-ref my_file 2)")); // gets file handle id
      TEST_EQ("", run("(write-char #\\o my_file)"));
      TEST_EQ("", run("(write-char #\\u my_file)"));
      TEST_EQ("", run("(write-char #\\t my_file)"));
      TEST_EQ("", run("(write-char #\\p my_file)"));
      TEST_EQ("", run("(write-char #\\u my_file)"));
      TEST_EQ("", run("(write-char #\\t my_file)"));
      TEST_EQ("0", run("(close-output-port my_file)"));
      TEST_EQ("-1", run("(close-output-port my_file)"));

      TEST_EQ("<port>: \"out.txt\"", run("(define my_file (open-input-file \"out.txt\"))"));
      TEST_EQ("#t", run("(input-port? my_file)"));
      TEST_EQ("#\\o", run("(read-char my_file)"));
      TEST_EQ("#\\u", run("(read-char my_file)"));
      TEST_EQ("#\\t", run("(read-char my_file)"));
      TEST_EQ("#\\p", run("(read-char my_file)"));
      TEST_EQ("#\\u", run("(read-char my_file)"));
      TEST_EQ("#\\t", run("(read-char my_file)"));
      TEST_EQ("#eof", run("(read-char my_file)"));
      TEST_EQ("#eof", run("(read-char my_file)"));
      TEST_EQ("0", run("(close-input-port my_file)"));
      TEST_EQ("-1", run("(close-input-port my_file)"));

      TEST_EQ("<port>: \"out.txt\"", run("(set! my_file (let ([fh (open-file \"out.txt\" #t)])(make-port #t \"out.txt\" fh (make-string 3) 3 3)))"));
      TEST_EQ("3", run("(%slot-ref my_file 5)")); // size of buffer
      TEST_EQ("#t", run("(input-port? my_file)"));
      TEST_EQ("#\\o", run("(read-char my_file)"));
      TEST_EQ("#\\u", run("(peek-char my_file)"));
      TEST_EQ("#\\u", run("(peek-char my_file)"));
      TEST_EQ("#\\u", run("(read-char my_file)"));
      TEST_EQ("#\\t", run("(peek-char my_file)"));
      TEST_EQ("#\\t", run("(read-char my_file)"));
      TEST_EQ("#\\p", run("(peek-char my_file)"));
      TEST_EQ("#\\p", run("(read-char my_file)"));
      TEST_EQ("#\\u", run("(peek-char my_file)"));
      TEST_EQ("#\\u", run("(read-char my_file)"));
      TEST_EQ("#\\t", run("(peek-char my_file)"));
      TEST_EQ("#\\t", run("(read-char my_file)"));
      TEST_EQ("#eof", run("(peek-char my_file)"));
      TEST_EQ("#eof", run("(read-char my_file)"));
      TEST_EQ("#eof", run("(read-char my_file)"));
      TEST_EQ("#t", run("(eof-object? (read-char my_file))"));
      TEST_EQ("0", run("(close-input-port my_file)"));
      TEST_EQ("#f", run("(eof-object? 3)"));
      }
    };

  struct writetests : public compile_fixture
    {
    void test()
      {
      build_libs();

      char buffer[1024];
      double d = 3.1415984351384361;
      sprintf(buffer, "%.20g", d);
      TEST_EQ("3.1415984351384360629", std::string(buffer));

      TEST_EQ("\"3.1415984351384360629\"", run("(num2str 3.1415984351384361 10)"));
      TEST_EQ("\"25\"", run("(num2str 25 10)"));
      TEST_EQ("\"31\"", run("(num2str 25 8)"));
      TEST_EQ("\"19\"", run("(num2str 25 16)"));
      TEST_EQ("\"1a\"", run("(num2str 26 16)"));
      TEST_EQ("\"3.1400000000000001243\"", run("(num2str 3.14 10)"));

      TEST_EQ("\"3.1415984351384360629\"", run("(number->string 3.1415984351384361)"));
      TEST_EQ("\"25\"", run("(number->string 25)"));
      TEST_EQ("\"31\"", run("(number->string 25 8)"));
      TEST_EQ("\"1a\"", run("(number->string 26 16)"));
      TEST_EQ("\"+nan.0\"", run("(number->string (/ 0.0 0.0))"));
      TEST_EQ("\"+inf.0\"", run("(number->string (/ 1.0 0.0))"));
      TEST_EQ("\"-inf.0\"", run("(number->string (/ -1.0 0.0))"));

      TEST_EQ("runtime error: str2num: contract violation", run("(str2num 25 10)"));
      TEST_EQ("#f", run("(str2num \"\" 10)"));
      TEST_EQ("25", run("(str2num \"25\" 10)"));
      TEST_EQ("21", run("(str2num \"25\" 8)"));
      TEST_EQ("255", run("(str2num \"ff\" 16)"));
      TEST_EQ("#f", run("(str2num \"3.14asdf\" 10)"));
      TEST_EQ("3.14", run("(str2num \"3.14\" 10)"));

      TEST_EQ("#f", run("(string->number \"\")"));
      TEST_EQ("25", run("(string->number \"25\")"));
      TEST_EQ("21", run("(string->number \"25\" 8)"));
      TEST_EQ("255", run("(string->number \"ff\" 16)"));
      TEST_EQ("#f", run("(string->number \"3.14asdf\")"));
      TEST_EQ("3.14", run("(string->number \"3.14\")"));

      TEST_EQ("<port>: \"out\"", run("(define out (make-port #f \"out\" 1 (make-string 3) 0 3))"));
      TEST_EQ("", run("(write-char #\\a out)"));
      TEST_EQ("", run("(write-string \" test\" out)"));
      TEST_EQ("", run("(flush-output-port out)"));
      TEST_EQ("", run("(write-string \"cd\" out)"));
      TEST_EQ("", run("(flush-output-port out)"));

      TEST_EQ("", run("(write-string \"\n\")"));
      TEST_EQ("", run("(write-string \"Hello\")"));
      TEST_EQ("", run("(write-string \" \")"));
      TEST_EQ("", run("(write-string \"world!\")"));
      TEST_EQ("", run("(write-string \"\n\")"));
      TEST_EQ("", run("(flush-output-port)"));
      }
    };

  struct char_comp : public compile_fixture {
    void test()
      {
      build_libs();

      TEST_EQ("#t", run(R"((char=? #\A #\A))"));
      TEST_EQ("#f", run(R"((char=? #\A #\a))"));

      TEST_EQ("runtime error: char=?: contract violation", run(R"((char=? #\A 2))"));
      TEST_EQ("runtime error: char=?: contract violation", run(R"((char=? #\A #\A #\b))"));

      TEST_EQ("#t", run(R"((char<? #\a #\b))"));
      TEST_EQ("#f", run(R"((char<? #\b #\a))"));
      TEST_EQ("#f", run(R"((char<? #\a #\a))"));

      TEST_EQ("#f", run(R"((char>? #\a #\b))"));
      TEST_EQ("#t", run(R"((char>? #\b #\a))"));
      TEST_EQ("#f", run(R"((char>? #\a #\a))"));

      TEST_EQ("#t", run(R"((char<=? #\a #\b))"));
      TEST_EQ("#f", run(R"((char<=? #\b #\a))"));
      TEST_EQ("#t", run(R"((char<=? #\a #\a))"));

      TEST_EQ("#f", run(R"((char>=? #\a #\b))"));
      TEST_EQ("#t", run(R"((char>=? #\b #\a))"));
      TEST_EQ("#t", run(R"((char>=? #\a #\a))"));

      TEST_EQ("#\\a", run(R"((char-downcase #\A))"));
      TEST_EQ("#\\a", run(R"((char-downcase #\a))"));
      TEST_EQ("#\\z", run(R"((char-downcase #\Z))"));
      TEST_EQ("#\\z", run(R"((char-downcase #\z))"));

      TEST_EQ("#\\A", run(R"((char-upcase #\A))"));
      TEST_EQ("#\\A", run(R"((char-upcase #\a))"));
      TEST_EQ("#\\Z", run(R"((char-upcase #\Z))"));
      TEST_EQ("#\\Z", run(R"((char-upcase #\z))"));

      TEST_EQ("#t", run(R"((char-ci=? #\A #\A))"));
      TEST_EQ("#t", run(R"((char-ci=? #\A #\a))"));
      TEST_EQ("#f", run(R"((char-ci=? #\b #\a))"));
      TEST_EQ("#f", run(R"((char-ci=? #\B #\a))"));
      TEST_EQ("#f", run(R"((char-ci=? #\b #\A))"));
      TEST_EQ("#f", run(R"((char-ci=? #\B #\A))"));

      TEST_EQ("#f", run(R"((char-ci<? #\A #\A))"));
      TEST_EQ("#f", run(R"((char-ci<? #\A #\a))"));
      TEST_EQ("#f", run(R"((char-ci<? #\b #\a))"));
      TEST_EQ("#f", run(R"((char-ci<? #\B #\a))"));
      TEST_EQ("#f", run(R"((char-ci<? #\b #\A))"));
      TEST_EQ("#f", run(R"((char-ci<? #\B #\A))"));

      TEST_EQ("#f", run(R"((char-ci>? #\A #\A))"));
      TEST_EQ("#f", run(R"((char-ci>? #\A #\a))"));
      TEST_EQ("#t", run(R"((char-ci>? #\b #\a))"));
      TEST_EQ("#t", run(R"((char-ci>? #\B #\a))"));
      TEST_EQ("#t", run(R"((char-ci>? #\b #\A))"));
      TEST_EQ("#t", run(R"((char-ci>? #\B #\A))"));

      TEST_EQ("#t", run(R"((char-ci<=? #\A #\A))"));
      TEST_EQ("#t", run(R"((char-ci<=? #\A #\a))"));
      TEST_EQ("#f", run(R"((char-ci<=? #\b #\a))"));
      TEST_EQ("#f", run(R"((char-ci<=? #\B #\a))"));
      TEST_EQ("#f", run(R"((char-ci<=? #\b #\A))"));
      TEST_EQ("#f", run(R"((char-ci<=? #\B #\A))"));

      TEST_EQ("#t", run(R"((char-ci>=? #\A #\A))"));
      TEST_EQ("#t", run(R"((char-ci>=? #\A #\a))"));
      TEST_EQ("#t", run(R"((char-ci>=? #\b #\a))"));
      TEST_EQ("#t", run(R"((char-ci>=? #\B #\a))"));
      TEST_EQ("#t", run(R"((char-ci>=? #\b #\A))"));
      TEST_EQ("#t", run(R"((char-ci>=? #\B #\A))"));

      TEST_EQ("#t", run(R"((char-upper-case? #\B))"));
      TEST_EQ("#f", run(R"((char-upper-case? #\b))"));
      TEST_EQ("#f", run(R"((char-upper-case? #\3))"));
      TEST_EQ("#f", run(R"((char-lower-case? #\B))"));
      TEST_EQ("#t", run(R"((char-lower-case? #\b))"));
      TEST_EQ("#f", run(R"((char-lower-case? #\3))"));
      TEST_EQ("#t", run(R"((char-alphabetic? #\B))"));
      TEST_EQ("#t", run(R"((char-alphabetic? #\b))"));
      TEST_EQ("#f", run(R"((char-alphabetic? #\3))"));
      TEST_EQ("#f", run(R"((char-numeric? #\B))"));
      TEST_EQ("#f", run(R"((char-numeric? #\b))"));
      TEST_EQ("#t", run(R"((char-numeric? #\3))"));
      TEST_EQ("#f", run(R"((char-whitespace? #\B))"));
      TEST_EQ("#f", run(R"((char-whitespace? #\b))"));
      TEST_EQ("#f", run(R"((char-whitespace? #\3))"));
      TEST_EQ("#t", run(R"((char-whitespace? #\space))"));
      TEST_EQ("#t", run(R"((char-whitespace? #\newline))"));
      TEST_EQ("#t", run(R"((char-whitespace? #\tab))"));
      }
    };

  struct fx_comp : public compile_fixture {
    void test()
      {
      TEST_EQ("#f", run("(fx=? 12 13)"));
      TEST_EQ("#f", run("(fx=? 13 12)"));
      TEST_EQ("#t", run("(fx=? 12 12)"));

      TEST_EQ("#t", run("(fx<? 12 13)"));
      TEST_EQ("#f", run("(fx<? 13 12)"));
      TEST_EQ("#f", run("(fx<? 12 12)"));

      TEST_EQ("#f", run("(fx>? 12 13)"));
      TEST_EQ("#t", run("(fx>? 13 12)"));
      TEST_EQ("#f", run("(fx>? 12 12)"));

      TEST_EQ("#t", run("(fx<=? 12 13)"));
      TEST_EQ("#f", run("(fx<=? 13 12)"));
      TEST_EQ("#t", run("(fx<=? 12 12)"));

      TEST_EQ("#f", run("(fx>=? 12 13)"));
      TEST_EQ("#t", run("(fx>=? 13 12)"));
      TEST_EQ("#t", run("(fx>=? 12 12)"));

      TEST_EQ("25", run("(fx+ 12 13)"));
      TEST_EQ("25", run("(fx+ 13 12)"));
      TEST_EQ("24", run("(fx+ 12 12)"));
      TEST_EQ("-1", run("(fx- 12 13)"));
      TEST_EQ("1", run("(fx- 13 12)"));
      TEST_EQ("0", run("(fx- 12 12)"));
      TEST_EQ("156", run("(fx* 12 13)"));
      TEST_EQ("156", run("(fx* 13 12)"));
      TEST_EQ("144", run("(fx* 12 12)"));
      TEST_EQ("-156", run("(fx* 12 -13)"));
      TEST_EQ("-156", run("(fx* -13 12)"));
      TEST_EQ("-144", run("(fx* 12 -12)"));
      TEST_EQ("6", run("(fx/ 12 2)"));
      TEST_EQ("1", run("(fx/ 13 12)"));
      TEST_EQ("2", run("(fx/ 12 5)"));
      TEST_EQ("-6", run("(fx/ -12 2)"));
      TEST_EQ("-1", run("(fx/ 13 -12)"));
      TEST_EQ("-2", run("(fx/ -12 5)"));
      }
    };

  struct named_let : public compile_fixture {
    void test()
      {
      std::string script = R"(
(let loop ([numbers '(3 -2 1 6 -5)]
           [nonneg '()]
           [neg '()])
  (cond [(null? numbers) (list nonneg neg)]
        [(>= (car numbers) 0) (loop (cdr numbers)
                                    (cons (car numbers) nonneg)
                                    neg)]
        [(< (car numbers) 0) (loop (cdr numbers)
                                    nonneg
                                    (cons (car numbers) neg))]
   ))
)";
      TEST_EQ("((6 1 3) (-5 -2))", run(script));
      }
    };

  struct list_ops : public compile_fixture {
    void test()
      {
      build_libs();
      TEST_EQ("#t", run("(list? '(1 2 3))"));
      TEST_EQ("#f", run("(list? (cons 1 2))"));
      TEST_EQ("#t", run("(list? (cons 1 (cons 2 (cons 3 ()))))"));
      TEST_EQ("#f", run("(list? (cons 1 (cons 2 (cons 3 4))))"));
      TEST_EQ("#t", run("(list? (list 1 2 3 4 5))"));
      TEST_EQ("#f", run("(list? (vector 1 2 3 4 5))"));
      TEST_EQ("#f", run("(list? \"a string\")"));

      TEST_EQ("(a (b) (c))", run("(append '(a (b)) '((c)))"));
      TEST_EQ("a", run("(append '() 'a)"));
      TEST_EQ("(4 5 6)", run("(list-tail (list 1 2 3 4 5 6) 3)"));
      TEST_EQ("(4 5 6)", run("(list-tail '(1 2 3 4 5 6) 3)"));
      TEST_EQ("(4 5 6)", run("(list-tail '(1 2 3 4 5 6) 3)"));
      TEST_EQ("4", run("(list-ref '(1 2 3 4 5 6) 3)"));
      TEST_EQ("4", run("(list-ref (list 1 2 3 4 5 6) 3)"));
      TEST_EQ("c", run("(list-ref '(a b c d) 2)"));
      TEST_EQ("c", run("(list-ref '(a b c d) (inexact->exact (round 1.8)))"));

      TEST_EQ("((a 1) (b 2) (c 3))", run("(define e (list (list 'a 1) (list 'b 2) (list 'c 3)))"));
      TEST_EQ("(a 1)", run("(assq 'a e)"));
      TEST_EQ("(b 2)", run("(assq 'b e)"));
      TEST_EQ("(c 3)", run("(assq 'c e)"));
      TEST_EQ("#f", run("(assq 'd e)"));
      TEST_EQ("((a))", run("(assoc (list 'a) '(((a)) ((b)) ((c))))"));
      TEST_EQ("(5 7)", run("(assv 5 '((2 3) (5 7) (11 13)))"));
      TEST_EQ("((a 1) (b 2) (c 3))", run("(set! e '((a 1 ) ( b 2) (c 3)) )"));
      TEST_EQ("(a 1)", run("(assv 'a e)"));
      TEST_EQ("#f", run("(assv 'd e)"));
      TEST_EQ("(a 1)", run("(assq 'a e)"));
      TEST_EQ("(a b)", run("(memq 'a '(a b))"));

      TEST_EQ("(5 4 3 2 1)", run("(reverse (list 1 2 3 4 5))"));
      }
    };

  struct symbol_ops : public compile_fixture {
    void test()
      {
      build_string_to_symbol();

      TEST_EQ("#t", run("(symbol? 'foo)"));
      TEST_EQ("#t", run("(symbol? (car '(a b)))"));
      TEST_EQ("#f", run("(symbol? \"bar\")"));
      TEST_EQ("#t", run("(symbol? 'nil)"));
      TEST_EQ("#f", run("(symbol? '())"));
      TEST_EQ("#f", run("(symbol? #f)"));

      TEST_EQ("abcdefghabcdefghabcdefghabcdefgh1", run("(quote abcdefghabcdefghabcdefghabcdefgh1)"));
      TEST_EQ("abcdefghabcdefghabcdefghabcdefgh2", run("(quote abcdefghabcdefghabcdefghabcdefgh2)"));

      TEST_EQ("\"sym\"", run("(symbol->string 'sym)"));
      TEST_EQ("\"thisisaverylongsymbol\"", run("(symbol->string 'thisisaverylongsymbol)"));
      TEST_EQ("runtime error: symbol->string: contract violation", run("(symbol->string '())"));

      TEST_EQ("((a 1) (b 2) (c 3))", run("(define e '((a 1 ) ( b 2) (c 3)) )"));
      TEST_EQ("(a 1)", run("(assv 'a e)"));
      TEST_EQ("#f", run("(assv 'd e)"));
      TEST_EQ("(a 1)", run("(assq 'a e)"));
      TEST_EQ("(a b)", run("(memq 'a '(a b))"));
      }
    };

  struct string_ops : public compile_fixture {
    void test()
      {
      build_libs();

      TEST_EQ("\"sym\"", run("(string-copy \"sym\")"));
      TEST_EQ("\"thisisaverylongstring\"", run("(string-copy \"thisisaverylongstring\")"));
      TEST_EQ("runtime error: string-copy: contract violation", run("(string-copy '())"));

      TEST_EQ("#f", run("(let ([s \"str\"])  (eq? s (string-copy s)))"));
      TEST_EQ("#t", run("(let ([s \"str\"])  (%eqv? s (string-copy s)))"));
      TEST_EQ("#t", run("(let ([s \"str\"])  (equal? s (string-copy s)))"));

      TEST_EQ("#t", run("(string=? \"Jan\" \"Jan\")"));
      TEST_EQ("#f", run("(string=? \"Jan\" \"Jan2\")"));
      TEST_EQ("#f", run("(string=? \"Jan\" 'Jan)"));

      TEST_EQ("runtime error: compare-strings: contract violation", run("(compare-strings \"Jan\" \"Jan\")"));
      TEST_EQ("0", run("(compare-strings \"bbb\" \"bbb\" 3)"));
      TEST_EQ("-1", run("(compare-strings \"aaa\" \"bbb\" 3)"));
      TEST_EQ("1", run("(compare-strings \"ccc\" \"bbb\" 3)"));
      TEST_EQ("-1", run("(compare-strings \"aaa\" \"abb\" 3)"));
      TEST_EQ("1", run("(compare-strings \"ccc\" \"cbb\" 3)"));

      TEST_EQ("#t", run(R"((string<? "abc" "def"))"));
      TEST_EQ("#f", run(R"((string<? "def" "abc"))"));
      TEST_EQ("#t", run(R"((string<? "ababc" "abdef"))"));
      TEST_EQ("#f", run(R"((string<? "abdef" "ababc"))"));
      TEST_EQ("#t", run(R"((string<? "abc" "abcd"))"));
      TEST_EQ("#f", run(R"((string<? "abcd" "abc"))"));
      TEST_EQ("#f", run(R"((string<? "abc" "abc"))"));

      TEST_EQ("#f", run(R"((string>? "abc" "def"))"));
      TEST_EQ("#t", run(R"((string>? "def" "abc"))"));
      TEST_EQ("#f", run(R"((string>? "ababc" "abdef"))"));
      TEST_EQ("#t", run(R"((string>? "abdef" "ababc"))"));
      TEST_EQ("#f", run(R"((string>? "abc" "abcd"))"));
      TEST_EQ("#t", run(R"((string>? "abcd" "abc"))"));
      TEST_EQ("#f", run(R"((string>? "abc" "abc"))"));

      TEST_EQ("#t", run(R"((string<=? "abc" "def"))"));
      TEST_EQ("#f", run(R"((string<=? "def" "abc"))"));
      TEST_EQ("#t", run(R"((string<=? "ababc" "abdef"))"));
      TEST_EQ("#f", run(R"((string<=? "abdef" "ababc"))"));
      TEST_EQ("#t", run(R"((string<=? "abc" "abcd"))"));
      TEST_EQ("#f", run(R"((string<=? "abcd" "abc"))"));
      TEST_EQ("#t", run(R"((string<=? "abc" "abc"))"));

      TEST_EQ("#f", run(R"((string>=? "abc" "def"))"));
      TEST_EQ("#t", run(R"((string>=? "def" "abc"))"));
      TEST_EQ("#f", run(R"((string>=? "ababc" "abdef"))"));
      TEST_EQ("#t", run(R"((string>=? "abdef" "ababc"))"));
      TEST_EQ("#f", run(R"((string>=? "abc" "abcd"))"));
      TEST_EQ("#t", run(R"((string>=? "abcd" "abc"))"));
      TEST_EQ("#t", run(R"((string>=? "abc" "abc"))"));

      TEST_EQ("runtime error: string-length: contract violation", run(R"((string>=? "abc" 5))"));

      TEST_EQ(R"((#\a #\b #\c))", run(R"((string->list "abc"))"));
      TEST_EQ(R"("abc")", run(R"((list->string (list #\a #\b #\c)))"));
      TEST_EQ(R"("")", run(R"((list->string '()))"));

      TEST_EQ("\"a\"", run("(char->string #\\a)"));

      TEST_EQ("runtime error: compare-strings-ci: contract violation", run("(compare-strings-ci \"Jan\" \"Jan\")"));
      TEST_EQ("0", run("(compare-strings-ci \"bbb\" \"bbb\" 3)"));
      TEST_EQ("0", run("(compare-strings-ci \"bbb\" \"bBb\" 3)"));
      TEST_EQ("0", run("(compare-strings-ci \"BBb\" \"bBb\" 3)"));
      TEST_EQ("-1", run("(compare-strings-ci \"aaa\" \"bbb\" 3)"));
      TEST_EQ("1", run("(compare-strings-ci \"ccc\" \"bbb\" 3)"));
      TEST_EQ("-1", run("(compare-strings-ci \"aaa\" \"abb\" 3)"));
      TEST_EQ("1", run("(compare-strings-ci \"ccc\" \"cbb\" 3)"));


      TEST_EQ("#t", run(R"((string-ci<? "abc" "DEF"))"));
      TEST_EQ("#f", run(R"((string-ci<? "def" "ABC"))"));
      TEST_EQ("#t", run(R"((string-ci<? "ababc" "aBDEf"))"));
      TEST_EQ("#f", run(R"((string-ci<? "abdef" "aBABc"))"));
      TEST_EQ("#t", run(R"((string-ci<? "abc" "abCd"))"));
      TEST_EQ("#f", run(R"((string-ci<? "abcd" "Abc"))"));
      TEST_EQ("#f", run(R"((string-ci<? "abc" "abC"))"));

      TEST_EQ("#f", run(R"((string-ci>? "abc" "DEF"))"));
      TEST_EQ("#t", run(R"((string-ci>? "def" "ABC"))"));
      TEST_EQ("#f", run(R"((string-ci>? "ababc" "aBDEf"))"));
      TEST_EQ("#t", run(R"((string-ci>? "abdef" "aBABc"))"));
      TEST_EQ("#f", run(R"((string-ci>? "abc" "abCd"))"));
      TEST_EQ("#t", run(R"((string-ci>? "abcd" "Abc"))"));
      TEST_EQ("#f", run(R"((string-ci>? "abc" "abC"))"));

      TEST_EQ("#t", run(R"((string-ci<=? "abc" "DEF"))"));
      TEST_EQ("#f", run(R"((string-ci<=? "def" "ABC"))"));
      TEST_EQ("#t", run(R"((string-ci<=? "ababc" "aBDEf"))"));
      TEST_EQ("#f", run(R"((string-ci<=? "abdef" "aBABc"))"));
      TEST_EQ("#t", run(R"((string-ci<=? "abc" "abCd"))"));
      TEST_EQ("#f", run(R"((string-ci<=? "abcd" "Abc"))"));
      TEST_EQ("#t", run(R"((string-ci<=? "abc" "abC"))"));

      TEST_EQ("#f", run(R"((string-ci>=? "abc" "DEF"))"));
      TEST_EQ("#t", run(R"((string-ci>=? "def" "ABC"))"));
      TEST_EQ("#f", run(R"((string-ci>=? "ababc" "aBDEf"))"));
      TEST_EQ("#t", run(R"((string-ci>=? "abdef" "aBABc"))"));
      TEST_EQ("#f", run(R"((string-ci>=? "abc" "abCd"))"));
      TEST_EQ("#t", run(R"((string-ci>=? "abcd" "Abc"))"));
      TEST_EQ("#t", run(R"((string-ci>=? "abc" "abC"))"));

      TEST_EQ("#f", run(R"((string-ci=? "abc" "DEF"))"));
      TEST_EQ("#f", run(R"((string-ci=? "def" "ABC"))"));
      TEST_EQ("#f", run(R"((string-ci=? "ababc" "aBDEf"))"));
      TEST_EQ("#f", run(R"((string-ci=? "abdef" "aBABc"))"));
      TEST_EQ("#f", run(R"((string-ci=? "abc" "abCd"))"));
      TEST_EQ("#f", run(R"((string-ci=? "abcd" "Abc"))"));
      TEST_EQ("#t", run(R"((string-ci=? "abc" "abC"))"));

      TEST_EQ(R"(("abc" 3 2.1))", run(R"((vector->list (vector "abc" 3 2.1)))"));
      TEST_EQ(R"(#(#\a #\b #\c))", run(R"((list->vector (list #\a #\b #\c)))"));
      TEST_EQ(R"(#())", run(R"((list->vector '()))"));
      TEST_EQ("(dah dah didah)", run("(vector->list '#(dah dah didah))"));
      TEST_EQ("#(dididit dah)", run("(list->vector '(dididit dah))"));

      TEST_EQ("#(#t #t #t)", run("(let ([v (vector 1 2 3)]) (vector-fill! v #t) v)"));

      TEST_EQ("\"zzzzzzzzzzzzzzzzzzzz\"", run("(let ([s \"thisismylongerstring\"]) (string-fill! s #\\z) s)"));

      TEST_EQ("\"cdefgh\"", run("(substring \"abcdefghijklm\" 2 8)"));
      TEST_EQ("\"cdefghijklm\"", run("(substring \"abcdefghijklm\" 2 13)"));
      TEST_EQ("\"cdefghijklm\"", run("(substring \"abcdefghijklm\" 2 14)"));
      TEST_EQ("\"cdefghijklm\"", run("(substring \"abcdefghijklm\" 2 15)"));
      TEST_EQ("runtime error: substring: out of bounds", run("(substring \"abcdefghijklm\" 2 16)"));
      TEST_EQ("runtime error: substring: out of bounds", run("(substring \"abcdefghijklm\" 2 35)"));

      TEST_EQ("\"abcdef\"", run("(string-append1 \"abc\" \"def\")"));
      TEST_EQ("\"abcddefg\"", run("(string-append1 \"abcd\" \"defg\")"));
      TEST_EQ("\"abcdefghijklmn\"", run("(string-append1 \"abcdefg\" \"hijklmn\")"));
      TEST_EQ("\"abcdefg1hijklmn2\"", run("(string-append1 \"abcdefg1\" \"hijklmn2\")"));

      TEST_EQ("\"\"", run("(string-append)"));
      TEST_EQ("\"Jan\"", run("(string-append \"Jan\")"));
      TEST_EQ("\"abcdef\"", run("(string-append \"abc\" \"def\")"));
      TEST_EQ("\"abcddefg\"", run("(string-append \"abcd\" \"defg\")"));
      TEST_EQ("\"abcdefghijklmn\"", run("(string-append \"abcdefg\" \"hijklmn\")"));
      TEST_EQ("\"abcdefg1hijklmn2\"", run("(string-append \"abcdefg1\" \"hijklmn2\")"));
      TEST_EQ("\"abcdefghijklm\"", run("(string-append \"abc\" \"def\" \"ghijklm\")"));

      TEST_EQ("63", run("(string-hash \"abc\")"));
      TEST_EQ("49", run("(string-hash \"ABC\")"));
      TEST_EQ("198", run("(string-hash \"a\")"));
      TEST_EQ("28", run("(string-hash \"aa\")"));
      TEST_EQ("209", run("(string-hash \"aaa\")"));
      TEST_EQ("57", run("(string-hash \"A\")"));
      TEST_EQ("32", run("(string-hash \"AA\")"));
      TEST_EQ("201", run("(string-hash \"AAA\")"));

      TEST_EQ("7", run("(string-hash (symbol->string 'e))"));
      TEST_EQ("23", run("(string-hash (symbol->string 'r))"));
      TEST_EQ("28", run("(string-hash \"aa\")"));
      TEST_EQ("32", run("(string-hash \"AA\")"));
      TEST_EQ("48", run("(string-hash (symbol->string 'o))"));
      TEST_EQ("49", run("(string-hash \"ABC\")"));
      TEST_EQ("57", run("(string-hash \"A\")"));
      TEST_EQ("59", run("(string-hash (symbol->string 'v))"));
      TEST_EQ("63", run("(string-hash \"abc\")"));
      TEST_EQ("69", run("(string-hash (symbol->string 'd))"));
      TEST_EQ("71", run("(string-hash (symbol->string 'f))"));
      TEST_EQ("72", run("(string-hash (symbol->string 'h))"));
      TEST_EQ("90", run("(string-hash (symbol->string 's))"));
      TEST_EQ("91", run("(string-hash (symbol->string 't))"));
      TEST_EQ("108", run("(string-hash (symbol->string 'm))"));
      TEST_EQ("109", run("(string-hash (symbol->string 'i))"));
      TEST_EQ("115", run("(string-hash (symbol->string 'l))"));
      TEST_EQ("121", run("(string-hash (symbol->string 'ad))"));
      TEST_EQ("123", run("(string-hash (symbol->string 'u))"));

      TEST_EQ("129", run("(string-hash (symbol->string 'z))"));
      TEST_EQ("129", run("(string-hash (symbol->string 'ah))"));

      TEST_EQ("130", run("(string-hash (symbol->string 'y))"));
      TEST_EQ("134", run("(string-hash (symbol->string 'j))"));
      TEST_EQ("137", run("(string-hash (symbol->string 'k))"));
      TEST_EQ("146", run("(string-hash (symbol->string 'g))"));
      TEST_EQ("156", run("(string-hash (symbol->string 'x))"));
      TEST_EQ("162", run("(string-hash (symbol->string 'w))"));
      TEST_EQ("174", run("(string-hash (symbol->string 'p))"));
      TEST_EQ("187", run("(string-hash (symbol->string 'af))"));
      TEST_EQ("195", run("(string-hash (symbol->string 'q))"));
      TEST_EQ("198", run("(string-hash \"a\")"));
      TEST_EQ("201", run("(string-hash \"AAA\")"));
      TEST_EQ("209", run("(string-hash \"aaa\")"));
      TEST_EQ("210", run("(string-hash (symbol->string 'n))"));
      TEST_EQ("246", run("(string-hash (symbol->string 'Martin))"));
      }
    };

  struct gcd_ops : public compile_fixture {
    void test()
      {
      build_libs();
      TEST_EQ("4", run("(gcd1 32 -36)"));
      TEST_EQ("4", run("(gcd1 32.0 -36)"));
      TEST_EQ("4", run("(gcd 32 -36)"));
      TEST_EQ("4", run("(gcd 32.0 -36)"));
      TEST_EQ("0", run("(gcd)"));
      TEST_EQ("2", run("(gcd 2)"));
      TEST_EQ("4", run("(gcd 32.0 -36 16 -4)"));
      TEST_EQ("2", run("(gcd 32.0 -36 16 -4 6)"));
      TEST_EQ("288", run("(lcm 32 -36)"));
      TEST_EQ("288", run("(lcm 32.0 -36)"));
      TEST_EQ("1", run("(lcm)"));
      TEST_EQ("5", run("(lcm 5.0)"));
      }
    };

  struct string_to_symbol_ops : public compile_fixture {
    void test()
      {
      build_string_to_symbol();
      TEST_EQ("abc", run("(string->symbol \"abc\")"));
      TEST_EQ("abc", run("(string->symbol \"abc\")"));
      TEST_EQ("#t", run("(eq? (string->symbol \"abc\") (string->symbol \"abc\"))"));
      TEST_EQ("#t", run("(eqv? (string->symbol \"abc\") (string->symbol \"abc\"))"));
      TEST_EQ("#t", run("(equal? (string->symbol \"abc\") (string->symbol \"abc\"))"));
      TEST_EQ("#f", run("(eq? (string->symbol \"abc\") (string->symbol \"def\"))"));
      }
    };

  struct control_ops : public compile_fixture {
    void test()
      {
      build_string_to_symbol();
      build_apply();
      build_r5rs();
      TEST_EQ("(b e h)", run("(map cadr '((a b) (d e) (g h)))"));
      TEST_EQ("(1 4 27 256 3125)", run("(map (lambda (n) (expt n n)) '(1 2 3 4 5))"));
      TEST_EQ("(5 7 9)", run("(map + '(1 2 3) '(4 5 6))"));
      TEST_EQ("(1 2)", run("(let ([count 0]) (map (lambda (ignored) (set! count (+ count 1)) count) '(a b)))"));

      TEST_EQ("#(0 1 4 9 16)", run("(let ([v (make-vector 5)]) (for-each (lambda (i) (vector-set! v i (* i i))) '(0 1 2 3 4)) v)"));

      TEST_EQ("3", run("3 #;(let ([v (make-vector 5)]) (for-each (lambda (i) (vector-set! v i (* i i))) '(0 1 2 3 4)) v)"));

      TEST_EQ("#(0 1 4 9 16)", run(R"(
#|
Here is a multiline comment, similar
to /* and */ in c/c++
|#
(let ([v (make-vector 5)]) 
     (for-each #; here is comment
        (lambda (i) (vector-set! v i (* i i))) ; here is also comment
        '(0 1 2 3 4)) 
     v) #; blabla
)"));

      TEST_EQ("1", run("(%slot-ref (cons 1 2) 0)"));
      TEST_EQ("2", run("(%slot-ref (cons 1 2) 1)"));

      TEST_EQ("<promise>", run(R"(
(define p
    (let ([result-ready? #f]
          [result #f])
          (%make-promise (lambda ()
                            (if result-ready?
                                result
                                (let ([x (+ 1 2)])
                                    (if result-ready?
                                        result
                                        (begin (set! result-ready? #t)
                                                (set! result x)
                                                result
                                        ))))))))
)"));

      TEST_EQ("#t", run("(closure? (%slot-ref p 0))"));
      TEST_EQ("#t", run("(promise? p)"));
      TEST_EQ("3", run("(force p)"));
      TEST_EQ("3", run("(force p)"));

      TEST_EQ("<promise>", run("(set! p (make-promise (lambda () (+ 5 6))))"));
      TEST_EQ("#t", run("(closure? (%slot-ref p 0))"));
      TEST_EQ("#t", run("(promise? p)"));
      TEST_EQ("11", run("(force p)"));
      TEST_EQ("11", run("(force p)"));
      TEST_EQ("13", run("(force 13)"));

      TEST_EQ("<promise>", run("(set! p (delay (* 3 4)))"));
      TEST_EQ("#t", run("(closure? (%slot-ref p 0))"));
      TEST_EQ("#t", run("(promise? p)"));
      TEST_EQ("12", run("(force p)"));
      TEST_EQ("12", run("(force p)"));
      }
    };

  struct do_ops : public compile_fixture {
    void test()
      {
      std::string script = R"(
(do ([vec (make-vector 5)] [i 0 (+ i 1)])
    ((= i 5) vec)
    (vector-set! vec i i))
)";
      TEST_EQ("#(0 1 2 3 4)", run(script));
      }
    };


  struct no_square_brackets : public compile_fixture {
    void test()
      {
      std::string script = R"(
(do ((vec (make-vector 5)) (i 0 (+ i 1)))
    ((= i 5) vec)
    (vector-set! vec i i))
)";
      TEST_EQ("#(0 1 2 3 4)", run(script));
      TEST_EQ("13", run("(cond(#f)(#f 12)(12 13)) "));
      TEST_EQ("1287", run("(let((b #t))(cond (else 1287))) "));
      TEST_EQ("2", run("(cond((cons 1 2) => (lambda(x) (cdr x)))) "));

      TEST_EQ("75", run("(case (+ 7 3) ((10 11 12) 75))"));
      TEST_EQ("41", run("(letrec ((f (lambda (x y) (+ x y))) (g (lambda(x) (+ x 12))))(f 16 (f (g 0) (+ 1 (g 0)))))"));
      }
    };

  struct calcc_extended : public compile_fixture_skiwi {
    void test()
      {
      build_values();
      build_dynamic_wind();
      TEST_EQ("1", run("(values 1)"));
      TEST_EQ("((multiple . values) 1 2)", run("(values 1 2)"));
      TEST_EQ("((multiple . values) 1 2 3)", run("(values 1 2 3)"));

      TEST_EQ("3", run("(call-with-values (lambda () (values 1 2)) +)"));
      TEST_EQ("runtime error: <lambda>: invalid number of arguments:", first_line(run("(call-with-values (lambda () 1) (lambda (x y) (+ x y)))")));

      TEST_EQ("5", run("(call-with-values (lambda () (values 4 5)) (lambda (a b) b))"));
      TEST_EQ("-1", run("(call-with-values * -)"));

      TEST_EQ("(2 1)", run("(define lst '()) (define before (lambda () (set! lst (cons 1 lst)))) (define thunk (lambda () (set! lst (cons 2 lst))))  (define after (lambda () (set! lst (cons 3 lst)))) (dynamic-wind before thunk after)"));
      TEST_EQ("(3 2 1)", run("(define lst '()) (define before (lambda () (set! lst (cons 1 lst)))) (define thunk (lambda () (set! lst (cons 2 lst))))  (define after (lambda () (set! lst (cons 3 lst)))) (dynamic-wind before thunk after) lst"));

      auto script =
        R"(
(let ((path '())
      (c #f))
  (let ((add (lambda (s)
              (set! path (cons s path)))))
    (dynamic-wind
      (lambda () (add 'connect))
      (lambda ()
        (add (call-with-current-continuation
              (lambda (c0)
                (set! c c0)
                'talk1))))
      (lambda () (add 'disconnect)))
    (if (< (length path) 4)
        (c 'talk2)
        (reverse path))))
)";

      TEST_EQ("(connect talk1 disconnect connect talk2 disconnect)", run(script));
      }
    };


  struct quasiquote_tests : public compile_fixture {
    void test()
      {
      build_string_to_symbol();
      build_apply();
      build_r5rs();
      TEST_EQ("(list 3 4)", run("`(list ,(+ 1 2) 4)"));
      TEST_EQ("(list a (quote a))", run("(let ((name 'a)) `(list ,name ',name))"));
      TEST_EQ("(a 3 4 5 6 b)", run("`(a ,(+ 1 2) ,@(map abs '(4 -5 6)) b)"));
      TEST_EQ("((foo 7) . cons)", run("`((foo ,(- 10 3)) ,@(cdr '(c)) . ,(car '(cons)))"));
      TEST_EQ("#(10 5 2 4 3 8)", run("`#(10 5 ,(sqrt 4) ,@(map sqrt '(16 9)) 8)"));
      TEST_EQ("(a (quasiquote (b c)))", run("`(a `(b c))"));
      TEST_EQ("(a (quasiquote (b c (quote d))))", run("`(a `(b c 'd))"));
      TEST_EQ("(a (quasiquote (b c (quasiquote d))))", run("`(a `(b c `d))"));
      TEST_EQ("(a (quasiquote (b c (quasiquote (quasiquote d)))))", run("`(a `(b c ``d))"));
      TEST_EQ("(a (quasiquote (b c (quasiquote (quote d)))))", run("`(a `(b c `'d))"));
      TEST_EQ("(quote (quote (quote d)))", run("`'''d"));
      TEST_EQ("(a (quasiquote (b (unquote (+ 1 2)))))", run("`(a `(b ,(+ 1 2)))"));
      TEST_EQ("(a (quasiquote (b (unquote (+ 1 2)) (unquote (foo 4 d)) e)) f)", run("`(a `(b ,(+ 1 2) ,(foo ,(+ 1 3) d) e) f)"));
      TEST_EQ("(a (quasiquote (b (unquote x) (unquote (quote y)) d)) e)", run("(let ((name1 'x) (name2 'y)) `(a `(b ,,name1 ,',name2 d) e))"));
      }
    };

  struct alternative_defines : public compile_fixture {
    void test()
      {
      build_string_to_symbol();
      build_apply();
      TEST_EQ("5", run("(define (lam x) x) (lam 5)"));
      TEST_EQ("23", run("(lam 23)"));
      TEST_EQ("15", run("(define (f . x) (apply + x)) (f 1 2 3 4 5)"));
      }
    };

  struct include_tests : public compile_fixture {
    void test()
      {
#ifdef _WIN32
      TEST_EQ("9", run("(include \"data\\\\include_test_1.scm\") result"));
      TEST_EQ("2", run("(include \"data\\\\include_test_2.scm\") result"));
      TEST_EQ("300", run("(define x (include \"data\\\\include_test_3.scm\"))"));
#else
      TEST_EQ("9", run("(include \"./data/include_test_1.scm\") result"));
      TEST_EQ("2", run("(include \"./data/include_test_2_linux.scm\") result"));
      TEST_EQ("300", run("(define x (include \"./data/include_test_3.scm\"))"));
#endif
      }
    };

  struct each_expression_in_program_as_separate_cps_tests : public compile_fixture {
    void test()
      {
      TEST_EQ("10", run("(define (twice x) (* x 2)) (twice 2) (twice 4) (twice 5)"));
      TEST_EQ("10", run("(define (twice x) (* x 2)) (begin (twice 2) (twice 4) (twice 5))"));
      TEST_EQ("5", run("(define (foo) (define x 5) x) (foo)"));
      }
    };

  struct bug_from_jaffer_test : public compile_fixture {
    void test()
      {
      build_string_to_symbol();
      build_apply();
      build_callcc();
      build_r5rs();

      TEST_EQ("#t", run("(define (get2) (let ((a 2)) a)) (define num? (lambda (x) (fixnum? x))) (num? (apply get2 '()))"));
      for (int i = 0; i < 10; ++i)
        TEST_EQ("#t", run("(string? (apply string-append1 (list \"foo\" \"bar\")))"));
      TEST_EQ("1", run("'#\\  1"));
      TEST_EQ("#\\ ", run("'#\\  #\\space"));

      TEST_EQ("#f", run("(equal? \"martin\" (symbol->string 'Martin))"));
      TEST_EQ("#f", run("(equal? \"martin\" \"Martin\")"));
      TEST_EQ("a", run("(quote a)"));
      TEST_EQ("(quote a)", run("(quote 'a)"));

      TEST_EQ("(a b c)", run("(quote (a b c))"));
      TEST_EQ("(a b c . d)", run("(quote (a b c . d))"));

      TEST_EQ("<lambda>", run("(define list-length (lambda(obj) (%call/cc (lambda(return) (letrec((r(lambda(obj) (cond((null? obj) 0) ((pair? obj) (+ (r (cdr obj)) 1)) (else (return #f)))))) (r obj))))))"));
      TEST_EQ("4", run("(list-length '(1 2 3 4))"));
      TEST_EQ("#f", run("(list-length '(a b . c))"));
      TEST_EQ("#f", run("(apply list-length (list '(a b . c)))"));

      TEST_EQ("#t", run("(eqv? #\\  #\\space)"));
      TEST_EQ("#t", run("(eqv? '#\\  #\\space)"));
      TEST_EQ("\"a\"", run("(substring \"ab\" 0 1)"));
#ifdef _WIN32      
      TEST_EQ("(1 2 3 4 5)", run("(include \"data\\\\include_test_4.scm\") (test)"));
#else
      TEST_EQ("(1 2 3 4 5)", run("(include \"./data/include_test_4_linux.scm\") (test)"));
#endif      
      }
    };

  struct jaffer_2_tests : public compile_fixture {
    void test()
      {

      build_string_to_symbol();
      build_apply();
      build_r5rs();
      TEST_EQ("", run("(error \"this is an error message\")"));
      TEST_EQ("", run("(error 'test \"this is an error message\")"));

      std::string script = R"(
(define old-+ +)
(define + (lambda (x y) (list y x)))
(+ 6 3)
)";
      TEST_EQ("(3 6)", run(script));

      std::string script2 = R"(
(define add3 (lambda (x) (+ 3 x)))
(add3 8)
)";
      TEST_EQ("(8 3)", run(script2));

      std::string script3 = R"(
(set! + old-+)
(+ 6 3)
)";
      TEST_EQ("9", run(script3));

      TEST_EQ("#t", run("(procedure? old-+)"));
      TEST_EQ("#t", run("(define plus old-+) (procedure? plus)"));
      TEST_EQ("#t", run("(procedure? +)"));
      TEST_EQ("0", run("(+)"));
      TEST_EQ("8", run("(define (tst) ((lambda () (+ var 3))))  (define var 5) (tst)"));


      TEST_EQ("8", run("(define x 5) (define add (lambda (y) (+ x y))) (add 3)"));
      TEST_EQ("13", run("(set! x 10) (add 3)"));
      TEST_EQ("103", run("(define x 100) (add 3)"));

      TEST_EQ("#t", run("(< (exact->inexact 2097151) (exact->inexact 2097152) (exact->inexact 2097153))"));
      TEST_EQ("#t", run("(< 1.1 1.2 1.3)"));

      TEST_EQ("1e+19", run("(str2num \"1.e19\" 10)"));
      TEST_EQ("1e+19", run("(string->number \"1.e19\")"));
      TEST_EQ("\"10000000000000000000.0\"", run("(number->string (string->number \"1.e19\"))"));
#ifdef _WIN32
      TEST_EQ("-nan(ind)", run("(string->number \"+nan.0\")"));
      TEST_EQ("-nan(ind)", run("(string->number \"-nan.0\")"));
#else
      TEST_EQ("-nan", run("(string->number \"+nan.0\")"));
      TEST_EQ("-nan", run("(string->number \"-nan.0\")"));
#endif
      TEST_EQ("inf", run("(string->number \"+inf.0\")"));
      TEST_EQ("-inf", run("(string->number \"-inf.0\")"));
      }
    };

  struct lambda_invalid_arg_tests : public compile_fixture {
    void test()
      {
      build_string_to_symbol();

      TEST_EQ("#f", run("(fx=? 1 2)"));
      TEST_EQ("#(1 2 3 4 5 6)", run("(let ([f (lambda (f) (f 1 2 3 4 5 6))]) (f vector))"));

      TEST_EQ("12", run("(let ([f (lambda () 12)]) (f))"));
      TEST_EQ("12", run("(let ([f (lambda () 12)]) (f 1))"));
      TEST_EQ("12", run("(let ([f (lambda () 12)]) (f 1 2))"));

      TEST_EQ("runtime error: <lambda>: invalid number of arguments", run("(let ([f (lambda (x) (fx+ x x))]) (f))"));
      TEST_EQ("4", run("(let ([f (lambda (x) (fx+ x x))]) (f 2))"));
      TEST_EQ("4", run("(let ([f (lambda (x) (fx+ x x))]) (f 2 3))"));
      TEST_EQ("runtime error: <lambda>: invalid number of arguments", run("(let ([f (lambda (x y) (fx* x (fx+ y y)))]) (f))"));
      TEST_EQ("runtime error: <lambda>: invalid number of arguments", run("(let ([f (lambda (x y) (fx* x (fx+ y y)))]) (f 1))"));
      TEST_EQ("12", run("(let ([f (lambda (x y) (fx* x (fx+ y y)))]) (f 2 3))"));

      TEST_EQ("()", run("(let ([f  (lambda x x)]) (f))"));
      TEST_EQ("(a)", run("(let ([f  (lambda x x)]) (f 'a))"));
      TEST_EQ("(a b)", run("(let ([f  (lambda x x)]) (f 'a 'b))"));
      TEST_EQ("(a b c)", run("(let ([f  (lambda x x)]) (f 'a 'b 'c))"));
      TEST_EQ("(a b c d)", run("(let ([f  (lambda x x)]) (f 'a 'b 'c 'd))"));

      TEST_EQ("runtime error: <lambda>: invalid number of arguments", run("(let ([f  (lambda (x . rest) (vector x rest))]) (f))"));
      TEST_EQ("#(a ())", run("(let ([f  (lambda (x . rest) (vector x rest))]) (f 'a ))"));
      TEST_EQ("#(a (b))", run("(let ([f  (lambda (x . rest) (vector x rest))]) (f 'a 'b ))"));
      TEST_EQ("#(a (b c))", run("(let ([f  (lambda (x . rest) (vector x rest))]) (f 'a 'b 'c))"));
      TEST_EQ("#(a (b c d))", run("(let ([f  (lambda (x . rest) (vector x rest))]) (f 'a 'b 'c 'd))"));

      TEST_EQ("runtime error: <lambda>: invalid number of arguments", run("(let ([f  (lambda (x y . rest) (vector x y rest))]) (f))"));
      TEST_EQ("runtime error: <lambda>: invalid number of arguments", run("(let ([f  (lambda (x y . rest) (vector x y rest))]) (f 'a ))"));
      TEST_EQ("#(a b ())", run("(let ([f  (lambda (x y . rest) (vector x y rest))]) (f 'a 'b ))"));
      TEST_EQ("#(a b (c))", run("(let ([f  (lambda (x y . rest) (vector x y rest))]) (f 'a 'b 'c))"));
      TEST_EQ("#(a b (c d))", run("(let ([f  (lambda (x y . rest) (vector x y rest))]) (f 'a 'b 'c 'd))"));

      }
    };

  struct string_set_tests : public compile_fixture {
    void test()
      {
      build_string_to_symbol();

      TEST_EQ("2", run("(let((t 1)) (and (begin(set! t(fxadd1 t)) t) t))"));
      TEST_EQ("14", run("(let((f(if (boolean? (lambda() 12)) (lambda() 13) (lambda() 14)))) (f)) "));
      TEST_EQ("12", run("(let([f 12])  (let([g(lambda() f)])  (g))) "));
      TEST_EQ("#t", run("(fx<? 1 2) "));
      TEST_EQ("#f", run("(let([f(lambda(x y) (fx<? x y))])      (f 10 10)) "));
      TEST_EQ("#f", run("(fx<? 10 10) "));
      TEST_EQ("#f", run("(fx<? 10 2) "));
      TEST_EQ("#t", run("(fx<=? 1 2) "));
      TEST_EQ("#t", run("(fx<=? 10 10) "));
      TEST_EQ("#f", run("(fx<=? 10 2) "));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([x 12]) (string-set! x 0 #\\a))"));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([x(string #\\a #\\b #\\c)] [y 12])(string-set! x 0 y))"));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([x(string #\\a #\\b #\\c)]  [y 12])  (string-set! x 8 y)) "));
      TEST_EQ("runtime error: string-set!: out of bounds", run("(let([x(string #\\a #\\b #\\c)]   [y #\\a]) (string-set! x 8 y))"));
      TEST_EQ("runtime error: string-set!: out of bounds", run("(let([x(string #\\a #\\b #\\c)]) (string-set! x 8 #\\a))"));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([x(string #\\a #\\b #\\c)]   [y #\\a])     (string-set! x - 1 y))"));

      /*
      ; next the general case
      ;;; 6 kinds of errors :
      ;;;   string is either :
      ;;;     lex - non - string, run - non - string, lex - string, valid
      ;;;   index is either :
      ;;;     lex - invalid, runtime - non - fixnum, runtime - above, runtime - below, valid
      ;;;   char is either :
      ;;;     lex - invalid, runtime - non - char, valid.
      ;;;  that's 4x5x3 = 60 tests!
      ;;;  If we skip over the lexical string check, (since I don't do it),
      ;;;  we have : 2x5x3 = 30 tests.
      */

      TEST_EQ("\"aXc\"", run("(let([s(string #\\a #\\b #\\c)][i 1][c #\\X]) (string-set! s i c) s)"));
      TEST_EQ("\"aXc\"", run("(let([s(string #\\a #\\b #\\c)][i 1]) (string-set! s i #\\X) s)"));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([s(string #\\a #\\b #\\c)][i 1][c 'X]) (string-set!  s i c) s)"));
      TEST_EQ("\"aXc\"", run("(let([s(string #\\a #\\b #\\c)][i 1][c #\\X]) (string-set! s 1 c) s) "));
      TEST_EQ("\"aXc\"", run("(let([s(string #\\a #\\b #\\c)][i 1]) (string-set! s 1 #\\X) s)"));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([s(string #\\a #\\b #\\c)][i 1][c 'X]) (string-set!  s 1 c) s)"));

      TEST_EQ("\"abcX\"", run("(let([s(string #\\a #\\b #\\c)][i 3][c #\\X]) (string-set! s i c) s)"));
      TEST_EQ("\"abcX\"", run("(let([s(string #\\a #\\b #\\c)][i 3]) (string-set! s i #\\X) s)   "));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([s(string #\\a #\\b #\\c)][i 3][c 'X]) (string-set! s i c) s) "));
      TEST_EQ("runtime error: string-set!: out of bounds", run("(let([s(string #\\a #\\b #\\c)][i -10][c #\\X]) (string-set! s i c) s) "));
      TEST_EQ("runtime error: string-set!: out of bounds", run("(let([s(string #\\a #\\b #\\c)][i -11]) (string-set! s i #\\X) s)  "));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([s(string #\\a #\\b #\\c)][i -1][c 'X]) (string-set! s i c) s) "));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([s(string #\\a #\\b #\\c)][i 'foo] [c #\\X]) (string-set! s i c) s) "));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([s(string #\\a #\\b #\\c)][i 'foo]) (string-set! s i #\\X) s)  "));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([s(string #\\a #\\b #\\c)][i 'foo] [c 'X]) (string-set! s i c) s)   "));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([s '(string #\\a #\\b #\\c)] [i 1] [c #\\X]) (string-set! s i c) s)"));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([s '(string #\\a #\\b #\\c)] [i 1]) (string-set! s i #\\X) s)    "));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([s '(string #\\a #\\b #\\c)] [i 1] [c 'X]) (string-set! s i c) s)  "));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([s '(string #\\a #\\b #\\c)] [i 1] [c #\\X]) (string-set! s 1 c) s)  "));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([s '(string #\\a #\\b #\\c)] [i 1]) (string-set! s 1 #\\X) s) "));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([s '(string #\\a #\\b #\\c)] [i 1] [c 'X]) (string-set! s 1 c) s) "));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([s '(string #\\a #\\b #\\c)] [i 3] [c #\\X]) (string-set! s i c) s) "));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([s '(string #\\a #\\b #\\c)] [i 3]) (string-set! s i #\\X) s) "));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([s '(string #\\a #\\b #\\c)] [i 3] [c 'X]) (string-set! s i c) s)"));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([s '(string #\\a #\\b #\\c)] [i -10] [c #\\X]) (string-set! s i c) s)"));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([s '(string #\\a #\\b #\\c)] [i -11]) (string-set! s i #\\X) s) "));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([s '(string #\\a #\\b #\\c)] [i -1] [c 'X]) (string-set! s i c) s) "));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([s '(string #\\a #\\b #\\c)] [i 'foo][c #\\X]) (string-set! s i c) s) "));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([s '(string #\\a #\\b #\\c)] [i 'foo]) (string-set! s i #\\X) s) "));
      TEST_EQ("runtime error: string-set!: contract violation", run("(let([s '(string #\\a #\\b #\\c)] [i 'foo][c 'X]) (string-set! s i c) s)  "));

      TEST_EQ("\"abc\"", run("(let([f(lambda(a b c) (string a b c))]) (f #\\a #\\b #\\c)) "));
      //TEST_EQ("", run("(let([f(lambda(a b c) (string a b c))])  (f #\\a 12 #\\c))  ")); // right now, string does no checking, so we comment these tests out
      TEST_EQ("\"abc\"", run("(let([f string])   (f #\\a #\\b #\\c)) "));
      //TEST_EQ("", run("(let([f string])    (f #\\a #\\b 'x)) "));
      TEST_EQ("\"abc\"", run("(string #\\a #\\b #\\c)  "));
      //TEST_EQ("", run("(string #\\a #\\b #t) "));
      }
    };

  struct cps_bug_test : public compile_fixture {
    void test()
      {
      build_string_to_symbol();
      build_apply();
      build_r5rs();
      TEST_EQ("5", run("(define x 5)"));
      TEST_EQ("120", run("(let ([x 5]) (define fact (lambda (n) (if (<= n 1) 1 (* n (fact (- n 1)))))) (fact x))"));
      TEST_EQ("#t", run("(define (get2) (let ((a 2)) a)) (define num? (lambda (x) (fixnum? x))) (num? (apply get2 '()))"));

      std::string script = R"(
(define test
  (lambda (fun . args)
    ((lambda (res) #t) (apply fun args) )))    
;((lambda (res) #t) (fun (car args)) )))    
(define + (lambda (x y) (list y x))) 
(define add3 (lambda (y) (+ y 3))) 
(test add3 6)	      
)";
      TEST_EQ("#t", run(script));
      }
    };

  struct basic_string_ports : public compile_fixture_skiwi {
    void test()
      {
      build_srfi6();

      TEST_EQ("<port>: \"input-string\"", run("(define p (open-input-string \"21 34\"))"));
      TEST_EQ("#t", run("(input-port? p)"));
      TEST_EQ("21", run("(read p)"));
      TEST_EQ("34", run("(read p)"));
      TEST_EQ("#t", run("(eof-object? (peek-char p))"));

      TEST_EQ("<port>: \"input-string\"", run("(define p2 (open-input-string \"(a . (b . (c . ()))) 34\"))"));
      TEST_EQ("#t", run("(input-port? p2)"));
      TEST_EQ("(a b c)", run("(read p2)"));
      TEST_EQ("34", run("(read p2)"));
      TEST_EQ("#t", run("(eof-object? (peek-char p2))"));

      std::string script = R"(
(let ([q (open-output-string)]
      [x '(a b c)])
          (write (car x) q)
          (write (cdr x) q)
          (get-output-string q))
)";
      TEST_EQ("\"a(b c)\"", run(script));

      std::string script2 = R"(
(let ([os (open-output-string)])
          (write "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz" os)          
          (get-output-string os))
)";
      TEST_EQ("\"\"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\"\"", run(script2));
      }
    };

  struct format_tests : public compile_fixture_skiwi {
    void test()
      {
      build_srfi6();
      build_srfi28();

      TEST_EQ("\"Hello, World!\"", run("(format \"Hello, ~a\" \"World!\")"));
      TEST_EQ("\"Error, list is too short: (one \"two\" 3)\n\"", run("(format \"Error, list is too short: ~s~%\" '(one \"two\" 3))"));
      }
    };

  struct flonums_with_e : public compile_fixture {
    void test()
      {
      TEST_EQ("#f", run("(define e 3) (flonum? e)"));
      TEST_EQ("#f", run("(define e1 3) (flonum? e1)"));
      }
    };

  struct read_tests : public compile_fixture_skiwi {
    void test()
      {
      build_srfi6();
      TEST_EQ("<port>: \"input-string\"", run("(define p (open-input-string \"21 ;;;This is a comment: : : !@##$%$^6&*(\n5\"))"));
      TEST_EQ("21", run("(read p)"));
      TEST_EQ("5", run("(read p)"));

      TEST_EQ("<port>: \"input-string\"", run("(define p2 (open-input-string \"21 ;;;This is a comment: : : !@##$%$^6&*(\"))"));
      TEST_EQ("21", run("(read p2)"));
      TEST_EQ("#eof", run("(read p2)"));

      TEST_EQ("<port>: \"input-string\"", run("(define p3 (open-input-string \"(cons ': (value-show (binding-value binding)))\"))"));
      TEST_EQ("(cons (quote :) (value-show (binding-value binding)))", run("(read p3)"));

      TEST_EQ("<port>: \"input-string\"", run("(define p4 (open-input-string \"(< 1 2)\"))"));
      TEST_EQ("(< 1 2)", run("(read p4)"));

      TEST_EQ("<port>: \"input-string\"", run("(define p5 (open-input-string \"3.14 -2.1\"))"));
      TEST_EQ("3.14", run("(read p5)"));
      TEST_EQ("-2.1", run("(read p5)"));
      }
    };

  struct trace_tests : public compile_fixture {
    void test()
      {
      build_libs();

      std::string script = R"(
(define (wrap var proc)
  (lambda args
    (let ((r (apply proc args)))
	  (write (cons var args))
	  (display " ==> ")
	  (write r)
	  (newline)
	  r)))
)";
      run(script);
      TEST_EQ("3", run("(define (f a b) (+ a b)) (set! f (wrap 'f f)) (f 1 2)"));

      TEST_EQ("3", run("(set! + (wrap '+ +)) (+ 1 2)"));
      TEST_EQ("-1", run("(set! + -) (+ 1 2)"));
      }
    };


  struct bugs_from_compiler_scm : public compile_fixture
    {
    void test()
      {
      build_libs();
      TEST_EQ("#f", run("(integer? (vector 1 2))"));
      TEST_EQ("#t", run("(integer? 1)"));
      TEST_EQ("#t", run("(integer? 3.0)"));
      TEST_EQ("#f", run("(integer? 1.2)"));

      std::string script = "(define line (list \")\" \"1\" \",\" \"2\" \",\" \"3\" \"RETURN(L\" \" \" ))"
        "(apply string-append (reverse line))";
      TEST_EQ("\" RETURN(L3,2,1)\"", run(script));
      //expand_and_format(script);

      TEST_EQ("55", run("(define y (list 1 2 3 4 5 6 7 8 9 10)) (apply + y)"));
      TEST_EQ("\"abcdefg \"", run("(apply string-append (list \"a\" \"b\" \"c\" \"d\" \"e\" \"f\" \"g\" \" \"))"));

      //TEST_EQ("", run("(apply reverse (list 'a 'b 'c 'd 'e 'f 'g 'h))")); // Here better error checking. apply should work on (. arg), but not on (arg) with arg a list

      TEST_EQ(R"(("a" "b" "c" "d" "e" "f" "g" " "))", run("(list \"a\" \"b\" \"c\" \"d\" \"e\" \"f\" \"g\" \" \")"));
      TEST_EQ("9240", run("(apply lcm (list 2 4 6 8 10 12 14 11))"));
      TEST_EQ("9240", run("(lcm 2 4 6 8 10 12 14 11)"));

      TEST_EQ("55", run("(define (plus . args) (let ((res 0)) (let loop ((args args)) (if (##null? args) res (begin (set! res (+ res (car args)))  (loop (cdr args))))))) (plus 1 2 3 4 5 6 7 8 9 10)"));
      TEST_EQ("55", run("(define (plus . args) (let ((res 0)) (let loop ((args args)) (if (##null? args) res (begin (set! res (+ res (car args)))  (loop (cdr args))))))) (apply plus (list  1 2 3 4 5 6 7 8 9 10))"));

      TEST_EQ("\"abcdefg \"", run("(string-append \"a\" \"b\" \"c\" \"d\" \"e\" \"f\" \"g\" \" \")"));

      TEST_EQ("((1) (2) (3) (4) (5) (6) (7) (8))", run("(apply list (list (cons 1 '()) (cons 2 '()) (cons 3 '()) (cons 4 '()) (cons 5 '()) (cons 6 '()) (cons 7 '()) (cons 8 '())))"));
      }
    };

  struct bugs_from_compiler_2 : public compile_fixture
    {
    void test()
      {
      run("(define s1 \"abcdef\")");
      run("(define s2 \"gh\")");
      run("\"asdfalksdjf;alksdjf;alksdjf;lakdsjflkasjdf\"");
      run("(define s (string-append1 s1 s2))");
      TEST_EQ("\"abcdefgh\"", run("s"));
      TEST_EQ("#\\000", run("(string-ref s 9)"));
      TEST_EQ("#\\000", run("(string-ref s 10)"));
      TEST_EQ("#\\000", run("(string-ref s 11)"));
      TEST_EQ("#\\000", run("(string-ref s 12)"));
      }
    };

  struct macros : public compile_fixture_skiwi
    {
    void test()
      {
      build_srfi6();
      build_srfi28();
      TEST_EQ("8", run("(define-macro (eight) '(+ 3 5))  (eight)"));
      TEST_EQ("7", run("(define-macro (sum x y) `(+ ,x ,y))  (sum 3 4)"));
      TEST_EQ("10", run("(sum (+ 4 5) (- 4 3))"));
      TEST_EQ("10", run("(define-macro (when2 test . args) `(if ,test (begin ,@args) #f))  (when2 (> 5 2) 7 8 9 10)"));
      TEST_EQ("#f", run("(when2 (< 5 2) 7 8 9 10)"));
      TEST_EQ("10", run("(define x 2) (define y 1) (when2 (< y x) 7 8 9 10)"));
      TEST_EQ("\"I love ham with brie.\"", run("(define-macro (sandwich TOPPING FILLING please) `(format \"I love ~a with ~a.\" ',FILLING ',TOPPING)) (sandwich brie ham now)"));
      TEST_EQ("\"I love bacon with banana.\"", run("(sandwich banana bacon please)"));
      //TEST_EQ("error:1:2: No matching case for calling pattern in macro: sandwich", run("(sandwich brie ham)")); TOCHECK
      TEST_EQ("95", run("(define-macro (sub5 x) `(begin (define-macro (subtract a b) `(- ,a ,b)) (subtract ,x 5) ) )  (sub5 100)"));

      build_mbe();
      run("(define-syntax and2 (syntax-rules() ((and2) #t) ((and2 test) test) ((and2 test1 test2 ...) (if test1(and2 test2 ...) #f))))");
      TEST_EQ("#t", run("(and2 #t #t #t #t)"));
      TEST_EQ("#f", run("(and2 #f #t #t #t)"));
      TEST_EQ("#f", run("(and2 #t #f #t #t)"));
      TEST_EQ("#f", run("(and2 #t #f #t #f)"));
      TEST_EQ("#f", run("(and2 #t #t #t #f)"));

      run("(define-syntax or2 (syntax-rules() ((or2) #f) ((or2 test) test) ((or2 test1 test2 ...) (let ((t test1)) (if t t (or2 test2 ...)))  )))");
      TEST_EQ("#t", run("(or2 #t #t #t #t)"));
      TEST_EQ("#t", run("(or2 #f #t #t #t)"));
      TEST_EQ("#t", run("(or2 #t #f #t #t)"));
      TEST_EQ("#t", run("(or2 #t #f #t #f)"));
      TEST_EQ("#t", run("(or2 #t #t #t #f)"));
      TEST_EQ("#f", run("(or2 #f #f #f #f)"));

      TEST_EQ("19", run("(defmacro mac1 (a b) `(+ ,a (* ,b 3))) (mac1 4 5)"));
      TEST_EQ("8", run("(defmacro eight2 () `8) (eight2)"));
      }
    };

  struct curry : public compile_fixture
    {
    void test()
      {
      build_libs();
      TEST_EQ("<lambda>", run("(curry list 1 2)"));
      TEST_EQ("(1 2 3)", run("((curry list 1 2) 3)"));
      TEST_EQ("<lambda>", run("(define pair_with_one (curry cons 1))"));
      TEST_EQ("(1 . 2)", run("(pair_with_one 2)"));
      TEST_EQ("(1)", run("(pair_with_one '())"));
      TEST_EQ("(1 2 . 3)", run("(pair_with_one (cons 2 3))"));

      TEST_EQ("<lambda>", run("(define lam (lambda (x y z) (list x y z)))"));
      TEST_EQ("(1 2 3)", run("(lam 1 2 3)"));
      TEST_EQ("<lambda>", run("(define foo (curry lam 1))"));
      TEST_EQ("(1 2 3)", run("(foo 2 3)"));
      TEST_EQ("<lambda>", run("(define bar (curry lam 1 2))"));
      TEST_EQ("(1 2 4)", run("(bar 4)"));
      }
    };

  struct csv : public compile_fixture_skiwi
    {
    void test()
      {
      build_csv();
      TEST_EQ("((1 2 3 4) (5 6 7 8))", run("(define lst '((1 2 3 4) (5 6 7 8)))"));
      TEST_EQ("0", run("(write-csv lst \"out.csv\")")); // 0 comes from close-output-port if all went well
      TEST_EQ(R"((("1.0" "2.0" "3.0" "4.0") ("5.0" "6.0" "7.0" "8.0")))", run("(define r (read-csv \"out.csv\"))")); //integers are converted to flonums before writing
      TEST_EQ("((1 2 3 4) (5 6 7 8))", run("(csv-map string->number r)"));
      TEST_EQ("((1 2 3 4) (5 6 7 8))", run("(csv->numbers r)"));

      TEST_EQ(R"((("Jan" #\K Symb ""Quoted string"") (5 6 7 8)))", run("(set! lst '((\"Jan\" #\\K Symb \"\\\"Quoted string\\\"\") (5 6 7 8)))"));
      TEST_EQ("0", run("(write-csv lst \"out.csv\")"));
      TEST_EQ(R"((("Jan" "K" "Symb" ""Quoted string"") ("5.0" "6.0" "7.0" "8.0")))", run("(define r (read-csv \"out.csv\"))"));
      TEST_EQ("4", run("(length (list-ref r 0))"));
      TEST_EQ("4", run("(length (list-ref r 1))"));
      }
    };

  struct read_numerics : public compile_fixture_skiwi
    {
    void test()
      {
      build_srfi6();
      build_csv();
      TEST_EQ("-8.97873e-05", run("(string->number \"-8.9787310571409761906e-05\")"));

      TEST_EQ("<port>: \"input-string\"", run("(define p (open-input-string \"(0.98718667030334472656 -8.9787310571409761906e-05 0.37939864397048950195)\"))"));
      TEST_EQ("(0.987187 -8.97873e-05 0.379399)", run("(read p)"));

      TEST_EQ("<port>: \"input-string\"", run("(define p (open-input-string \"1,2,3\"))"));
      TEST_EQ("\"1,2,3\"", run("(read-line p)"));

      TEST_EQ("<port>: \"input-string\"", run("(define p (open-input-string \"1,2,3\"))"));
      TEST_EQ("(\"1\" \"2\" \"3\")", run("(parse-line (open-input-string (read-line p)))"));

      TEST_EQ("<port>: \"input-string\"", run("(define p (open-input-string \"-64.329442,52.656052,-48.669564\"))"));
      TEST_EQ("\"-64.329442,52.656052,-48.669564\"", run("(read-line p)"));

      TEST_EQ("<port>: \"input-string\"", run("(define p (open-input-string \"-64.329442,52.656052,-48.669564\"))"));
      TEST_EQ("(\"-64.329442\" \"52.656052\" \"-48.669564\")", run("(parse-line (open-input-string (read-line p)))"));

      //TEST_EQ("1", run("(read p)"));
      //TEST_EQ(",", run("(%read-identifier (read-char p) p)"));
      //TEST_EQ("2", run("(read p)"));
      //TEST_EQ(",", run("(read p)"));
      //TEST_EQ("3", run("(read p)"));      

      //TEST_EQ("<port>: \"input-string\"", run("(define p (open-input-string \"-64.329442,52.656052,-48.669564\"))"));
      //TEST_EQ("-64.3294", run("(read p)"));
      //TEST_EQ(",", run("(read p)"));
      }
    };

  struct srfi1 : public compile_fixture_skiwi
    {
    void test()
      {
      build_srfi1();
      TEST_EQ("(0 1 2 3 4)", run("(iota 5)"));
      }
    };


  uint64_t g_my_address;

  uint64_t get_my_address()
    {
    return g_my_address;
    }

  void scm_load_simulation(uint64_t addr, const char* script);

  struct load_simulation : public compile_fixture
    {

    void test()
      {
      build_libs();
      g_my_address = (uint64_t)this;

      external_function ef;
      ef.name = "my-address";
      ef.address = (uint64_t)&get_my_address;
      ef.return_type = external_function::T_INT64;
      externals[ef.name] = ef;

      ef.name = "load-simulation";
      ef.address = (uint64_t)&scm_load_simulation;
      ef.return_type = external_function::T_VOID;
      ef.arguments.push_back(external_function::T_INT64);
      ef.arguments.push_back(external_function::T_CHAR_POINTER);
      externals[ef.name] = ef;

      run("(define addr (foreign-call my-address))");
      run("(foreign-call load-simulation addr \"13\")");

      int sz = (int)std::distance(env->begin(), env->end());
      printf("Environment has %d elements\n", sz);

      run("(foreign-call load-simulation addr \"(define x 14)\")");

      sz = (int)std::distance(env->begin(), env->end());
      printf("Environment has %d elements\n", sz);

      TEST_EQ("14", run("x"));
      }
    };

  void scm_load_simulation(uint64_t addr, const char* in)
    {
    load_simulation* p_ls = (load_simulation*)addr;
    std::string script(in);

    void* rbx = p_ls->ctxt.rbx;
    void* rdi = p_ls->ctxt.rdi;
    void* rsi = p_ls->ctxt.rsi;
    void* rsp = p_ls->ctxt.rsp;
    void* rbp = p_ls->ctxt.rbp;
    void* r12 = p_ls->ctxt.r12;
    void* r13 = p_ls->ctxt.r13;
    void* r14 = p_ls->ctxt.r14;
    void* r15 = p_ls->ctxt.r15;

    std::string out = p_ls->run(script);
    std::cout << out << std::endl;

    p_ls->ctxt.rbx = rbx;
    p_ls->ctxt.rdi = rdi;
    p_ls->ctxt.rsi = rsi;
    p_ls->ctxt.rsp = rsp;
    p_ls->ctxt.rbp = rbp;
    p_ls->ctxt.r12 = r12;
    p_ls->ctxt.r13 = r13;
    p_ls->ctxt.r14 = r14;
    p_ls->ctxt.r15 = r15;

    int sz = (int)std::distance(p_ls->env->begin(), p_ls->env->end());
    printf("Environment has %d elements\n", sz);
    }

  void load_test()
    {
    using namespace skiwi;
    skiwi_parameters params;
    params.trace = nullptr;
    params.stderror = &std::cout;
    params.stdoutput = nullptr;
    scheme_with_skiwi(nullptr, nullptr, params);
#ifdef _WIN32
    skiwi_run("(load \"data\\\\load_test_1.scm\")");
#else
    skiwi_run("(load \"./data/load_test_1.scm\")");
#endif
    uint64_t res = skiwi_run_raw("result");
    TEST_EQ("9", skiwi_raw_to_string(res));

#ifdef _WIN32
    std::string script = R"(
(load (string-append "data\\" "load_test_2.scm"))
(add3 5)
)";
#else
    std::string script = R"(
(load (string-append "./data/" "load_test_2.scm"))
(add3 5)
)";
#endif
    res = skiwi_run_raw(script);
    TEST_EQ("8", skiwi_raw_to_string(res));
    skiwi_quit();
    }

  struct eval_test : public compile_fixture_skiwi
    {
    void test()
      {
      using namespace skiwi;
      build_eval();
      uint64_t res = skiwi_run_raw("(eval '(+ 3 4))");
      TEST_EQ("7", skiwi_raw_to_string(res));
      res = skiwi_run_raw("(define (eval-formula formula) (eval `(let([x 2] [y 3]) ,formula)))");
      TEST_EQ("<lambda>", skiwi_raw_to_string(res));
      res = skiwi_run_raw("(eval-formula '(+ x y))");
      TEST_EQ("5", skiwi_raw_to_string(res));
      res = skiwi_run_raw("(eval-formula '(+ (* x y) y))");
      TEST_EQ("9", skiwi_raw_to_string(res));
      }
    };

  struct getenvtest : public compile_fixture
    {
    void test()
      {
      TEST_EQ("#f", run("(getenv \"Jan\")"));

      TEST_EQ("#t", run("(putenv \"SkiwiTest\" \"DummyValue\")"));
      TEST_EQ("\"DummyValue\"", run("(getenv \"SkiwiTest\")"));

      TEST_EQ("#t", run("(putenv \"SkiwiTest\" \"DummyValu\")"));
      TEST_EQ("\"DummyValu\"", run("(getenv \"SkiwiTest\")"));

      TEST_EQ("#t", run("(putenv \"SkiwiTest\" \"DummyVal\")"));
      TEST_EQ("\"DummyVal\"", run("(getenv \"SkiwiTest\")"));

      TEST_EQ("#t", run("(putenv \"SkiwiTest\" \"DummyVa\")"));
      TEST_EQ("\"DummyVa\"", run("(getenv \"SkiwiTest\")"));

      TEST_EQ("#t", run("(putenv \"SkiwiTest\" \"DummyV\")"));
      TEST_EQ("\"DummyV\"", run("(getenv \"SkiwiTest\")"));

      TEST_EQ("#t", run("(putenv \"SkiwiTest\" \"Dummy\")"));
      TEST_EQ("\"Dummy\"", run("(getenv \"SkiwiTest\")"));

      TEST_EQ("#t", run("(putenv \"SkiwiTest\" \"\")"));

#ifdef _WIN32
      TEST_EQ("#f", run("(getenv \"SkiwiTest\")"));
#else
      TEST_EQ("\"\"", run("(getenv \"SkiwiTest\")"));
#endif
      }
    };

  struct filetest : public compile_fixture
    {
    void test()
      {
      TEST_EQ("#f", run("(file-exists? \"asdfjwedsfsde\")"));
#ifdef _WIN32
      TEST_EQ("#t", run("(file-exists?  \"data\\\\include_test_1.scm\")"));
#else
      TEST_EQ("#t", run("(file-exists?  \"./data/include_test_1.scm\")"));
#endif
      }
    };

  void debug_test()
    {
    using namespace skiwi;
    skiwi_parameters params;
    params.trace = nullptr;
    params.stderror = &std::cout;
    params.stdoutput = nullptr;
    scheme_with_skiwi(nullptr, nullptr, params);

#ifdef _WIN32
    std::string script = R"(
(load (string-append "data\\" "load_test_2.scm"))
(add4 5)
(define x 7)
)";

#else

    std::string script = R"(
(load (string-append "./data/" "load_test_2.scm"))
(add4 5)
(define x 7)
)";

#endif
    uint64_t res = skiwi_run_raw(script);
    TEST_EQ("runtime error: closure expected:", first_line(skiwi_raw_to_string(res)));
    skiwi_quit();
    }

  struct hex_test : public compile_fixture
    {
    void test()
      {
      TEST_EQ("255", run("#xff"));
      TEST_EQ("255", run("#xFF"));
      TEST_EQ("255", run("#xFf"));
      TEST_EQ("255", run("#xfF"));
      TEST_EQ("243", run("#xf3"));
      TEST_EQ("11229389", run("#xab58cd"));
      TEST_EQ("error: Unsupported number syntax", run("#xg"));
      }
    };

  struct binary_test : public compile_fixture
    {
    void test()
      {
      TEST_EQ("18", run("#b10010"));
      TEST_EQ("1", run("#b0000000001"));
      TEST_EQ("127", run("#b1111111"));
      TEST_EQ("error: Unsupported number syntax", run("#b333"));
      }
    };

  struct octal_test : public compile_fixture
    {
    void test()
      {
      TEST_EQ("4104", run("#o10010"));
      TEST_EQ("59", run("#o73"));
      TEST_EQ("error: Unsupported number syntax", run("#o9"));
      TEST_EQ("error: Unsupported number syntax", run("#o8"));
      }
    };

  struct empty_let_crash : public compile_fixture
    {
    void test()
      {
      TEST_EQ("<lambda>", run("(define (x) 5)"));
      TEST_EQ("#undefined", run("(let ((m (x))))")); // Fixed this crash in cps_convert_begin_simple: if begin statement has no arguments, then add nop
      }
    };

  void load_error_during_load()
    {
    using namespace skiwi;
    skiwi_parameters params;
    params.trace = nullptr;
    params.stderror = &std::cout;
    params.stdoutput = nullptr;
    scheme_with_skiwi(nullptr, nullptr, params);
#ifdef _WIN32
    uint64_t res = skiwi_run_raw("(apply load '(\"data\\\\load_test_3.scm\"))");
#else
    uint64_t res = skiwi_run_raw("(apply load '(\"./data/load_test_3.scm\"))");
#endif
    TEST_EQ("runtime error: closure expected:", first_line(skiwi_raw_to_string(res)));

#ifdef _WIN32
    res = skiwi_run_raw("(begin (apply load '(\"data\\\\load_test_3.scm\")) (+ 1 2))");
#else
    res = skiwi_run_raw("(begin (apply load '(\"./data/load_test_3.scm\")) (+ 1 2))");
#endif
    TEST_EQ("runtime error: closure expected:", first_line(skiwi_raw_to_string(res)));
    skiwi_quit();
    }

  void load_error()
    {
    using namespace skiwi;
    skiwi_parameters params;
    params.trace = nullptr;
    params.stderror = &std::cout;
    params.stdoutput = &std::cout;
    scheme_with_skiwi(nullptr, nullptr, params);
#ifdef _WIN32
    uint64_t res = skiwi_run_raw("(begin (load \"data\\\\load_test_4.scm\") (define equal? +))");
#else
    uint64_t res = skiwi_run_raw("(begin (load \"./data/load_test_4.scm\") (define equal? +))");
#endif
    TEST_EQ("<procedure>", skiwi_raw_to_string(res));
    skiwi_quit();
    }

  struct many_vars_in_lambda_test : public compile_fixture
    {
    void test()
      {
      TEST_EQ("<lambda>", run("(define (add10 a b c d e f g h i j) (+ a b c d e f g h i j))"));
      TEST_EQ("55", run("(add10 1 2 3 4 5 6 7 8 9 10)"));
      }
    };

  }

SKIWI_END

void run_all_compile_tests()
  {
  using namespace SKIWI;
#ifndef ONLY_LAST  
  fixnums().test();
  bools().test();
  test_for_nil().test();
  chars().test();
  doubles().test();
  add1().test();
  add1_optimized().test();
  sub1().test();
  sub1_optimized().test();
  add_fixnums().test();
  add_fixnums_opt().test();
  add_flonums().test();
  add_flonums_optimized().test();
  add_flonums_and_fixnums().test();
  add_flonums_and_fixnums_optimized().test();
  sub().test();
  sub_optimized().test();
  mul().test();
  mul_optimized().test();
  divtest().test();
  div_optimized().test();
  add_incorrect_argument().test();
  combination_of_math_ops().test();
  combination_of_math_ops_optimized().test();
  equal().test();
  equal_optimized().test();
  not_equal().test();
  not_equal_optimized().test();
  less().test();
  less_optimized().test();
  leq().test();
  leq_optimized().test();
  greater().test();
  greater_optimized().test();
  geq().test();
  geq_optimized().test();
  compare_incorrect_argument().test();
  iftest().test();
  andtest().test();
  and_optimized().test();
  ortest().test();
  or_optimized().test();
  let().test();
  let_star().test();
  let_optimized().test();
  arithmetic().test();
  arithmetic_optimized().test();
  globals().test();
  globals_optimized().test();
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
  is_procedure().test();
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
  make_port_test().test();
  make_port2_test().test();
  r5rs_test().test();
  callcc_test().test();
  minmax_test().test();
  ieee745_test().test();
  make_port3_test().test();
  writetests().test();
  char_comp().test();
  fx_comp().test();
  named_let().test();
  list_ops().test();
  gcd_ops().test();
  string_ops().test();
  symbol_ops().test();
  string_to_symbol_ops().test();
  control_ops().test();
  do_ops().test();
  no_square_brackets().test();
  calcc_extended().test();
  alternative_defines().test();
  include_tests().test();
  each_expression_in_program_as_separate_cps_tests().test();
  bug_from_jaffer_test().test();
  quasiquote_tests().test();
  jaffer_2_tests().test();
  lambda_invalid_arg_tests().test();
  string_set_tests().test();
  cps_bug_test().test();
  basic_string_ports().test();
  format_tests().test();
  flonums_with_e().test();
  read_tests().test();
  trace_tests().test();
  bugs_from_compiler_scm().test();
  bugs_from_compiler_2().test();
  macros().test();
  curry().test();
  csv().test();
  read_numerics().test();
  srfi1().test();
  load_simulation().test();
  eval_test().test();
  getenvtest().test();
  filetest().test();
  load_test();
  debug_test();
  hex_test().test();
  binary_test().test();
  octal_test().test();

  load_error_during_load();
  load_error();
  load_test();
  empty_let_crash().test();

  many_vars_in_lambda_test().test();
#endif
  }