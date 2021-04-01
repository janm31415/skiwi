#include "primitives_lib.h"
#include "primitives.h"
#include "globals.h"
#include "asm_aux.h"
#include "context.h"
#include "context_defs.h"
#include "types.h"
#include "compile_error.h"

#include <sstream>

SKIWI_BEGIN

void compile_simplified_primitives_library(primitive_map& pm, repl_data& rd, environment_map& env, context& ctxt, ASM::asmcode& code, const compiler_options& options)
  {
  compile_data data = create_compile_data(ctxt.total_heap_size, ctxt.globals_end - ctxt.globals, (uint32_t)ctxt.number_of_locals, &ctxt);
  code.add(ASM::asmcode::GLOBAL, "scheme_entry");
  label = 0;
  function_map fm = generate_simplified_function_map();
#ifdef _WIN32
  /*
  windows parameters calling convention: rcx, rdx, r8, r9
  First parameter (rcx) points to the context.
  We store the pointer to the context in register r10.
  */
  code.add(ASM::asmcode::MOV, CONTEXT, ASM::asmcode::RCX);
#else
  /*
  Linux parameters calling convention: rdi, rsi, rdx, rcx, r8, r9
  First parameter (rdi) points to the context.
  We store the pointer to the context in register r10.
  */
  code.add(ASM::asmcode::MOV, CONTEXT, ASM::asmcode::RDI);
#endif

  /*
  Save the current content of the registers in the context
  */
  //store_registers(code);

  for (auto it = fm.begin(); it != fm.end(); ++it)
    {
    environment_entry e;
    e.pos = (uint64_t)rd.global_index * 8;
    ++rd.global_index;
    if (rd.global_index > data.globals_stack) // too many globals declared
      throw_error(too_many_globals);
    e.st = environment_entry::st_global;
    env->push_outer(it->first, e);
    //new_alpha->push_outer(it->first, it->first);

    std::string label_name = "L_" + it->first;
    code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::LABELADDRESS, label_name);
    code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, procedure_tag);
    code.add(ASM::asmcode::MOV, ASM::asmcode::R11, GLOBALS);
    code.add(ASM::asmcode::MOV, ASM::asmcode::MEM_R11, e.pos, ASM::asmcode::RAX);

    // We save the primitive constant twice. The second save has ### in front of its name, and is used during inlining for checking whether the primitive was redefined or not.
    e.pos = (uint64_t)rd.global_index * 8;
    ++rd.global_index;
    if (rd.global_index > data.globals_stack) // too many globals declared
      throw_error(too_many_globals);
    e.st = environment_entry::st_global;
    std::stringstream str;
    str << "###" << it->first;
    env->push_outer(str.str(), e);
    code.add(ASM::asmcode::MOV, ASM::asmcode::MEM_R11, e.pos, ASM::asmcode::RAX);
    }

  /*Restore the registers to their original state*/
  //load_registers(code);

  /*Return to the caller*/
  code.add(ASM::asmcode::RET);

  code.push();
  for (auto it = fm.begin(); it != fm.end(); ++it)
    {
    primitive_entry pe;
    pe.label_name = "L_" + it->first;
    pe.address = 0;
    pm.insert(std::pair<std::string, primitive_entry>(it->first, pe));
    code.add(ASM::asmcode::LABEL_ALIGNED, pe.label_name);
    it->second(code, options);
    }
  
  compile_bitwise_and_2(code, options);
  compile_bitwise_or_2(code, options);
  compile_bitwise_xor_2(code, options);
  compile_add_2(code, options);
  compile_divide_2(code, options);
  compile_multiply_2(code, options);
  compile_subtract_2(code, options);
  compile_equal_2(code, options);
  compile_not_equal_2(code, options);
  compile_less_2(code, options);
  compile_leq_2(code, options);
  compile_geq_2(code, options);
  compile_greater_2(code, options);
  compile_max_2(code, options);
  compile_min_2(code, options);
  compile_fold_binary(code, options);
  compile_pairwise_compare(code, options);
  compile_mark(code, options);
  compile_recursively_equal(code, options, "L_recursively_equal");
  compile_structurally_equal(code, options, "L_structurally_equal");
  compile_member_cmp_eqv(code, options);
  compile_member_cmp_eq(code, options);
  compile_member_cmp_equal(code, options);
  compile_assoc_cmp_eqv(code, options);
  compile_assoc_cmp_eq(code, options);
  compile_assoc_cmp_equal(code, options);
  //compile_apply_fake_cps_identity(code, options);
  
  code.pop();
  }

