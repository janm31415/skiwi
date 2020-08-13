#include "compiler.h"
#include "compile_error.h"
#include "context.h"
#include "context_defs.h"
#include "asm_aux.h"
#include "inlines.h"
#include "macro_data.h"
#include "preprocess.h"
#include "primitives.h"
#include "types.h"
#include "globals.h"
#include "cinput_data.h"
#include <map>
#include <string>
#include <sstream>
#include <cctype>

SKIWI_BEGIN

using namespace ASM;

function_map generate_function_map()
  {
  function_map fm;
  fm.insert(std::pair<std::string, fun_ptr>("+", &compile_add));
  fm.insert(std::pair<std::string, fun_ptr>("-", &compile_sub));
  fm.insert(std::pair<std::string, fun_ptr>("*", &compile_mul));
  fm.insert(std::pair<std::string, fun_ptr>("/", &compile_div));
  fm.insert(std::pair<std::string, fun_ptr>("=", &compile_equal));
  fm.insert(std::pair<std::string, fun_ptr>("!=", &compile_not_equal));
  fm.insert(std::pair<std::string, fun_ptr>("<", &compile_less));
  fm.insert(std::pair<std::string, fun_ptr>("<=", &compile_leq));
  fm.insert(std::pair<std::string, fun_ptr>(">", &compile_greater));
  fm.insert(std::pair<std::string, fun_ptr>(">=", &compile_geq));
  fm.insert(std::pair<std::string, fun_ptr>("%allocate-symbol", &compile_allocate_symbol));
  fm.insert(std::pair<std::string, fun_ptr>("%slot-ref", &compile_slot_ref));
  fm.insert(std::pair<std::string, fun_ptr>("%slot-set!", &compile_slot_set));
  fm.insert(std::pair<std::string, fun_ptr>("%undefined", &compile_undefined));
  fm.insert(std::pair<std::string, fun_ptr>("%quiet-undefined", &compile_quiet_undefined));
  fm.insert(std::pair<std::string, fun_ptr>("add1", &compile_add1));
  fm.insert(std::pair<std::string, fun_ptr>("%apply", &compile_apply));
  fm.insert(std::pair<std::string, fun_ptr>("arithmetic-shift", &compile_arithmetic_shift));
  fm.insert(std::pair<std::string, fun_ptr>("assoc", &compile_assoc));
  fm.insert(std::pair<std::string, fun_ptr>("assq", &compile_assq));
  fm.insert(std::pair<std::string, fun_ptr>("assv", &compile_assv));
  fm.insert(std::pair<std::string, fun_ptr>("bitwise-and", &compile_bitwise_and));
  fm.insert(std::pair<std::string, fun_ptr>("bitwise-not", &compile_bitwise_not));
  fm.insert(std::pair<std::string, fun_ptr>("bitwise-or", &compile_bitwise_or));
  fm.insert(std::pair<std::string, fun_ptr>("bitwise-xor", &compile_bitwise_xor));
  fm.insert(std::pair<std::string, fun_ptr>("boolean?", &compile_is_boolean));
  fm.insert(std::pair<std::string, fun_ptr>("car", &compile_car));
  fm.insert(std::pair<std::string, fun_ptr>("cdr", &compile_cdr));
  fm.insert(std::pair<std::string, fun_ptr>("char?", &compile_is_char));
  fm.insert(std::pair<std::string, fun_ptr>("char=?", &compile_char_equal));
  fm.insert(std::pair<std::string, fun_ptr>("char<?", &compile_char_less));
  fm.insert(std::pair<std::string, fun_ptr>("char>?", &compile_char_greater));
  fm.insert(std::pair<std::string, fun_ptr>("char<=?", &compile_char_leq));
  fm.insert(std::pair<std::string, fun_ptr>("char>=?", &compile_char_geq));
  fm.insert(std::pair<std::string, fun_ptr>("char->fixnum", &compile_char_to_fixnum));
  fm.insert(std::pair<std::string, fun_ptr>("close-file", &compile_close_file));
  fm.insert(std::pair<std::string, fun_ptr>("closure", &compile_closure));
  fm.insert(std::pair<std::string, fun_ptr>("closure?", &compile_is_closure));
  fm.insert(std::pair<std::string, fun_ptr>("closure-ref", &compile_closure_ref));
  fm.insert(std::pair<std::string, fun_ptr>("cons", &compile_cons));
  fm.insert(std::pair<std::string, fun_ptr>("compare-strings", &compile_compare_strings));
  fm.insert(std::pair<std::string, fun_ptr>("compare-strings-ci", &compile_compare_strings_ci));
  fm.insert(std::pair<std::string, fun_ptr>("eof-object?", &compile_is_eof_object));
  fm.insert(std::pair<std::string, fun_ptr>("eq?", &compile_is_eq));
  fm.insert(std::pair<std::string, fun_ptr>("eqv?", &compile_is_eqv));
  fm.insert(std::pair<std::string, fun_ptr>("%eqv?", &compile_is_eqv_structurally));
  fm.insert(std::pair<std::string, fun_ptr>("equal?", &compile_is_equal));
  fm.insert(std::pair<std::string, fun_ptr>("%error", &compile_error));
  fm.insert(std::pair<std::string, fun_ptr>("%eval", &compile_eval));
  fm.insert(std::pair<std::string, fun_ptr>("file-exists?", &compile_file_exists));
  fm.insert(std::pair<std::string, fun_ptr>("fixnum?", &compile_is_fixnum));
  fm.insert(std::pair<std::string, fun_ptr>("fixnum->char", &compile_fixnum_to_char));
  fm.insert(std::pair<std::string, fun_ptr>("fixnum->flonum", &compile_fixnum_to_flonum));
  fm.insert(std::pair<std::string, fun_ptr>("fixnum-expt", &compile_fixnum_expt));
  fm.insert(std::pair<std::string, fun_ptr>("flonum?", &compile_is_flonum));
  fm.insert(std::pair<std::string, fun_ptr>("flonum->fixnum", &compile_flonum_to_fixnum));
  fm.insert(std::pair<std::string, fun_ptr>("flonum-expt", &compile_flonum_expt));
  fm.insert(std::pair<std::string, fun_ptr>("%flush-output-port", &compile_flush_output_port));
  fm.insert(std::pair<std::string, fun_ptr>("fx=?", &compile_fx_equal));
  fm.insert(std::pair<std::string, fun_ptr>("fx<?", &compile_fx_less));
  fm.insert(std::pair<std::string, fun_ptr>("fx>?", &compile_fx_greater));
  fm.insert(std::pair<std::string, fun_ptr>("fx<=?", &compile_fx_leq));
  fm.insert(std::pair<std::string, fun_ptr>("fx>=?", &compile_fx_geq));
  fm.insert(std::pair<std::string, fun_ptr>("fx+", &compile_fx_add));
  fm.insert(std::pair<std::string, fun_ptr>("fx-", &compile_fx_sub));
  fm.insert(std::pair<std::string, fun_ptr>("fx*", &compile_fx_mul));
  fm.insert(std::pair<std::string, fun_ptr>("fx/", &compile_fx_div));
  fm.insert(std::pair<std::string, fun_ptr>("fxadd1", &compile_fx_add1));
  fm.insert(std::pair<std::string, fun_ptr>("fxsub1", &compile_fx_sub1));
  fm.insert(std::pair<std::string, fun_ptr>("fxzero?", &compile_fx_is_zero));
  fm.insert(std::pair<std::string, fun_ptr>("getenv", &compile_getenv));
  fm.insert(std::pair<std::string, fun_ptr>("halt", &compile_halt));
  fm.insert(std::pair<std::string, fun_ptr>("ieee754-sign", &compile_ieee754_sign));
  fm.insert(std::pair<std::string, fun_ptr>("ieee754-exponent", &compile_ieee754_exponent));
  fm.insert(std::pair<std::string, fun_ptr>("ieee754-mantissa", &compile_ieee754_mantissa));
  fm.insert(std::pair<std::string, fun_ptr>("ieee754-sin", &compile_ieee754_sin));
  fm.insert(std::pair<std::string, fun_ptr>("ieee754-cos", &compile_ieee754_cos));
  fm.insert(std::pair<std::string, fun_ptr>("ieee754-tan", &compile_ieee754_tan));
  fm.insert(std::pair<std::string, fun_ptr>("ieee754-asin", &compile_ieee754_asin));
  fm.insert(std::pair<std::string, fun_ptr>("ieee754-acos", &compile_ieee754_acos));
  fm.insert(std::pair<std::string, fun_ptr>("ieee754-atan1", &compile_ieee754_atan1));
  fm.insert(std::pair<std::string, fun_ptr>("ieee754-log", &compile_ieee754_log));
  fm.insert(std::pair<std::string, fun_ptr>("ieee754-round", &compile_ieee754_round));
  fm.insert(std::pair<std::string, fun_ptr>("ieee754-sqrt", &compile_ieee754_sqrt));
  fm.insert(std::pair<std::string, fun_ptr>("ieee754-truncate", &compile_ieee754_truncate));
  fm.insert(std::pair<std::string, fun_ptr>("ieee754-pi", &compile_ieee754_pi));
  fm.insert(std::pair<std::string, fun_ptr>("input-port?", &compile_is_input_port));
  fm.insert(std::pair<std::string, fun_ptr>("load", &compile_load));
  fm.insert(std::pair<std::string, fun_ptr>("length", &compile_length));
  fm.insert(std::pair<std::string, fun_ptr>("list", &compile_list));
  fm.insert(std::pair<std::string, fun_ptr>("make-port", &compile_make_port));
  fm.insert(std::pair<std::string, fun_ptr>("%make-promise", &compile_make_promise));
  fm.insert(std::pair<std::string, fun_ptr>("make-string", &compile_make_string));
  fm.insert(std::pair<std::string, fun_ptr>("make-vector", &compile_make_vector));
  fm.insert(std::pair<std::string, fun_ptr>("max", &compile_max));
  fm.insert(std::pair<std::string, fun_ptr>("member", &compile_member));
  fm.insert(std::pair<std::string, fun_ptr>("memq", &compile_memq));
  fm.insert(std::pair<std::string, fun_ptr>("memv", &compile_memv));
  fm.insert(std::pair<std::string, fun_ptr>("min", &compile_min));
  fm.insert(std::pair<std::string, fun_ptr>("not", &compile_not));
  fm.insert(std::pair<std::string, fun_ptr>("null?", &compile_is_nil));
  fm.insert(std::pair<std::string, fun_ptr>("num2str", &compile_num2str));
  fm.insert(std::pair<std::string, fun_ptr>("open-file", &compile_open_file));
  fm.insert(std::pair<std::string, fun_ptr>("output-port?", &compile_is_output_port));
  fm.insert(std::pair<std::string, fun_ptr>("%peek-char", &compile_peek_char));
  fm.insert(std::pair<std::string, fun_ptr>("pair?", &compile_is_pair));
  fm.insert(std::pair<std::string, fun_ptr>("port?", &compile_is_port));
  fm.insert(std::pair<std::string, fun_ptr>("procedure?", &compile_is_procedure));
  fm.insert(std::pair<std::string, fun_ptr>("promise?", &compile_is_promise));
  fm.insert(std::pair<std::string, fun_ptr>("putenv", &compile_putenv));
  fm.insert(std::pair<std::string, fun_ptr>("quotient", &compile_quotient));
  fm.insert(std::pair<std::string, fun_ptr>("%read-char", &compile_read_char));
  fm.insert(std::pair<std::string, fun_ptr>("reclaim", &compile_reclaim));
  fm.insert(std::pair<std::string, fun_ptr>("reclaim-garbage", &compile_reclaim_garbage));
  fm.insert(std::pair<std::string, fun_ptr>("remainder", &compile_remainder));
  fm.insert(std::pair<std::string, fun_ptr>("set-car!", &compile_set_car));
  fm.insert(std::pair<std::string, fun_ptr>("set-cdr!", &compile_set_cdr));
  fm.insert(std::pair<std::string, fun_ptr>("str2num", &compile_str2num));
  fm.insert(std::pair<std::string, fun_ptr>("string", &compile_string));
  fm.insert(std::pair<std::string, fun_ptr>("string-hash", &compile_string_hash));
  fm.insert(std::pair<std::string, fun_ptr>("string?", &compile_is_string));
  fm.insert(std::pair<std::string, fun_ptr>("string-append1", &compile_string_append1));
  fm.insert(std::pair<std::string, fun_ptr>("string-copy", &compile_string_copy));
  fm.insert(std::pair<std::string, fun_ptr>("string-fill!", &compile_string_fill));
  fm.insert(std::pair<std::string, fun_ptr>("string-length", &compile_string_length));
  fm.insert(std::pair<std::string, fun_ptr>("string-ref", &compile_string_ref));
  fm.insert(std::pair<std::string, fun_ptr>("string-set!", &compile_string_set));
  fm.insert(std::pair<std::string, fun_ptr>("substring", &compile_substring));
  fm.insert(std::pair<std::string, fun_ptr>("symbol->string", &compile_symbol_to_string));
  fm.insert(std::pair<std::string, fun_ptr>("symbol?", &compile_is_symbol));
  fm.insert(std::pair<std::string, fun_ptr>("sub1", &compile_sub1));
  fm.insert(std::pair<std::string, fun_ptr>("vector", &compile_vector));
  fm.insert(std::pair<std::string, fun_ptr>("vector?", &compile_is_vector));
  fm.insert(std::pair<std::string, fun_ptr>("vector-fill!", &compile_vector_fill));
  fm.insert(std::pair<std::string, fun_ptr>("vector-length", &compile_vector_length));
  fm.insert(std::pair<std::string, fun_ptr>("vector-ref", &compile_vector_ref));
  fm.insert(std::pair<std::string, fun_ptr>("vector-set!", &compile_vector_set));
  fm.insert(std::pair<std::string, fun_ptr>("%write-char", &compile_write_char));
  fm.insert(std::pair<std::string, fun_ptr>("%write-string", &compile_write_string));
  fm.insert(std::pair<std::string, fun_ptr>("zero?", &compile_is_zero));
  return fm;
  }



