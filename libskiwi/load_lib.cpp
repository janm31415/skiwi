#include "load_lib.h"
#include "macro_data.h"
#include <iostream>
#include <sstream>

#include <jtk/file_utils.h>

SKIWI_BEGIN

namespace
  {
  compiler_options make_baselib_options(const compiler_options& options)
    {
    compiler_options ops;
    ops.safe_promises = options.safe_promises;
    ops.safe_cons = options.safe_cons;
    ops.safe_flonums = options.safe_flonums;
    ops.safe_primitives = options.safe_primitives;
    ops.primitives_inlined = options.primitives_inlined;
    ops.do_alpha_conversion = false;
    return ops;
    }

  std::string read_file_in_module_path(const std::string& relative_path)
    {
    std::string modulepath = jtk::get_folder(jtk::get_executable_path()) + std::string("scm/");
    std::string filepath = modulepath + relative_path;
#ifdef _WIN32
    std::wstring wfilepath = jtk::convert_string_to_wstring(filepath);
    std::ifstream f{ wfilepath };
#else
    std::ifstream f{ filepath };
#endif
    if (f.is_open())
      {
      std::stringstream ss;
      ss << f.rdbuf();
      f.close();
      return ss.str();
      }
    std::stringstream ss2;
    ss2 << std::string(filepath.begin(), filepath.end()) << " not found";
    throw std::runtime_error(ss2.str().c_str());
    return "";
    }
  }

bool load_lib(const std::string& libname, environment_map& env, repl_data& rd, macro_data& md, context& ctxt, ASM::asmcode& code, const primitive_map& pm, const compiler_options& options)
  {
  std::string script;
  try
    {
    script = read_file_in_module_path(libname);
    }
  catch (std::runtime_error e)
    {
    std::cout << e.what() << "\n";  
    std::cout << "Trying to read scheme libraries in subfolder scm of your binaries folder\n";
    std::cout << "It appears that this folder or the corresponding library files do not exist\n";
    return false;
    }
  if (script.empty())
    return false;
  auto tokens = tokenize(script);
  std::reverse(tokens.begin(), tokens.end());
  Program prog = make_program(tokens);
  std::map<std::string, external_function> empty_externals;
  compile(env, rd, md, ctxt, code, prog, pm, empty_externals, options);
  return true;
  }

bool load_apply(environment_map& env, repl_data& rd, macro_data& md, context& ctxt, ASM::asmcode& code, const primitive_map& pm, const compiler_options& options)
  {
  compiler_options ops = make_baselib_options(options);
  return load_lib("core/apply.scm", env, rd, md, ctxt, code, pm, ops);
  }

bool load_modules(environment_map& env, repl_data& rd, macro_data& md, context& ctxt, ASM::asmcode& code, const primitive_map& pm, const compiler_options& options)
  {
  compiler_options ops = make_baselib_options(options);
  return load_lib("core/modules.scm", env, rd, md, ctxt, code, pm, ops);
  }

bool load_string_to_symbol(environment_map& env, repl_data& rd, macro_data& md, context& ctxt, ASM::asmcode& code, const primitive_map& pm, const compiler_options& options)
  {
  compiler_options ops = make_baselib_options(options);
  return load_lib("core/symbol-table.scm", env, rd, md, ctxt, code, pm, ops);
  }

bool load_r5rs(environment_map& env, repl_data& rd, macro_data& md, context& ctxt, ASM::asmcode& code, const primitive_map& pm, const compiler_options& options)
  {
  compiler_options ops = make_baselib_options(options);
  return load_lib("core/r5rs.scm", env, rd, md, ctxt, code, pm, ops);
  }

bool load_callcc(environment_map& env, repl_data& rd, macro_data& md, context& ctxt, ASM::asmcode& code, const primitive_map& pm, const compiler_options& options)
  {
  compiler_options ops = make_baselib_options(options);
  ops.do_cps_conversion = false;
  return load_lib("core/callcc.scm", env, rd, md, ctxt, code, pm, ops);
  }

SKIWI_END
