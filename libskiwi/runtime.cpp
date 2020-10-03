#include "runtime.h"
#include "types.h"
#include "context.h"
#include "compiler.h"

#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

SKIWI_BEGIN

namespace
  {

  std::string uchar_to_hex(unsigned char i)
    {
    std::string hex;
    int h1 = (i >> 4) & 0x0f;
    if (h1 < 10)
      hex += '0' + char(h1);
    else
      hex += 'A' + char(h1) - 10;
    int h2 = (i) & 0x0f;
    if (h2 < 10)
      hex += '0' + char(h2);
    else
      hex += 'A' + char(h2) - 10;
    return hex;
    }

  std::string uint64_to_hex(uint64_t i)
    {
    unsigned char* ptr = (unsigned char*)&i;
    std::string out = "0x";
    for (int j = 7; j >= 0; --j)
      out += uchar_to_hex(ptr[j]);
    return out;
    }

  bool is_pair(uint64_t rax)
    {
    if ((rax & block_mask) == block_tag)
      {
      uint64_t* addr = get_address_from_block(rax);
      uint64_t header = *addr;
      return block_header_is_pair(header);
      }
    return false;
    }

  struct task
    {
    task(const char* text) : txt(text), rax((uint64_t)-1), second_item_of_pair(0) {}
    task(const std::string& text) : txt(text), rax((uint64_t)-1), second_item_of_pair(0) {}
    task(uint64_t rx) : rax(rx), second_item_of_pair(0) {}
    task(uint64_t rx, int si) : rax(rx), second_item_of_pair(si) {}
    std::string txt;
    uint64_t rax;
    int second_item_of_pair;
    };

  void print_ptr(uint64_t rax2, std::ostream& out, std::shared_ptr<SKIWI::environment<SKIWI::environment_entry>> env, const repl_data& rd, const context* p_ctxt)
    {
    std::vector<std::string> texts;
    std::vector<task> todo;
    todo.emplace_back(rax2);

    while (!todo.empty())
      {
      uint64_t rax = todo.back().rax;
      int second_item_of_pair = todo.back().second_item_of_pair;
      std::string txt = todo.back().txt;
      todo.pop_back();
      std::stringstream str;
      str << std::setprecision(out.precision());
      if (!txt.empty())
        {
        texts.push_back(txt);
        }
      else if ((rax & fixnum_mask) == fixnum_tag)
        {
        int64_t value = (int64_t)(((int64_t)rax) >> (int64_t)fixnum_shift);
        str << value;
        texts.push_back(str.str());
        }
      else if ((rax & block_mask) == block_tag)
        {
        uint64_t* addr = get_address_from_block(rax);
        uint64_t header = *addr;
        if (block_header_is_flonum(header))
          {
          assert(get_block_size(header) == 1);
          double* d = reinterpret_cast<double*>((uint64_t*)(addr + 1));
          str << std::defaultfloat << *d;
          texts.push_back(str.str());
          }
        else if (block_header_is_closure(header))
          {
          str << "<lambda>";
          texts.push_back(str.str());
          }
        else if (block_header_is_pair(header))
          {
          //In general, the rule for printing a pair is as follows : use the dot notation always,
          //but if the dot is immediately followed by an open parenthesis, then remove the dot, the
          //open parenthesis, and the matching close parenthesis.Thus, (0 . (1 . 2)) becomes
          //(0 1 . 2), and (1 . (2 . (3 . ()))) becomes (1 2 3).
          assert(get_block_size(header) == 2);
          uint64_t first = *((uint64_t*)(addr + 1));
          uint64_t second = *((uint64_t*)(addr + 2));
          if (!second_item_of_pair)
            {
            str << "(";
            texts.push_back(str.str());
            }
          size_t idx0 = todo.size();
          todo.emplace_back(first, 0);
          if (second == nil)
            {
            }
          else
            {
            if (!is_pair(second))
              todo.emplace_back(" . ");
            else
              todo.emplace_back(" ");
            todo.emplace_back(second, 1);
            }
          if (!second_item_of_pair)
            todo.emplace_back(")");
          std::reverse(todo.begin() + idx0, todo.end());
          }
        else if (block_header_is_vector(header))
          {
          str << "#(";
          texts.push_back(str.str());
          size_t idx0 = todo.size();
          for (int i = 0; i < get_block_size(header); ++i)
            {
            if (i)
              todo.emplace_back(" ");
            uint64_t item = *((uint64_t*)(addr + i + 1));
            todo.emplace_back(item, 0);
            }
          todo.emplace_back(")");
          std::reverse(todo.begin() + idx0, todo.end());
          }
        else if (block_header_is_string(header))
          {
          str << "\"";
          char* ch = (char*)(addr + 1);
          str << ch;
          str << "\"";
          texts.push_back(str.str());
          }
        else if (block_header_is_symbol(header))
          {
          char* ch = (char*)(addr + 1);
          str << ch;
          texts.push_back(str.str());
          }
        else if (block_header_is_port(header))
          {
          uint64_t name_address = *((uint64_t*)(addr + 1 + 1));
          str << "<port>: ";
          texts.push_back(str.str());
          todo.emplace_back(name_address, second_item_of_pair);
          }
        else if (block_header_is_promise(header))
          {
          str << "<promise>";
          texts.push_back(str.str());
          }
        else
          {
          str << "<unknown " << uint64_to_hex(rax) << ">";
          texts.push_back(str.str());
          }
        }
      else if (rax == bool_t)
        {
        str << "#t";
        texts.push_back(str.str());
        }
      else if (rax == bool_f)
        {
        str << "#f";
        texts.push_back(str.str());
        }
      else if (rax == nil)
        {
        str << "()";
        texts.push_back(str.str());
        }
      else if ((rax & char_mask) == char_tag)
        {
        unsigned char ch = (unsigned char)(rax >> 8);
        if (ch > 31 && ch < 127)
          str << "#\\" << ch;
        else
          str << "#\\" << std::fixed << std::setfill('0') << std::setw(3) << int(ch) << std::defaultfloat;
        texts.push_back(str.str());
        }
      else if (rax == skiwi_undefined)
        {
        str << "#undefined";
        texts.push_back(str.str());
        }
      else if (rax == skiwi_quiet_undefined)
        {
        }
      else if (rax == eof_tag)
        {
        str << "#eof";
        texts.push_back(str.str());
        }
      else if ((rax & procedure_mask) == procedure_tag)
        {
        str << "<procedure>";
        texts.push_back(str.str());
        }
      else if (rax == unresolved_tag)
        {
        str << "unknown variable";
        if (p_ctxt)
          {
          str << ": ";
          print_last_global_variable_used(str, env, rd, p_ctxt);
          }
        texts.push_back(str.str());
        }
      else if (rax == unalloc_tag)
        {
        str << "compiler error, to check: ";
        if (p_ctxt)
          {
          str << ": ";
          print_last_global_variable_used(str, env, rd, p_ctxt);
          }
        texts.push_back(str.str());
        }
      else if ((rax & error_mask) == error_tag)
        {
        uint64_t error_id = (rax >> 8) & ((1 << 8) - 1);
        //uint64_t original_rax_lowest_48 = (rax >> 16);
        runtime_error re = (runtime_error)error_id;
        if (re == re_silent)
          return;
        str << "runtime error: ";
        switch (re)
          {
          case re_lambda_invalid_number_of_arguments: str << "<lambda>: invalid number of arguments"; break;
          case re_add_contract_violation: str << "+: contract violation"; break;
          case re_add1_contract_violation: str << "add1: contract violation"; break;
          case re_div_contract_violation: str << "/: contract violation"; break;
          case re_mul_contract_violation: str << "*: contract violation"; break;
          case re_equal_contract_violation: str << "=: contract violation"; break;
          case re_not_equal_contract_violation: str << "!=: contract violation"; break;
          case re_less_contract_violation: str << "<: contract violation"; break;
          case re_leq_contract_violation: str << "<=: contract violation"; break;
          case re_greater_contract_violation: str << ">: contract violation"; break;
          case re_geq_contract_violation: str << ">=: contract violation"; break;
          case re_sub_contract_violation: str << "-: contract violation"; break;
          case re_sub1_contract_violation: str << "sub1: contract violation"; break;
          case re_invalid_program_termination: str << "invalid program termination"; break;
          case re_closure_expected: str << "closure expected"; break;
          case re_vector_ref_contract_violation: str << "vector-ref: contract violation"; break;
          case re_vector_ref_out_of_bounds: str << "vector-ref: out of bounds"; break;
          case re_vector_set_contract_violation: str << "vector-set!: contract violation"; break;
          case re_vector_set_out_of_bounds: str << "vector-set!: out of bounds"; break;
          case re_closure_ref_contract_violation: str << "closure-ref: contract violation"; break;
          case re_closure_ref_out_of_bounds: str << "closure-ref: out of bounds"; break;
          case re_is_zero_contract_violation: str << "zero?: contract violation"; break;
          case re_cons_contract_violation: str << "cons: contract violation"; break;
          case re_car_contract_violation: str << "car: contract violation"; break;
          case re_cdr_contract_violation: str << "cdr: contract violation"; break;
          case re_vector_contract_violation: str << "vector: contract violation"; break;
          case re_make_vector_contract_violation: str << "make-vector: contract violation"; break;
          case re_vector_length_contract_violation: str << "vector-length: contract violation"; break;
          case re_string_contract_violation: str << "vector-length: contract violation"; break;
          case re_make_string_contract_violation: str << "make-string: contract violation"; break;
          case re_string_length_contract_violation: str << "string-length: contract violation"; break;
          case re_string_ref_contract_violation: str << "string-ref: contract violation"; break;
          case re_string_ref_out_of_bounds: str << "string-ref: out of bounds"; break;
          case re_string_set_contract_violation: str << "string-set!: contract violation"; break;
          case re_string_set_out_of_bounds: str << "string-set!: out of bounds"; break;
          case re_eq_contract_violation: str << "eq?: contract violation"; break;
          case re_eqv_contract_violation: str << "eqv?: contract violation"; break;
          case re_eqvstruct_contract_violation: str << "%eqv?: contract violation"; break;
          case re_isequal_contract_violation: str << "equal?: contract violation"; break;
          case re_length_contract_violation: str << "length: contract violation"; break;
          case re_set_car_contract_violation: str << "set-car!: contract violation"; break;
          case re_set_cdr_contract_violation: str << "set-cdr!: contract violation"; break;
          case re_fixnum_to_char_contract_violation: str << "fixnum->char: contract violation"; break;
          case re_char_to_fixnum_contract_violation: str << "char->fixnum: contract violation"; break;
          case re_char_equal_contract_violation: str << "char=?: contract violation"; break;
          case re_char_less_contract_violation: str << "char<?: contract violation"; break;
          case re_char_greater_contract_violation: str << "char>?: contract violation"; break;
          case re_char_leq_contract_violation: str << "char<=?: contract violation"; break;
          case re_char_geq_contract_violation: str << "char>=?: contract violation"; break;
          case re_fx_equal_contract_violation: str << "fx=?: contract violation"; break;
          case re_fx_less_contract_violation: str << "fx<?: contract violation"; break;
          case re_fx_greater_contract_violation: str << "fx>?: contract violation"; break;
          case re_fx_leq_contract_violation: str << "fx<=?: contract violation"; break;
          case re_fx_geq_contract_violation: str << "fx>=?: contract violation"; break;
          case re_fx_add_contract_violation: str << "fx+: contract violation"; break;
          case re_fx_sub_contract_violation: str << "fx-: contract violation"; break;
          case re_fx_mul_contract_violation: str << "fx*: contract violation"; break;
          case re_fx_div_contract_violation: str << "fx/: contract violation"; break;
          case re_fx_add1_contract_violation: str << "fxadd1: contract violation"; break;
          case re_fx_sub1_contract_violation: str << "fxsub1: contract violation"; break;
          case re_fx_is_zero_contract_violation: str << "fxzero?: contract violation"; break;
          case re_bitwise_and_contract_violation:str << "bitwise-and: contract violation"; break;
          case re_bitwise_not_contract_violation:str << "bitwise-not: contract violation"; break;
          case re_bitwise_or_contract_violation:str << "bitwise-or: contract violation"; break;
          case re_bitwise_xor_contract_violation:str << "bitwise-xor: contract violation"; break;
          case re_make_vector_heap_overflow: str << "make-vector: heap overflow"; break;
          case re_closure_heap_overflow: str << "closure: heap overflow"; break;
          case re_vector_heap_overflow: str << "vector: heap overflow"; break;
          case re_make_string_heap_overflow: str << "make-string: heap overflow"; break;
          case re_string_heap_overflow: str << "string: heap overflow"; break;
          case re_list_heap_overflow: str << "list: heap overflow"; break;
          case re_cons_heap_overflow: str << "cons: heap overflow"; break;
          case re_symbol_heap_overflow: str << "symbol: heap overflow"; break;
          case re_flonum_heap_overflow: str << "flonum: heap overflow"; break;
          case re_foreign_call_contract_violation: str << "foreign-call: contract violation"; break;
          case re_memv_contract_violation: str << "memv: contract violation"; break;
          case re_memq_contract_violation: str << "memq: contract violation"; break;
          case re_member_contract_violation: str << "member: contract violation"; break;
          case re_assv_contract_violation: str << "assv: contract violation"; break;
          case re_assq_contract_violation: str << "assq: contract violation"; break;
          case re_assoc_contract_violation: str << "assoc: contract violation"; break;
          case re_apply_contract_violation: str << "apply: contract violation"; break;
          case re_make_port_contract_violation: str << "make-port: contract violation"; break;
          case re_make_port_heap_overflow: str << "make-port: heap overflow"; break;
          case re_write_char_contract_violation: str << "write-char: contract violation"; break;
          case re_flush_output_port_contract_violation: str << "flush-output-port: contract violation"; break;
          case re_ieee754_sign_contract_violation: str << "ieee754-sign: contract violation"; break;
          case re_ieee754_exponent_contract_violation: str << "ieee754-exponent: contract violation"; break;
          case re_ieee754_mantissa_contract_violation: str << "ieee754-mantissa: contract violation"; break;
          case re_max_contract_violation: str << "max: contract violation"; break;
          case re_min_contract_violation: str << "min: contract violation"; break;
          case re_arithmetic_shift_contract_violation: str << "arithmetic-shift: contract violation"; break;
          case re_quotient_contract_violation: str << "quotient: contract violation"; break;
          case re_remainder_contract_violation: str << "remainder: contract violation"; break;
          case re_fixnum_to_flonum_contract_violation: str << "fixnum->flonum: contract violation"; break;
          case re_flonum_to_fixnum_contract_violation: str << "flonum->fixnum: contract violation"; break;
          case re_fixnum_to_flonum_heap_overflow: str << "fixnum->flonum: heap overflow"; break;
          case re_ieee754_sin_contract_violation: str << "ieee754-sin: contract violation"; break;
          case re_ieee754_cos_contract_violation: str << "ieee754-cos: contract violation"; break;
          case re_ieee754_tan_contract_violation: str << "ieee754-tan: contract violation"; break;
          case re_ieee754_asin_contract_violation: str << "ieee754-asin: contract violation"; break;
          case re_ieee754_acos_contract_violation: str << "ieee754-acos: contract violation"; break;
          case re_ieee754_atan1_contract_violation: str << "ieee754-atan1: contract violation"; break;
          case re_ieee754_log_contract_violation: str << "ieee754-log: contract violation"; break;
          case re_ieee754_round_contract_violation: str << "ieee754-round: contract violation"; break;
          case re_ieee754_truncate_contract_violation: str << "ieee754-truncate: contract violation"; break;
          case re_ieee754_sqrt_contract_violation: str << "ieee754-sqrt: contract violation"; break;
          case re_ieee754_pi_contract_violation: str << "ieee754-pi contract violation"; break;
          case re_fixnum_expt_contract_violation: str << "fixnum-expt: contract violation"; break;
          case re_flonum_expt_contract_violation: str << "flonum-expt: contract violation"; break;
          case re_is_port_contract_violation: str << "port?: contract violation"; break;
          case re_is_input_port_contract_violation: str << "input-port?: contract violation"; break;
          case re_is_output_port_contract_violation: str << "output-port?: contract violation"; break;
          case re_open_file_contract_violation: str << "open-file: contract violation"; break;
          case re_close_file_contract_violation: str << "close-file: contract violation"; break;
          case re_port_ref_contract_violation: str << "port-ref: contract violation"; break;
          case re_port_ref_out_of_bounds: str << "port-ref: out of bounds"; break;
          case re_read_char_contract_violation: str << "read-char: contract violation"; break;
          case re_peek_char_contract_violation: str << "peek-char: contract violation"; break;
          case re_num2str_contract_violation: str << "num2str: contract violation"; break;
          case re_str2num_contract_violation: str << "str2num: contract violation"; break;
          case re_write_string_contract_violation: str << "write-string: contract violation"; break;
          case re_string_copy_contract_violation: str << "string-copy: contract violation"; break;
          case re_symbol_to_string_contract_violation: str << "symbol->string: contract violation"; break;
          case re_string_copy_heap_overflow: str << "string-copy: heap overflow"; break;
          case re_symbol_to_string_heap_overflow: str << "symbol->string: heap overflow"; break;
          case re_compare_strings_contract_violation: str << "compare-strings: contract violation"; break;
          case re_compare_strings_ci_contract_violation: str << "compare-strings-ci: contract violation"; break;
          case re_vector_fill_contract_violation: str << "vector-fill!: contract violation"; break;
          case re_string_fill_contract_violation: str << "string-fill!: contract violation"; break;
          case re_substring_contract_violation: str << "substring: contract violation"; break;
          case re_substring_heap_overflow: str << "substring: heap overflow"; break;
          case re_substring_out_of_bounds: str << "substring: out of bounds"; break;
          case re_string_append_contract_violation: str << "string-append: contract violation"; break;
          case re_string_append_heap_overflow: str << "string-append: heap overflow"; break;
          case re_string_hash_contract_violation: str << "string-hash: contract violation"; break;
          case re_allocate_symbol_contract_violation: str << "%allocate-symbol: contract violation"; break;
          case re_allocate_symbol_heap_overflow: str << "%allocate-symbol: heap overflow"; break;
          case re_make_promise_contract_violation: str << "make-promise: contract violation"; break;
          case re_make_promise_heap_overflow: str << "make-promise: heap overflow"; break;
          case re_slot_ref_out_of_bounds:str << "%slot-ref: out of bounds"; break;
          case re_slot_ref_contract_violation: str << "%slot-ref: contract violation"; break;
          case re_slot_set_out_of_bounds:str << "%slot-set!: out of bounds"; break;
          case re_slot_set_contract_violation: str << "%slot-set!: contract violation"; break;
          case re_division_by_zero: str << "division by zero"; break;
          case re_heap_full: str << "heap is full"; break;
          case re_load_contract_violation: str << "load: contract violation"; break;
          case re_eval_contract_violation: str << "%eval: contract violation"; break;
          case re_getenv_contract_violation: str << "getenv: contract violation"; break;
          case re_getenv_heap_overflow: str << "getenv: heap overflow"; break;
          case re_putenv_contract_violation: str << "putenv: contract violation"; break;
          case re_file_exists_contract_violation: str << "file-exists?: contract violation"; break;
          default: str << "unknown error"; break;
          }
        if (p_ctxt)
          str << ": ";
        print_last_global_variable_used(str, env, rd, p_ctxt);
        //str << "\nInstead I got ";
        texts.push_back(str.str());
        //todo.emplace_back(original_rax_lowest_48, 0);        
        }
      else
        {
        str << "<unknown " << uint64_to_hex(rax) << ">";
        texts.push_back(str.str());
        }
      }
    for (const auto& s : texts)
      out << s;
    }
  }

