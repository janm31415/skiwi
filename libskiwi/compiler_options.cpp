#include "compiler_options.h"

SKIWI_BEGIN

compiler_options::compiler_options()
  {
  do_alpha_conversion = true;
  do_assignable_variables_conversion = true;
  do_define_conversion = true;
  do_closure_conversion = true;
  do_cps_conversion = true;  
  do_free_variables_analysis = true;
  do_linear_scan = true;
  do_linear_scan_indices_computation = true;
  do_simplify_to_core_forms = true;
  do_single_begin_conversion = true;
  do_tail_call_analysis = true;
  do_global_define_env_allocation = true;
  do_collect_quotes = true;
  do_quote_conversion = true;
  do_quasiquote_conversion = true;
  lsa_algo = lsa_detailed;
  primitives_inlined = true;
  safe_primitives = true;
  safe_cons = true;
  safe_flonums = true;
  safe_promises = true;
  garbage_collection = true;
  do_handle_include = true;
  do_lambda_to_let_conversion = true;
  do_constant_folding = true;
  do_constant_propagation = true;
  do_expand_macros = true;
  standard_bindings = false;

  fast_expression_targetting = true;
  parallel = true;
  keep_variable_stack = true;
  }

SKIWI_END
