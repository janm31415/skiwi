#pragma once

#include "namespace.h"
#include <stdint.h>
#include "libskiwi_api.h"
#include "linear_scan.h"

SKIWI_BEGIN

struct compiler_options
  {
  SKIWI_SCHEME_API compiler_options();

  bool do_handle_include;
  bool do_alpha_conversion;
  bool do_assignable_variables_conversion;
  bool do_lambda_to_let_conversion;
  bool do_define_conversion;
  bool do_closure_conversion;
  bool do_cinput_conversion;
  bool do_cps_conversion;
  bool do_free_variables_analysis;
  bool do_linear_scan;
  bool do_linear_scan_indices_computation;
  bool do_simplify_to_core_forms;
  bool do_single_begin_conversion;
  bool do_tail_call_analysis;
  bool do_global_define_env_allocation;
  bool do_collect_quotes;
  bool do_quote_conversion;
  bool do_quasiquote_conversion;
  linear_scan_algorithm lsa_algo;
  bool primitives_inlined;
  bool do_constant_folding;
  bool do_constant_propagation;
  bool do_expand_macros;
  bool standard_bindings; // if true, user guarantees that primitives are not redefined. This allows faster code when inlining.
  bool safe_primitives;  
  // cons and flonums create small entries on the heap. With good functioning garbage collection, testing for heap overflow should not be necessary
  bool safe_cons;
  bool safe_flonums;
  bool safe_promises;
  bool garbage_collection;
  bool fast_expression_targetting;
  bool parallel;
  bool keep_variable_stack; // default true: adds last used globals to a debug stack for better error reporting
  };

SKIWI_END