namespace
  {
  struct registered_functions
    {
    const function_map* primitives;
    const function_map* inlined_primitives;
    const std::map<std::string, external_function>* externals;
    };

  bool is_inlined_primitive(const std::string& prim_name)
    {
    if (prim_name.size() > 2)
      {
      return (prim_name[0] == '#' && prim_name[1] == '#');
      }
    return false;
    }

  function_map generate_inlined_primitives()
    {
    function_map fm;
    /*
    primitive names with a prefix ## are the inlined versions.
    */
    fm.insert(std::pair<std::string, fun_ptr>("##remainder", &inline_remainder));
    fm.insert(std::pair<std::string, fun_ptr>("##quotient", &inline_quotient));
    fm.insert(std::pair<std::string, fun_ptr>("##arithmetic-shift", &inline_arithmetic_shift));
    fm.insert(std::pair<std::string, fun_ptr>("##%undefined", &inline_undefined));
    fm.insert(std::pair<std::string, fun_ptr>("##%quiet-undefined", &inline_quiet_undefined));
    fm.insert(std::pair<std::string, fun_ptr>("##vector-length", &inline_vector_length));
    fm.insert(std::pair<std::string, fun_ptr>("##fixnum->flonum", &inline_fixnum_to_flonum));
    fm.insert(std::pair<std::string, fun_ptr>("##flonum->fixnum", &inline_flonum_to_fixnum));
    fm.insert(std::pair<std::string, fun_ptr>("##fixnum->char", &inline_fixnum_to_char));
    fm.insert(std::pair<std::string, fun_ptr>("##char->fixnum", &inline_char_to_fixnum));
    fm.insert(std::pair<std::string, fun_ptr>("##char<?", &inline_fx_less));
    fm.insert(std::pair<std::string, fun_ptr>("##char<=?", &inline_fx_leq));
    fm.insert(std::pair<std::string, fun_ptr>("##char>?", &inline_fx_greater));
    fm.insert(std::pair<std::string, fun_ptr>("##char>=?", &inline_fx_geq));
    fm.insert(std::pair<std::string, fun_ptr>("##char=?", &inline_fx_equal));
    fm.insert(std::pair<std::string, fun_ptr>("##bitwise-and", &inline_bitwise_and));
    fm.insert(std::pair<std::string, fun_ptr>("##bitwise-not", &inline_bitwise_not));
    fm.insert(std::pair<std::string, fun_ptr>("##bitwise-or", &inline_bitwise_or));
    fm.insert(std::pair<std::string, fun_ptr>("##bitwise-xor", &inline_bitwise_xor));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-sign", &inline_ieee754_sign));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-mantissa", &inline_ieee754_mantissa));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-exponent", &inline_ieee754_exponent));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-pi", &inline_ieee754_pi));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-fxsin", &inline_ieee754_fxsin));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-fxcos", &inline_ieee754_fxcos));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-fxtan", &inline_ieee754_fxtan));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-fxasin", &inline_ieee754_fxasin));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-fxacos", &inline_ieee754_fxacos));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-fxatan1", &inline_ieee754_fxatan1));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-fxlog", &inline_ieee754_fxlog));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-fxround", &inline_ieee754_fxround));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-fxtruncate", &inline_ieee754_fxtruncate));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-fxsqrt", &inline_ieee754_fxsqrt));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-flsin", &inline_ieee754_flsin));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-flcos", &inline_ieee754_flcos));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-fltan", &inline_ieee754_fltan));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-flasin", &inline_ieee754_flasin));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-flacos", &inline_ieee754_flacos));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-flatan1", &inline_ieee754_flatan1));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-fllog", &inline_ieee754_fllog));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-flround", &inline_ieee754_flround));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-fltruncate", &inline_ieee754_fltruncate));
    fm.insert(std::pair<std::string, fun_ptr>("##ieee754-flsqrt", &inline_ieee754_flsqrt));
    fm.insert(std::pair<std::string, fun_ptr>("##memq", &inline_memq));
    fm.insert(std::pair<std::string, fun_ptr>("##assq", &inline_assq));
    fm.insert(std::pair<std::string, fun_ptr>("##not", &inline_not));
    fm.insert(std::pair<std::string, fun_ptr>("##cons", &inline_cons));
    fm.insert(std::pair<std::string, fun_ptr>("##car", &inline_car));
    fm.insert(std::pair<std::string, fun_ptr>("##cdr", &inline_cdr));
    fm.insert(std::pair<std::string, fun_ptr>("##set-car!", &inline_set_car));
    fm.insert(std::pair<std::string, fun_ptr>("##set-cdr!", &inline_set_cdr));
    fm.insert(std::pair<std::string, fun_ptr>("##eq?", &inline_eq));
    fm.insert(std::pair<std::string, fun_ptr>("##fxadd1", &inline_fx_add1));
    fm.insert(std::pair<std::string, fun_ptr>("##fxsub1", &inline_fx_sub1));
    fm.insert(std::pair<std::string, fun_ptr>("##fxmax", &inline_fx_max));
    fm.insert(std::pair<std::string, fun_ptr>("##fxmin", &inline_fx_min));
    fm.insert(std::pair<std::string, fun_ptr>("##fxzero?", &inline_fx_is_zero));
    fm.insert(std::pair<std::string, fun_ptr>("##fx+", &inline_fx_add));
    fm.insert(std::pair<std::string, fun_ptr>("##fx-", &inline_fx_sub));
    fm.insert(std::pair<std::string, fun_ptr>("##fx*", &inline_fx_mul));
    fm.insert(std::pair<std::string, fun_ptr>("##fx/", &inline_fx_div));
    fm.insert(std::pair<std::string, fun_ptr>("##fx<?", &inline_fx_less));
    fm.insert(std::pair<std::string, fun_ptr>("##fx<=?", &inline_fx_leq));
    fm.insert(std::pair<std::string, fun_ptr>("##fx>?", &inline_fx_greater));
    fm.insert(std::pair<std::string, fun_ptr>("##fx>=?", &inline_fx_geq));
    fm.insert(std::pair<std::string, fun_ptr>("##fx=?", &inline_fx_equal));
    fm.insert(std::pair<std::string, fun_ptr>("##fx!=?", &inline_fx_not_equal));
    fm.insert(std::pair<std::string, fun_ptr>("##fladd1", &inline_fl_add1));
    fm.insert(std::pair<std::string, fun_ptr>("##flsub1", &inline_fl_sub1));
    fm.insert(std::pair<std::string, fun_ptr>("##flmax", &inline_fl_max));
    fm.insert(std::pair<std::string, fun_ptr>("##flmin", &inline_fl_min));
    fm.insert(std::pair<std::string, fun_ptr>("##flzero?", &inline_fl_is_zero));
    fm.insert(std::pair<std::string, fun_ptr>("##fl+", &inline_fl_add));
    fm.insert(std::pair<std::string, fun_ptr>("##fl*", &inline_fl_mul));
    fm.insert(std::pair<std::string, fun_ptr>("##fl/", &inline_fl_div));
    fm.insert(std::pair<std::string, fun_ptr>("##fl-", &inline_fl_sub));
    fm.insert(std::pair<std::string, fun_ptr>("##fl<?", &inline_fl_less));
    fm.insert(std::pair<std::string, fun_ptr>("##fl<=?", &inline_fl_leq));
    fm.insert(std::pair<std::string, fun_ptr>("##fl>?", &inline_fl_greater));
    fm.insert(std::pair<std::string, fun_ptr>("##fl>=?", &inline_fl_geq));
    fm.insert(std::pair<std::string, fun_ptr>("##fl=?", &inline_fl_equal));
    fm.insert(std::pair<std::string, fun_ptr>("##fl!=?", &inline_fl_not_equal));
    fm.insert(std::pair<std::string, fun_ptr>("##fixnum?", &inline_is_fixnum));
    fm.insert(std::pair<std::string, fun_ptr>("##flonum?", &inline_is_flonum));
    fm.insert(std::pair<std::string, fun_ptr>("##pair?", &inline_is_pair));
    fm.insert(std::pair<std::string, fun_ptr>("##vector?", &inline_is_vector));
    fm.insert(std::pair<std::string, fun_ptr>("##string?", &inline_is_string));
    fm.insert(std::pair<std::string, fun_ptr>("##symbol?", &inline_is_symbol));
    fm.insert(std::pair<std::string, fun_ptr>("##closure?", &inline_is_closure));
    fm.insert(std::pair<std::string, fun_ptr>("##procedure?", &inline_is_procedure));
    fm.insert(std::pair<std::string, fun_ptr>("##boolean?", &inline_is_boolean));
    fm.insert(std::pair<std::string, fun_ptr>("##null?", &inline_is_nil));
    fm.insert(std::pair<std::string, fun_ptr>("##eof-object?", &inline_is_eof_object));
    fm.insert(std::pair<std::string, fun_ptr>("##char?", &inline_is_char));
    fm.insert(std::pair<std::string, fun_ptr>("##promise?", &inline_is_promise));
    fm.insert(std::pair<std::string, fun_ptr>("##port?", &inline_is_port));
    fm.insert(std::pair<std::string, fun_ptr>("##input-port?", &inline_is_input_port));
    fm.insert(std::pair<std::string, fun_ptr>("##output-port?", &inline_is_output_port));
    return fm;
    }

  void compile_expression(registered_functions& fns, environment_map& env, repl_data& rd, compile_data& data, asmcode& code, const Expression& expr, const primitive_map& pm, const compiler_options& options, asmcode::operand target = asmcode::RAX, bool expire_registers = true);

  struct get_scan_index_helper
    {
    uint64_t si;

    template <class F>
    void operator()(const F& i)
      {
      si = i.scan_index;
      }

    void operator()(const Literal& i)
      {
      get_scan_index_helper gsih;
      std::visit(gsih, i);
      si = gsih.si;
      }
    };

  uint64_t get_scan_index(const Expression& expr)
    {
    get_scan_index_helper gsih;
    std::visit(gsih, expr);
    return gsih.si;
    }

  void register_allocation_expire_old_intervals(uint64_t time_point, compile_data& data)
    {
    if (time_point < data.first_ra_map_time_point_to_elapse) // first quick check to see whether there are some registers to free
      return;
    uint64_t first_ra_map_time_point_to_elapse = (uint64_t)-1;
    auto it = data.ra_map.begin();
    while (it != data.ra_map.end())
      {
      reg_alloc_data rad = it->first;
      std::string var_name = it->second;
      if (rad.live_range.last <= time_point) // can be removed
        {
        if (rad.type == reg_alloc_data::t_register)
          data.ra->make_register_available(rad.reg);
        else
          data.ra->make_local_available(rad.local_id);
        it = data.ra_map.erase(it);
        }
      else
        {
        first_ra_map_time_point_to_elapse = std::min<uint64_t>(first_ra_map_time_point_to_elapse, rad.live_range.last);
        ++it;
        }
      }
    data.first_ra_map_time_point_to_elapse = first_ra_map_time_point_to_elapse;
    }

  bool target_is_valid(asmcode::operand target)
    {
    if (target == asmcode::R15)
      return false;
    if (target == asmcode::R13)
      return false;
    return true;
    }

  bool expression_can_be_targetted(const Expression& expr)
    {
    return std::holds_alternative<Variable>(expr) || std::holds_alternative<Literal>(expr);
    //return std::holds_alternative<Literal>(expr);
    }

  void compile_nop(asmcode& code, asmcode::operand target)
    {
    assert(target_is_valid(target));
    code.add(asmcode::MOV, target, asmcode::NUMBER, undefined);
    }

  void compile_fixnum(registered_functions&, environment_map&, repl_data&, compile_data&, asmcode& code, const Fixnum& f, const compiler_options&, asmcode::operand target)
    {
    assert(target_is_valid(target));
    code.add(asmcode::MOV, target, asmcode::NUMBER, int2fixnum(f.value));
    }

  void compile_flonum(registered_functions&, environment_map&, repl_data&, compile_data&, asmcode& code, const Flonum& f, const compiler_options& ops, asmcode::operand target)
    {
    assert(target_is_valid(target));
    if (ops.safe_primitives && ops.safe_flonums)
      {
      code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 2);
      check_heap(code, re_flonum_heap_overflow);
      }
    uint64_t header = make_block_header(1, T_FLONUM);
    uint64_t v = *reinterpret_cast<const uint64_t*>(&f.value);
    code.add(asmcode::MOV, asmcode::R15, ALLOC);
    code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
    code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, v);
    code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::RAX);
    code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
    code.add(asmcode::MOV, target, asmcode::R15);
    }

  void compile_true(registered_functions&, environment_map&, repl_data&, compile_data&, asmcode& code, const compiler_options&, asmcode::operand target)
    {
    assert(target_is_valid(target));
    code.add(asmcode::MOV, target, asmcode::NUMBER, bool_t);
    }

  void compile_false(registered_functions&, environment_map&, repl_data&, compile_data&, asmcode& code, const compiler_options&, asmcode::operand target)
    {
    assert(target_is_valid(target));
    code.add(asmcode::MOV, target, asmcode::NUMBER, bool_f);
    }

  void compile_nil(registered_functions&, environment_map&, repl_data&, compile_data&, asmcode& code, const compiler_options&, asmcode::operand target)
    {
    assert(target_is_valid(target));
    code.add(asmcode::MOV, target, asmcode::NUMBER, nil);
    }

  void compile_character(registered_functions&, environment_map&, repl_data&, compile_data&, asmcode& code, const Character& c, const compiler_options&, asmcode::operand target)
    {
    assert(target_is_valid(target));
    int64_t i = int64_t(c.value) << 8;
    i |= char_tag;
    code.add(asmcode::MOV, target, asmcode::NUMBER, i);
    }

  void compile_string(registered_functions&, environment_map&, repl_data&, compile_data&, asmcode& code, const String& s, const compiler_options& ops, asmcode::operand target)
    {
    assert(target_is_valid(target));
    std::string str = s.value;
    int nr_of_args = (int)str.length();

    if (ops.safe_primitives)
      {
      code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, ((nr_of_args >> 3) + 2));
      check_heap(code, re_string_heap_overflow);
      }

    code.add(asmcode::MOV, asmcode::R15, ALLOC);
    code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);


    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, (uint64_t)string_tag << (uint64_t)block_shift);
    code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, ((nr_of_args >> 3) + 1));
    code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
    code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));

    for (int i = 0; i < nr_of_args; ++i)
      {
      code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, (uint64_t)str[i]);
      code.add(asmcode::MOV, BYTE_MEM_ALLOC, asmcode::AL);
      code.add(asmcode::INC, ALLOC);
      }
    code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
    int end = ((nr_of_args >> 3) + 1) << 3;
    for (int i = nr_of_args; i < end; ++i)
      {
      code.add(asmcode::MOV, BYTE_MEM_ALLOC, asmcode::AL);
      code.add(asmcode::INC, ALLOC);
      }

    code.add(asmcode::MOV, target, asmcode::R15);
    }

  void compile_symbol(registered_functions&, environment_map&, repl_data&, compile_data&, asmcode& code, const Symbol& s, const compiler_options& ops, asmcode::operand target)
    {
    assert(target_is_valid(target));
    std::string str = s.value;
    int nr_of_args = (int)str.length();

    if (ops.safe_primitives)
      {
      code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, ((nr_of_args >> 3) + 2));
      check_heap(code, re_symbol_heap_overflow);
      }

    code.add(asmcode::MOV, asmcode::R15, ALLOC);
    code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);


    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, (uint64_t)symbol_tag << (uint64_t)block_shift);
    code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, ((nr_of_args >> 3) + 1));
    code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
    code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));

    for (int i = 0; i < nr_of_args; ++i)
      {
      code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, (uint64_t)str[i]);
      code.add(asmcode::MOV, BYTE_MEM_ALLOC, asmcode::AL);
      code.add(asmcode::INC, ALLOC);
      }
    code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
    int end = ((nr_of_args >> 3) + 1) << 3;
    for (int i = nr_of_args; i < end; ++i)
      {
      code.add(asmcode::MOV, BYTE_MEM_ALLOC, asmcode::AL);
      code.add(asmcode::INC, ALLOC);
      }

    code.add(asmcode::MOV, target, asmcode::R15);
    }

  void compile_literal(registered_functions& fns, environment_map& env, repl_data& rd, compile_data& data, asmcode& code, const Literal& lit, const compiler_options& options, asmcode::operand target)
    {
    assert(target_is_valid(target));
    if (std::holds_alternative<Fixnum>(lit))
      {
      compile_fixnum(fns, env, rd, data, code, std::get<Fixnum>(lit), options, target);
      }
    else if (std::holds_alternative<True>(lit))
      {
      compile_true(fns, env, rd, data, code, options, target);
      }
    else if (std::holds_alternative<False>(lit))
      {
      compile_false(fns, env, rd, data, code, options, target);
      }
    else if (std::holds_alternative<Nil>(lit))
      {
      compile_nil(fns, env, rd, data, code, options, target);
      }
    else if (std::holds_alternative<Character>(lit))
      {
      compile_character(fns, env, rd, data, code, std::get<Character>(lit), options, target);
      }
    else if (std::holds_alternative<Flonum>(lit))
      {
      compile_flonum(fns, env, rd, data, code, std::get<Flonum>(lit), options, target);
      }
    else if (std::holds_alternative<String>(lit))
      {
      compile_string(fns, env, rd, data, code, std::get<String>(lit), options, target);
      }
    else if (std::holds_alternative<Symbol>(lit))
      {
      compile_symbol(fns, env, rd, data, code, std::get<Symbol>(lit), options, target);
      }
    else
      throw_error(not_implemented);
    }

  void compile_if(registered_functions& fns, environment_map& env, repl_data& rd, compile_data& cd, asmcode& code, const If& i, const primitive_map& pm, const compiler_options& ops)
    {
    assert(i.arguments.size() == 3);
    if (i.arguments.size() != 3)
      throw_error(i.line_nr, i.column_nr, i.filename, invalid_number_of_arguments);
    compile_expression(fns, env, rd, cd, code, i.arguments.front(), pm, ops);
    uint64_t lab1 = label++;
    uint64_t lab2 = label++;
    code.add(asmcode::CMP, asmcode::RAX, asmcode::NUMBER, bool_f);
    code.add(asmcode::JE, label_to_string(lab1));
    compile_expression(fns, env, rd, cd, code, i.arguments[1], pm, ops);
    code.add(asmcode::JMP, label_to_string(lab2));
    code.add(asmcode::LABEL, label_to_string(lab1));
    compile_expression(fns, env, rd, cd, code, i.arguments[2], pm, ops);
    code.add(asmcode::LABEL, label_to_string(lab2));
    }

  void compile_funcall(registered_functions& fns, environment_map& env, repl_data& rd, compile_data& cd, asmcode& code, const FunCall& fun, const primitive_map& pm, const compiler_options& ops)
    {
    const auto& arg_reg = get_argument_registers();

    for (size_t i = 0; i < fun.arguments.size(); ++i)
      {
      std::stringstream str;
      str << i + 1;
      code.add(asmcode::COMMENT, "push function arg " + str.str() + " to stack");
      compile_expression(fns, env, rd, cd, code, fun.arguments[i], pm, ops);
      //code.add(asmcode::PUSH, asmcode::RAX);
      push(code, asmcode::RAX);
      }

    code.add(asmcode::COMMENT, "compute function");
    compile_expression(fns, env, rd, cd, code, fun.fun.front(), pm, ops); // the function itself should be evaluated after its arguments. Not clear why, but bugs otherwise.

    auto no_closure = label_to_string(label++);
    std::string error;
    code.add(asmcode::COMMENT, "check whether function is actually closure or primitive object");
    jump_if_arg_is_not_block(code, asmcode::RAX, asmcode::R15, no_closure);
    if (ops.safe_primitives)
      {
      error = label_to_string(label++);
      code.add(asmcode::MOV, asmcode::R11, asmcode::RAX);
      code.add(asmcode::AND, asmcode::R11, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
      jump_if_arg_does_not_point_to_closure(code, asmcode::R11, asmcode::R15, error);
      }
    code.add(asmcode::COMMENT, "save function in rcx as self");
    code.add(asmcode::MOV, arg_reg[0], asmcode::RAX);
    for (int i = (int)fun.arguments.size() - 1; i >= 0; --i)
      {
      int j = i + 1;
      std::stringstream str;
      str << j;
      code.add(asmcode::COMMENT, "get function arg " + str.str() + " from stack");
      if (j >= arg_reg.size())
        {
        uint32_t local_id = (uint32_t)(j - (int)arg_reg.size());
        //code.add(asmcode::POP, asmcode::RAX);
        pop(code, asmcode::RAX);
        if (local_id >= cd.local_stack_size)
          throw_error(too_many_locals);
        save_to_local(code, local_id);
        }
      else
        {
        const auto& reg = arg_reg[j];
        //code.add(asmcode::POP, reg);
        pop(code, reg);
        }
      }

    // now is the moment to call garbage collection
    // all arguments are in the registers or locals
    // there are no other local variables due to cps conversion
    if (ops.garbage_collection)
      {
      std::string garbage_error = label_to_string(label++);
      std::string continue_label = label_to_string(label++);
      code.add(asmcode::CMP, ALLOC, LIMIT);
      code.add(asmcode::JLES, continue_label);
      code.add(asmcode::CMP, ALLOC, FROM_SPACE_END);
      code.add(asmcode::JGES, garbage_error);
      code.add(asmcode::COMMENT, "jump to reclaim-garbage");

      auto pm_it = pm.find("reclaim-garbage");
      if (pm_it == pm.end())
        throw_error(primitive_unknown);

      code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, pm_it->second.address);
      code.add(asmcode::MOV, CONTINUE, asmcode::LABELADDRESS, continue_label);
      code.add(asmcode::JMP, asmcode::RAX);
      error_label(code, garbage_error, re_heap_full);
      code.add(asmcode::LABEL, continue_label);
      }

    code.add(asmcode::COMMENT, "number of arguments in r11");
    code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, fun.arguments.size() + 1); // can be removed possibly, if reclaim keeps r11 untouched
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
    code.add(asmcode::MOV, asmcode::R15, asmcode::MEM_RAX, CELLS(1));
    code.add(asmcode::COMMENT, "jump to closure label");
    code.add(asmcode::JMP, asmcode::R15);

    code.add(asmcode::LABEL, no_closure);
    // RAX contains address to primitive procedure
    if (ops.safe_primitives)
      {
      code.add(asmcode::MOV, asmcode::R15, asmcode::RAX);
      code.add(asmcode::AND, asmcode::R15, asmcode::NUMBER, procedure_mask);
      code.add(asmcode::CMP, asmcode::R15, asmcode::NUMBER, procedure_tag);
      code.add(asmcode::JNE, error);
      }

    code.add(asmcode::MOV, asmcode::R15, asmcode::RAX);
    code.add(asmcode::AND, asmcode::R15, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);

    // skip first argument, as this is a continuation
    for (int i = (int)fun.arguments.size() - 1; i >= 1; --i)
      {
      int j = i - 1;
      if (j >= arg_reg.size())
        {
        uint32_t local_id = (uint32_t)(j - (int)arg_reg.size());
        //code.add(asmcode::POP, asmcode::RAX);
        pop(code, asmcode::RAX);
        if (local_id >= cd.local_stack_size)
          throw_error(too_many_locals);
        save_to_local(code, local_id, asmcode::RAX, asmcode::R11);
        }
      else
        {
        const auto& reg = arg_reg[j];
        //code.add(asmcode::POP, reg);
        pop(code, reg);
        }
      }

    auto continue_primitive = label_to_string(label++);
    code.add(asmcode::MOV, CONTINUE, asmcode::LABELADDRESS, continue_primitive);
    code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, fun.arguments.size() - 1);
    code.add(asmcode::JMP, asmcode::R15); // call primitive
    code.add(asmcode::LABEL, continue_primitive);

    code.add(asmcode::COMMENT, "pop continuation object after prim object call");
    //code.add(asmcode::POP, asmcode::RCX); // get closure
    pop(code, asmcode::RCX);
    code.add(asmcode::MOV, asmcode::RDX, asmcode::RAX);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    if (ops.safe_primitives)
      {
      jump_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
      }
    code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
    if (ops.safe_primitives)
      {
      jump_if_arg_does_not_point_to_closure(code, asmcode::RAX, asmcode::R11, error);
      }
    code.add(asmcode::MOV, asmcode::R15, asmcode::MEM_RAX, CELLS(1));
    code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::COMMENT, "call continuation object after prim object call");
    code.add(asmcode::JMP, asmcode::R15);

    if (ops.safe_primitives)
      {
      error_label(code, error, re_closure_expected);
      }
    }

  void compile_variable_arity_rest_arg(asmcode& code, const compiler_options& ops, size_t arg_pos)
    {
    std::string not_empty = label_to_string(label++);
    std::string done = label_to_string(label++);
    std::string done2 = label_to_string(label++);
    std::string repeat = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 0);
    code.add(asmcode::JGS, not_empty);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, nil);
    code.add(asmcode::JMP, CONTINUE);
    code.add(asmcode::LABEL, not_empty);
    code.add(asmcode::PUSH, asmcode::RCX);
    code.add(asmcode::PUSH, asmcode::RDX);
    if (ops.safe_primitives)
      {
      code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);
      code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 1);
      code.add(asmcode::ADD, asmcode::RAX, asmcode::R11); // shl 1 and add for * 3
      check_heap(code, re_list_heap_overflow);
      }
    code.add(asmcode::MOV, asmcode::R15, ALLOC);
    code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);

    uint64_t header = make_block_header(2, T_PAIR);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);

    code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);

    if (arg_pos < get_argument_registers().size())
      {
      code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), get_argument_registers()[arg_pos]);
      }
    else
      {
      load_local(code, arg_pos - get_argument_registers().size(), asmcode::RDX, asmcode::RCX);
      code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::RDX);
      }
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JE, done);

    int max_iter = 6;
    for (int i = 1; i < max_iter; ++i)
      {
      code.add(asmcode::MOV, asmcode::RCX, ALLOC);
      code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(3));
      code.add(asmcode::OR, asmcode::RCX, asmcode::NUMBER, block_tag);
      code.add(asmcode::MOV, MEM_ALLOC, CELLS(2), asmcode::RCX);
      code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(3));
      code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);

      if ((arg_pos + i) < get_argument_registers().size())
        {
        code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), get_argument_registers()[arg_pos + i]);
        }
      else
        {
        load_local(code, arg_pos + i - get_argument_registers().size(), asmcode::RDX, asmcode::RCX);
        code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::RDX);
        }
      code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, i + 1);
      code.add(asmcode::JE, done);
      }

    code.add(asmcode::SUB, asmcode::R11, asmcode::NUMBER, max_iter);

    code.add(asmcode::PUSH, asmcode::RSI);

    code.add(asmcode::MOV, asmcode::RDX, LOCAL);
    code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS((arg_pos + max_iter - get_argument_registers().size())));
    code.add(asmcode::LABEL, repeat);
    code.add(asmcode::MOV, asmcode::RSI, asmcode::MEM_RDX);

    code.add(asmcode::MOV, asmcode::RCX, ALLOC);
    code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(3));
    code.add(asmcode::OR, asmcode::RCX, asmcode::NUMBER, block_tag);
    code.add(asmcode::MOV, MEM_ALLOC, CELLS(2), asmcode::RCX);
    code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(3));
    code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
    code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::RSI);
    code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1));
    code.add(asmcode::DEC, asmcode::R11);
    code.add(asmcode::JES, done2);
    code.add(asmcode::JMPS, repeat);

    code.add(asmcode::LABEL, done2);
    code.add(asmcode::POP, asmcode::RSI);

    code.add(asmcode::LABEL, done);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, nil);
    code.add(asmcode::MOV, MEM_ALLOC, CELLS(2), asmcode::RAX);
    code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(3));
    code.add(asmcode::MOV, asmcode::RAX, asmcode::R15);
    code.add(asmcode::POP, asmcode::RDX);
    code.add(asmcode::POP, asmcode::RCX);
    code.add(asmcode::JMP, CONTINUE);
    }

  void compile_lambda(registered_functions& fns, environment_map& env, repl_data& rd, compile_data& cd, asmcode& code, const Lambda& lam, const primitive_map& pm, const compiler_options& ops)
    {
    compile_data new_cd = create_compile_data(cd.heap_size, cd.globals_stack, cd.ra->number_of_locals(), cd.p_ctxt);
    new_cd.halt_label = cd.halt_label;
    new_cd.ra->make_all_available();
    code.push();
    auto lab = label_to_string(label++);
    code.add(asmcode::LABEL_ALIGNED, lab);
    environment_map new_env = std::make_shared<environment<environment_entry>>(env);
    for (size_t i = 0; i < lam.variables.size(); ++i)
      {
      const std::string& var_name = lam.variables[i];
      if (i < get_argument_registers().size())
        {
        environment_entry e;
        e.st = environment_entry::st_register;
        e.pos = (uint64_t)get_argument_registers()[i];
        e.live_range = lam.live_ranges[i];
        new_env->push(var_name, e);
        new_cd.ra->make_register_unavailable(get_argument_registers()[i]);
        new_cd.ra_map[make_reg_alloc_data(reg_alloc_data::t_register, e.pos, lam.live_ranges[i])] = var_name;
        new_cd.first_ra_map_time_point_to_elapse = std::min<uint64_t>(new_cd.first_ra_map_time_point_to_elapse, lam.live_ranges[i].last);
        }
      else
        {
        size_t j = i - get_argument_registers().size();
        environment_entry e;
        e.st = environment_entry::st_local;
        e.pos = (uint64_t)j;
        e.live_range = lam.live_ranges[i];
        new_env->push(var_name, e);
        new_cd.ra->make_local_unavailable((uint32_t)j);
        new_cd.ra_map[make_reg_alloc_data(reg_alloc_data::t_local, e.pos, lam.live_ranges[i])] = var_name;
        new_cd.first_ra_map_time_point_to_elapse = std::min<uint64_t>(new_cd.first_ra_map_time_point_to_elapse, lam.live_ranges[i].last);
        }
      }
    std::string error;
    if (ops.safe_primitives)
      {
      error = label_to_string(label++);
      uint64_t nr_of_args_necessary = lam.variables.size();
      if (lam.variable_arity && nr_of_args_necessary)
        --nr_of_args_necessary;
      code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, nr_of_args_necessary);
      code.add(asmcode::JL, error);
      }
    if (lam.variable_arity)
      {
      auto cont = label_to_string(label++);
      code.add(asmcode::MOV, CONTINUE, asmcode::LABELADDRESS, cont);
      size_t arg_pos = lam.variables.size() - 1;

      // R11 = self + cps + real vars + rest where rest counts as 1 => rest = R11 - self - cps - real vars = R11 - (lam.variables.size() - 1)
      code.add(asmcode::SUB, asmcode::R11, asmcode::NUMBER, lam.variables.size() - 1);
      compile_variable_arity_rest_arg(code, ops, arg_pos);
      code.add(asmcode::LABEL, cont);
      if (arg_pos < get_argument_registers().size())
        {
        code.add(asmcode::MOV, get_argument_registers()[arg_pos], asmcode::RAX);
        }
      else
        {
        size_t j = arg_pos - get_argument_registers().size();
        save_to_local(code, j);
        }
      }
    if (lam.body.size() != 1)
      throw_error(lam.line_nr, lam.column_nr, lam.filename, bad_syntax);
    compile_expression(fns, new_env, rd, new_cd, code, lam.body.front(), pm, ops);
    code.add(asmcode::RET);
    if (ops.safe_primitives)
      {
      error_label(code, error, re_lambda_invalid_number_of_arguments);
      }
    code.pop();
    code.add(asmcode::MOV, asmcode::RAX, asmcode::LABELADDRESS, lab);
    //code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, procedure_tag);
    }

  void compile_set(registered_functions& fns, environment_map& env, repl_data& rd, compile_data& cd, asmcode& code, const Set& s, const primitive_map& pm, const compiler_options& ops)
    {
    if (s.originates_from_define || s.originates_from_quote)
      {
      compile_expression(fns, env, rd, cd, code, s.value.front(), pm, ops);

      environment_entry e;
      if (!env->find(e, s.name))
        throw_error(s.line_nr, s.column_nr, s.filename, invalid_variable_name);

      code.add(asmcode::MOV, asmcode::R15, GLOBALS);
      code.add(asmcode::MOV, asmcode::MEM_R15, e.pos, asmcode::RAX);

      // We don't need to update the environment with the new global variable or quote, as this is 
      // already done by the passes global_define_env and quote_conversion.
      // We do this in the passes so that function names are already known in the body of a recursive function.     
      }
    else //set! in global namespace, so we are changing the value of a global variable
      {
      environment_entry e;
      if (!env->find(e, s.name))
        throw_error(s.line_nr, s.column_nr, s.filename, invalid_variable_name);

      //set! in global namespace
      compile_expression(fns, env, rd, cd, code, s.value.front(), pm, ops);

      code.add(asmcode::MOV, asmcode::R15, GLOBALS);
      code.add(asmcode::MOV, asmcode::MEM_R15, e.pos, asmcode::RAX);

      }
    }

  void add_global_variable_to_debug_info(asmcode& code, uint64_t pos, const compiler_options& ops)
    {
    if (ops.keep_variable_stack)
      {
      code.add(asmcode::COMMENT, "Start debug info");
      code.add(asmcode::COMMENT, "\tCycling the call stack");
      for (int i = SKIWI_VARIABLE_DEBUG_STACK_SIZE - 2; i >= 0; --i) // move items on the stack further down
        {
        code.add(asmcode::MOV, asmcode::R15, LAST_GLOBAL_VARIABLE_USED + CELLS(i));
        code.add(asmcode::MOV, LAST_GLOBAL_VARIABLE_USED + CELLS(i + 1), asmcode::R15);
        }
      code.add(asmcode::COMMENT, "\tAdding the latest item to the call stack");
      code.add(asmcode::MOV, LAST_GLOBAL_VARIABLE_USED, asmcode::NUMBER, pos); // push last item on the stack
      code.add(asmcode::COMMENT, "Stop debug info");
      }
    }

  void compile_variable(registered_functions&, environment_map& env, repl_data& rd, compile_data& cd, asmcode& code, const Variable& var, const compiler_options& ops, asmcode::operand target)
    {
    assert(target_is_valid(target));
    environment_entry e;
    if (!env->find(e, var.name))
      {
      /*
      This variable was not found in the environment. However: it is possible that this variable will be defined by a load at runtime.
      Therefore we now allocate a global position for this variable. It might be filled in later by load.
      */

      std::string name_before_alpha_conversion = get_variable_name_before_alpha(var.name);

      rd.alpha_conversion_env->push(name_before_alpha_conversion, var.name);
      environment_entry ne;
      ne.st = environment_entry::st_global;
      ne.pos = rd.global_index * 8;
      ++(rd.global_index);
      env->push_outer(var.name, ne);
      uint64_t* addr = cd.p_ctxt->globals + (ne.pos >> 3);
      *addr = unresolved_tag; // This is a new address, previously equal to unalloc_tag. To avoid that gc stops here when cleaning, we change its value to unresolved_tag.

      code.add(asmcode::MOV, asmcode::R15, GLOBALS);
      code.add(asmcode::MOV, target, asmcode::MEM_R15, ne.pos);
      add_global_variable_to_debug_info(code, ne.pos, ops);
      }
    else
      {
      switch (e.st)
        {
        case environment_entry::st_local:
          load_local(code, e.pos, target, asmcode::R15); break;
        case environment_entry::st_register:
          if (target != (asmcode::operand)e.pos)
            code.add(asmcode::MOV, target, (asmcode::operand)e.pos); break;
        case environment_entry::st_global:
          code.add(asmcode::MOV, asmcode::R15, GLOBALS);
          code.add(asmcode::MOV, target, asmcode::MEM_R15, e.pos);
          add_global_variable_to_debug_info(code, e.pos, ops);
          break;
        }
      }
    }

  std::vector<asmcode::operand> get_inlined_args()
    {
    std::vector<asmcode::operand> vec;
    vec.push_back(asmcode::RAX);
    vec.push_back(asmcode::RBX);
    //vec.push_back(asmcode::R11);
    return vec;
    }

  void compile_prim_call_inlined(registered_functions& fns, environment_map& env, repl_data& rd, compile_data& data, asmcode& code, const PrimitiveCall& prim, const primitive_map& pm, const compiler_options& options)
    {
    assert(is_inlined_primitive(prim.primitive_name));

    static std::vector<asmcode::operand> args = get_inlined_args();

    int nr_prim_args = (int)prim.arguments.size();

    assert(nr_prim_args <= args.size());

    if (nr_prim_args == 1)
      {
      compile_expression(fns, env, rd, data, code, prim.arguments.back(), pm, options);
      }
    else if (nr_prim_args > 1)
      {
      bool expressions_can_be_targetted = options.fast_expression_targetting;
      for (const auto& expr : prim.arguments)
        expressions_can_be_targetted &= expression_can_be_targetted(expr);

      if (expressions_can_be_targetted)
        {
        for (int i = nr_prim_args - 1; i >= 0; --i)
          compile_expression(fns, env, rd, data, code, prim.arguments[i], pm, options, args[i], false); // since we are going backwards, we cannot expire intervals. It is safe to go backwards, as the expressions are simple
        }
      else
        {
        if (!prim.arguments.empty())
          {
          for (size_t i = 0; i < nr_prim_args - 1; ++i)
            {
            compile_expression(fns, env, rd, data, code, prim.arguments[i], pm, options);
            //code.add(asmcode::PUSH, asmcode::RAX);
            push(code, asmcode::RAX);
            }
          compile_expression(fns, env, rd, data, code, prim.arguments.back(), pm, options);
          code.add(asmcode::MOV, args[nr_prim_args - 1], asmcode::RAX);
          for (int i = nr_prim_args - 2; i >= 0; --i)
            {
            //code.add(asmcode::POP, args[i]);
            pop(code, args[i]);
            }
          }
        }
      }

    auto inlined_pm_it = fns.inlined_primitives->find(prim.primitive_name);
    if (inlined_pm_it == fns.inlined_primitives->end())
      throw_error(prim.line_nr, prim.column_nr, prim.filename, primitive_unknown, prim.primitive_name);
    code.add(asmcode::COMMENT, "inlining " + inlined_pm_it->first);

    inlined_pm_it->second(code, options);
    }

  void compile_prim_call_not_inlined(registered_functions& fns, environment_map& env, repl_data& rd, compile_data& data, asmcode& code, const PrimitiveCall& prim, const primitive_map& pm, const compiler_options& options)
    {
    assert(!is_inlined_primitive(prim.primitive_name));

    const auto& arg_reg = get_argument_registers();
    int nr_prim_args = (int)prim.arguments.size();
    /*
    First we check whether the registers and locals for arguments are free.
    If they are not free we first push them onto the stack, and pop them at the end.
    */
    std::vector<uint32_t> locals_that_were_not_free;
    std::vector<asmcode::operand> registers_that_were_not_free;
    for (int i = 0; i < nr_prim_args; ++i)
      {
      if (i >= (int)arg_reg.size())
        {
        uint32_t local_id = (uint32_t)(i - (int)arg_reg.size());
        if (local_id >= data.local_stack_size)
          throw_error(too_many_locals);
        if (!data.ra->is_free_local(local_id))
          {
          locals_that_were_not_free.push_back(local_id);
          load_local(code, local_id);
          //code.add(asmcode::PUSH, asmcode::RAX); //possibly one call here? todo
          push(code, asmcode::RAX);
          }
        }
      else
        {
        const auto& reg = arg_reg[i];
        if (!data.ra->is_free_register(reg))
          {
          registers_that_were_not_free.push_back(reg);
          //code.add(asmcode::PUSH, reg);
          push(code, reg);
          }
        }
      }

    bool expressions_can_be_targetted = options.fast_expression_targetting;
    for (const auto& expr : prim.arguments)
      expressions_can_be_targetted &= expression_can_be_targetted(expr);
    expressions_can_be_targetted &= (prim.arguments.size() <= arg_reg.size());
    if (expressions_can_be_targetted && !prim.arguments.empty()) // check whether no overlap with variables
      {
      for (int i = 0; i < nr_prim_args; ++i)
        {
        const auto& arg = prim.arguments[i];
        if (std::holds_alternative<Variable>(arg))
          {
          const Variable& var = std::get<Variable>(arg);
          environment_entry e;
          if (env->find(e, var.name) && (e.st == environment_entry::st_register))
            {
            if (e.pos < (uint64_t)arg_reg[i])
              {
              expressions_can_be_targetted = false;
              }
            }
          }
        }
      }

    if (expressions_can_be_targetted)
      {
      // we choose this order of evaluation as it has most likelyhood to not interact with variables saved in register (I think)
      for (int i = 0; i < prim.arguments.size(); ++i)
        compile_expression(fns, env, rd, data, code, prim.arguments[i], pm, options, arg_reg[i], false);
      }
    else
      {
      if (nr_prim_args > 0)
        {
        for (int i = 0; i < nr_prim_args - 1; ++i)
          {
          compile_expression(fns, env, rd, data, code, prim.arguments[i], pm, options);
          //code.add(asmcode::PUSH, asmcode::RAX);
          push(code, asmcode::RAX);
          }
        compile_expression(fns, env, rd, data, code, prim.arguments.back(), pm, options);
        int i = (int)prim.arguments.size() - 1;
        if (i >= arg_reg.size())
          {
          uint32_t local_id = (uint32_t)(i - (int)arg_reg.size());
          if (local_id >= data.local_stack_size)
            throw_error(too_many_locals);
          save_to_local(code, local_id);
          }
        else
          {
          const auto& reg = arg_reg[i];
          code.add(asmcode::MOV, reg, asmcode::RAX);
          }
        }

      for (int i = nr_prim_args - 2; i >= 0; --i)
        {
        if (i >= arg_reg.size())
          {
          uint32_t local_id = (uint32_t)(i - (int)arg_reg.size());
          if (local_id >= data.local_stack_size)
            throw_error(too_many_locals);
          //code.add(asmcode::POP, asmcode::RAX);
          pop(code, asmcode::RAX);
          save_to_local(code, local_id);
          }
        else
          {
          const auto& reg = arg_reg[i];
          //code.add(asmcode::POP, reg);
          pop(code, reg);
          }
        }
      }

    // here call primitive    

    std::string continue_label = label_to_string(label++);

    if (prim.primitive_name == "halt")
      {
      if (options.garbage_collection)
        {
        code.add(asmcode::COMMENT, "clean all locals for gc");
        //Here we clear all our registers, so that they can be cleaned up by the gc
        code.add(asmcode::MOV, asmcode::R15, LOCAL);
        code.add(asmcode::MOV, asmcode::R11, GLOBALS);
        auto repeat_clean_locals = label_to_string(label++);
        code.add(asmcode::LABEL, repeat_clean_locals);
        code.add(asmcode::MOV, asmcode::MEM_R15, asmcode::NUMBER, unalloc_tag);
        code.add(asmcode::ADD, asmcode::R15, asmcode::NUMBER, 8);
        code.add(asmcode::CMP, asmcode::R15, asmcode::R11);
        code.add(asmcode::JLS, repeat_clean_locals);
        code.add(asmcode::COMMENT, "clean all registers for gc");
        code.add(asmcode::MOV, asmcode::RDX, asmcode::NUMBER, unalloc_tag);
        code.add(asmcode::MOV, asmcode::RSI, asmcode::NUMBER, unalloc_tag);
        code.add(asmcode::MOV, asmcode::RDI, asmcode::NUMBER, unalloc_tag);
        code.add(asmcode::MOV, asmcode::R8, asmcode::NUMBER, unalloc_tag);
        code.add(asmcode::MOV, asmcode::R9, asmcode::NUMBER, unalloc_tag);
        code.add(asmcode::MOV, asmcode::R12, asmcode::NUMBER, unalloc_tag);
        code.add(asmcode::MOV, asmcode::R14, asmcode::NUMBER, unalloc_tag);
        std::string garbage_error = label_to_string(label++);
        std::string continue_label2 = label_to_string(label++);
        code.add(asmcode::CMP, ALLOC, LIMIT);
        code.add(asmcode::JLES, continue_label2);
        code.add(asmcode::CMP, ALLOC, FROM_SPACE_END);
        code.add(asmcode::JGES, garbage_error);
        code.add(asmcode::COMMENT, "jump to reclaim-garbage");
        auto pm_it = pm.find("reclaim-garbage");
        if (pm_it == pm.end())
          throw_error(primitive_unknown);
        code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, pm_it->second.address);
        code.add(asmcode::MOV, CONTINUE, asmcode::LABELADDRESS, continue_label2);
        code.add(asmcode::JMP, asmcode::RAX);
        error_label(code, garbage_error, re_heap_full);
        code.add(asmcode::LABEL, continue_label2);

        }
      code.add(asmcode::MOV, CONTINUE, asmcode::LABELADDRESS, data.halt_label);
      }
    else
      code.add(asmcode::MOV, CONTINUE, asmcode::LABELADDRESS, continue_label);

    code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, prim.arguments.size());

    auto pm_it = pm.find(prim.primitive_name);
    if (pm_it == pm.end())
      throw_error(prim.line_nr, prim.column_nr, prim.filename, primitive_unknown, prim.primitive_name);
    code.add(asmcode::COMMENT, "jump to " + pm_it->first);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, pm_it->second.address);
    code.add(asmcode::JMP, asmcode::RAX);

    code.add(asmcode::LABEL, continue_label);

    /*
    Restore the registers and locals that were not free in reverse order
    */
    for (auto it = locals_that_were_not_free.rbegin(); it != locals_that_were_not_free.rend(); ++it)
      {
      uint32_t local_id = *it;
      if (local_id >= data.local_stack_size)
        throw_error(too_many_locals);
      //code.add(asmcode::POP, asmcode::R11);
      pop(code, asmcode::R11);
      save_to_local(code, local_id, asmcode::R11, asmcode::R15);
      }
    for (auto it = registers_that_were_not_free.rbegin(); it != registers_that_were_not_free.rend(); ++it)
      {
      //code.add(asmcode::POP, *it);
      pop(code, *it);
      }
    }

  void compile_prim_call(registered_functions& fns, environment_map& env, repl_data& rd, compile_data& data, asmcode& code, const PrimitiveCall& prim, const primitive_map& pm, const compiler_options& options)
    {
    if (is_inlined_primitive(prim.primitive_name))
      compile_prim_call_inlined(fns, env, rd, data, code, prim, pm, options);
    else
      compile_prim_call_not_inlined(fns, env, rd, data, code, prim, pm, options);
    }

  void compile_prim_object(registered_functions&, environment_map& env, repl_data&, compile_data&, asmcode& code, const PrimitiveCall& prim, const primitive_map&, const compiler_options& ops)
    {
    environment_entry e;
    if (!env->find(e, prim.primitive_name))
      {
      throw_error(prim.line_nr, prim.column_nr, prim.filename, primitive_unknown, prim.primitive_name);
      }
    code.add(asmcode::MOV, asmcode::R15, GLOBALS);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_R15, e.pos);
    add_global_variable_to_debug_info(code, e.pos, ops);
    }

  void compile_prim(registered_functions& fns, environment_map& env, repl_data& rd, compile_data& data, asmcode& code, const PrimitiveCall& prim, const primitive_map& pm, const compiler_options& options)
    {
    if (prim.as_object)
      compile_prim_object(fns, env, rd, data, code, prim, pm, options);
    else
      compile_prim_call(fns, env, rd, data, code, prim, pm, options);
    }

  std::vector<asmcode::operand> get_windows_calling_registers()
    {
    std::vector<asmcode::operand> calling_registers;
    calling_registers.push_back(asmcode::RCX);
    calling_registers.push_back(asmcode::RDX);
    calling_registers.push_back(asmcode::R8);
    calling_registers.push_back(asmcode::R9);
    return calling_registers;
    }

  std::vector<asmcode::operand> get_linux_calling_registers()
    {
    std::vector<asmcode::operand> calling_registers;
    calling_registers.push_back(asmcode::RDI);
    calling_registers.push_back(asmcode::RSI);
    calling_registers.push_back(asmcode::RDX);
    calling_registers.push_back(asmcode::RCX);
    calling_registers.push_back(asmcode::R8);
    calling_registers.push_back(asmcode::R9);
    return calling_registers;
    }

  std::vector<asmcode::operand> get_floating_point_registers()
    {
    std::vector<asmcode::operand> calling_registers;
    calling_registers.push_back(asmcode::XMM0);
    calling_registers.push_back(asmcode::XMM1);
    calling_registers.push_back(asmcode::XMM2);
    calling_registers.push_back(asmcode::XMM3);
    calling_registers.push_back(asmcode::XMM4);
    calling_registers.push_back(asmcode::XMM5);
    return calling_registers;
    }

  void compile_foreign_call(registered_functions& fns, environment_map& env, repl_data& rd, compile_data& data, asmcode& code, const ForeignCall& foreign, const primitive_map& pm, const compiler_options& ops)
    {
    std::string error;
    if (ops.safe_primitives)
      error = label_to_string(label++);

#ifdef _WIN32
    static std::vector<asmcode::operand> arg_reg = get_windows_calling_registers();
#else
    static std::vector<asmcode::operand> arg_reg = get_linux_calling_registers();
#endif
    static std::vector<asmcode::operand> arg_float_reg = get_floating_point_registers();


    if (foreign.arguments.size() > arg_reg.size())
      throw_error(foreign.line_nr, foreign.column_nr, foreign.filename, not_implemented);

    //auto ext_it = std::find_if(fns.externals->begin(), fns.externals->end(), [&](const external_function& f) { return f.name == foreign.foreign_name; });
    auto ext_it = fns.externals->find(foreign.foreign_name);
    if (ext_it == fns.externals->end())
      throw_error(foreign.line_nr, foreign.column_nr, foreign.filename, foreign_call_unknown, foreign.foreign_name);
    const external_function& ext = ext_it->second;

    if (ext.arguments.size() < foreign.arguments.size())
      throw_error(foreign.line_nr, foreign.column_nr, foreign.filename, invalid_number_of_arguments, foreign.foreign_name);

    /*
    First we check whether the registers are free.
    If they are not free we first push them onto the stack, and pop them at the end.
    */
    std::vector<asmcode::operand> registers_that_were_not_free;
    for (size_t i = 0; i < foreign.arguments.size(); ++i)
      {
      const auto& reg = arg_reg[i];
      if (!data.ra->is_free_register(reg))
        {
        registers_that_were_not_free.push_back(reg);
        //code.add(asmcode::PUSH, reg);
        push(code, reg);
        }
      }

#ifndef _WIN32
    std::vector<asmcode::operand> arg_regs;
    int regular_arg_id = 0;
    int floating_arg_id = 0;
    for (size_t i = 0; i < foreign.arguments.size(); ++i)
      {
      if (ext.arguments[i] == external_function::T_DOUBLE)
        {
        arg_regs.push_back(arg_float_reg[floating_arg_id]);
        ++floating_arg_id;
        }
      else
        {
        arg_regs.push_back(arg_reg[regular_arg_id]);
        ++regular_arg_id;
        }
      }
#endif

    for (size_t i = 0; i < foreign.arguments.size(); ++i)
      {
      compile_expression(fns, env, rd, data, code, foreign.arguments[i], pm, ops);
      //code.add(asmcode::PUSH, asmcode::RAX);
      push(code, asmcode::RAX);
      }

    for (int i = (int)foreign.arguments.size() - 1; i >= 0; --i)
      {
      switch (ext.arguments[i])
        {
        case external_function::T_BOOL:
        {
#ifdef _WIN32
        auto reg = arg_reg[i];
#else
        auto reg = arg_regs[i];
#endif
        //code.add(asmcode::POP, reg);
        pop(code, reg);
        code.add(asmcode::CMP, reg, asmcode::NUMBER, bool_f);
        code.add(asmcode::SETNE, asmcode::AL);
        code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
        code.add(asmcode::MOV, reg, asmcode::RAX);
        break;
        }
        case external_function::T_INT64:
        {
#ifdef _WIN32
        auto reg = arg_reg[i];
#else
        auto reg = arg_regs[i];
#endif
        //code.add(asmcode::POP, reg);
        pop(code, reg);
        if (ops.safe_primitives)
          {
          code.add(asmcode::TEST, reg, asmcode::NUMBER, 1);
          code.add(asmcode::JNE, error);
          }
        code.add(asmcode::SHR, reg, asmcode::NUMBER, 1);
        break;
        }
        case external_function::T_SCM:
        {
#ifdef _WIN32
        auto reg = arg_reg[i];
#else
        auto reg = arg_regs[i];
#endif
        //code.add(asmcode::POP, reg);
        pop(code, reg);
        break;
        }
        case external_function::T_DOUBLE:
        {
#ifdef _WIN32
        auto reg = arg_float_reg[i];
#else
        auto reg = arg_regs[i];
#endif
        //code.add(asmcode::POP, asmcode::R11);
        pop(code, asmcode::R11);
        if (ops.safe_primitives)
          jump_if_arg_is_not_block(code, asmcode::R11, asmcode::R15, error);
        code.add(asmcode::AND, asmcode::R11, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
        if (ops.safe_primitives)
          jump_if_arg_does_not_point_to_flonum(code, asmcode::R11, asmcode::R15, error);
        code.add(asmcode::MOVSD, reg, asmcode::MEM_R11, CELLS(1));
        break;
        }
        case external_function::T_CHAR_POINTER:
        {
#ifdef _WIN32
        auto reg = arg_reg[i];
#else
        auto reg = arg_regs[i];
#endif
        //code.add(asmcode::POP, asmcode::R11);
        pop(code, asmcode::R11);
        if (ops.safe_primitives)
          jump_if_arg_is_not_block(code, asmcode::R11, asmcode::R15, error);
        code.add(asmcode::AND, asmcode::R11, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
        if (ops.safe_primitives)
          jump_if_arg_does_not_point_to_string(code, asmcode::R11, asmcode::R15, error);
        code.add(asmcode::ADD, asmcode::R11, asmcode::NUMBER, CELLS(1));
        code.add(asmcode::MOV, reg, asmcode::R11);
        break;
        }
        default: throw_error(foreign.line_nr, foreign.column_nr, foreign.filename, not_implemented);
        }
      }

    code.add(asmcode::MOV, ALLOC_SAVED, ALLOC); // this line is here so that our foreign calls can access free heap memory
    save_before_foreign_call(code);
    align_stack(code);
    code.add(asmcode::MOV, asmcode::R15, CONTEXT); // r15 should be saved by the callee but r10 not, so we save the context in r15
#ifdef _WIN32
    code.add(asmcode::SUB, asmcode::RSP, asmcode::NUMBER, 32);
#else
    code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
#endif
    code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, ext.address);
    code.add(asmcode::CALL, asmcode::R11);
    code.add(asmcode::MOV, CONTEXT, asmcode::R15); // now we restore the context
    restore_stack(code);
    restore_after_foreign_call(code);
    code.add(asmcode::MOV, ALLOC, ALLOC_SAVED); // foreign calls should have updated free heap memory if they used some
    switch (ext.return_type)
      {
      case external_function::T_BOOL:
      {
      code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
      code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
      break;
      }
      case external_function::T_INT64:
      {
      code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 1);
      break;
      }
      case external_function::T_SCM:
      {
      break;
      }
      case external_function::T_DOUBLE:
      {
      if (ops.safe_primitives && ops.safe_flonums)
        {
        code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 2);
        check_heap(code, re_flonum_heap_overflow);
        }
      uint64_t header = make_block_header(1, T_FLONUM);
      code.add(asmcode::MOV, asmcode::R11, ALLOC);
      code.add(asmcode::OR, asmcode::R11, asmcode::NUMBER, block_tag);
      code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
      code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
      code.add(asmcode::MOVQ, asmcode::RAX, asmcode::XMM0);
      code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::RAX);
      code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
      code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);
      break;
      }
      case external_function::T_CHAR_POINTER:
      {
      auto done = label_to_string(label++);
      auto repeat = label_to_string(label++);
      code.add(asmcode::PUSH, asmcode::RCX);
      code.add(asmcode::MOV, asmcode::RCX, asmcode::RAX); // char pointer in rcx
      code.add(asmcode::MOV, asmcode::R15, asmcode::RAX); // char pointer in r15
      code.add(asmcode::XOR, asmcode::R11, asmcode::R11); // r11 is counter for string length, initialize to 0
      code.add(asmcode::LABEL, repeat);
      code.add(asmcode::MOV, asmcode::AL, asmcode::BYTE_MEM_R15); // get first character in al
      code.add(asmcode::CMP, asmcode::AL, asmcode::NUMBER, 0); // is zero?
      code.add(asmcode::JES, done);
      code.add(asmcode::INC, asmcode::R15);
      code.add(asmcode::INC, asmcode::R11);
      code.add(asmcode::JMPS, repeat);
      code.add(asmcode::LABEL, done);

      if (ops.safe_primitives)
        {
        code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);
        code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 3);
        code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, 2);
        check_heap(code, re_string_heap_overflow);
        }

      code.add(asmcode::SHR, asmcode::R11, asmcode::NUMBER, 3);
      code.add(asmcode::INC, asmcode::R11);

      code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, (uint64_t)string_tag << (uint64_t)block_shift);
      code.add(asmcode::OR, asmcode::RAX, asmcode::R11); // header is now correct in rax

      code.add(asmcode::MOV, asmcode::R15, ALLOC);
      code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);

      code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX); // save header
      code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));

      auto copy_done = label_to_string(label++);
      auto copy_repeat = label_to_string(label++);

      code.add(asmcode::LABEL, copy_repeat);
      code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
      code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
      code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));
      code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 0); // is zero?
      code.add(asmcode::JES, copy_done);
      code.add(asmcode::DEC, asmcode::R11);
      code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));
      code.add(asmcode::JMPS, copy_repeat);
      code.add(asmcode::LABEL, copy_done);

      code.add(asmcode::MOV, asmcode::RAX, asmcode::R15);
      code.add(asmcode::POP, asmcode::RCX);
      break;
      }
      case external_function::T_VOID:
      {
      code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
      break;
      }
      default: throw_error(foreign.line_nr, foreign.column_nr, foreign.filename, not_implemented);
      }
    /*
    Restore the registers that were not free in reverse order
    */
    for (auto it = registers_that_were_not_free.rbegin(); it != registers_that_were_not_free.rend(); ++it)
      {
      //code.add(asmcode::POP, *it);
      pop(code, *it);
      }

    if (ops.safe_primitives)
      {
      auto skip_error = label_to_string(label++);
      code.add(asmcode::JMPS, skip_error);
      error_label(code, error, re_foreign_call_contract_violation);
      code.add(asmcode::LABEL, skip_error);
      }
    }

  void compile_let(registered_functions& fns, environment_map& env, repl_data& rd, compile_data& data, asmcode& code, const Let& let, const primitive_map& pm, const compiler_options& options)
    {
    auto new_env = std::make_shared<environment<environment_entry>>(env);
    for (int i = 0; i < let.bindings.size(); ++i)
      {
      compile_expression(fns, env, rd, data, code, let.bindings[i].second, pm, options);
      environment_entry e;
      e.live_range = let.live_ranges[i];
      if (data.ra->free_register_available())
        {
        e.st = environment_entry::st_register;
        e.pos = (uint64_t)data.ra->get_next_available_register();
        code.add(asmcode::MOV, (asmcode::operand)e.pos, asmcode::RAX);
        data.ra_map[make_reg_alloc_data(reg_alloc_data::t_register, e.pos, let.live_ranges[i])] = let.bindings[i].first;
        data.first_ra_map_time_point_to_elapse = std::min<uint64_t>(data.first_ra_map_time_point_to_elapse, let.live_ranges[i].last);
        }
      else if (data.ra->free_local_available())
        {
        e.st = environment_entry::st_local;
        e.pos = (uint64_t)data.ra->get_next_available_local();
        save_to_local(code, e.pos);
        data.ra_map[make_reg_alloc_data(reg_alloc_data::t_local, e.pos, let.live_ranges[i])] = let.bindings[i].first;
        data.first_ra_map_time_point_to_elapse = std::min<uint64_t>(data.first_ra_map_time_point_to_elapse, let.live_ranges[i].last);
        }
      else
        {
        throw std::runtime_error("no local storage available anymore");
        }

      new_env->push(let.bindings[i].first, e);
      }
    compile_expression(fns, new_env, rd, data, code, let.body.front(), pm, options);
    }

  void compile_begin(registered_functions& fns, environment_map& env, repl_data& rd, compile_data& data, asmcode& code, const Begin& beg, const primitive_map& pm, const compiler_options& options)
    {
    for (const auto& expr : beg.arguments)
      {
      compile_expression(fns, env, rd, data, code, expr, pm, options);
      }
    }

  void compile_expression(registered_functions& fns, environment_map& env, repl_data& rd, compile_data& data, asmcode& code, const Expression& expr, const primitive_map& pm, const compiler_options& options, asmcode::operand target, bool expire_registers)
    {
    assert(target_is_valid(target));
    if (std::holds_alternative<Literal>(expr))
      {
      compile_literal(fns, env, rd, data, code, std::get<Literal>(expr), options, target);
      }
    else if (std::holds_alternative<Begin>(expr))
      {
      compile_begin(fns, env, rd, data, code, std::get<Begin>(expr), pm, options);
      }
    else if (std::holds_alternative<Let>(expr))
      {
      compile_let(fns, env, rd, data, code, std::get<Let>(expr), pm, options);
      }
    else if (std::holds_alternative<PrimitiveCall>(expr))
      {
      compile_prim(fns, env, rd, data, code, std::get<PrimitiveCall>(expr), pm, options);
      }
    else if (std::holds_alternative<ForeignCall>(expr))
      {
      compile_foreign_call(fns, env, rd, data, code, std::get<ForeignCall>(expr), pm, options);
      }
    else if (std::holds_alternative<Variable>(expr))
      {
      compile_variable(fns, env, rd, data, code, std::get<Variable>(expr), options, target);
      }
    else if (std::holds_alternative<If>(expr))
      {
      compile_if(fns, env, rd, data, code, std::get<If>(expr), pm, options);
      }
    else if (std::holds_alternative<Set>(expr))
      {
      compile_set(fns, env, rd, data, code, std::get<Set>(expr), pm, options);
      }
    else if (std::holds_alternative<Lambda>(expr))
      {
      compile_lambda(fns, env, rd, data, code, std::get<Lambda>(expr), pm, options);
      }
    else if (std::holds_alternative<FunCall>(expr))
      {
      compile_funcall(fns, env, rd, data, code, std::get<FunCall>(expr), pm, options);
      }
    else if (std::holds_alternative<Nop>(expr))
      {
      compile_nop(code, target);
      }
    else
      throw_error(not_implemented);
    if (expire_registers)
      register_allocation_expire_old_intervals(get_scan_index(expr), data);
    }

  void compile_program(registered_functions& fns, environment_map& env, repl_data& rd, compile_data& data, asmcode& code, const Program& prog, const primitive_map& pm, const compiler_options& options)
    {
    for (const auto& expr : prog.expressions)
      {
      if (std::holds_alternative<Begin>(expr))
        {
        for (const auto& expr2 : std::get<Begin>(expr).arguments)
          {
          data.halt_label = label_to_string(label++);
          compile_expression(fns, env, rd, data, code, expr2, pm, options);
          code.add(asmcode::LABEL, data.halt_label);
          data.ra->make_all_available();
          data.ra_map.clear();
          data.first_ra_map_time_point_to_elapse = (uint64_t)-1;
          }
        }
      else
        {
        data.halt_label = label_to_string(label++);
        compile_expression(fns, env, rd, data, code, expr, pm, options);
        code.add(asmcode::LABEL, data.halt_label);
        }
      }
    code.add(asmcode::JMP, "L_finish");
    }
  }

