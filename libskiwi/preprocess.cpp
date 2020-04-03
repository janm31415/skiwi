#include "preprocess.h"
#include "alpha_conversion.h"
#include "assignable_var_conversion.h"
#include "cps_conversion.h"
#include "closure_conversion.h"
#include "constant_folding.h"
#include "constant_propagation.h"
#include "define_conversion.h"
#include "free_var_analysis.h"
#include "global_define_env.h"
#include "inline_primitives_conversion.h"
#include "linear_scan.h"
#include "linear_scan_index.h"
#include "lambda_to_let_conversion.h"
#include "include_handler.h"
#include "macro_expander.h"
#include "quasiquote_conversion.h"
#include "quote_collector.h"
#include "quote_conversion.h"
#include "simplify_to_core.h"
#include "single_begin_conversion.h"
#include "tail_call_analysis.h"
#include "tail_calls_check.h"


#include <ctime>

namespace
  {

  class timer
    {
    public:
      timer() : m_start(std::clock())
        {
        }

      ~timer()
        {
        }

      double get_time_elapsed_in_sec() const
        {
        std::clock_t time_elapsed = std::clock() - m_start;
        return static_cast<double>(time_elapsed) / static_cast<double>(CLOCKS_PER_SEC);
        }

      void start_timer()
        {
        m_start = std::clock();
        }

    private:
      std::clock_t m_start;
    };

  }

SKIWI_BEGIN

namespace
  {
  timer g_timer;

  void tic()
    {
    g_timer.start_timer();
    }

  void debug_string(const char* txt)
    { 
    txt;
    //printf(txt); printf("\n");
    }

  void toc()
    {
    auto t = g_timer.get_time_elapsed_in_sec();
    t;
    //printf("Time spent: %fs\n", t);
    }
  }

void preprocess(environment_map& env, repl_data& data, macro_data& md, context& ctxt, Program& prog, const primitive_map& pm, const compiler_options& options)
  {
  tic();
  debug_string("start handle_include_command");
  if (options.do_handle_include)
    handle_include_command(prog);
  debug_string("done handle_include_command");
  toc();
  tic();
  debug_string("start macro_expander");
  if (options.do_expand_macros)
    expand_macros(prog, env, data, md, ctxt, pm, options);
  debug_string("done macro_expander");
  toc();
  tic();
  debug_string("start quasiquote_conversion");
  if (options.do_quasiquote_conversion)
    quasiquote_conversion(prog);
  debug_string("done quasiquote_conversion");
  toc();
  tic();
  debug_string("start define_conversion");
  if (options.do_define_conversion)
    define_conversion(prog);
  debug_string("done define_conversion");
  toc();
  tic();
  debug_string("start single_begin_conversion");
  if (options.do_single_begin_conversion)
    single_begin_conversion(prog);
  debug_string("done single_begin_conversion");
  toc();
  tic();
  debug_string("start simplify_to_core_forms");
  if (options.do_simplify_to_core_forms)
    simplify_to_core_forms(prog);
  debug_string("done simplify_to_core_forms");
  toc();
  tic();
  debug_string("start alpha_conversion");
  alpha_conversion(prog, data.alpha_conversion_index, data.alpha_conversion_env, options.do_alpha_conversion);
  debug_string("done alpha_conversion");
  toc();
  tic();
  debug_string("start collect_quotes");
  if (options.do_collect_quotes)
    collect_quotes(prog, data);
  debug_string("done collect_quotes");
  toc();
  tic();
  debug_string("start quote_conversion");
  if (options.do_quote_conversion)
    quote_conversion(prog, data, env, ctxt);
  debug_string("done quote_conversion");
  toc();
  tic();
  debug_string("start global_define_environment_allocation");
  if (options.do_global_define_env_allocation)
    global_define_environment_allocation(prog, env, data, ctxt);
  debug_string("done global_define_environment_allocation");
  toc();
  tic();
  debug_string("start cps_conversion");
  if (options.do_cps_conversion)
    cps_conversion(prog, options);
  debug_string("done cps_conversion");
  toc();
  tic();
  debug_string("start lambda_to_let_conversion");
  if (options.do_lambda_to_let_conversion)
    lambda_to_let_conversion(prog);
  debug_string("done lambda_to_let_conversion");
  toc();
  tic();
  debug_string("start assignable_variable_conversion");
  if (options.do_assignable_variables_conversion)
    assignable_variable_conversion(prog, options);
  debug_string("done assignable_variable_conversion");
  toc();
  tic();
  // this run of constant propagation can remove the need for closures
  debug_string("start constant_propagation");
  if (options.do_constant_propagation)
    constant_propagation(prog);
  debug_string("done constant_propagation");
  toc();
  tic();
  debug_string("start free_variable_analysis");
  if (options.do_free_variables_analysis)
    free_variable_analysis(prog, env);  
  debug_string("done free_variable_analysis");
  toc();
  tic();
  debug_string("start closure_conversion");
  if (options.do_closure_conversion)
    closure_conversion(prog, options);
  debug_string("done closure_conversion");
  toc();
  tic();
  debug_string("start inline_primitives_conversion");
  if (options.primitives_inlined)
    inline_primitives(prog, data.alpha_conversion_index, options.safe_primitives, options.standard_bindings);
  debug_string("done inline_primitives_conversion");
  toc();
  tic();
  // this 2nd run of constant propagation is necessary, because the inlinging step might have introduced new opportunities for simplification
  debug_string("start 2nd run constant_propagation");
  if (options.do_constant_propagation)
    constant_propagation(prog);
  debug_string("done 2nd run constant_propagation");
  toc();
  tic();
  debug_string("start constant_folding");
  if (options.do_constant_folding)
    constant_folding(prog);
  debug_string("done  constant_folding");
  toc();
  tic(); 
  debug_string("start tail_call_analysis");
  if (options.do_tail_call_analysis)
    {
    tail_call_analysis(prog);
    if (!only_tail_calls(prog))
      throw std::runtime_error("preprocess: something went wrong: continuation passing style conversion failed");
    }
  debug_string("done tail_call_analysis");
  toc();
  tic();
  debug_string("start do_linear_scan_indices_computation");
  if (options.do_linear_scan_indices_computation)
    compute_linear_scan_index(prog);
  debug_string("done do_linear_scan_indices_computation");
  toc();
  tic();
  debug_string("start do_linear_scan");
  if (options.do_linear_scan)
    linear_scan(prog, options.lsa_algo, options);  
  debug_string("done linear_scan");
  toc();
  }

SKIWI_END
