#include "load_lib.h"
#include "macro_data.h"
#include "exepath.h"
#include <iostream>

#include <encoding.h>

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
    wchar_t* path = _wgetenv(L"SKIWI_MODULE_PATH");
    if (path == nullptr)
      throw std::runtime_error("SKIWI_MODULE_PATH is not in the environment!");
    std::wstring wfolder(path);    
    std::wstring wrelative(relative_path.begin(), relative_path.end());
    std::wstring filepath = wfolder + wrelative;
    std::ifstream f{ filepath };
    if (f.is_open())
      {
      std::stringstream ss;
      ss << f.rdbuf();
      f.close();
      return ss.str();
      }
    return "";
    }
  }

bool load_lib(const std::string& libname, environment_map& env, repl_data& rd, macro_data& md, context& ctxt, asmcode& code, const primitive_map& pm, const compiler_options& options)
  {
  std::string script;
  try
    {
  script = read_file_in_module_path(libname);
    }
  catch (std::runtime_error e)
    {
    std::cout << e.what() << "\n";
    std::wstring w_environment_location = get_folder(get_exe_path());
    std::replace(w_environment_location.begin(), w_environment_location.end(), '\\', '/');
    w_environment_location.append(L"scm/");
    std::string environment = JAM::convert_wstring_to_string(w_environment_location);
    std::cout << "You have to set the environment variable SKIWI_MODULE_PATH\n";
    std::cout << "to the correct location. This should be the folder where the\n";
    std::cout << "Skiwi scm files are located. Probably this is folder\n";
    std::cout << environment << "\n";
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

bool load_apply(environment_map& env, repl_data& rd, macro_data& md, context& ctxt, asmcode& code, const primitive_map& pm, const compiler_options& options)
  {
  compiler_options ops = make_baselib_options(options);
  return load_lib("core/apply.scm", env, rd, md, ctxt, code, pm, ops);
  }

bool load_modules(environment_map& env, repl_data& rd, macro_data& md, context& ctxt, asmcode& code, const primitive_map& pm, const compiler_options& options)
  {
  compiler_options ops = make_baselib_options(options);
  return load_lib("core/modules.scm", env, rd, md, ctxt, code, pm, ops);
  }

bool load_string_to_symbol(environment_map& env, repl_data& rd, macro_data& md, context& ctxt, asmcode& code, const primitive_map& pm, const compiler_options& options)
  {
  compiler_options ops = make_baselib_options(options);
  return load_lib("core/symbol-table.scm", env, rd, md, ctxt, code, pm, ops);
  }

bool load_r5rs(environment_map& env, repl_data& rd, macro_data& md, context& ctxt, asmcode& code, const primitive_map& pm, const compiler_options& options)
  {
  compiler_options ops = make_baselib_options(options);
  return load_lib("core/r5rs.scm", env, rd, md, ctxt, code, pm, ops);
  }

bool load_callcc(environment_map& env, repl_data& rd, macro_data& md, context& ctxt, asmcode& code, const primitive_map& pm, const compiler_options& options)
  {
  compiler_options ops = make_baselib_options(options);
  ops.do_cps_conversion = false;
  return load_lib("core/callcc.scm", env, rd, md, ctxt, code, pm, ops);
  }

SKIWI_END