void print_last_global_variable_used(std::ostream& out, std::shared_ptr<SKIWI::environment<SKIWI::environment_entry>> env, const repl_data& rd, const context* p_ctxt)
  {
  if (!p_ctxt)
    return;
  if (!env.get())
    return;

  out << "\ncall stack:\n";
  for (int stack_item = SKIWI_VARIABLE_DEBUG_STACK_SIZE - 1; stack_item >= 0; --stack_item)
    {
    uint64_t pos = p_ctxt->last_global_variable_used[stack_item];
    std::pair<std::string, SKIWI::environment_entry> res;
    if (env->find_if(res, [&](const std::pair<std::string, SKIWI::environment_entry>& v) { return v.second.pos == pos; }))
      {
      out << std::setw(4) << stack_item << ": ";
      std::string varname = get_variable_name_before_alpha(res.first);
      if (varname.substr(0, 3) == "#%q")
        {
        std::stringstream ss;
        ss << varname.substr(3);
        uint64_t ind;
        ss >> ind;
        for (const auto& q : rd.quote_to_index)
          {
          if (q.second == ind)
            {
            out << "(quote " << q.first << ")\n";
            }
          }
        }
      else
        out << varname << "\n";
      }
    }
  }

void scheme_runtime(uint64_t rax, std::ostream& out, std::shared_ptr<SKIWI::environment<SKIWI::environment_entry>> env, const repl_data& rd, const context* p_ctxt)
  {
  print_ptr(rax, out, env, rd, p_ctxt);
  }

SKIWI_END
