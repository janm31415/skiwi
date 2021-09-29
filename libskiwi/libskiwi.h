#pragma once

#include "libskiwi_api.h"

#include <stdint.h>
#include <ostream>
#include <vector>

namespace skiwi
  {

  class scm_type;

#ifdef _SKIWI_FOR_ARM
typedef uint8_t* skiwi_compiled_function_ptr;
#else
  typedef uint64_t(*skiwi_compiled_function_ptr)(void*, ...);
#endif
  struct skiwi_parameters
    {
    SKIWI_SCHEME_API skiwi_parameters();
    uint64_t heap_size;
    uint64_t globals_stack;
    uint32_t local_stack;
    uint64_t scheme_stack;
    std::ostream* trace;
    std::ostream* stderror;
    std::ostream* stdoutput;
    };

  /*
  Initializes skiwi.
  Call func, passing it data and return what func returns.
  */
  SKIWI_SCHEME_API void* scheme_with_skiwi(void *(*func)(void *), void *data = nullptr, skiwi_parameters params = skiwi_parameters());

  SKIWI_SCHEME_API void skiwi_repl(int argc = 0, char** argv = nullptr);

  SKIWI_SCHEME_API void skiwi_quit();

  SKIWI_SCHEME_API skiwi_compiled_function_ptr skiwi_compile(const std::string& scheme_expression);  

  SKIWI_SCHEME_API void* skiwi_get_context();

  SKIWI_SCHEME_API void* skiwi_clone_context(void* ctxt);

  SKIWI_SCHEME_API void skiwi_destroy_clone_context(void* ctxt);

  template <typename... Args>
  uint64_t skiwi_run_raw(skiwi_compiled_function_ptr fun, void* ctxt, Args... args)
    {
    uint64_t result = 39; // hardcoded skiwi_undefined
    if (fun)
      {
      result = fun(ctxt, args...);
      }
    return result;
    }

  SKIWI_SCHEME_API uint64_t skiwi_run_raw(const std::string& scheme_expression);
  SKIWI_SCHEME_API uint64_t skiwi_runf_raw(const std::string& scheme_file);
  SKIWI_SCHEME_API std::string skiwi_raw_to_string(uint64_t scm_value);

  SKIWI_SCHEME_API void skiwi_run(const std::string& scheme_expression);
  SKIWI_SCHEME_API void skiwi_runf(const std::string& scheme_file);

  SKIWI_SCHEME_API void skiwi_show_help();
  SKIWI_SCHEME_API void skiwi_show_expand(const std::string& input);
  SKIWI_SCHEME_API void skiwi_show_assembly(const std::string& input);
  SKIWI_SCHEME_API void skiwi_show_unresolved();
  SKIWI_SCHEME_API void skiwi_show_external_primitives(const std::string& arg);
  SKIWI_SCHEME_API void skiwi_show_memory();
  SKIWI_SCHEME_API void skiwi_show_environment();

  SKIWI_SCHEME_API std::string skiwi_expand(const std::string& scheme_expression);

  SKIWI_SCHEME_API std::string skiwi_last_global_variable_used();

  enum external_type
    {
    skiwi_bool,
    skiwi_char_pointer,
    skiwi_double,
    skiwi_int64,
    skiwi_void,
    skiwi_scm
    };

  SKIWI_SCHEME_API void register_external_primitive(const std::string& name, void* func_ptr, external_type return_type, const std::vector<external_type>& arguments, const char* help_text = nullptr);
  SKIWI_SCHEME_API void register_external_primitive(const std::string& name, void* func_ptr, external_type return_type, const char* help_text = nullptr);
  SKIWI_SCHEME_API void register_external_primitive(const std::string& name, void* func_ptr, external_type return_type, external_type arg1, const char* help_text = nullptr);
  SKIWI_SCHEME_API void register_external_primitive(const std::string& name, void* func_ptr, external_type return_type, external_type arg1, external_type arg2, const char* help_text = nullptr);
  SKIWI_SCHEME_API void register_external_primitive(const std::string& name, void* func_ptr, external_type return_type, external_type arg1, external_type arg2, external_type arg3, const char* help_text = nullptr);
  SKIWI_SCHEME_API void register_external_primitive(const std::string& name, void* func_ptr, external_type return_type, external_type arg1, external_type arg2, external_type arg3, external_type arg4, const char* help_text = nullptr);

  class scm_type
    {
    public:
      SKIWI_SCHEME_API scm_type();
      SKIWI_SCHEME_API scm_type(uint64_t rax);
      SKIWI_SCHEME_API ~scm_type();

      SKIWI_SCHEME_API operator uint64_t() const { return scm_value; }

      SKIWI_SCHEME_API bool is_fixnum() const;
      SKIWI_SCHEME_API bool is_bool_true() const;
      SKIWI_SCHEME_API bool is_bool_false() const;
      SKIWI_SCHEME_API bool is_nil() const;
      SKIWI_SCHEME_API bool is_char() const;
      SKIWI_SCHEME_API bool is_undefined() const;
      SKIWI_SCHEME_API bool is_skiwi_quiet_undefined() const;
      SKIWI_SCHEME_API bool is_eof() const;
      SKIWI_SCHEME_API bool is_procedure() const;
      SKIWI_SCHEME_API bool is_error() const;
      SKIWI_SCHEME_API bool is_block() const;
      SKIWI_SCHEME_API bool is_flonum() const;
      SKIWI_SCHEME_API bool is_closure() const;
      SKIWI_SCHEME_API bool is_pair() const;
      SKIWI_SCHEME_API bool is_vector() const;
      SKIWI_SCHEME_API bool is_string() const;
      SKIWI_SCHEME_API bool is_symbol() const;
      SKIWI_SCHEME_API bool is_port() const;
      SKIWI_SCHEME_API bool is_promise() const;

      SKIWI_SCHEME_API int64_t get_fixnum() const;
      SKIWI_SCHEME_API double get_flonum() const;
      SKIWI_SCHEME_API double get_number() const;
      SKIWI_SCHEME_API std::vector<scm_type> get_vector() const;
      SKIWI_SCHEME_API std::pair<scm_type, scm_type> get_pair() const;
      SKIWI_SCHEME_API std::vector<scm_type> get_list() const;
      SKIWI_SCHEME_API double* flonum_address() const;
      SKIWI_SCHEME_API std::string get_closure_name() const;

      SKIWI_SCHEME_API uint64_t value() const { return scm_value; }
    private:
      uint64_t scm_value;
    };

  SKIWI_SCHEME_API scm_type make_fixnum(int64_t value);
  SKIWI_SCHEME_API scm_type make_nil();
  SKIWI_SCHEME_API scm_type make_true();
  SKIWI_SCHEME_API scm_type make_false();
  SKIWI_SCHEME_API scm_type make_undefined();
  SKIWI_SCHEME_API scm_type make_skiwi_quiet_undefined();
  SKIWI_SCHEME_API scm_type make_flonum(double value);
  SKIWI_SCHEME_API scm_type make_pair(scm_type first, scm_type second);
  SKIWI_SCHEME_API scm_type make_list(const std::vector<scm_type>& lst);
  SKIWI_SCHEME_API scm_type make_vector(const std::vector<scm_type>& vec);
  

  SKIWI_SCHEME_API void set_prompt(const std::string& prompt_text);
  SKIWI_SCHEME_API void set_welcome_message(const std::string& welcome_message_text);
  SKIWI_SCHEME_API void set_help_text(const std::string& help_text);

  SKIWI_SCHEME_API void save_compiler_data();
  SKIWI_SCHEME_API void restore_compiler_data();  
  }