void compile_cinput_parameters(cinput_data& cinput, environment_map& env, asmcode& code)
  {
#ifdef _WIN32
  for (int j = 0; j < (int)cinput.parameters.size(); ++j)
    {
    if (cinput.parameters[j].second == cinput_data::cin_int)
      {
      std::string name = cinput.parameters[j].first;

      environment_entry e;
      if (!env->find(e, name) || e.st != environment_entry::st_global)
        throw_error(invalid_c_input_syntax);
      code.add(asmcode::MOV, asmcode::RAX, GLOBALS);
      if (j == 0)
        {
        code.add(asmcode::SHL, asmcode::RDX, asmcode::NUMBER, 1);
        code.add(asmcode::MOV, asmcode::MEM_RAX, e.pos, asmcode::RDX);
        }
      else if (j == 1)
        {
        code.add(asmcode::SHL, asmcode::R8, asmcode::NUMBER, 1);
        code.add(asmcode::MOV, asmcode::MEM_RAX, e.pos, asmcode::R8);
        }
      else if (j == 2)
        {
        code.add(asmcode::SHL, asmcode::R9, asmcode::NUMBER, 1);
        code.add(asmcode::MOV, asmcode::MEM_RAX, e.pos, asmcode::R9);
        }
      else
        {
        int addr = j - 3;
        code.add(asmcode::MOV, asmcode::RCX, asmcode::MEM_RSP, (40 + addr * 8));
        code.add(asmcode::SHL, asmcode::RCX, asmcode::NUMBER, 1);
        code.add(asmcode::MOV, asmcode::MEM_RAX, e.pos, asmcode::RCX);
        }
      }
    else if (cinput.parameters[j].second == cinput_data::cin_double)
      {
      std::string name = cinput.parameters[j].first;

      environment_entry e;
      if (!env->find(e, name) || e.st != environment_entry::st_global)
        throw_error(invalid_c_input_syntax);

      uint64_t header = make_block_header(1, T_FLONUM);
      code.add(asmcode::MOV, asmcode::R15, ALLOC);
      code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);
      code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
      code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);

      if (j == 0)
        code.add(asmcode::MOVQ, MEM_ALLOC, CELLS(1), asmcode::XMM1);
      else if (j == 1)
        code.add(asmcode::MOVQ, MEM_ALLOC, CELLS(1), asmcode::XMM2);
      else if (j == 2)
        code.add(asmcode::MOVQ, MEM_ALLOC, CELLS(1), asmcode::XMM3);
      else
        {
        int addr = j - 3;
        code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RSP, (40 + addr * 8));
        code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::RAX);
        }

      code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
      code.add(asmcode::MOV, asmcode::RAX, GLOBALS);
      code.add(asmcode::MOV, asmcode::MEM_RAX, e.pos, asmcode::R15);
      }
    }