void compile_primitives_library(primitive_map& pm, repl_data& rd, environment_map& env, context& ctxt, ASM::asmcode& code, const compiler_options& options)
  {
  //std::shared_ptr<environment<alpha_conversion_data>> new_alpha = std::make_shared<environment<alpha_conversion_data>>(rd.alpha_conversion_env);

  compile_data data = create_compile_data(ctxt.total_heap_size, ctxt.globals_end - ctxt.globals, (uint32_t)ctxt.number_of_locals, &ctxt);
  code.add(ASM::asmcode::GLOBAL, "scheme_entry");
  label = 0;
  function_map fm = generate_function_map();
#ifdef _WIN32
  /*
  windows parameters calling convention: rcx, rdx, r8, r9
  First parameter (rcx) points to the context.
  We store the pointer to the context in register r10.
  */
  code.add(ASM::asmcode::MOV, CONTEXT, ASM::asmcode::RCX);
#else
  /*
  Linux parameters calling convention: rdi, rsi, rdx, rcx, r8, r9
  First parameter (rdi) points to the context.
  We store the pointer to the context in register r10.
  */
  code.add(ASM::asmcode::MOV, CONTEXT, ASM::asmcode::RDI);
#endif

  /*
  Save the current content of the registers in the context
  */
  //store_registers(code);

  for (auto it = fm.begin(); it != fm.end(); ++it)
    {
    environment_entry e;
    e.pos = (uint64_t)rd.global_index * 8;
    ++rd.global_index;
    if (rd.global_index > data.globals_stack) // too many globals declared
      throw_error(too_many_globals);
    e.st = environment_entry::st_global;
    env->push_outer(it->first, e);
    //new_alpha->push_outer(it->first, it->first);

    std::string label_name = "L_" + it->first;
    code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::LABELADDRESS, label_name);
    code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, procedure_tag);  
    code.add(ASM::asmcode::MOV, ASM::asmcode::R11, GLOBALS);
    code.add(ASM::asmcode::MOV, ASM::asmcode::MEM_R11, e.pos, ASM::asmcode::RAX);

    // We save the primitive constant twice. The second save has ### in front of its name, and is used during inlining for checking whether the primitive was redefined or not.
    e.pos = (uint64_t)rd.global_index * 8;
    ++rd.global_index;
    if (rd.global_index > data.globals_stack) // too many globals declared
      throw_error(too_many_globals);
    e.st = environment_entry::st_global;
    std::stringstream str;
    str << "###" << it->first;
    env->push_outer(str.str(), e);
    code.add(ASM::asmcode::MOV, ASM::asmcode::MEM_R11, e.pos, ASM::asmcode::RAX);
    }

  /*Restore the registers to their original state*/
  //load_registers(code);

  /*Return to the caller*/
  code.add(ASM::asmcode::RET);

  code.push();
  for (auto it = fm.begin(); it != fm.end(); ++it)
    {
    primitive_entry pe;
    pe.label_name = "L_"+it->first;
    pe.address = 0;
    pm.insert(std::pair<std::string, primitive_entry>(it->first, pe));
    code.add(ASM::asmcode::LABEL_ALIGNED, pe.label_name);
    it->second(code, options);
    }
  compile_bitwise_and_2(code, options);  
  compile_bitwise_or_2(code, options);
  compile_bitwise_xor_2(code, options);
  compile_add_2(code, options);  
  compile_divide_2(code, options);
  compile_multiply_2(code, options);
  compile_subtract_2(code, options);
  compile_equal_2(code, options);
  compile_not_equal_2(code, options);
  compile_less_2(code, options);
  compile_leq_2(code, options);
  compile_geq_2(code, options);
  compile_greater_2(code, options);
  compile_max_2(code, options);
  compile_min_2(code, options);
  compile_fold_binary(code, options);
  compile_pairwise_compare(code, options);
  compile_mark(code, options);
  compile_recursively_equal(code, options, "L_recursively_equal");
  compile_structurally_equal(code, options, "L_structurally_equal");
  compile_member_cmp_eqv(code, options);
  compile_member_cmp_eq(code, options);
  compile_member_cmp_equal(code, options);
  compile_assoc_cmp_eqv(code, options);
  compile_assoc_cmp_eq(code, options);
  compile_assoc_cmp_equal(code, options);
  compile_apply_fake_cps_identity(code, options);
  code.pop();  

  //rd.alpha_conversion_env = new_alpha;
  }

void assign_primitive_addresses(primitive_map& pm, const ASM::first_pass_data& d, uint64_t address_start)
  {
  for (auto& pe : pm)
    {
    auto it = d.label_to_address.find(pe.second.label_name);
    if (it == d.label_to_address.end())
      throw std::runtime_error("Error during primitives library generation");
    pe.second.address = address_start + it->second;
    }
  }

SKIWI_END