#else 
  for (int j = 0; j < (int)cinput.parameters.size(); ++j)
    {
    if (cinput.parameters[j].second == cinput_data::cin_int)
      {
      std::string name = cinput.parameters[j].first;

      environment_entry e;
      if (!env->find(e, name) || e.st != environment_entry::st_global)
        throw_error(invalid_c_input_syntax);
      code.add(asmcode::MOV, asmcode::RAX, GLOBALS);
      if (j == 0)
        {
        code.add(asmcode::SHL, asmcode::RSI, asmcode::NUMBER, 1);
        code.add(asmcode::MOV, asmcode::MEM_RAX, e.pos, asmcode::RSI);
        }
      else if (j == 1)
        {
        code.add(asmcode::SHL, asmcode::RDX, asmcode::NUMBER, 1);
        code.add(asmcode::MOV, asmcode::MEM_RAX, e.pos, asmcode::RDX);
        }
      else if (j == 2)
        {
        code.add(asmcode::SHL, asmcode::RCX, asmcode::NUMBER, 1);
        code.add(asmcode::MOV, asmcode::MEM_RAX, e.pos, asmcode::RCX);
        }
      else if (j == 3)
        {
        code.add(asmcode::SHL, asmcode::R8, asmcode::NUMBER, 1); 
        code.add(asmcode::MOV, asmcode::MEM_RAX, e.pos, asmcode::R8);
        }
      else if (j == 4)
        {
        code.add(asmcode::SHL, asmcode::R9, asmcode::NUMBER, 1);
        code.add(asmcode::MOV, asmcode::MEM_RAX, e.pos, asmcode::R9);
        }
      else
        {
        int addr = j-4;
        code.add(asmcode::MOV, asmcode::RCX, asmcode::MEM_RSP, 8*addr);
        code.add(asmcode::SHL, asmcode::RCX, asmcode::NUMBER, 1);
        code.add(asmcode::MOV, asmcode::MEM_RAX, e.pos, asmcode::RCX);
        }
      }
    else if (cinput.parameters[j].second == cinput_data::cin_double)
      {
      std::string name = cinput.parameters[j].first;

      environment_entry e;
      if (!env->find(e, name) || e.st != environment_entry::st_global)
        throw_error(invalid_c_input_syntax);

      uint64_t header = make_block_header(1, T_FLONUM);
      code.add(asmcode::MOV, asmcode::R15, ALLOC);
      code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);
      code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
      code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);

      if (j == 0)
        code.add(asmcode::MOVQ, MEM_ALLOC, CELLS(1), asmcode::XMM0);
      else if (j == 1)
        code.add(asmcode::MOVQ, MEM_ALLOC, CELLS(1), asmcode::XMM1);
      else if (j == 2)
        code.add(asmcode::MOVQ, MEM_ALLOC, CELLS(1), asmcode::XMM2);
      else if (j == 3)
        code.add(asmcode::MOVQ, MEM_ALLOC, CELLS(1), asmcode::XMM3);
      else if (j == 4)
        code.add(asmcode::MOVQ, MEM_ALLOC, CELLS(1), asmcode::XMM4);
      else if (j == 5)
        code.add(asmcode::MOVQ, MEM_ALLOC, CELLS(1), asmcode::XMM5);
      else if (j == 6)
        code.add(asmcode::MOVQ, MEM_ALLOC, CELLS(1), asmcode::XMM6);
      else if (j == 7)
        code.add(asmcode::MOVQ, MEM_ALLOC, CELLS(1), asmcode::XMM7);
      else
        {
        int addr = j - 7;
        code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RSP, 8 * addr);
        code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::RAX);
        }

      code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
      code.add(asmcode::MOV, asmcode::RAX, GLOBALS);
      code.add(asmcode::MOV, asmcode::MEM_RAX, e.pos, asmcode::R15);
      }
    }
#endif
  }

void compile(environment_map& env, repl_data& rd, macro_data& md, context& ctxt, asmcode& code, Program& prog, const primitive_map& pm, const std::map<std::string, external_function>& external_functions, const compiler_options& options)
  {
  static function_map prims = generate_function_map();
  static function_map inlined_prims = generate_inlined_primitives();
  registered_functions fns;
  fns.primitives = &prims;
  fns.inlined_primitives = &inlined_prims;
  fns.externals = &external_functions;

  cinput_data cinput;

  preprocess(env, rd, md, ctxt, cinput, prog, pm, options);

  label = 0;

  compile_data data = create_compile_data(ctxt.total_heap_size, ctxt.globals_end - ctxt.globals, (uint32_t)ctxt.number_of_locals, &ctxt);

  data.ra->make_all_available();
  data.ra_map.clear();
  data.first_ra_map_time_point_to_elapse = (uint64_t)-1;



  if (rd.global_index > data.globals_stack) // too many globals declared
    throw_error(too_many_globals);

  code.add(asmcode::GLOBAL, "scheme_entry");

#ifdef _WIN32
  /*
  windows parameters calling convention: rcx, rdx, r8, r9
  First parameter (rcx) points to the context.
  We store the pointer to the context in register r10.
  */
  code.add(asmcode::MOV, CONTEXT, asmcode::RCX);
  
#else
  /*
  Linux parameters calling convention: rdi, rsi, rdx, rcx, r8, r9
  First parameter (rdi) points to the context.
  We store the pointer to the context in register r10.
  */
  code.add(asmcode::MOV, CONTEXT, asmcode::RDI);
#endif  

  /*
  Save the current content of the registers in the context
  */
  store_registers(code);

  code.add(asmcode::MOV, asmcode::R11, asmcode::LABELADDRESS, "L_error");
  code.add(asmcode::MOV, ERROR, asmcode::R11); // store the error label in the context

  code.add(asmcode::MOV, STACK_REGISTER, STACK); // get the stack location from the context and put it in the dedicated register
  code.add(asmcode::MOV, STACK_SAVE, STACK_REGISTER);  // save the current stack position. At the end of this method we'll restore STACK to STACK_SAVE, as scheme
                                                       // does not pop every stack position that was pushed due to continuation passing style.

  code.add(asmcode::MOV, ALLOC, ALLOC_SAVED);

  compile_cinput_parameters(cinput, env, code);

  /*
  Align stack with 16 byte boundary
  */
  code.add(asmcode::AND, asmcode::RSP, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF0);

  /*
  Make registers point to unalloc_tag, so that gc cannot crash due to old register content
  */
  code.add(asmcode::MOV, asmcode::RCX, asmcode::NUMBER, unalloc_tag);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::NUMBER, unalloc_tag);
  code.add(asmcode::MOV, asmcode::RSI, asmcode::NUMBER, unalloc_tag);
  code.add(asmcode::MOV, asmcode::RDI, asmcode::NUMBER, unalloc_tag);
  code.add(asmcode::MOV, asmcode::R8, asmcode::NUMBER, unalloc_tag);
  code.add(asmcode::MOV, asmcode::R9, asmcode::NUMBER, unalloc_tag);
  code.add(asmcode::MOV, asmcode::R12, asmcode::NUMBER, unalloc_tag);
  code.add(asmcode::MOV, asmcode::R14, asmcode::NUMBER, unalloc_tag);

  /*Call the main procedure*/
  code.add(asmcode::JMP, "L_scheme_entry");

  code.add(asmcode::LABEL, "L_error");

  code.add(asmcode::LABEL, "L_finish");

  code.add(asmcode::MOV, ALLOC_SAVED, ALLOC);

  code.add(asmcode::MOV, STACK_REGISTER, STACK_SAVE); // restore the scheme stack to its saved position
  code.add(asmcode::MOV, STACK, STACK_REGISTER);

  /*Restore the registers to their original state*/
  load_registers(code);

  /*Return to the caller*/
  code.add(asmcode::RET);

  /*Compile the main procedure*/
  code.push();
  code.add(asmcode::GLOBAL, "L_scheme_entry");
  compile_program(fns, env, rd, data, code, prog, pm, options);
  if (options.do_cps_conversion)
    {
    code.add(asmcode::COMMENT, "error: invalid program termination");
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, (uint64_t)re_invalid_program_termination);
    code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 8);
    code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, error_tag);
    code.add(asmcode::JMP, "L_error");
    }
  else
    code.add(asmcode::JMP, "L_finish");
  code.pop();
  }

SKIWI_END
