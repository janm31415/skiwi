#include "primitives.h"

#include "file_utils.h"

#include "asm_aux.h"
#include "c_prim_decl.h"
#include "context.h"
#include "context_defs.h"
#include "globals.h"
#include "inlines.h"
#include "types.h"

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <fcntl.h>
#include <fstream>

#include <chrono>

SKIWI_BEGIN

using namespace ASM;

namespace
  {
  std::vector<asmcode::operand> compute_argument_registers()
    {
    std::vector<asmcode::operand> usable_registers;
    usable_registers.push_back(asmcode::RCX);
    usable_registers.push_back(asmcode::RDX);
    usable_registers.push_back(asmcode::RSI);
    usable_registers.push_back(asmcode::RDI);
    usable_registers.push_back(asmcode::R8);
    usable_registers.push_back(asmcode::R9);
    usable_registers.push_back(asmcode::R12);
    usable_registers.push_back(asmcode::R14);
    return usable_registers;
    }
  }

const std::vector<asmcode::operand>& get_argument_registers()
  {
  static std::vector<asmcode::operand> reg = compute_argument_registers();
  return reg;
  }

void compile_is_eq(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::CMP, asmcode::RCX, asmcode::RDX);
  // if comparison (last cmp call) succeeded, then sete puts 1 in al, otherwise 0
  code.add(asmcode::SETE, asmcode::AL);
  // movzx extends the bits in al to the full rax register
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);

  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_eq_contract_violation);
    }
  }

void compile_is_eqv(asmcode& code, const compiler_options& ops)
  {
  /*
  //From Bones:
  //`eqv?' performs /structural/ comparison, which means it compares
  //  the contents of its two arguments, in case they are of equal
  //  type. That means it will will return `#t' if both arguments have
  //  the same type and identical contents, even if the arguments are
  //  not numbers or characters.

  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNE, error);
    }
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::RDX);
  std::string lab_eq = label_to_string(label++);
  std::string lab = label_to_string(label++);
  code.add(asmcode::JMP, lab);
  compile_structurally_equal(code, ops, lab_eq);
  code.add(asmcode::LABEL, lab);
  code.add(asmcode::CALL, lab_eq);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_eqv_contract_violation);
    }
    */

  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::CMP, asmcode::RCX, asmcode::RDX);
  // if comparison (last cmp call) succeeded, then sete puts 1 in al, otherwise 0
  code.add(asmcode::SETE, asmcode::AL);
  // movzx extends the bits in al to the full rax register
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);

  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_eqv_contract_violation);
    }
  }

void compile_is_eqv_structurally(asmcode& code, const compiler_options& ops)
  {
  //From Bones:
  //`eqv?' performs /structural/ comparison, which means it compares
  //  the contents of its two arguments, in case they are of equal
  //  type. That means it will will return `#t' if both arguments have
  //  the same type and identical contents, even if the arguments are
  //  not numbers or characters.

  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNE, error);
    }
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::RDX);
  std::string lab_eq = label_to_string(label++);
  std::string lab = label_to_string(label++);
  code.add(asmcode::JMP, lab);
  compile_structurally_equal(code, ops, lab_eq);
  code.add(asmcode::LABEL, lab);
  code.add(asmcode::CALL, lab_eq);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_eqvstruct_contract_violation);
    }
  }

void compile_is_equal(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNE, error);
    }
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::RDX);
  std::string lab_eq = label_to_string(label++);
  std::string lab = label_to_string(label++);
  code.add(asmcode::JMP, lab);
  compile_recursively_equal(code, ops, lab_eq);
  code.add(asmcode::LABEL, lab);
  code.add(asmcode::CALL, lab_eq);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_isequal_contract_violation);
    }
  }

void compile_closure(asmcode& code, const compiler_options& ops)
  {
  if (ops.safe_primitives)
    {
    code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);
    code.add(asmcode::INC, asmcode::RAX);
    check_heap(code, re_closure_heap_overflow);
    }
  auto done = label_to_string(label++);
  auto done2 = label_to_string(label++);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);
  code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, (uint64_t)closure_tag << (uint64_t)block_shift);
  code.add(asmcode::OR, asmcode::RAX, asmcode::R15);
  code.add(asmcode::MOV, asmcode::R15, ALLOC);
  code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::RCX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(2), asmcode::RDX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(3), asmcode::RSI);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 3);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(4), asmcode::RDI);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 4);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(5), asmcode::R8);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 5);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(6), asmcode::R9);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 6);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(7), asmcode::R12);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 7);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(8), asmcode::R14);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 8);
  code.add(asmcode::JES, done);

  code.add(asmcode::SUB, asmcode::R11, asmcode::NUMBER, 8);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(9));
  code.add(asmcode::MOV, asmcode::RDX, LOCAL);

  auto lab = label_to_string(label++);
  code.add(asmcode::LABEL, lab);

  code.add(asmcode::MOV, asmcode::RCX, asmcode::MEM_RDX);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RCX);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::DEC, asmcode::R11);
  code.add(asmcode::JE, done2);
  code.add(asmcode::JMPS, lab);

  code.add(asmcode::LABEL, done);
  code.add(asmcode::INC, asmcode::R11);
  code.add(asmcode::SHL, asmcode::R11, asmcode::NUMBER, 3);
  code.add(asmcode::ADD, ALLOC, asmcode::R11);
  code.add(asmcode::LABEL, done2);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R15);
  code.add(asmcode::JMP, CONTINUE);
  }

void compile_closure_ref(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  // here check whether it is a closure
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_closure(code, asmcode::RCX, asmcode::R11, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::SAR, asmcode::RDX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    {
    // optionally, check whether rdx is in bounds.
    std::string in_bounds = label_to_string(label++);
    std::string not_in_bounds = label_to_string(label++);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
    code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, block_size_mask);
    code.add(asmcode::AND, asmcode::RAX, asmcode::R15); // get size of closure
    code.add(asmcode::CMP, asmcode::RDX, asmcode::RAX);
    code.add(asmcode::JGES, not_in_bounds);
    code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, 0);
    code.add(asmcode::JGES, in_bounds);
    error_label(code, not_in_bounds, re_closure_ref_out_of_bounds);
    code.add(asmcode::LABEL, in_bounds);
    }

  code.add(asmcode::INC, asmcode::RDX); // increase 1 for header
  code.add(asmcode::SHL, asmcode::RDX, asmcode::NUMBER, 3);
  code.add(asmcode::ADD, asmcode::RCX, asmcode::RDX);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);

  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_closure_ref_contract_violation);
    }
  }

void compile_string_copy(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1); // rcx should be positive
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_string(code, asmcode::RCX, asmcode::R11, error);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
    code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, block_size_mask);
    code.add(asmcode::AND, asmcode::RAX, asmcode::R11);
    code.add(asmcode::INC, asmcode::RAX);
    check_heap(code, re_string_copy_heap_overflow);
    }
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, block_size_mask);
  code.add(asmcode::AND, asmcode::RAX, asmcode::R11);
  code.add(asmcode::MOV, asmcode::R11, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, (uint64_t)string_tag << (uint64_t)block_shift);
  code.add(asmcode::OR, asmcode::RAX, asmcode::R15);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R15, ALLOC);
  code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));

  auto repeat = label_to_string(label++);
  auto done = label_to_string(label++);

  code.add(asmcode::LABEL, repeat);
  code.add(asmcode::TEST, asmcode::R11, asmcode::R11);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::DEC, asmcode::R11);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::JMPS, repeat);
  code.add(asmcode::LABEL, done);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::R15);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_string_copy_contract_violation);
    }
  }

void compile_symbol_to_string(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1); // rcx should be positive
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_symbol(code, asmcode::RCX, asmcode::R11, error);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
    code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, block_size_mask);
    code.add(asmcode::AND, asmcode::RAX, asmcode::R11);
    code.add(asmcode::INC, asmcode::RAX);
    check_heap(code, re_symbol_to_string_heap_overflow);
    }
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, block_size_mask);
  code.add(asmcode::AND, asmcode::RAX, asmcode::R11);
  code.add(asmcode::MOV, asmcode::R11, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, (uint64_t)string_tag << (uint64_t)block_shift);
  code.add(asmcode::OR, asmcode::RAX, asmcode::R15);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R15, ALLOC);
  code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));

  auto repeat = label_to_string(label++);
  auto done = label_to_string(label++);

  code.add(asmcode::LABEL, repeat);
  code.add(asmcode::TEST, asmcode::R11, asmcode::R11);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::DEC, asmcode::R11);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::JMPS, repeat);
  code.add(asmcode::LABEL, done);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::R15);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_symbol_to_string_contract_violation);
    }
  }

void compile_string_hash(asmcode& code, const compiler_options& ops)
  {

  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xfffffffffffffff8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_string(code, asmcode::RCX, asmcode::R11, error);
    }

  code.add(asmcode::PUSH, asmcode::RBX);
  code.add(asmcode::PUSH, asmcode::RDX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::MEM_RCX);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, block_size_mask);
  code.add(asmcode::AND, asmcode::R11, asmcode::RAX);
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));

  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::DEC, asmcode::R11);
  code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, 3498574187);
  code.add(asmcode::MUL, asmcode::R15);
  code.add(asmcode::MOV, asmcode::RBX, asmcode::RAX);

  auto repeat = label_to_string(label++);
  auto done = label_to_string(label++);

  code.add(asmcode::LABEL, repeat);
  code.add(asmcode::TEST, asmcode::R11, asmcode::R11);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
  code.add(asmcode::MUL, asmcode::R15);
  code.add(asmcode::XOR, asmcode::RBX, asmcode::RAX);
  code.add(asmcode::DEC, asmcode::R11);
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::JMPS, repeat);
  code.add(asmcode::LABEL, done);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::SHR, asmcode::RBX, asmcode::NUMBER, 8);
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::SHR, asmcode::RBX, asmcode::NUMBER, 8);
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::SHR, asmcode::RBX, asmcode::NUMBER, 8);
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::SHR, asmcode::RBX, asmcode::NUMBER, 8);
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::SHR, asmcode::RBX, asmcode::NUMBER, 8);
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::SHR, asmcode::RBX, asmcode::NUMBER, 8);
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::SHR, asmcode::RBX, asmcode::NUMBER, 8);
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 255);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::POP, asmcode::RDX);
  code.add(asmcode::POP, asmcode::RBX);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_string_hash_contract_violation);
    }
  }

void compile_string_append1(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    jump_if_arg_is_not_block(code, asmcode::RDX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xfffffffffffffff8);
  code.add(asmcode::AND, asmcode::RDX, asmcode::NUMBER, 0xfffffffffffffff8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_string(code, asmcode::RCX, asmcode::R11, error);
    jump_if_arg_does_not_point_to_string(code, asmcode::RDX, asmcode::R11, error);
    }

  code.add(asmcode::PUSH, asmcode::RBX);
  code.add(asmcode::PUSH, asmcode::RCX);
  string_length(code, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::RBX, asmcode::R15);
  code.add(asmcode::POP, asmcode::RCX);
  code.add(asmcode::PUSH, asmcode::RDX);
  string_length(code, asmcode::RDX);
  code.add(asmcode::POP, asmcode::RDX);

  if (ops.safe_primitives) // TO CHECK: string length should be divided by 8 here I thing before heap check
    {
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
    code.add(asmcode::POP, asmcode::RBX);
    code.add(asmcode::PUSH, asmcode::RCX);
    code.add(asmcode::MOV, asmcode::RCX, asmcode::RAX);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::R15);
    code.add(asmcode::MOV, asmcode::R11, asmcode::R15);
    //code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 3); // added as action on comment // TO CHECK: string length should be divided by 8 here I thing before heap check
    //code.add(asmcode::INC, asmcode::RAX);                     // not tested yet  ==> removed again: made compiler.scm fail!!
    check_heap(code, re_string_append_heap_overflow);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
    code.add(asmcode::MOV, asmcode::RBX, asmcode::RCX);
    code.add(asmcode::POP, asmcode::RCX);
    code.add(asmcode::PUSH, asmcode::RAX);
    code.add(asmcode::MOV, asmcode::R15, asmcode::R11);
    }

  /*
  status: rcx points to string 1
          rdx points to string 2
          rbx contains string 1 length
          r15 contains string 2 length
  */

  code.add(asmcode::PUSH, asmcode::R15); // save string 2 length
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::ADD, asmcode::RAX, asmcode::R15);
  code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::INC, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::RAX); // save block length in r11
  code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, (uint64_t)string_tag << (uint64_t)block_shift);
  code.add(asmcode::OR, asmcode::RAX, asmcode::R15);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R15, ALLOC);
  code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));

  code.add(asmcode::DEC, asmcode::R11);
  code.add(asmcode::SHL, asmcode::R11, asmcode::NUMBER, 3);
  code.add(asmcode::ADD, ALLOC, asmcode::R11);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::NUMBER, 0); // ending 0 character
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));

  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1)); // skip header string 1
  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1)); // skip header string 2

  auto repeat = label_to_string(label++);
  auto done = label_to_string(label++);
  auto repeat2 = label_to_string(label++);
  auto done2 = label_to_string(label++);
  code.add(asmcode::MOV, asmcode::R11, asmcode::R15); // get start of string block
  code.add(asmcode::AND, asmcode::R11, asmcode::NUMBER, 0xfffffffffffffff8); // remove block tag
  code.add(asmcode::ADD, asmcode::R11, asmcode::NUMBER, CELLS(1)); // skip header

  code.add(asmcode::LABEL, repeat);
  code.add(asmcode::TEST, asmcode::RBX, asmcode::RBX);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::AL, asmcode::BYTE_MEM_RCX);
  code.add(asmcode::MOV, asmcode::BYTE_MEM_R11, asmcode::AL);
  code.add(asmcode::DEC, asmcode::RBX);
  code.add(asmcode::INC, asmcode::RCX);
  code.add(asmcode::INC, asmcode::R11);
  code.add(asmcode::JMPS, repeat);
  code.add(asmcode::LABEL, done);

  code.add(asmcode::POP, asmcode::RBX); // get length of string 2 in rbx
  code.add(asmcode::LABEL, repeat2);
  code.add(asmcode::TEST, asmcode::RBX, asmcode::RBX);
  code.add(asmcode::JES, done2);
  code.add(asmcode::MOV, asmcode::AL, asmcode::BYTE_MEM_RDX);
  code.add(asmcode::MOV, asmcode::BYTE_MEM_R11, asmcode::AL);
  code.add(asmcode::DEC, asmcode::RBX);
  code.add(asmcode::INC, asmcode::RDX);
  code.add(asmcode::INC, asmcode::R11);
  code.add(asmcode::JMPS, repeat2);
  code.add(asmcode::LABEL, done2);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::R15);
  code.add(asmcode::POP, asmcode::RBX);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_string_append_contract_violation);
    }
  }

void compile_substring(asmcode& code, const compiler_options& ops)
  {
  std::string error, not_in_bounds;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    not_in_bounds = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 3);
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    code.add(asmcode::TEST, asmcode::RSI, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, 0);
    code.add(asmcode::JL, not_in_bounds);
    code.add(asmcode::CMP, asmcode::RSI, asmcode::NUMBER, 0);
    code.add(asmcode::JL, not_in_bounds);
    code.add(asmcode::CMP, asmcode::RDX, asmcode::RSI);
    code.add(asmcode::JG, not_in_bounds);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RSI);
    code.add(asmcode::SUB, asmcode::RAX, asmcode::RDX);
    code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 1);
    check_heap(code, re_substring_heap_overflow);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xfffffffffffffff8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_string(code, asmcode::RCX, asmcode::R11, error);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
    code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, block_size_mask);
    code.add(asmcode::AND, asmcode::RAX, asmcode::R11);
    code.add(asmcode::MOV, asmcode::R11, asmcode::RSI);
    code.add(asmcode::SHR, asmcode::R11, asmcode::NUMBER, 4);
    code.add(asmcode::INC, asmcode::R11);
    code.add(asmcode::CMP, asmcode::RAX, asmcode::R11);
    code.add(asmcode::JL, not_in_bounds);
    }

  code.add(asmcode::MOV, asmcode::RAX, asmcode::RSI);
  code.add(asmcode::SUB, asmcode::RAX, asmcode::RDX);
  code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 4);
  code.add(asmcode::INC, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::RAX);
  code.add(asmcode::SHL, asmcode::R11, asmcode::NUMBER, 3);
  code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, (uint64_t)string_tag << (uint64_t)block_shift);
  code.add(asmcode::OR, asmcode::RAX, asmcode::R15);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R15, ALLOC);
  code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::RAX, ALLOC);
  code.add(asmcode::ADD, ALLOC, asmcode::R11);

  code.add(asmcode::SHR, asmcode::RDX, asmcode::NUMBER, 1);
  code.add(asmcode::SHR, asmcode::RSI, asmcode::NUMBER, 1);
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::ADD, asmcode::RCX, asmcode::RDX);

  code.add(asmcode::PUSH, asmcode::RBX);
  auto repeat = label_to_string(label++);
  auto done = label_to_string(label++);
  code.add(asmcode::LABEL, repeat);
  code.add(asmcode::CMP, asmcode::RDX, asmcode::RSI);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::BL, asmcode::BYTE_MEM_RCX);
  code.add(asmcode::MOV, asmcode::BYTE_MEM_RAX, asmcode::BL);
  code.add(asmcode::INC, asmcode::RDX);
  code.add(asmcode::INC, asmcode::RCX);
  code.add(asmcode::INC, asmcode::RAX);
  code.add(asmcode::JMPS, repeat);
  code.add(asmcode::LABEL, done);

  auto repeat2 = label_to_string(label++);
  auto done2 = label_to_string(label++);
  code.add(asmcode::XOR, asmcode::RBX, asmcode::RBX); // make remaining bytes of string equal to 0
  code.add(asmcode::LABEL, repeat2);
  code.add(asmcode::MOV, asmcode::BYTE_MEM_RAX, asmcode::BL);
  code.add(asmcode::INC, asmcode::RAX);
  code.add(asmcode::CMP, asmcode::RAX, ALLOC);
  code.add(asmcode::JES, done2);
  code.add(asmcode::JMPS, repeat2);
  code.add(asmcode::LABEL, done2);

  code.add(asmcode::POP, asmcode::RBX);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::R15);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_substring_contract_violation);
    error_label(code, not_in_bounds, re_substring_out_of_bounds);
    }
  }

void compile_allocate_symbol(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_string(code, asmcode::RCX, asmcode::R11, error);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
    code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, block_size_mask);
    code.add(asmcode::AND, asmcode::RAX, asmcode::R11);
    code.add(asmcode::INC, asmcode::RAX);
    check_heap(code, re_allocate_symbol_heap_overflow);
    }
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, block_size_mask);
  code.add(asmcode::AND, asmcode::RAX, asmcode::R11);
  code.add(asmcode::MOV, asmcode::R11, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, (uint64_t)symbol_tag << (uint64_t)block_shift);
  code.add(asmcode::OR, asmcode::RAX, asmcode::R15);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R15, ALLOC);
  code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));

  auto repeat = label_to_string(label++);
  auto done = label_to_string(label++);

  code.add(asmcode::LABEL, repeat);
  code.add(asmcode::TEST, asmcode::R11, asmcode::R11);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::DEC, asmcode::R11);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::JMPS, repeat);
  code.add(asmcode::LABEL, done);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::R15);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_allocate_symbol_contract_violation);
    }
  }

void compile_make_string(asmcode& code, const compiler_options& ops)
  {
  auto fill_with_value = label_to_string(label++);
  auto fill_random = label_to_string(label++);
  auto fill_remainder = label_to_string(label++);
  auto done1 = label_to_string(label++);
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 0); // rcx should be positive
    code.add(asmcode::JL, error);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 4);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, 2);
    check_heap(code, re_make_string_heap_overflow);
    }
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
  code.add(asmcode::JE, fill_with_value);
  if (ops.safe_primitives)
    {
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    }

  code.add(asmcode::SHR, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::INC, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, (uint64_t)string_tag << (uint64_t)block_shift);
  code.add(asmcode::OR, asmcode::RAX, asmcode::R15);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R15, ALLOC);
  code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));

  code.add(asmcode::LABEL, fill_random);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 8);
  code.add(asmcode::JLS, fill_remainder);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::NUMBER, 0xffffffffffffffff);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::SUB, asmcode::RCX, asmcode::NUMBER, 8);
  code.add(asmcode::JMPS, fill_random);
  code.add(asmcode::LABEL, fill_remainder);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::NUMBER, 0);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 0);
  code.add(asmcode::JES, done1);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 0x00000000000000ff);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, done1);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 0x000000000000ffff);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 2);
  code.add(asmcode::JES, done1);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 0x0000000000ffffff);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 3);
  code.add(asmcode::JES, done1);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 0x00000000ffffffff);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 4);
  code.add(asmcode::JES, done1);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 0x000000ffffffffff);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 5);
  code.add(asmcode::JES, done1);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 0x0000ffffffffffff);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 6);
  code.add(asmcode::JES, done1);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 0x00ffffffffffffff);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::LABEL, done1);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R15);
  code.add(asmcode::JMP, CONTINUE);
  /*
  code.add(asmcode::SHR, asmcode::RCX, asmcode::NUMBER, 4);
  code.add(asmcode::INC, asmcode::RCX);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, (uint64_t)string_tag << (uint64_t)block_shift);
  code.add(asmcode::OR, asmcode::RAX, asmcode::R15);
  code.add(asmcode::MOV, asmcode::R15, ALLOC);
  code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::LABEL, fill_zero);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::NUMBER, 0);
  code.add(asmcode::DEC, asmcode::RCX);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 0);
  code.add(asmcode::JNES, fill_zero);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R15);
  code.add(asmcode::JMP, CONTINUE);
  */


  code.add(asmcode::LABEL, fill_with_value);
  code.add(asmcode::SHR, asmcode::RDX, asmcode::NUMBER, 8); // get character value in dl
  code.add(asmcode::SHR, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::INC, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, (uint64_t)string_tag << (uint64_t)block_shift);
  code.add(asmcode::OR, asmcode::RAX, asmcode::R15);
  code.add(asmcode::MOV, asmcode::R15, ALLOC);
  code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));

  auto fill_rest = label_to_string(label++);
  auto fill_full = label_to_string(label++);
  auto done = label_to_string(label++);
  code.add(asmcode::LABEL, fill_full);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::JLS, fill_rest);

  code.add(asmcode::MOV, BYTE_MEM_ALLOC, asmcode::DL);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 1, asmcode::DL);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 2, asmcode::DL);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 3, asmcode::DL);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 4, asmcode::DL);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 5, asmcode::DL);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 6, asmcode::DL);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 7, asmcode::DL);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::SUB, asmcode::RCX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::JMPS, fill_full);

  code.add(asmcode::LABEL, fill_rest);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::NUMBER, 0);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 0);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, asmcode::DL);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 1, asmcode::DL);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 2);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 2, asmcode::DL);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 3);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 3, asmcode::DL);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 4);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 4, asmcode::DL);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 5);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 5, asmcode::DL);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 6);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 6, asmcode::DL);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 7);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 7, asmcode::DL);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));

  code.add(asmcode::MOV, asmcode::RAX, asmcode::R15);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_make_string_contract_violation);
    }
  }

void compile_fixnum_to_char(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }

  code.add(asmcode::SHL, asmcode::RCX, asmcode::NUMBER, 7);
  code.add(asmcode::OR, asmcode::RCX, asmcode::NUMBER, char_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_fixnum_to_char_contract_violation);
    }
  }

void compile_char_to_fixnum(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    code.add(asmcode::MOV, asmcode::R11, asmcode::RCX);
    code.add(asmcode::AND, asmcode::R11, asmcode::NUMBER, char_mask);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, char_tag);
    code.add(asmcode::JNES, error);
    }

  code.add(asmcode::SHR, asmcode::RCX, asmcode::NUMBER, 7);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_char_to_fixnum_contract_violation);
    }
  }

void compile_fx_less(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  std::string t = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::CMP, asmcode::RCX, asmcode::RDX);
  code.add(asmcode::JLS, t);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, t);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_fx_less_contract_violation);
    }
  }

void compile_fx_greater(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  std::string t = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::CMP, asmcode::RCX, asmcode::RDX);
  code.add(asmcode::JGS, t);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, t);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_fx_greater_contract_violation);
    }
  }

void compile_fx_leq(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  std::string t = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::CMP, asmcode::RCX, asmcode::RDX);
  code.add(asmcode::JLES, t);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, t);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_fx_leq_contract_violation);
    }
  }

void compile_fx_geq(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  std::string t = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::CMP, asmcode::RCX, asmcode::RDX);
  code.add(asmcode::JGES, t);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, t);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_fx_geq_contract_violation);
    }
  }

void compile_fx_add1(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 2);
  code.add(asmcode::ADD, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_fx_add1_contract_violation);
    }
  }

void compile_fx_sub1(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::SUB, asmcode::RAX, asmcode::NUMBER, 2);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_fx_sub1_contract_violation);
    }
  }

void compile_fx_is_zero(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 0);
  code.add(asmcode::SETE, asmcode::AL);
  // movzx extends the bits in al to the full rax register
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_fx_is_zero_contract_violation);
    }
  }

void compile_fx_add(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::ADD, asmcode::RAX, asmcode::RDX);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_fx_add_contract_violation);
    }
  }

void compile_fx_sub(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::SUB, asmcode::RAX, asmcode::RDX);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_fx_sub_contract_violation);
    }
  }

void compile_fx_mul(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::IMUL, asmcode::RDX);
  code.add(asmcode::SAR, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_fx_mul_contract_violation);
    }
  }

void compile_fx_div(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  std::string division_by_zero;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    division_by_zero = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::RDX);
    code.add(asmcode::JES, division_by_zero);
    }
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::RCX, asmcode::RDX);
  code.add(asmcode::XOR, asmcode::RDX, asmcode::RDX);
  code.add(asmcode::CQO);
  code.add(asmcode::IDIV, asmcode::RCX);
  code.add(asmcode::SAL, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_fx_div_contract_violation);
    error_label(code, division_by_zero, re_division_by_zero);
    }
  }

void compile_fx_equal(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::CMP, asmcode::RCX, asmcode::RDX);
  code.add(asmcode::SETE, asmcode::AL);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_fx_equal_contract_violation);
    }
  }

void compile_char_less(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  std::string t = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    code.add(asmcode::MOV, asmcode::R11, asmcode::RCX);
    code.add(asmcode::AND, asmcode::R11, asmcode::NUMBER, char_mask);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, char_tag);
    code.add(asmcode::JNES, error);
    code.add(asmcode::MOV, asmcode::R11, asmcode::RDX);
    code.add(asmcode::AND, asmcode::R11, asmcode::NUMBER, char_mask);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, char_tag);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::CMP, asmcode::RCX, asmcode::RDX);
  code.add(asmcode::JLS, t);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, t);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_char_less_contract_violation);
    }
  }

void compile_char_greater(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  std::string t = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    code.add(asmcode::MOV, asmcode::R11, asmcode::RCX);
    code.add(asmcode::AND, asmcode::R11, asmcode::NUMBER, char_mask);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, char_tag);
    code.add(asmcode::JNES, error);
    code.add(asmcode::MOV, asmcode::R11, asmcode::RDX);
    code.add(asmcode::AND, asmcode::R11, asmcode::NUMBER, char_mask);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, char_tag);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::CMP, asmcode::RCX, asmcode::RDX);
  code.add(asmcode::JGS, t);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, t);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_char_greater_contract_violation);
    }
  }

void compile_char_leq(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  std::string t = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    code.add(asmcode::MOV, asmcode::R11, asmcode::RCX);
    code.add(asmcode::AND, asmcode::R11, asmcode::NUMBER, char_mask);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, char_tag);
    code.add(asmcode::JNES, error);
    code.add(asmcode::MOV, asmcode::R11, asmcode::RDX);
    code.add(asmcode::AND, asmcode::R11, asmcode::NUMBER, char_mask);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, char_tag);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::CMP, asmcode::RCX, asmcode::RDX);
  code.add(asmcode::JLES, t);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, t);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_char_leq_contract_violation);
    }
  }

void compile_char_geq(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  std::string t = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    code.add(asmcode::MOV, asmcode::R11, asmcode::RCX);
    code.add(asmcode::AND, asmcode::R11, asmcode::NUMBER, char_mask);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, char_tag);
    code.add(asmcode::JNES, error);
    code.add(asmcode::MOV, asmcode::R11, asmcode::RDX);
    code.add(asmcode::AND, asmcode::R11, asmcode::NUMBER, char_mask);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, char_tag);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::CMP, asmcode::RCX, asmcode::RDX);
  code.add(asmcode::JGES, t);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, t);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_char_geq_contract_violation);
    }
  }

void compile_char_equal(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    code.add(asmcode::MOV, asmcode::R11, asmcode::RCX);
    code.add(asmcode::AND, asmcode::R11, asmcode::NUMBER, char_mask);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, char_tag);
    code.add(asmcode::JNES, error);
    code.add(asmcode::MOV, asmcode::R11, asmcode::RDX);
    code.add(asmcode::AND, asmcode::R11, asmcode::NUMBER, char_mask);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, char_tag);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::CMP, asmcode::RCX, asmcode::RDX);
  code.add(asmcode::SETE, asmcode::AL);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_char_equal_contract_violation);
    }
  }

void compile_string(asmcode& code, const compiler_options& ops)
  {
  if (ops.safe_primitives)
    {
    code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);
    code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 3);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, 2);
    check_heap(code, re_string_heap_overflow);
    }
  auto done = label_to_string(label++);
  auto repeat = label_to_string(label++);
  auto remainder = label_to_string(label++);
  auto remainder_loop = label_to_string(label++);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);
  code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::INC, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, (uint64_t)string_tag << (uint64_t)block_shift);
  code.add(asmcode::OR, asmcode::RAX, asmcode::R15);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R15, ALLOC);
  code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::NUMBER, 0);

  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 0);
  code.add(asmcode::JE, done);
  code.add(asmcode::SHR, asmcode::RCX, asmcode::NUMBER, 8);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, asmcode::CL);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
  code.add(asmcode::JE, done);
  code.add(asmcode::SHR, asmcode::RDX, asmcode::NUMBER, 8);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 1, asmcode::DL);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
  code.add(asmcode::JE, done);
  code.add(asmcode::SHR, asmcode::RSI, asmcode::NUMBER, 8);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RSI);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 2, asmcode::AL);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 3);
  code.add(asmcode::JE, done);
  code.add(asmcode::SHR, asmcode::RDI, asmcode::NUMBER, 8);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RDI);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 3, asmcode::AL);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 4);
  code.add(asmcode::JE, done);
  code.add(asmcode::SHR, asmcode::R8, asmcode::NUMBER, 8);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R8);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 4, asmcode::AL);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 5);
  code.add(asmcode::JE, done);
  code.add(asmcode::SHR, asmcode::R9, asmcode::NUMBER, 8);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R9);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 5, asmcode::AL);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 6);
  code.add(asmcode::JE, done);
  code.add(asmcode::SHR, asmcode::R12, asmcode::NUMBER, 8);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R12);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 6, asmcode::AL);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 7);
  code.add(asmcode::JE, done);
  code.add(asmcode::SHR, asmcode::R14, asmcode::NUMBER, 8);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R14);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 7, asmcode::AL);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::NUMBER, 0);
  code.add(asmcode::SUB, asmcode::R11, asmcode::NUMBER, 8);
  code.add(asmcode::MOV, asmcode::RDX, LOCAL);
  code.add(asmcode::LABEL, repeat);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 8);
  code.add(asmcode::JL, remainder);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RDX);
  code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 8);
  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, asmcode::AL);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RDX);
  code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 8);
  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 1, asmcode::AL);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RDX);
  code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 8);
  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 2, asmcode::AL);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RDX);
  code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 8);
  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 3, asmcode::AL);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RDX);
  code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 8);
  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 4, asmcode::AL);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RDX);
  code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 8);
  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 5, asmcode::AL);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RDX);
  code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 8);
  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 6, asmcode::AL);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RDX);
  code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 8);
  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, 7, asmcode::AL);

  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::NUMBER, 0);
  code.add(asmcode::SUB, asmcode::R11, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::JMP, repeat);

  code.add(asmcode::LABEL, remainder);
  code.add(asmcode::MOV, asmcode::RCX, ALLOC);
  code.add(asmcode::LABEL, remainder_loop);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 0);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RDX);
  code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 8);
  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::BYTE_MEM_RCX, asmcode::AL);
  code.add(asmcode::INC, asmcode::RCX);
  code.add(asmcode::DEC, asmcode::R11);
  code.add(asmcode::JMPS, remainder_loop);

  code.add(asmcode::LABEL, done);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R15);
  code.add(asmcode::JMP, CONTINUE);

  /*
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);
  code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, (uint64_t)string_tag << (uint64_t)block_shift);
  code.add(asmcode::OR, asmcode::RAX, asmcode::R15);
  code.add(asmcode::MOV, asmcode::R15, ALLOC);
  code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::RCX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(2), asmcode::RDX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(3), asmcode::RSI);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 3);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(4), asmcode::RDI);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 4);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(5), asmcode::R8);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 5);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(6), asmcode::R9);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 6);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(7), asmcode::R12);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 7);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(8), asmcode::R14);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 8);
  code.add(asmcode::JES, done);


  code.add(asmcode::JMP, CONTINUE);
  */
  }

void compile_string_length(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  auto done = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_string(code, asmcode::RCX, asmcode::R11, error);
    }

  string_length(code, asmcode::RCX);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::R15);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_string_length_contract_violation);
    }
  }

void compile_string_ref(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  // here check whether it is a string
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_string(code, asmcode::RCX, asmcode::R11, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::SAR, asmcode::RDX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    {
    // optionally, check whether rdx is in bounds.
    std::string in_bounds = label_to_string(label++);
    std::string not_in_bounds = label_to_string(label++);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
    code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, block_size_mask);
    code.add(asmcode::AND, asmcode::RAX, asmcode::R15);
    code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
    code.add(asmcode::CMP, asmcode::RDX, asmcode::RAX);
    code.add(asmcode::JGES, not_in_bounds);
    code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, 0);
    code.add(asmcode::JGES, in_bounds);
    error_label(code, not_in_bounds, re_string_ref_out_of_bounds);
    code.add(asmcode::LABEL, in_bounds);
    }

  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, 8); // increase 8 for header
  code.add(asmcode::ADD, asmcode::RCX, asmcode::RDX);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 255);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 8);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, char_tag);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_string_ref_contract_violation);
    }
  }

void compile_string_set(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 3);
    code.add(asmcode::JNE, error);
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  // here check whether it is a string and a char
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_string(code, asmcode::RCX, asmcode::R11, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    code.add(asmcode::MOV, asmcode::R11, asmcode::RSI);
    code.add(asmcode::AND, asmcode::R11, asmcode::NUMBER, char_mask);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, char_tag);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::SAR, asmcode::RDX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    {
    // optionally, check whether rdx is in bounds.
    std::string in_bounds = label_to_string(label++);
    std::string not_in_bounds = label_to_string(label++);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
    code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, block_size_mask);
    code.add(asmcode::AND, asmcode::RAX, asmcode::R15);
    code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
    code.add(asmcode::CMP, asmcode::RDX, asmcode::RAX);
    code.add(asmcode::JGES, not_in_bounds);
    code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, 0);
    code.add(asmcode::JGES, in_bounds);
    error_label(code, not_in_bounds, re_string_set_out_of_bounds);
    code.add(asmcode::LABEL, in_bounds);
    }

  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, 8); // increase 8 for header
  code.add(asmcode::ADD, asmcode::RCX, asmcode::RDX);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RSI);
  code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 8);
  code.add(asmcode::MOV, asmcode::BYTE_MEM_RCX, asmcode::AL);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RSI);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_string_set_contract_violation);
    }
  }

void compile_vector_length(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_vector(code, asmcode::RCX, asmcode::R11, error);
    }
  //code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, block_size_mask);
  code.add(asmcode::AND, asmcode::RAX, asmcode::R11);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_vector_length_contract_violation);
    }
  }

void compile_string_fill(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  auto length_done = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_string(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::PUSH, asmcode::RCX);
  string_length(code, asmcode::RCX);
  // r15 contains the length, rcx is pointer to string, rdx is character
  code.add(asmcode::POP, asmcode::RCX);

  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::SHR, asmcode::RDX, asmcode::NUMBER, 8); // get character value in dl


  auto fill_rest = label_to_string(label++);
  auto fill_full = label_to_string(label++);
  auto done = label_to_string(label++);
  code.add(asmcode::LABEL, fill_full);
  code.add(asmcode::CMP, asmcode::R15, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::JLS, fill_rest);

  code.add(asmcode::MOV, asmcode::BYTE_MEM_RCX, asmcode::DL);
  code.add(asmcode::MOV, asmcode::BYTE_MEM_RCX, 1, asmcode::DL);
  code.add(asmcode::MOV, asmcode::BYTE_MEM_RCX, 2, asmcode::DL);
  code.add(asmcode::MOV, asmcode::BYTE_MEM_RCX, 3, asmcode::DL);
  code.add(asmcode::MOV, asmcode::BYTE_MEM_RCX, 4, asmcode::DL);
  code.add(asmcode::MOV, asmcode::BYTE_MEM_RCX, 5, asmcode::DL);
  code.add(asmcode::MOV, asmcode::BYTE_MEM_RCX, 6, asmcode::DL);
  code.add(asmcode::MOV, asmcode::BYTE_MEM_RCX, 7, asmcode::DL);
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::SUB, asmcode::R15, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::JMPS, fill_full);

  code.add(asmcode::LABEL, fill_rest);
  code.add(asmcode::MOV, asmcode::MEM_RCX, asmcode::NUMBER, 0);
  code.add(asmcode::CMP, asmcode::R15, asmcode::NUMBER, 0);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::BYTE_MEM_RCX, asmcode::DL);
  code.add(asmcode::CMP, asmcode::R15, asmcode::NUMBER, 1);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::BYTE_MEM_RCX, 1, asmcode::DL);
  code.add(asmcode::CMP, asmcode::R15, asmcode::NUMBER, 2);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::BYTE_MEM_RCX, 2, asmcode::DL);
  code.add(asmcode::CMP, asmcode::R15, asmcode::NUMBER, 3);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::BYTE_MEM_RCX, 3, asmcode::DL);
  code.add(asmcode::CMP, asmcode::R15, asmcode::NUMBER, 4);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::BYTE_MEM_RCX, 4, asmcode::DL);
  code.add(asmcode::CMP, asmcode::R15, asmcode::NUMBER, 5);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::BYTE_MEM_RCX, 5, asmcode::DL);
  code.add(asmcode::CMP, asmcode::R15, asmcode::NUMBER, 6);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::BYTE_MEM_RCX, 6, asmcode::DL);
  code.add(asmcode::CMP, asmcode::R15, asmcode::NUMBER, 7);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::BYTE_MEM_RCX, 7, asmcode::DL);
  code.add(asmcode::LABEL, done);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, skiwi_quiet_undefined);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_string_fill_contract_violation);
    }
  }

void compile_vector_fill(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_vector(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, block_size_mask);
  code.add(asmcode::AND, asmcode::RAX, asmcode::R11);
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));

  auto repeat = label_to_string(label++);
  auto done = label_to_string(label++);

  code.add(asmcode::LABEL, repeat);
  code.add(asmcode::TEST, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::MEM_RCX, asmcode::RDX);
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::DEC, asmcode::RAX);
  code.add(asmcode::JMPS, repeat);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, skiwi_quiet_undefined);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_vector_fill_contract_violation);
    }
  }

void compile_make_vector(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 0);
    code.add(asmcode::JE, error);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 3);
    code.add(asmcode::JGE, error);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::SAR, asmcode::RAX, asmcode::NUMBER, 1);
    code.add(asmcode::INC, asmcode::RAX);
    check_heap(code, re_make_vector_heap_overflow);
    }
  auto done = label_to_string(label++);
  auto fill = label_to_string(label++);
  auto fill_undefined = label_to_string(label++);
  code.add(asmcode::SAR, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, (uint64_t)vector_tag << (uint64_t)block_shift);
  code.add(asmcode::OR, asmcode::RAX, asmcode::R15);
  code.add(asmcode::MOV, asmcode::R15, ALLOC);
  code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
  code.add(asmcode::JES, fill);
  code.add(asmcode::LABEL, fill_undefined);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 0);
  code.add(asmcode::JES, done);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::NUMBER, skiwi_undefined);
  code.add(asmcode::DEC, asmcode::RCX);
  code.add(asmcode::JMPS, fill_undefined);
  code.add(asmcode::LABEL, fill);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 0);
  code.add(asmcode::JES, done);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RDX);
  code.add(asmcode::DEC, asmcode::RCX);
  code.add(asmcode::JMPS, fill);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R15);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_make_vector_contract_violation);
    }
  }

void compile_vector(asmcode& code, const compiler_options& ops)
  {
  auto done = label_to_string(label++);
  auto done2 = label_to_string(label++);
  if (ops.safe_primitives)
    {
    code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);
    code.add(asmcode::INC, asmcode::RAX);
    check_heap(code, re_vector_heap_overflow);
    }
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);
  code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, (uint64_t)vector_tag << (uint64_t)block_shift);
  code.add(asmcode::OR, asmcode::RAX, asmcode::R15);
  code.add(asmcode::MOV, asmcode::R15, ALLOC);
  code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 0);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::RCX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(2), asmcode::RDX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(3), asmcode::RSI);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 3);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(4), asmcode::RDI);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 4);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(5), asmcode::R8);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 5);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(6), asmcode::R9);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 6);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(7), asmcode::R12);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 7);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(8), asmcode::R14);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 8);
  code.add(asmcode::JES, done);

  code.add(asmcode::SUB, asmcode::R11, asmcode::NUMBER, 8);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(9));
  code.add(asmcode::MOV, asmcode::RDX, LOCAL);

  auto lab = label_to_string(label++);
  code.add(asmcode::LABEL, lab);

  code.add(asmcode::MOV, asmcode::RCX, asmcode::MEM_RDX);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RCX);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::DEC, asmcode::R11);
  code.add(asmcode::JE, done2);
  code.add(asmcode::JMPS, lab);

  code.add(asmcode::LABEL, done);
  code.add(asmcode::INC, asmcode::R11);
  code.add(asmcode::SHL, asmcode::R11, asmcode::NUMBER, 3);
  code.add(asmcode::ADD, ALLOC, asmcode::R11);
  code.add(asmcode::LABEL, done2);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R15);
  code.add(asmcode::JMP, CONTINUE);
  }

void compile_slot_ref(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::SAR, asmcode::RDX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    {
    // optionally, check whether rdx is in bounds.
    std::string in_bounds = label_to_string(label++);
    std::string not_in_bounds = label_to_string(label++);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
    code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, block_size_mask);
    code.add(asmcode::AND, asmcode::RAX, asmcode::R15); // get size of vector
    code.add(asmcode::CMP, asmcode::RDX, asmcode::RAX);
    code.add(asmcode::JGES, not_in_bounds);
    code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, 0);
    code.add(asmcode::JGES, in_bounds);
    error_label(code, not_in_bounds, re_slot_ref_out_of_bounds);
    code.add(asmcode::LABEL, in_bounds);
    }

  code.add(asmcode::INC, asmcode::RDX); // increase 1 for header
  code.add(asmcode::SHL, asmcode::RDX, asmcode::NUMBER, 3);
  code.add(asmcode::ADD, asmcode::RCX, asmcode::RDX);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);

  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_slot_ref_contract_violation);
    }
  }

void compile_slot_set(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 3);
    code.add(asmcode::JNES, error);
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::SAR, asmcode::RDX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    {
    // optionally, check whether rdx is in bounds.
    std::string in_bounds = label_to_string(label++);
    std::string not_in_bounds = label_to_string(label++);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
    code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, block_size_mask);
    code.add(asmcode::AND, asmcode::RAX, asmcode::R15); // get size of block
    code.add(asmcode::CMP, asmcode::RDX, asmcode::RAX);
    code.add(asmcode::JGES, not_in_bounds);
    code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, 0);
    code.add(asmcode::JGES, in_bounds);
    error_label(code, not_in_bounds, re_slot_set_out_of_bounds);
    code.add(asmcode::LABEL, in_bounds);
    }

  code.add(asmcode::INC, asmcode::RDX); // increase 1 for header
  code.add(asmcode::SHL, asmcode::RDX, asmcode::NUMBER, 3);
  code.add(asmcode::ADD, asmcode::RCX, asmcode::RDX);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RSI);
  code.add(asmcode::MOV, asmcode::MEM_RCX, asmcode::RAX);

  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_slot_set_contract_violation);
    }
  }

void compile_vector_ref(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  // here check whether it is a vector
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_vector(code, asmcode::RCX, asmcode::R11, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::SAR, asmcode::RDX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    {
    // optionally, check whether rdx is in bounds.
    std::string in_bounds = label_to_string(label++);
    std::string not_in_bounds = label_to_string(label++);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
    code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, block_size_mask);
    code.add(asmcode::AND, asmcode::RAX, asmcode::R15); // get size of vector
    code.add(asmcode::CMP, asmcode::RDX, asmcode::RAX);
    code.add(asmcode::JGES, not_in_bounds);
    code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, 0);
    code.add(asmcode::JGES, in_bounds);
    error_label(code, not_in_bounds, re_vector_ref_out_of_bounds);
    code.add(asmcode::LABEL, in_bounds);
    }

  code.add(asmcode::INC, asmcode::RDX); // increase 1 for header
  code.add(asmcode::SHL, asmcode::RDX, asmcode::NUMBER, 3);
  code.add(asmcode::ADD, asmcode::RCX, asmcode::RDX);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);

  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_vector_ref_contract_violation);
    }
  }

void compile_vector_set(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 3);
    code.add(asmcode::JNES, error);
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  // here check whether it is a vector
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_vector(code, asmcode::RCX, asmcode::R11, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::SAR, asmcode::RDX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    {
    // optionally, check whether rdx is in bounds.
    std::string in_bounds = label_to_string(label++);
    std::string not_in_bounds = label_to_string(label++);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
    code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, block_size_mask);
    code.add(asmcode::AND, asmcode::RAX, asmcode::R15); // get size of vector
    code.add(asmcode::CMP, asmcode::RDX, asmcode::RAX);
    code.add(asmcode::JGES, not_in_bounds);
    code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, 0);
    code.add(asmcode::JGES, in_bounds);
    error_label(code, not_in_bounds, re_vector_set_out_of_bounds);
    code.add(asmcode::LABEL, in_bounds);
    }

  code.add(asmcode::INC, asmcode::RDX); // increase 1 for header
  code.add(asmcode::SHL, asmcode::RDX, asmcode::NUMBER, 3);
  code.add(asmcode::ADD, asmcode::RCX, asmcode::RDX);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RSI);
  code.add(asmcode::MOV, asmcode::MEM_RCX, asmcode::RAX);

  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_vector_set_contract_violation);
    }
  }

void compile_halt(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::JMP, CONTINUE);
  }

void compile_add1(asmcode& code, const compiler_options& ops)
  {
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 0);
  std::string lab_arg_ok = label_to_string(label++);
  std::string lab_is_fixnum = label_to_string(label++);
  std::string error;
  code.add(asmcode::JNES, lab_arg_ok);
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX); // no arguments, return error here
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_arg_ok);
  code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, lab_is_fixnum);

  // here check whether it is a block
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }

  // get address of block
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);

  // here check whether it is a flonum
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::R11, error);
    }

  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RCX, CELLS(1));
  double d = 1.0;
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&d)));
  code.add(asmcode::MOVQ, asmcode::XMM1, asmcode::RAX);
  code.add(asmcode::ADDSD, asmcode::XMM0, asmcode::XMM1);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::R11, ALLOC);
  code.add(asmcode::OR, asmcode::R11, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOVQ, MEM_ALLOC, CELLS(1), asmcode::XMM0);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);

  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_is_fixnum);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 2);
  code.add(asmcode::ADD, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_add1_contract_violation);
    }
  }


void compile_sub1(asmcode& code, const compiler_options& ops)
  {
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 0);
  std::string lab_arg_ok = label_to_string(label++);
  std::string lab_is_fixnum = label_to_string(label++);
  std::string error;
  code.add(asmcode::JNES, lab_arg_ok);
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX); // no arguments, return error here
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_arg_ok);
  code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, lab_is_fixnum);

  // here check whether it is a block
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }

  // get address of block
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);

  // here check whether it is a flonum
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::R11, error);
    }

  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RCX);
  double d = 1.0;
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&d)));
  code.add(asmcode::MOVQ, asmcode::XMM1, asmcode::RAX);
  code.add(asmcode::SUBSD, asmcode::XMM0, asmcode::XMM1);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::R11, ALLOC);
  code.add(asmcode::OR, asmcode::R11, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOVQ, MEM_ALLOC, CELLS(1), asmcode::XMM0);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);


  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_is_fixnum);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, -2);
  code.add(asmcode::ADD, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_sub1_contract_violation);
    }
  }

void compile_bitwise_and_2(asmcode& code, const compiler_options& ops)
  {
  code.add(asmcode::LABEL, "L_bitwise_and_2");
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::TEST, asmcode::RAX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::RET);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_bitwise_and_contract_violation);
    }
  }

void compile_bitwise_or_2(asmcode& code, const compiler_options& ops)
  {
  code.add(asmcode::LABEL, "L_bitwise_or_2");
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::TEST, asmcode::RAX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::OR, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::RET);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_bitwise_or_contract_violation);
    }
  }

void compile_bitwise_xor_2(asmcode& code, const compiler_options& ops)
  {
  code.add(asmcode::LABEL, "L_bitwise_xor_2");
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::TEST, asmcode::RAX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::RET);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_bitwise_xor_contract_violation);
    }
  }

void compile_max_2(asmcode& code, const compiler_options& ops)
  {
  code.add(asmcode::LABEL, "L_max_2");
  auto l1 = label_to_string(label++);
  auto l2 = label_to_string(label++);
  auto l3 = label_to_string(label++);
  auto done = label_to_string(label++);
  std::string error;
  if (ops.safe_primitives)
    error = label_to_string(label++);
  code.add(asmcode::TEST, asmcode::RAX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l1);
  else
    code.add(asmcode::JES, l1); // rax is fixnum
  //rax is flonum
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l3); // rax is flonum and rbx is fixnum
  else
    code.add(asmcode::JES, l3); // rax is flonum and rbx is fixnum
  // rax and rbx are flonum
  // here check whether they are a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  // check whether they contain flonums
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::UCOMISD, asmcode::XMM0, asmcode::XMM1);
  if (ops.safe_primitives)
    code.add(asmcode::JA, done);
  else
    code.add(asmcode::JAS, done);
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::XMM1);
  if (ops.safe_primitives)
    code.add(asmcode::JMP, done);
  else
    code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l1);
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, l2); // rax and rbx are fixnum
  // rax is fixnum and rbx is flonum
  fix2int(code, asmcode::RAX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM0, asmcode::RAX);
  // here check whether RBX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::UCOMISD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::JAS, done);
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l2);
  code.add(asmcode::CMP, asmcode::RAX, asmcode::RBX);
  auto done_fx = label_to_string(label++);
  code.add(asmcode::JGS, done_fx);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::LABEL, done_fx);
  code.add(asmcode::RET);
  code.add(asmcode::JMPS, done);

  code.add(asmcode::LABEL, l3);
  // rax is flonum and rbx is fixnum
  // here check whether RAX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  fix2int(code, asmcode::RBX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM1, asmcode::RBX);
  int2fix(code, asmcode::RBX);
  code.add(asmcode::UCOMISD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::JAS, done);
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::XMM1);
  //code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::MOV, asmcode::RAX, TEMP_FLONUM);
  code.add(asmcode::MOVSD, asmcode::MEM_RAX, CELLS(1), asmcode::XMM0);
  code.add(asmcode::MOV, asmcode::RAX, TEMP_FLONUM);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, block_tag);
  code.add(asmcode::RET);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_max_contract_violation);
    }
  }

void compile_min_2(asmcode& code, const compiler_options& ops)
  {
  code.add(asmcode::LABEL, "L_min_2");
  auto l1 = label_to_string(label++);
  auto l2 = label_to_string(label++);
  auto l3 = label_to_string(label++);
  auto done = label_to_string(label++);
  std::string error;
  if (ops.safe_primitives)
    error = label_to_string(label++);
  code.add(asmcode::TEST, asmcode::RAX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l1);
  else
    code.add(asmcode::JES, l1); // rax is fixnum
  //rax is flonum
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l3); // rax is flonum and rbx is fixnum
  else
    code.add(asmcode::JES, l3); // rax is flonum and rbx is fixnum
  // rax and rbx are flonum
  // here check whether they are a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  // check whether they contain flonums
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::UCOMISD, asmcode::XMM0, asmcode::XMM1);
  if (ops.safe_primitives)
    code.add(asmcode::JB, done);
  else
    code.add(asmcode::JBS, done);
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::XMM1);
  if (ops.safe_primitives)
    code.add(asmcode::JMP, done);
  else
    code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l1);
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, l2); // rax and rbx are fixnum
  // rax is fixnum and rbx is flonum
  fix2int(code, asmcode::RAX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM0, asmcode::RAX);
  // here check whether RBX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::UCOMISD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::JBS, done);
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l2);
  code.add(asmcode::CMP, asmcode::RAX, asmcode::RBX);
  auto done_fx = label_to_string(label++);
  code.add(asmcode::JLS, done_fx);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::LABEL, done_fx);
  code.add(asmcode::RET);
  code.add(asmcode::JMPS, done);

  code.add(asmcode::LABEL, l3);
  // rax is flonum and rbx is fixnum
  // here check whether RAX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  fix2int(code, asmcode::RBX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM1, asmcode::RBX);
  int2fix(code, asmcode::RBX);
  code.add(asmcode::UCOMISD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::JBS, done);
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::XMM1);
  //code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::MOV, asmcode::RAX, TEMP_FLONUM);
  code.add(asmcode::MOVSD, asmcode::MEM_RAX, CELLS(1), asmcode::XMM0);
  code.add(asmcode::MOV, asmcode::RAX, TEMP_FLONUM);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, block_tag);
  code.add(asmcode::RET);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_min_contract_violation);
    }
  }


void compile_add_2(asmcode& code, const compiler_options& ops)
  {
  code.add(asmcode::LABEL, "L_add_2");
  auto l1 = label_to_string(label++);
  auto l2 = label_to_string(label++);
  auto l3 = label_to_string(label++);
  auto done = label_to_string(label++);
  std::string error;
  if (ops.safe_primitives)
    error = label_to_string(label++);
  code.add(asmcode::TEST, asmcode::RAX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l1);
  else
    code.add(asmcode::JES, l1); // rax is fixnum
  //rax is flonum
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l3); // rax is flonum and rbx is fixnum
  else
    code.add(asmcode::JES, l3); // rax is flonum and rbx is fixnum
  // rax and rbx are flonum
  // here check whether they are a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  // check whether they contain flonums
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::ADDSD, asmcode::XMM0, asmcode::XMM1);
  if (ops.safe_primitives)
    code.add(asmcode::JMP, done);
  else
    code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l1);
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, l2); // rax and rbx are fixnum
  // rax is fixnum and rbx is flonum
  fix2int(code, asmcode::RAX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM0, asmcode::RAX);
  // here check whether RBX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_short_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::ADDSD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l2);
  code.add(asmcode::ADD, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::RET);

  code.add(asmcode::LABEL, l3);
  // rax is flonum and rbx is fixnum
  // here check whether RAX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  fix2int(code, asmcode::RBX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM1, asmcode::RBX);
  code.add(asmcode::ADDSD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::MOV, asmcode::RAX, TEMP_FLONUM);
  code.add(asmcode::MOVSD, asmcode::MEM_RAX, CELLS(1), asmcode::XMM0);
  code.add(asmcode::MOV, asmcode::RAX, TEMP_FLONUM);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, block_tag);
  code.add(asmcode::RET);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_add_contract_violation);
    }
  }

void compile_subtract_2(asmcode& code, const compiler_options& ops)
  {
  code.add(asmcode::LABEL, "L_subtract_2");
  auto l1 = label_to_string(label++);
  auto l2 = label_to_string(label++);
  auto l3 = label_to_string(label++);
  auto done = label_to_string(label++);
  std::string error;
  if (ops.safe_primitives)
    error = label_to_string(label++);
  code.add(asmcode::TEST, asmcode::RAX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l1);
  else
    code.add(asmcode::JES, l1); // rax is fixnum
  //rax is flonum
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l3); // rax is flonum and rbx is fixnum
  else
    code.add(asmcode::JES, l3); // rax is flonum and rbx is fixnum
  // rax and rbx are flonum
  // here check whether they are a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  // check whether they contain flonums
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::SUBSD, asmcode::XMM0, asmcode::XMM1);
  if (ops.safe_primitives)
    code.add(asmcode::JMP, done);
  else
    code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l1);
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, l2); // rax and rbx are fixnum
  // rax is fixnum and rbx is flonum
  fix2int(code, asmcode::RAX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM0, asmcode::RAX);
  // here check whether RBX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_short_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error); 
    }
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::SUBSD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l2);
  code.add(asmcode::SUB, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::RET);

  code.add(asmcode::LABEL, l3);
  // rax is flonum and rbx is fixnum
  // here check whether RAX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  fix2int(code, asmcode::RBX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM1, asmcode::RBX);
  code.add(asmcode::SUBSD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::MOV, asmcode::RAX, TEMP_FLONUM);
  code.add(asmcode::MOVSD, asmcode::MEM_RAX, CELLS(1), asmcode::XMM0);
  code.add(asmcode::MOV, asmcode::RAX, TEMP_FLONUM);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, block_tag);
  code.add(asmcode::RET);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_sub_contract_violation);
    }
  }

void compile_multiply_2(asmcode& code, const compiler_options& ops)
  {
  code.add(asmcode::LABEL, "L_multiply_2");
  auto l1 = label_to_string(label++);
  auto l2 = label_to_string(label++);
  auto l3 = label_to_string(label++);
  auto done = label_to_string(label++);
  std::string error;
  if (ops.safe_primitives)
    error = label_to_string(label++);
  code.add(asmcode::TEST, asmcode::RAX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l1);
  else
    code.add(asmcode::JES, l1); // rax is fixnum
  //rax is flonum
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l3); // rax is flonum and rbx is fixnum
  else
    code.add(asmcode::JES, l3); // rax is flonum and rbx is fixnum
  // rax and rbx are flonum
  // here check whether they are a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  // check whether they contain flonums
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::MULSD, asmcode::XMM0, asmcode::XMM1);
  if (ops.safe_primitives)
    code.add(asmcode::JMP, done);
  else
    code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l1);
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, l2); // rax and rbx are fixnum
  // rax is fixnum and rbx is flonum
  fix2int(code, asmcode::RAX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM0, asmcode::RAX);
  // here check whether RBX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_short_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error); 
    }
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::MULSD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l2);
  code.add(asmcode::IMUL, asmcode::RBX);
  code.add(asmcode::SAR, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::RET);

  code.add(asmcode::LABEL, l3);
  // rax is flonum and rbx is fixnum
  // here check whether RAX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  fix2int(code, asmcode::RBX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM1, asmcode::RBX);
  code.add(asmcode::MULSD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::MOV, asmcode::RAX, TEMP_FLONUM);
  code.add(asmcode::MOVSD, asmcode::MEM_RAX, CELLS(1), asmcode::XMM0);
  code.add(asmcode::MOV, asmcode::RAX, TEMP_FLONUM);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, block_tag);
  code.add(asmcode::RET);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_mul_contract_violation);
    }
  }


void compile_divide_2(asmcode& code, const compiler_options& ops)
  {
  code.add(asmcode::LABEL, "L_divide_2");
  auto l1 = label_to_string(label++);
  auto l2 = label_to_string(label++);
  auto l3 = label_to_string(label++);
  auto done = label_to_string(label++);
  std::string error;
  if (ops.safe_primitives)
    error = label_to_string(label++);
  code.add(asmcode::TEST, asmcode::RAX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l1);
  else
    code.add(asmcode::JES, l1); // rax is fixnum
  //rax is flonum
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l3); // rax is flonum and rbx is fixnum
  else
    code.add(asmcode::JES, l3); // rax is flonum and rbx is fixnum
  // rax and rbx are flonum
  // here check whether they are a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  // check whether they contain flonums
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::DIVSD, asmcode::XMM0, asmcode::XMM1);
  if (ops.safe_primitives)
    code.add(asmcode::JMP, done);
  else
    code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l1);
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, l2); // rax and rbx are fixnum
  // rax is fixnum and rbx is flonum
  fix2int(code, asmcode::RAX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM0, asmcode::RAX);
  // here check whether RBX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error); 
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::DIVSD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l2);
  fix2int(code, asmcode::RAX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM0, asmcode::RAX);
  fix2int(code, asmcode::RBX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM1, asmcode::RBX);
  code.add(asmcode::DIVSD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l3);
  // rax is flonum and rbx is fixnum
  // here check whether RAX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  fix2int(code, asmcode::RBX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM1, asmcode::RBX);
  code.add(asmcode::DIVSD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::MOV, asmcode::RAX, TEMP_FLONUM);
  code.add(asmcode::MOVSD, asmcode::MEM_RAX, CELLS(1), asmcode::XMM0);
  code.add(asmcode::MOV, asmcode::RAX, TEMP_FLONUM);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, block_tag);
  code.add(asmcode::RET);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_div_contract_violation);
    }
  }

void compile_pairwise_compare(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::LABEL, "L_pairwise_compare");
  code.add(asmcode::PUSH, asmcode::RBX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
  std::string lab_arg_ok = label_to_string(label++);
  std::string lab_arg_no = label_to_string(label++);
  std::string lab_arg_yes = label_to_string(label++);
  code.add(asmcode::JGS, lab_arg_ok);

  code.add(asmcode::LABEL, lab_arg_no);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f); // less than 2 arguments, return false
  code.add(asmcode::POP, asmcode::RBX);
  code.add(asmcode::JMP, CONTINUE);

  code.add(asmcode::LABEL, lab_arg_ok);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX); // 1st arg
  code.add(asmcode::MOV, asmcode::RBX, asmcode::RDX); // 2nd arg
  code.add(asmcode::CALL, asmcode::R15);
  code.add(asmcode::CMP, asmcode::AL, asmcode::NUMBER, bool_f);
  code.add(asmcode::JES, lab_arg_no);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
  code.add(asmcode::JE, lab_arg_yes);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::MOV, asmcode::RBX, asmcode::RSI); // 3th arg
  code.add(asmcode::CALL, asmcode::R15);
  code.add(asmcode::CMP, asmcode::AL, asmcode::NUMBER, bool_f);
  code.add(asmcode::JES, lab_arg_no);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 3);
  code.add(asmcode::JE, lab_arg_yes);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::MOV, asmcode::RBX, asmcode::RDI); // 4th arg
  code.add(asmcode::CALL, asmcode::R15);
  code.add(asmcode::CMP, asmcode::AL, asmcode::NUMBER, bool_f);
  code.add(asmcode::JES, lab_arg_no);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 4);
  code.add(asmcode::JE, lab_arg_yes); // JES works for assembly but not for vm

  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::MOV, asmcode::RBX, asmcode::R8); // 5th arg
  code.add(asmcode::CALL, asmcode::R15);
  code.add(asmcode::CMP, asmcode::AL, asmcode::NUMBER, bool_f);
  code.add(asmcode::JES, lab_arg_no);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 5);
  code.add(asmcode::JES, lab_arg_yes); 

  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::MOV, asmcode::RBX, asmcode::R9); // 6th arg
  code.add(asmcode::CALL, asmcode::R15);
  code.add(asmcode::CMP, asmcode::AL, asmcode::NUMBER, bool_f);
  code.add(asmcode::JES, lab_arg_no); 
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 6);
  code.add(asmcode::JES, lab_arg_yes);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::MOV, asmcode::RBX, asmcode::R12); // 7th arg
  code.add(asmcode::CALL, asmcode::R15);
  code.add(asmcode::CMP, asmcode::AL, asmcode::NUMBER, bool_f);
  code.add(asmcode::JE, lab_arg_no);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 7);
  code.add(asmcode::JES, lab_arg_yes);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::MOV, asmcode::RBX, asmcode::R14); // 8th arg
  code.add(asmcode::CALL, asmcode::R15);
  code.add(asmcode::CMP, asmcode::AL, asmcode::NUMBER, bool_f);
  code.add(asmcode::JE, lab_arg_no);
  code.add(asmcode::SUB, asmcode::R11, asmcode::NUMBER, 8);
  code.add(asmcode::TEST, asmcode::R11, asmcode::R11);
  code.add(asmcode::JES, lab_arg_yes);
  code.add(asmcode::MOV, asmcode::RDX, LOCAL);
  auto lab3 = label_to_string(label++);
  code.add(asmcode::LABEL, lab3);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::MOV, asmcode::RBX, asmcode::MEM_RDX);
  code.add(asmcode::CALL, asmcode::R15);
  code.add(asmcode::CMP, asmcode::AL, asmcode::NUMBER, bool_f);
  code.add(asmcode::JE, lab_arg_no);
  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::DEC, asmcode::R11);
  code.add(asmcode::JES, lab_arg_yes);
  code.add(asmcode::JMPS, lab3);

  code.add(asmcode::LABEL, lab_arg_yes);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t); // less than 2 arguments, return false
  code.add(asmcode::POP, asmcode::RBX);
  code.add(asmcode::JMP, CONTINUE);
  }

void compile_fold_binary(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::LABEL, "L_fold_binary_operation");
  code.add(asmcode::PUSH, asmcode::RBX);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX); // 1st arg
  code.add(asmcode::MOV, asmcode::RBX, asmcode::RDX); // 2nd arg
  code.add(asmcode::CALL, asmcode::R15);

  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
  auto lab1 = label_to_string(label++);
  auto lab2 = label_to_string(label++);
  auto lab3 = label_to_string(label++);
  auto done = label_to_string(label++);
  code.add(asmcode::JNES, lab1);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::TEST, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, lab2);
  code.add(asmcode::MOV, asmcode::R11, TEMP_FLONUM);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_R11);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_R11, CELLS(1));
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::RAX);
  code.add(asmcode::MOV, asmcode::RAX, ALLOC);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, block_tag);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::LABEL, lab2);
  code.add(asmcode::POP, asmcode::RBX);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab1);
  code.add(asmcode::MOV, asmcode::RBX, asmcode::RSI); //3rd arg
  code.add(asmcode::CALL, asmcode::R15);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 3);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::RBX, asmcode::RDI); //4th arg
  code.add(asmcode::CALL, asmcode::R15);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 4);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::RBX, asmcode::R8); //5th arg
  code.add(asmcode::CALL, asmcode::R15);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 5);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::RBX, asmcode::R9); //6th arg
  code.add(asmcode::CALL, asmcode::R15);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 6);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::RBX, asmcode::R12); //7th arg
  code.add(asmcode::CALL, asmcode::R15);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 7);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::RBX, asmcode::R14); //8th arg
  code.add(asmcode::CALL, asmcode::R15);
  code.add(asmcode::SUB, asmcode::R11, asmcode::NUMBER, 8);
  code.add(asmcode::MOV, asmcode::RDX, LOCAL);
  code.add(asmcode::TEST, asmcode::R11, asmcode::R11);
  code.add(asmcode::JES, done); 
  code.add(asmcode::LABEL, lab3);
  code.add(asmcode::MOV, asmcode::RBX, asmcode::MEM_RDX);
  code.add(asmcode::CALL, asmcode::R15);
  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::DEC, asmcode::R11);
  code.add(asmcode::JE, done);
  code.add(asmcode::JMPS, lab3);
  }

void compile_bitwise_and(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
  std::string lab_arg_ok = label_to_string(label++);
  std::string lab_one_arg = label_to_string(label++);
  code.add(asmcode::JGS, lab_arg_ok);
  code.add(asmcode::JES, lab_one_arg);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, (uint64_t)re_bitwise_and_contract_violation);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 8);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, error_tag);
  code.add(asmcode::JMP, ERROR);
  code.add(asmcode::LABEL, lab_one_arg); // only 1 argument
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 0xffffffffffffffff);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_arg_ok);
  code.add(asmcode::MOV, asmcode::R15, asmcode::LABELADDRESS, "L_bitwise_and_2");
  code.add(asmcode::JMP, "L_fold_binary_operation");
  }

void compile_bitwise_not(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::XOR, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFFF);
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFFE);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_bitwise_not_contract_violation);
    }
  }

void compile_arithmetic_shift(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  auto shift_right = label_to_string(label++);
  code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, 0);
  code.add(asmcode::JLS, shift_right);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::SHR, asmcode::RDX, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RCX, asmcode::RDX);
  code.add(asmcode::SAL, asmcode::RAX, asmcode::CL);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, shift_right);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::SAR, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::NEG, asmcode::RDX);
  code.add(asmcode::SHR, asmcode::RDX, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RCX, asmcode::RDX);
  code.add(asmcode::SAR, asmcode::RAX, asmcode::CL);
  code.add(asmcode::SAL, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_arithmetic_shift_contract_violation);
    }
  }

void compile_bitwise_or(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
  std::string lab_arg_ok = label_to_string(label++);
  std::string lab_one_arg = label_to_string(label++);
  code.add(asmcode::JGS, lab_arg_ok);
  code.add(asmcode::JES, lab_one_arg);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, (uint64_t)re_bitwise_or_contract_violation);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 8);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, error_tag);
  code.add(asmcode::JMP, ERROR);
  code.add(asmcode::LABEL, lab_one_arg); // only 1 argument
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_arg_ok);
  code.add(asmcode::MOV, asmcode::R15, asmcode::LABELADDRESS, "L_bitwise_or_2");
  code.add(asmcode::JMP, "L_fold_binary_operation");
  }

void compile_bitwise_xor(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
  std::string lab_arg_ok = label_to_string(label++);
  std::string lab_one_arg = label_to_string(label++);
  code.add(asmcode::JGS, lab_arg_ok);
  code.add(asmcode::JES, lab_one_arg);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, (uint64_t)re_bitwise_xor_contract_violation);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 8);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, error_tag);
  code.add(asmcode::JMP, ERROR);
  code.add(asmcode::LABEL, lab_one_arg); // only 1 argument
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_arg_ok);
  code.add(asmcode::MOV, asmcode::R15, asmcode::LABELADDRESS, "L_bitwise_xor_2");
  code.add(asmcode::JMP, "L_fold_binary_operation");
  }

void compile_max(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
  std::string lab_arg_ok = label_to_string(label++);
  std::string lab_one_arg = label_to_string(label++);
  code.add(asmcode::JGS, lab_arg_ok);
  code.add(asmcode::JES, lab_one_arg);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, (uint64_t)re_max_contract_violation);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 8);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, error_tag);
  code.add(asmcode::JMP, ERROR);
  code.add(asmcode::LABEL, lab_one_arg); // only 1 argument
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_arg_ok);
  code.add(asmcode::MOV, asmcode::R15, asmcode::LABELADDRESS, "L_max_2");
  code.add(asmcode::JMP, "L_fold_binary_operation");
  }

void compile_min(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
  std::string lab_arg_ok = label_to_string(label++);
  std::string lab_one_arg = label_to_string(label++);
  code.add(asmcode::JGS, lab_arg_ok);
  code.add(asmcode::JES, lab_one_arg);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, (uint64_t)re_min_contract_violation);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 8);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, error_tag);
  code.add(asmcode::JMP, ERROR);
  code.add(asmcode::LABEL, lab_one_arg); // only 1 argument
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_arg_ok);
  code.add(asmcode::MOV, asmcode::R15, asmcode::LABELADDRESS, "L_min_2");
  code.add(asmcode::JMP, "L_fold_binary_operation");
  }

void compile_add(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
  std::string lab_arg_ok = label_to_string(label++);
  std::string lab_one_arg = label_to_string(label++);
  code.add(asmcode::JGS, lab_arg_ok);
  code.add(asmcode::JES, lab_one_arg);
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX); // no arguments, return error here
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_one_arg); // only 1 argument
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_arg_ok);
  code.add(asmcode::MOV, asmcode::R15, asmcode::LABELADDRESS, "L_add_2");
  code.add(asmcode::JMP, "L_fold_binary_operation");
  }

void compile_sub(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    }
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
  std::string lab_arg_ok = label_to_string(label++);
  std::string lab_one_arg = label_to_string(label++);
  code.add(asmcode::JGS, lab_arg_ok); 
  code.add(asmcode::JES, lab_one_arg);
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX); // no arguments, return error here
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_one_arg); // only 1 argument
  code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
  std::string rcx_is_fixnum = label_to_string(label++);
  std::string done = label_to_string(label++);
  code.add(asmcode::JES, rcx_is_fixnum);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RCX, CELLS(1));
  code.add(asmcode::MOV, asmcode::RCX, asmcode::NUMBER, 0x8000000000000000);
  code.add(asmcode::MOVQ, asmcode::XMM1, asmcode::RCX);
  code.add(asmcode::XORPD, asmcode::XMM0, asmcode::XMM1);
  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOVSD, MEM_ALLOC, CELLS(1), asmcode::XMM0);
  code.add(asmcode::MOV, asmcode::RAX, ALLOC);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, block_tag);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::JMP, CONTINUE);

  code.add(asmcode::LABEL, rcx_is_fixnum);
  code.add(asmcode::NEG, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_arg_ok);
  code.add(asmcode::MOV, asmcode::R15, asmcode::LABELADDRESS, "L_subtract_2");
  code.add(asmcode::JMP, "L_fold_binary_operation");
  if (ops.safe_primitives)
    {
    error_label(code, error, re_sub_contract_violation);
    }
  }

void compile_mul(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
  std::string lab_arg_ok = label_to_string(label++);
  std::string lab_one_arg = label_to_string(label++);
  code.add(asmcode::JGS, lab_arg_ok);
  code.add(asmcode::JES, lab_one_arg);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 2); // no arguments, return error here
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_one_arg); // only 1 argument
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_arg_ok);
  code.add(asmcode::MOV, asmcode::R15, asmcode::LABELADDRESS, "L_multiply_2");
  code.add(asmcode::JMP, "L_fold_binary_operation");
  }

void compile_div(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    }
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
  std::string lab_arg_ok = label_to_string(label++);
  std::string lab_one_arg = label_to_string(label++);
  code.add(asmcode::JGS, lab_arg_ok); 
  code.add(asmcode::JES, lab_one_arg);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 2); // no arguments, return error here
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_one_arg); // only 1 argument
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  double d = 1.0;
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&d)));
  code.add(asmcode::MOVQ, asmcode::XMM0, asmcode::RAX);
  code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
  std::string rcx_is_fixnum = label_to_string(label++);
  std::string done = label_to_string(label++);
  code.add(asmcode::JES, rcx_is_fixnum);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RCX, CELLS(1));
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, rcx_is_fixnum);
  fix2int(code, asmcode::RCX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM1, asmcode::RCX);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::DIVSD, asmcode::XMM0, asmcode::XMM1);
  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOVSD, MEM_ALLOC, CELLS(1), asmcode::XMM0);
  code.add(asmcode::MOV, asmcode::RAX, ALLOC);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, block_tag);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_arg_ok);
  code.add(asmcode::MOV, asmcode::R15, asmcode::LABELADDRESS, "L_divide_2");
  code.add(asmcode::JMP, "L_fold_binary_operation");
  if (ops.safe_primitives)
    {
    error_label(code, error, re_div_contract_violation);
    }
  }

void compile_equal(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::MOV, asmcode::R15, asmcode::LABELADDRESS, "L_equal_2");
  code.add(asmcode::JMP, "L_pairwise_compare");
  }

void compile_not_equal(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::MOV, asmcode::R15, asmcode::LABELADDRESS, "L_not_equal_2");
  code.add(asmcode::JMP, "L_pairwise_compare");
  }

void compile_less(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::MOV, asmcode::R15, asmcode::LABELADDRESS, "L_less_2");
  code.add(asmcode::JMP, "L_pairwise_compare");
  }

void compile_leq(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::MOV, asmcode::R15, asmcode::LABELADDRESS, "L_leq_2");
  code.add(asmcode::JMP, "L_pairwise_compare");
  }

void compile_geq(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::MOV, asmcode::R15, asmcode::LABELADDRESS, "L_geq_2");
  code.add(asmcode::JMP, "L_pairwise_compare");
  }

void compile_greater(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::MOV, asmcode::R15, asmcode::LABELADDRESS, "L_greater_2");
  code.add(asmcode::JMP, "L_pairwise_compare");
  }

void compile_equal_2(asmcode& code, const compiler_options& ops)
  {
  code.add(asmcode::LABEL, "L_equal_2");
  auto l1 = label_to_string(label++);
  auto l2 = label_to_string(label++);
  auto l3 = label_to_string(label++);
  auto done = label_to_string(label++);
  std::string error;
  if (ops.safe_primitives)
    error = label_to_string(label++);
  code.add(asmcode::TEST, asmcode::RAX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l1);
  else
    code.add(asmcode::JES, l1); // rax is fixnum
  //rax is flonum
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l3); // rax is flonum and rbx is fixnum
  else
    code.add(asmcode::JES, l3); // rax is flonum and rbx is fixnum
  // rax and rbx are flonum
  // here check whether they are a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  // check whether they contain flonums
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::CMPEQPD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM0);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  if (ops.safe_primitives)
    code.add(asmcode::JMP, done);
  else
    code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l1);
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, l2); // rax and rbx are fixnum
  // rax is fixnum and rbx is flonum
  fix2int(code, asmcode::RAX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM0, asmcode::RAX);
  // here check whether RBX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::CMPEQPD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM0);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l2);
  code.add(asmcode::CMP, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::SETE, asmcode::AL);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::JMPS, done);

  code.add(asmcode::LABEL, l3);
  // rax is flonum and rbx is fixnum
  // here check whether RAX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  fix2int(code, asmcode::RBX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM1, asmcode::RBX);
  int2fix(code, asmcode::RBX);
  code.add(asmcode::CMPEQPD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM0);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::RET);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_equal_contract_violation);
    }
  }

void compile_not_equal_2(asmcode& code, const compiler_options& ops)
  {
  code.add(asmcode::LABEL, "L_not_equal_2");
  auto l1 = label_to_string(label++);
  auto l2 = label_to_string(label++);
  auto l3 = label_to_string(label++);
  auto done = label_to_string(label++);
  std::string error;
  if (ops.safe_primitives)
    error = label_to_string(label++);
  code.add(asmcode::TEST, asmcode::RAX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l1);
  else
    code.add(asmcode::JES, l1); // rax is fixnum
  //rax is flonum
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l3); // rax is flonum and rbx is fixnum
  else
    code.add(asmcode::JES, l3); // rax is flonum and rbx is fixnum
  // rax and rbx are flonum
  // here check whether they are a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  // check whether they contain flonums
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::CMPEQPD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM0);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  if (ops.safe_primitives)
    code.add(asmcode::JMP, done);
  else
    code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l1);
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, l2); // rax and rbx are fixnum
  // rax is fixnum and rbx is flonum
  fix2int(code, asmcode::RAX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM0, asmcode::RAX);
  // here check whether RBX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::CMPEQPD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM0);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l2);
  code.add(asmcode::CMP, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::SETE, asmcode::AL);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::JMPS, done);

  code.add(asmcode::LABEL, l3);
  // rax is flonum and rbx is fixnum
  // here check whether RAX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  fix2int(code, asmcode::RBX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM1, asmcode::RBX);
  int2fix(code, asmcode::RBX);
  code.add(asmcode::CMPEQPD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM0);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::XOR, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::RET);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_not_equal_contract_violation);
    }
  }

void compile_less_2(asmcode& code, const compiler_options& ops)
  {
  code.add(asmcode::LABEL, "L_less_2");
  auto l1 = label_to_string(label++);
  auto l2 = label_to_string(label++);
  auto l3 = label_to_string(label++);
  auto done = label_to_string(label++);
  std::string error;
  if (ops.safe_primitives)
    error = label_to_string(label++);
  code.add(asmcode::TEST, asmcode::RAX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l1);
  else
    code.add(asmcode::JES, l1); // rax is fixnum
  //rax is flonum
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l3); // rax is flonum and rbx is fixnum
  else
    code.add(asmcode::JES, l3); // rax is flonum and rbx is fixnum
  // rax and rbx are flonum
  // here check whether they are a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  // check whether they contain flonums
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::CMPLTPD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM0);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  if (ops.safe_primitives)
    code.add(asmcode::JMP, done);
  else
    code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l1);
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, l2); // rax and rbx are fixnum
  // rax is fixnum and rbx is flonum
  fix2int(code, asmcode::RAX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM0, asmcode::RAX);
  // here check whether RBX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::CMPLTPD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM0);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l2);
  code.add(asmcode::CMP, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::SETL, asmcode::AL);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::JMPS, done);

  code.add(asmcode::LABEL, l3);
  // rax is flonum and rbx is fixnum
  // here check whether RAX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  fix2int(code, asmcode::RBX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM1, asmcode::RBX);
  int2fix(code, asmcode::RBX);
  code.add(asmcode::CMPLTPD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM0);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::RET);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_less_contract_violation);
    }
  }

void compile_leq_2(asmcode& code, const compiler_options& ops)
  {
  code.add(asmcode::LABEL, "L_leq_2");
  auto l1 = label_to_string(label++);
  auto l2 = label_to_string(label++);
  auto l3 = label_to_string(label++);
  auto done = label_to_string(label++);
  std::string error;
  if (ops.safe_primitives)
    error = label_to_string(label++);
  code.add(asmcode::TEST, asmcode::RAX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l1);
  else
    code.add(asmcode::JES, l1); // rax is fixnum
  //rax is flonum
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l3); // rax is flonum and rbx is fixnum
  else
    code.add(asmcode::JES, l3); // rax is flonum and rbx is fixnum
  // rax and rbx are flonum
  // here check whether they are a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  // check whether they contain flonums
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::CMPLEPD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM0);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  if (ops.safe_primitives)
    code.add(asmcode::JMP, done);
  else
    code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l1);
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, l2); // rax and rbx are fixnum
  // rax is fixnum and rbx is flonum
  fix2int(code, asmcode::RAX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM0, asmcode::RAX);
  // here check whether RBX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::CMPLEPD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM0);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l2);
  code.add(asmcode::CMP, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::SETLE, asmcode::AL);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::JMPS, done);

  code.add(asmcode::LABEL, l3);
  // rax is flonum and rbx is fixnum
  // here check whether RAX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  fix2int(code, asmcode::RBX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM1, asmcode::RBX);
  int2fix(code, asmcode::RBX);
  code.add(asmcode::CMPLEPD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM0);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::RET);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_leq_contract_violation);
    }
  }

void compile_geq_2(asmcode& code, const compiler_options& ops)
  {
  code.add(asmcode::LABEL, "L_geq_2");
  auto l1 = label_to_string(label++);
  auto l2 = label_to_string(label++);
  auto l3 = label_to_string(label++);
  auto done = label_to_string(label++);
  std::string error;
  if (ops.safe_primitives)
    error = label_to_string(label++);
  code.add(asmcode::TEST, asmcode::RAX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l1);
  else
    code.add(asmcode::JES, l1); // rax is fixnum
  //rax is flonum
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l3); // rax is flonum and rbx is fixnum
  else
    code.add(asmcode::JES, l3); // rax is flonum and rbx is fixnum
  // rax and rbx are flonum
  // here check whether they are a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  // check whether they contain flonums
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::CMPLEPD, asmcode::XMM1, asmcode::XMM0);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM1);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  if (ops.safe_primitives)
    code.add(asmcode::JMP, done);
  else
    code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l1);
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, l2); // rax and rbx are fixnum
  // rax is fixnum and rbx is flonum
  fix2int(code, asmcode::RAX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM0, asmcode::RAX);
  // here check whether RBX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::CMPLEPD, asmcode::XMM1, asmcode::XMM0);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM1);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l2);
  code.add(asmcode::CMP, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::SETGE, asmcode::AL);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::JMPS, done);

  code.add(asmcode::LABEL, l3);
  // rax is flonum and rbx is fixnum
  // here check whether RAX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  fix2int(code, asmcode::RBX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM1, asmcode::RBX);
  int2fix(code, asmcode::RBX);
  code.add(asmcode::CMPLEPD, asmcode::XMM1, asmcode::XMM0);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM1);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::RET);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_geq_contract_violation);
    }
  }

void compile_greater_2(asmcode& code, const compiler_options& ops)
  {
  code.add(asmcode::LABEL, "L_greater_2");
  auto l1 = label_to_string(label++);
  auto l2 = label_to_string(label++);
  auto l3 = label_to_string(label++);
  auto done = label_to_string(label++);
  std::string error;
  if (ops.safe_primitives)
    error = label_to_string(label++);
  code.add(asmcode::TEST, asmcode::RAX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l1);
  else
    code.add(asmcode::JES, l1); // rax is fixnum
  //rax is flonum
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    code.add(asmcode::JE, l3); // rax is flonum and rbx is fixnum
  else
    code.add(asmcode::JES, l3); // rax is flonum and rbx is fixnum
  // rax and rbx are flonum
  // here check whether they are a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  // check whether they contain flonums
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::CMPLTPD, asmcode::XMM1, asmcode::XMM0);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM1);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  if (ops.safe_primitives)
    code.add(asmcode::JMP, done);
  else
    code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l1);
  code.add(asmcode::TEST, asmcode::RBX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, l2); // rax and rbx are fixnum
  // rax is fixnum and rbx is flonum
  fix2int(code, asmcode::RAX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM0, asmcode::RAX);
  // here check whether RBX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_if_arg_is_not_block(code, asmcode::RBX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RBX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::CMPLTPD, asmcode::XMM1, asmcode::XMM0);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM1);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l2);
  code.add(asmcode::CMP, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::SETG, asmcode::AL);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);

  code.add(asmcode::JMPS, done);

  code.add(asmcode::LABEL, l3);
  // rax is flonum and rbx is fixnum
  // here check whether RAX is a block
  if (ops.safe_primitives)
    {
    code.add(asmcode::PUSH, asmcode::R11);
    jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::R11, error);
    code.add(asmcode::POP, asmcode::R11);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  fix2int(code, asmcode::RBX);
  code.add(asmcode::CVTSI2SD, asmcode::XMM1, asmcode::RBX);
  int2fix(code, asmcode::RBX);
  code.add(asmcode::CMPLTPD, asmcode::XMM1, asmcode::XMM0);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM1);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::RET);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_greater_contract_violation);
    }
  }

void compile_not(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, bool_f);
  // if comparison (last cmp call) succeeded, then sete puts 1 in al, otherwise 0
  code.add(asmcode::SETE, asmcode::AL);
  // movzx extends the bits in al to the full rax register
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  }

void compile_is_fixnum(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::SETE, asmcode::AL);
  // movzx extends the bits in al to the full rax register
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  }

void compile_is_flonum(asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::RAX, lab_false);
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::RCX, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  }

void compile_is_vector(asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::RAX, lab_false);
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_vector(code, asmcode::RCX, asmcode::RCX, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  }

void compile_is_pair(asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::RAX, lab_false);
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_pair(code, asmcode::RCX, asmcode::RCX, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  }

void compile_is_string(asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::RAX, lab_false);
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_string(code, asmcode::RCX, asmcode::RCX, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  }

void compile_is_symbol(asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::RAX, lab_false);
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_symbol(code, asmcode::RCX, asmcode::RCX, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  }

void compile_is_promise(asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::RAX, lab_false);
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_promise(code, asmcode::RCX, asmcode::RCX, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  }

void compile_is_closure(asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::RAX, lab_false);
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_closure(code, asmcode::RCX, asmcode::RCX, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  }

void compile_is_procedure(asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  auto check_closure = label_to_string(label++);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, procedure_mask);
  code.add(asmcode::CMP, asmcode::RAX, asmcode::NUMBER, procedure_tag);
  code.add(asmcode::JNES, check_closure);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, check_closure);
  jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::RAX, lab_false);
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_closure(code, asmcode::RCX, asmcode::RCX, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  }

void compile_is_nil(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, nil);
  code.add(asmcode::SETE, asmcode::AL);
  // movzx extends the bits in al to the full rax register
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  }

void compile_is_eof_object(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, eof_tag);
  code.add(asmcode::SETE, asmcode::AL);
  // movzx extends the bits in al to the full rax register
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  }

void compile_is_char(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::CMP, asmcode::CL, asmcode::NUMBER, char_tag);
  // if comparison (last cmp call) succeeded, then sete puts 1 in al, otherwise 0
  code.add(asmcode::SETE, asmcode::AL);
  // movzx extends the bits in al to the full rax register
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  }

void compile_is_boolean(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::CL, asmcode::NUMBER, 247);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, bool_f);
  code.add(asmcode::SETE, asmcode::AL);
  // movzx extends the bits in al to the full rax register
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  }

void compile_is_zero(asmcode& code, const compiler_options& ops)
  {
  auto l1 = label_to_string(label++);
  auto l2 = label_to_string(label++);
  auto l3 = label_to_string(label++);
  auto done = label_to_string(label++);
  std::string error;
  if (ops.safe_primitives)
    error = label_to_string(label++);
  code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, l1); // rcx is fixnum
  //rcx is flonum  
  //here check whether rcx is a block
  if (ops.safe_primitives)
    {
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  // check whether it contains a flonum
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RCX, CELLS(1));
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::MOVQ, asmcode::XMM1, asmcode::RAX);
  code.add(asmcode::CMPEQPD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM0);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, l1);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 0);
  code.add(asmcode::SETE, asmcode::AL);
  // movzx extends the bits in al to the full rax register
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_is_zero_contract_violation);
    }
  }

void compile_length(asmcode& code, const compiler_options& ops)
  {
  auto lab_start = label_to_string(label++);
  auto lab_finish = label_to_string(label++);
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::XOR, asmcode::R15, asmcode::R15);
  code.add(asmcode::LABEL, lab_start);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, nil);
  code.add(asmcode::JES, lab_finish);
  if (ops.safe_primitives)
    {
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_pair(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::INC, asmcode::R15);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX, CELLS(2));
  code.add(asmcode::MOV, asmcode::RCX, asmcode::RAX);
  code.add(asmcode::JMPS, lab_start);
  code.add(asmcode::LABEL, lab_finish);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R15);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_length_contract_violation);
    }
  }

void compile_list(asmcode& code, const compiler_options& ops)
  {
  std::string not_empty = label_to_string(label++);
  std::string done = label_to_string(label++);
  std::string repeat = label_to_string(label++);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 0);
  code.add(asmcode::JNES, not_empty);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, nil);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, not_empty);

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
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::RCX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
  code.add(asmcode::JE, done);

  code.add(asmcode::MOV, asmcode::RCX, ALLOC);
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(3));
  code.add(asmcode::OR, asmcode::RCX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(2), asmcode::RCX);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(3));
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::RDX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
  code.add(asmcode::JE, done);

  code.add(asmcode::MOV, asmcode::RCX, ALLOC);
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(3));
  code.add(asmcode::OR, asmcode::RCX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(2), asmcode::RCX);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(3));
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::RSI);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 3);
  code.add(asmcode::JE, done);

  code.add(asmcode::MOV, asmcode::RCX, ALLOC);
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(3));
  code.add(asmcode::OR, asmcode::RCX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(2), asmcode::RCX);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(3));
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::RDI);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 4);
  code.add(asmcode::JE, done);

  code.add(asmcode::MOV, asmcode::RCX, ALLOC);
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(3));
  code.add(asmcode::OR, asmcode::RCX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(2), asmcode::RCX);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(3));
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::R8);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 5);
  code.add(asmcode::JE, done);

  code.add(asmcode::MOV, asmcode::RCX, ALLOC);
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(3));
  code.add(asmcode::OR, asmcode::RCX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(2), asmcode::RCX);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(3));
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::R9);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 6);
  code.add(asmcode::JE, done);

  code.add(asmcode::MOV, asmcode::RCX, ALLOC);
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(3));
  code.add(asmcode::OR, asmcode::RCX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(2), asmcode::RCX);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(3));
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::R12);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 7);
  code.add(asmcode::JE, done);

  code.add(asmcode::MOV, asmcode::RCX, ALLOC);
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(3));
  code.add(asmcode::OR, asmcode::RCX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(2), asmcode::RCX);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(3));
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::R14);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 8);
  code.add(asmcode::JE, done);

  code.add(asmcode::SUB, asmcode::R11, asmcode::NUMBER, 8);
  code.add(asmcode::MOV, asmcode::RDX, LOCAL);
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
  code.add(asmcode::JES, done);
  code.add(asmcode::JMPS, repeat);

  code.add(asmcode::LABEL, done);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, nil);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(2), asmcode::RAX);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(3));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R15);
  code.add(asmcode::JMP, CONTINUE);


  }

void compile_cons(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    error = label_to_string(label++);
  uint64_t header = make_block_header(2, T_PAIR);
  if (ops.safe_primitives)
    {
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    if (ops.safe_cons)
      {
      code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 3);
      check_heap(code, re_cons_heap_overflow);
      }
    }
  code.add(asmcode::MOV, asmcode::R11, ALLOC);
  code.add(asmcode::OR, asmcode::R11, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::RCX);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(2), asmcode::RDX);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(3));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_cons_contract_violation);
    }
  }

void compile_set_car(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    error = label_to_string(label++);
  if (ops.safe_primitives)
    {
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  // check whether it contains a pair
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_pair(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::MOV, asmcode::MEM_RCX, CELLS(1), asmcode::RDX);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_set_car_contract_violation);
    }
  }

void compile_set_cdr(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    error = label_to_string(label++);
  if (ops.safe_primitives)
    {
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  // check whether it contains a pair
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_pair(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::MOV, asmcode::MEM_RCX, CELLS(2), asmcode::RDX);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_set_cdr_contract_violation);
    }
  }

void compile_car(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    error = label_to_string(label++);
  if (ops.safe_primitives)
    {
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  // check whether it contains a pair
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_pair(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX, CELLS(1));
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_car_contract_violation);
    }
  }

void compile_cdr(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    error = label_to_string(label++);
  if (ops.safe_primitives)
    {
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  // check whether it contains a pair
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_pair(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX, CELLS(2));
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_cdr_contract_violation);
    }
  }

void compile_mark(asmcode& code, const compiler_options&)
  {
  /*mark should not change rax or r11. clobbers rbx, rdx, r8, r9*/
  code.add(asmcode::LABEL, "L_mark");
  auto done = label_to_string(label++);
  auto not_marked_yet = label_to_string(label++);


  code.add(asmcode::MOV, asmcode::R15, asmcode::MEM_RAX);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::R15);
  code.add(asmcode::AND, asmcode::RDX, asmcode::NUMBER, block_mask);
  code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, block_tag);
  code.add(asmcode::JNE, done);
  code.add(asmcode::AND, asmcode::R15, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::CMP, asmcode::R15, FROM_SPACE);
  code.add(asmcode::JL, done);
  code.add(asmcode::CMP, asmcode::R15, FROM_SPACE_END);
  code.add(asmcode::JGE, done);
  code.add(asmcode::MOV, asmcode::RBX, asmcode::MEM_R15); // rbx contains header
  code.add(asmcode::MOV, asmcode::R9, asmcode::NUMBER, block_mask_bit);
  code.add(asmcode::TEST, asmcode::RBX, asmcode::R9);
  code.add(asmcode::JES, not_marked_yet);
  // this block was already marked. The header is replaced by the new address, marked
  // with block_mask_bit, so what we need to do now is remove the mark, and update
  // [rax] with the new address
  code.add(asmcode::MOV, asmcode::RDX, asmcode::NUMBER, ~block_mask_bit);
  code.add(asmcode::AND, asmcode::RBX, asmcode::RDX);
  code.add(asmcode::MOV, asmcode::MEM_RAX, asmcode::RBX);
  code.add(asmcode::RET);
  code.add(asmcode::LABEL, not_marked_yet);

  code.add(asmcode::MOV, asmcode::R8, asmcode::RBX);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::NUMBER, block_size_mask);
  code.add(asmcode::AND, asmcode::R8, asmcode::RDX); // get size of block in r8

  code.add(asmcode::MOV, asmcode::MEM_RDI, asmcode::RBX); // write unmarked header in tospace
  code.add(asmcode::MOV, asmcode::RDX, asmcode::RDI);
  code.add(asmcode::OR, asmcode::RDX, asmcode::NUMBER, block_tag); // mark as block
  code.add(asmcode::MOV, asmcode::MEM_RAX, asmcode::RDX); // let original pointer point to the object in the new location
  code.add(asmcode::OR, asmcode::RDX, asmcode::R9); // mark address of new position.
  code.add(asmcode::MOV, asmcode::MEM_R15, asmcode::RDX); // update old position of header with marked new address

  code.add(asmcode::ADD, asmcode::RDI, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::ADD, asmcode::R15, asmcode::NUMBER, CELLS(1));
  // copy the remaining entries in the block
  auto copy = label_to_string(label++);
  code.add(asmcode::LABEL, copy);
  code.add(asmcode::TEST, asmcode::R8, asmcode::R8);
  code.add(asmcode::JES, done);
  code.add(asmcode::DEC, asmcode::R8);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::MEM_R15);
  code.add(asmcode::MOV, asmcode::MEM_RDI, asmcode::RDX); // =>crash
  code.add(asmcode::ADD, asmcode::RDI, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::ADD, asmcode::R15, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::JMPS, copy);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::RET);
  }

void compile_reclaim(asmcode& code, const compiler_options& ops)
  {
  auto reclaim = label_to_string(label++);
  code.add(asmcode::CMP, ALLOC, LIMIT);
  code.add(asmcode::JGS, reclaim);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, reclaim);
  compile_reclaim_garbage(code, ops);
  }

void compile_reclaim_garbage(asmcode& code, const compiler_options&)
  {
  auto no_heap = label_to_string(label++);

  code.add(asmcode::CMP, ALLOC, FROM_SPACE_END);
  code.add(asmcode::JG, no_heap);

  code.add(asmcode::PUSH, asmcode::RBX);

  code.add(asmcode::MOV, asmcode::R11, GLOBALS); // run over all registers (in GC_SAVE) and locals. R11 contains the number of items.
  code.add(asmcode::SUB, asmcode::R11, GC_SAVE);
  code.add(asmcode::SHR, asmcode::R11, asmcode::NUMBER, 3);

  code.add(asmcode::MOV, asmcode::RAX, GC_SAVE);
  code.add(asmcode::MOV, asmcode::MEM_RAX, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::MEM_RAX, CELLS(1), asmcode::RDX);
  code.add(asmcode::MOV, asmcode::MEM_RAX, CELLS(2), asmcode::RSI);
  code.add(asmcode::MOV, asmcode::MEM_RAX, CELLS(3), asmcode::RDI);
  code.add(asmcode::MOV, asmcode::MEM_RAX, CELLS(4), asmcode::R8);
  code.add(asmcode::MOV, asmcode::MEM_RAX, CELLS(5), asmcode::R9);
  code.add(asmcode::MOV, asmcode::MEM_RAX, CELLS(6), asmcode::R12);
  code.add(asmcode::MOV, asmcode::MEM_RAX, CELLS(7), asmcode::R14);


  code.add(asmcode::MOV, asmcode::RSI, TO_SPACE); // rsi = alloc-ptr
  code.add(asmcode::MOV, asmcode::RDI, asmcode::RSI);

  /*
  All local variables are now in memory in sequential order, as gc_save and locals follow each other (see context creation).
  We will now mark them with the block_mask_bit.
  */
  auto mark_local_rep = label_to_string(label++);
  auto mark_local_done = label_to_string(label++);
  code.add(asmcode::LABEL, mark_local_rep);
  code.add(asmcode::TEST, asmcode::R11, asmcode::R11);
  code.add(asmcode::JES, mark_local_done);
  code.add(asmcode::CALL, "L_mark");
  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::DEC, asmcode::R11);
  code.add(asmcode::JMPS, mark_local_rep);
  code.add(asmcode::LABEL, mark_local_done);

  /*
  Next we mark the globals with the block_mask_bit.
  */
  code.add(asmcode::MOV, asmcode::RAX, GLOBALS);
  code.add(asmcode::MOV, asmcode::R11, GLOBALS_END);
  code.add(asmcode::SUB, asmcode::R11, GLOBALS);
  code.add(asmcode::SHR, asmcode::R11, asmcode::NUMBER, 3);
  auto mark_global_rep = label_to_string(label++);
  auto mark_global_done = label_to_string(label++);
  //auto skip_mark = label_to_string(label++);
  code.add(asmcode::LABEL, mark_global_rep);
  code.add(asmcode::TEST, asmcode::R11, asmcode::R11);
  code.add(asmcode::JES, mark_global_done);
  code.add(asmcode::CMP, asmcode::MEM_RAX, asmcode::NUMBER, unalloc_tag);
  code.add(asmcode::JES, mark_global_done);
  //code.add(asmcode::JES, skip_mark);
  code.add(asmcode::CALL, "L_mark");
  //code.add(asmcode::LABEL, skip_mark);
  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::DEC, asmcode::R11);
  code.add(asmcode::JMPS, mark_global_rep);
  code.add(asmcode::LABEL, mark_global_done);



  auto cmp_rsi_rdi_loop = label_to_string(label++);
  /*
  Next we update the items saved on the stack.
  */

  code.add(asmcode::MOV, asmcode::RAX, STACK);
  code.add(asmcode::MOV, asmcode::R11, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::RAX, STACK_TOP);
  code.add(asmcode::SUB, asmcode::R11, asmcode::RAX);
  code.add(asmcode::SHR, asmcode::R11, asmcode::NUMBER, 3);
  //code.add(asmcode::SUB, asmcode::R11, asmcode::NUMBER, 2); // r11 and rbx are pushed on the stack at the top of this method
  auto mark_rsp_rep = label_to_string(label++);
  code.add(asmcode::LABEL, mark_rsp_rep);
  code.add(asmcode::TEST, asmcode::R11, asmcode::R11);
  code.add(asmcode::JES, cmp_rsi_rdi_loop);
  code.add(asmcode::CALL, "L_mark");
  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::DEC, asmcode::R11);
  code.add(asmcode::JMPS, mark_rsp_rep);



  code.add(asmcode::LABEL, cmp_rsi_rdi_loop);
  auto rsi_equals_rdi = label_to_string(label++);
  auto simple_block = label_to_string(label++);
  auto complicated_block = label_to_string(label++);
  auto closure_block = label_to_string(label++);
  code.add(asmcode::CMP, asmcode::RSI, asmcode::RDI);
  code.add(asmcode::JE, rsi_equals_rdi);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RSI); // get header
  code.add(asmcode::MOV, asmcode::RCX, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::NUMBER, block_size_mask);
  code.add(asmcode::AND, asmcode::RCX, asmcode::RDX); // get size of block in rcx
  code.add(asmcode::MOV, asmcode::RDX, asmcode::RAX);
  code.add(asmcode::SHR, asmcode::RDX, asmcode::NUMBER, block_shift);
  code.add(asmcode::AND, asmcode::RDX, asmcode::NUMBER, block_header_mask); // rdx contains the type
  code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, closure_tag);
  code.add(asmcode::JES, closure_block);
  code.add(asmcode::JGS, complicated_block);
  /*
  code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, string_tag);
  code.add(asmcode::JES, simple_block);
  code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, symbol_tag);
  code.add(asmcode::JES, simple_block);
  code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, flonum_tag);
  code.add(asmcode::JES, simple_block);
  code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, closure_tag);
  code.add(asmcode::JES, closure_block);
  code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, pair_tag);
  code.add(asmcode::JES, complicated_block);
  code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, vector_tag);
  code.add(asmcode::JES, complicated_block);
  */
  code.add(asmcode::LABEL, simple_block);
  code.add(asmcode::INC, asmcode::RCX);
  code.add(asmcode::SHL, asmcode::RCX, asmcode::NUMBER, 3);
  code.add(asmcode::ADD, asmcode::RSI, asmcode::RCX);
  code.add(asmcode::JMP, cmp_rsi_rdi_loop);
  code.add(asmcode::LABEL, closure_block);
  code.add(asmcode::ADD, asmcode::RSI, asmcode::NUMBER, CELLS(1)); // skip label address of closure
  code.add(asmcode::DEC, asmcode::RCX);
  // now treat closure as complicated block
  code.add(asmcode::LABEL, complicated_block);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RSI);
  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, CELLS(1));
  auto complicated_block_loop = label_to_string(label++);
  auto complicated_block_done = label_to_string(label++);
  code.add(asmcode::LABEL, complicated_block_loop);
  code.add(asmcode::TEST, asmcode::RCX, asmcode::RCX);
  code.add(asmcode::JES, complicated_block_done);
  code.add(asmcode::CALL, "L_mark");
  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::DEC, asmcode::RCX);
  code.add(asmcode::JMPS, complicated_block_loop);
  code.add(asmcode::LABEL, complicated_block_done);
  code.add(asmcode::MOV, asmcode::RSI, asmcode::RAX);
  code.add(asmcode::JMP, cmp_rsi_rdi_loop);
  code.add(asmcode::LABEL, rsi_equals_rdi);

  // swap spaces
  code.add(asmcode::MOV, asmcode::RDX, FROM_SPACE);
  code.add(asmcode::MOV, asmcode::RAX, TO_SPACE);
  code.add(asmcode::MOV, FROM_SPACE, asmcode::RAX);
  code.add(asmcode::MOV, TO_SPACE, asmcode::RDX);
  code.add(asmcode::MOV, asmcode::RDX, FROM_SPACE_END);
  code.add(asmcode::MOV, asmcode::RAX, TO_SPACE_END);
  code.add(asmcode::MOV, FROM_SPACE_END, asmcode::RAX);
  code.add(asmcode::MOV, TO_SPACE_END, asmcode::RDX);

  code.add(asmcode::MOV, ALLOC, asmcode::RSI);
  code.add(asmcode::MOV, asmcode::RAX, FROM_SPACE_END);
  code.add(asmcode::MOV, asmcode::R11, FROMSPACE_RESERVE);
  code.add(asmcode::SHL, asmcode::R11, asmcode::NUMBER, 3);
  code.add(asmcode::SUB, asmcode::RAX, asmcode::R11);
  code.add(asmcode::MOV, LIMIT, asmcode::RAX);

  code.add(asmcode::MOV, asmcode::RAX, GC_SAVE);
  code.add(asmcode::MOV, asmcode::RCX, asmcode::MEM_RAX);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOV, asmcode::RSI, asmcode::MEM_RAX, CELLS(2));
  code.add(asmcode::MOV, asmcode::RDI, asmcode::MEM_RAX, CELLS(3));
  code.add(asmcode::MOV, asmcode::R8, asmcode::MEM_RAX, CELLS(4));
  code.add(asmcode::MOV, asmcode::R9, asmcode::MEM_RAX, CELLS(5));
  code.add(asmcode::MOV, asmcode::R12, asmcode::MEM_RAX, CELLS(6));
  code.add(asmcode::MOV, asmcode::R14, asmcode::MEM_RAX, CELLS(7));
  /*
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, unalloc_tag);
  code.add(asmcode::MOV, asmcode::R15, TO_SPACE);
  code.add(asmcode::MOV, asmcode::R11, TO_SPACE_END);
  code.add(asmcode::SUB, asmcode::R11, TO_SPACE);
  code.add(asmcode::SHR, asmcode::R11, asmcode::NUMBER, 3);
  auto make_zero_loop = label_to_string(label++);
  auto make_zero_done = label_to_string(label++);
  code.add(asmcode::LABEL, make_zero_loop);
  code.add(asmcode::TEST, asmcode::R11, asmcode::R11);
  code.add(asmcode::JES, make_zero_done);
  code.add(asmcode::DEC, asmcode::R11);
  code.add(asmcode::MOV, asmcode::MEM_R15, asmcode::RAX);
  code.add(asmcode::ADD, asmcode::R15, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::JMPS, make_zero_loop);
  code.add(asmcode::LABEL, make_zero_done);
  */
  code.add(asmcode::POP, asmcode::RBX); // continue value

  code.add(asmcode::CMP, ALLOC, FROM_SPACE_END);
  code.add(asmcode::JGS, no_heap);

  code.add(asmcode::JMP, CONTINUE);

  error_label(code, no_heap, re_heap_full);
  }

void compile_structurally_equal(asmcode& code, const compiler_options&, const std::string& label_name)
  {
  /*
  compares two objects structurally:
  rax and r11 are the input arguments
  rax contains a boolean on output
  r15 is clobbered
  */
  code.add(asmcode::LABEL, label_name);
  auto not_equal = label_to_string(label++);
  auto fail = label_to_string(label++);
  auto cleanup = label_to_string(label++);
  code.add(asmcode::CMP, asmcode::RAX, asmcode::R11);
  code.add(asmcode::JNES, not_equal);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::RET);
  code.add(asmcode::LABEL, fail);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::RET);
  code.add(asmcode::LABEL, not_equal);
  jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::R15, fail);
  jump_short_if_arg_is_not_block(code, asmcode::R11, asmcode::R15, fail);
  //get addresses of the blocks
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::R11, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  //compare headers
  code.add(asmcode::MOV, asmcode::R15, asmcode::MEM_RAX);
  code.add(asmcode::CMP, asmcode::MEM_R11, asmcode::R15);
  code.add(asmcode::JNES, fail);
  //compare contents
  code.add(asmcode::PUSH, asmcode::RCX);
  code.add(asmcode::PUSH, asmcode::RSI);
  //get length in rcx
  code.add(asmcode::MOV, asmcode::RCX, asmcode::R15);
  code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, block_size_mask);
  code.add(asmcode::AND, asmcode::RCX, asmcode::R15);
  //get address again in r15, r11 still contains the other address
  code.add(asmcode::MOV, asmcode::R15, asmcode::RAX);
  //init to true
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);

  auto repeat = label_to_string(label++);
  auto fail2 = label_to_string(label++);
  code.add(asmcode::LABEL, repeat);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 0);
  code.add(asmcode::JES, cleanup);
  //get next items
  code.add(asmcode::ADD, asmcode::R15, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::ADD, asmcode::R11, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::RSI, asmcode::MEM_R15);
  code.add(asmcode::CMP, asmcode::RSI, asmcode::MEM_R11);
  code.add(asmcode::JNES, fail2);
  code.add(asmcode::DEC, asmcode::RCX);
  code.add(asmcode::JMPS, repeat);
  code.add(asmcode::LABEL, fail2);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::LABEL, cleanup);
  code.add(asmcode::POP, asmcode::RSI);
  code.add(asmcode::POP, asmcode::RCX);
  code.add(asmcode::RET);
  }

void compile_recursively_equal(asmcode& code, const compiler_options&, const std::string& label_name)
  {
  /*
  compares two objects structurally:
  rax and r11 are the input arguments
  rax contains a boolean on output
  r15 is clobbered
  */
  code.add(asmcode::LABEL, label_name);
  auto not_equal = label_to_string(label++);
  auto fail = label_to_string(label++);
  auto cleanup = label_to_string(label++);
  auto simple_type = label_to_string(label++);
  code.add(asmcode::CMP, asmcode::RAX, asmcode::R11);
  code.add(asmcode::JNES, not_equal);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::RET);
  code.add(asmcode::LABEL, fail);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::RET);
  code.add(asmcode::LABEL, not_equal);
  jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::R15, fail);
  jump_short_if_arg_is_not_block(code, asmcode::R11, asmcode::R15, fail);
  //get addresses of the blocks
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::R11, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  //compare headers
  code.add(asmcode::MOV, asmcode::R15, asmcode::MEM_RAX);
  code.add(asmcode::CMP, asmcode::MEM_R11, asmcode::R15);
  code.add(asmcode::JNES, fail);
  code.add(asmcode::PUSH, asmcode::R15);
  code.add(asmcode::SHR, asmcode::R15, asmcode::NUMBER, block_shift);
  code.add(asmcode::AND, asmcode::R15, asmcode::NUMBER, block_header_mask); // r15 now contains the type of block
  code.add(asmcode::CMP, asmcode::R15, asmcode::NUMBER, closure_tag); // if smaller than 'closure_tag', then simple type  
  code.add(asmcode::POP, asmcode::R15);
  code.add(asmcode::JL, simple_type);
  //compare contents
  code.add(asmcode::PUSH, asmcode::RCX);
  //code.add(asmcode::PUSH, asmcode::RSI);
  //get length in rcx
  code.add(asmcode::MOV, asmcode::RCX, asmcode::R15);
  code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, block_size_mask);
  code.add(asmcode::AND, asmcode::RCX, asmcode::R15);
  //get address again in r15, r11 still contains the other address
  code.add(asmcode::MOV, asmcode::R15, asmcode::RAX);
  //init to true
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);

  auto repeat = label_to_string(label++);
  code.add(asmcode::LABEL, repeat);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 0);
  code.add(asmcode::JES, cleanup);
  //get next items
  code.add(asmcode::ADD, asmcode::R15, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::ADD, asmcode::R11, asmcode::NUMBER, CELLS(1));

  code.add(asmcode::PUSH, asmcode::R11);
  code.add(asmcode::PUSH, asmcode::R15);
  code.add(asmcode::PUSH, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_R11);
  code.add(asmcode::MOV, asmcode::R11, asmcode::MEM_R15);
  code.add(asmcode::CALL, label_name);
  code.add(asmcode::POP, asmcode::RCX);
  code.add(asmcode::POP, asmcode::R15);
  code.add(asmcode::POP, asmcode::R11);
  code.add(asmcode::CMP, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JES, cleanup);

  code.add(asmcode::DEC, asmcode::RCX);
  code.add(asmcode::JMPS, repeat);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::LABEL, cleanup);
  //code.add(asmcode::POP, asmcode::RSI);
  code.add(asmcode::POP, asmcode::RCX);
  code.add(asmcode::RET);

  code.add(asmcode::LABEL, simple_type);
  code.add(asmcode::PUSH, asmcode::RCX);
  code.add(asmcode::PUSH, asmcode::RSI);
  //get length in rcx
  code.add(asmcode::MOV, asmcode::RCX, asmcode::R15);
  code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, block_size_mask);
  code.add(asmcode::AND, asmcode::RCX, asmcode::R15);
  //get address again in r15, r11 still contains the other address
  code.add(asmcode::MOV, asmcode::R15, asmcode::RAX);
  //init to true
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);

  auto repeat2 = label_to_string(label++);
  auto fail2 = label_to_string(label++);
  auto cleanup2 = label_to_string(label++);
  code.add(asmcode::LABEL, repeat2);
  code.add(asmcode::CMP, asmcode::RCX, asmcode::NUMBER, 0);
  code.add(asmcode::JES, cleanup2);
  //get next items
  code.add(asmcode::ADD, asmcode::R15, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::ADD, asmcode::R11, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::RSI, asmcode::MEM_R15);
  code.add(asmcode::CMP, asmcode::RSI, asmcode::MEM_R11);
  code.add(asmcode::JNES, fail2);
  code.add(asmcode::DEC, asmcode::RCX);
  code.add(asmcode::JMPS, repeat2);
  code.add(asmcode::LABEL, fail2);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::LABEL, cleanup2);
  code.add(asmcode::POP, asmcode::RSI);
  code.add(asmcode::POP, asmcode::RCX);
  code.add(asmcode::RET);
  }

void compile_member_cmp_eq(asmcode& code, const compiler_options&)
  {
  // this method gets its arguments in rax and r15, and sets the Z flag
  code.add(asmcode::LABEL, "L_compile_member_cmp_eq");
  code.add(asmcode::CMP, asmcode::RAX, asmcode::R15);
  code.add(asmcode::RET);
  }

void compile_member_cmp_equal(asmcode& code, const compiler_options&)
  {
  // this method gets its arguments in rax and r15, and sets the Z flag
  code.add(asmcode::LABEL, "L_compile_member_cmp_equal");
  code.add(asmcode::PUSH, asmcode::R11);
  code.add(asmcode::PUSH, asmcode::R15);
  code.add(asmcode::PUSH, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::R15);
  code.add(asmcode::MOV, asmcode::R15, asmcode::LABELADDRESS, "L_recursively_equal");
  code.add(asmcode::CALL, asmcode::R15);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, bool_t);
  code.add(asmcode::CMP, asmcode::RAX, asmcode::R11);
  code.add(asmcode::POP, asmcode::RAX);
  code.add(asmcode::POP, asmcode::R15);
  code.add(asmcode::POP, asmcode::R11);
  code.add(asmcode::RET);
  }

void compile_member_cmp_eqv(asmcode& code, const compiler_options&)
  {
  // this method gets its arguments in rax and r15, and sets the Z flag
  code.add(asmcode::LABEL, "L_compile_member_cmp_eqv");
  code.add(asmcode::PUSH, asmcode::R11);
  code.add(asmcode::PUSH, asmcode::R15);
  code.add(asmcode::PUSH, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::R15);
  code.add(asmcode::MOV, asmcode::R15, asmcode::LABELADDRESS, "L_structurally_equal");
  code.add(asmcode::CALL, asmcode::R15);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, bool_t);
  code.add(asmcode::CMP, asmcode::RAX, asmcode::R11);
  code.add(asmcode::POP, asmcode::RAX);
  code.add(asmcode::POP, asmcode::R15);
  code.add(asmcode::POP, asmcode::R11);
  code.add(asmcode::RET);
  }


void compile_assoc_cmp_eq(asmcode& code, const compiler_options&)
  {
  // this method gets its arguments in rax and r15, and sets the Z flag
  code.add(asmcode::LABEL, "L_compile_assoc_cmp_eq");
  code.add(asmcode::CMP, asmcode::RAX, asmcode::MEM_R15, CELLS(1));
  code.add(asmcode::RET);
  }

void compile_assoc_cmp_equal(asmcode& code, const compiler_options&)
  {
  // this method gets its arguments in rax and r15, and sets the Z flag
  code.add(asmcode::LABEL, "L_compile_assoc_cmp_equal");
  code.add(asmcode::PUSH, asmcode::R11);
  code.add(asmcode::PUSH, asmcode::R15);
  code.add(asmcode::PUSH, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::MEM_R15, CELLS(1));
  code.add(asmcode::MOV, asmcode::R15, asmcode::LABELADDRESS, "L_recursively_equal");
  code.add(asmcode::CALL, asmcode::R15);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, bool_t);
  code.add(asmcode::CMP, asmcode::RAX, asmcode::R11);
  code.add(asmcode::POP, asmcode::RAX);
  code.add(asmcode::POP, asmcode::R15);
  code.add(asmcode::POP, asmcode::R11);
  code.add(asmcode::RET);
  }

void compile_assoc_cmp_eqv(asmcode& code, const compiler_options&)
  {
  // this method gets its arguments in rax and r15, and sets the Z flag
  code.add(asmcode::LABEL, "L_compile_assoc_cmp_eqv");
  code.add(asmcode::PUSH, asmcode::R11);
  code.add(asmcode::PUSH, asmcode::R15);
  code.add(asmcode::PUSH, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::MEM_R15, CELLS(1));
  code.add(asmcode::MOV, asmcode::R15, asmcode::LABELADDRESS, "L_structurally_equal");
  code.add(asmcode::CALL, asmcode::R15);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, bool_t);
  code.add(asmcode::CMP, asmcode::RAX, asmcode::R11);
  code.add(asmcode::POP, asmcode::RAX);
  code.add(asmcode::POP, asmcode::R15);
  code.add(asmcode::POP, asmcode::R11);
  code.add(asmcode::RET);
  }

namespace
  {
  void _compile_member(asmcode& code, const compiler_options& ops, const std::string& label_call, runtime_error re)
    {
    std::string error;
    if (ops.safe_primitives)
      {
      error = label_to_string(label++);
      code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
      code.add(asmcode::JNES, error);
      }
    std::string fail = label_to_string(label++);
    std::string success = label_to_string(label++);
    std::string repeat = label_to_string(label++);
    code.add(asmcode::LABEL, repeat);
    code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, nil);
    code.add(asmcode::JES, fail);

    if (ops.safe_primitives)
      {
      jump_short_if_arg_is_not_block(code, asmcode::RDX, asmcode::R11, error);
      }
    code.add(asmcode::AND, asmcode::RDX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
    if (ops.safe_primitives)
      {
      jump_short_if_arg_does_not_point_to_pair(code, asmcode::RDX, asmcode::R11, error);
      }
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::MOV, asmcode::R11, asmcode::LABELADDRESS, label_call);

    // get car
    code.add(asmcode::MOV, asmcode::R15, asmcode::MEM_RDX, CELLS(1));
    code.add(asmcode::CALL, asmcode::R11);
    code.add(asmcode::JES, success);
    code.add(asmcode::MOV, asmcode::RDX, asmcode::MEM_RDX, CELLS(2));
    code.add(asmcode::JMPS, repeat);

    code.add(asmcode::LABEL, fail);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
    code.add(asmcode::JMP, CONTINUE);

    code.add(asmcode::LABEL, success);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RDX);
    code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, block_tag); // add tag again
    code.add(asmcode::JMP, CONTINUE);

    if (ops.safe_primitives)
      {
      error_label(code, error, re);
      }
    }

  void _compile_assoc(asmcode& code, const compiler_options& ops, const std::string& label_call, runtime_error re)
    {
    std::string error;
    if (ops.safe_primitives)
      {
      error = label_to_string(label++);
      code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
      code.add(asmcode::JNE, error);
      }
    std::string fail = label_to_string(label++);
    std::string success = label_to_string(label++);
    std::string repeat = label_to_string(label++);
    std::string skip_nil = label_to_string(label++);
    code.add(asmcode::LABEL, repeat);
    code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, nil);
    code.add(asmcode::JE, fail);

    if (ops.safe_primitives)
      {
      jump_short_if_arg_is_not_block(code, asmcode::RDX, asmcode::R11, error);
      }
    code.add(asmcode::AND, asmcode::RDX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
    if (ops.safe_primitives)
      {
      jump_short_if_arg_does_not_point_to_pair(code, asmcode::RDX, asmcode::R11, error);
      }
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    // get car
    code.add(asmcode::MOV, asmcode::R15, asmcode::MEM_RDX, CELLS(1));
    // test for nil
    code.add(asmcode::CMP, asmcode::R15, asmcode::NUMBER, nil);
    code.add(asmcode::JES, skip_nil);

    if (ops.safe_primitives)
      {
      jump_short_if_arg_is_not_block(code, asmcode::R15, asmcode::R11, error);
      }
    code.add(asmcode::AND, asmcode::R15, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
    if (ops.safe_primitives)
      {
      jump_short_if_arg_does_not_point_to_pair(code, asmcode::R15, asmcode::R11, error);
      }

    code.add(asmcode::MOV, asmcode::R11, asmcode::LABELADDRESS, label_call);
    code.add(asmcode::CALL, asmcode::R11);
    code.add(asmcode::JES, success);
    code.add(asmcode::LABEL, skip_nil);
    code.add(asmcode::MOV, asmcode::RDX, asmcode::MEM_RDX, CELLS(2));
    code.add(asmcode::JMPS, repeat);

    code.add(asmcode::LABEL, fail);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
    code.add(asmcode::JMP, CONTINUE);

    code.add(asmcode::LABEL, success);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::R15);
    code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, block_tag); // add tag again
    code.add(asmcode::JMP, CONTINUE);

    if (ops.safe_primitives)
      {
      error_label(code, error, re);
      }
    }
  }

void compile_error(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, (uint64_t)re_silent);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 8);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, error_tag);
  code.add(asmcode::JMP, ERROR);
  }

void compile_memv(asmcode& code, const compiler_options& ops)
  {
  _compile_member(code, ops, "L_compile_member_cmp_eqv", re_memv_contract_violation);
  }

void compile_memq(asmcode& code, const compiler_options& ops)
  {
  _compile_member(code, ops, "L_compile_member_cmp_eq", re_memq_contract_violation);
  }

void compile_member(asmcode& code, const compiler_options& ops)
  {
  _compile_member(code, ops, "L_compile_member_cmp_equal", re_member_contract_violation);
  }

void compile_assv(asmcode& code, const compiler_options& ops)
  {
  _compile_assoc(code, ops, "L_compile_assoc_cmp_eqv", re_assv_contract_violation);
  }

void compile_assq(asmcode& code, const compiler_options& ops)
  {
  _compile_assoc(code, ops, "L_compile_assoc_cmp_eq", re_assq_contract_violation);
  }

void compile_assoc(asmcode& code, const compiler_options& ops)
  {
  _compile_assoc(code, ops, "L_compile_assoc_cmp_equal", re_assoc_contract_violation);
  }

void compile_apply_fake_cps_identity(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::LABEL_ALIGNED, "L_identity");
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RDX);
  code.add(asmcode::JMP, "L_return_from_identity");
  }

void compile_apply(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  std::string proc = label_to_string(label++);
  std::string clos = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JL, error);
    }

  //we move all registers in GC_SAVE, then all args are linear in memory (GC_SAVE is followed by LOCALS)
  code.add(asmcode::MOV, asmcode::RAX, GC_SAVE);
  code.add(asmcode::MOV, asmcode::MEM_RAX, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::MEM_RAX, CELLS(1), asmcode::RDX);
  code.add(asmcode::MOV, asmcode::MEM_RAX, CELLS(2), asmcode::RSI);
  code.add(asmcode::MOV, asmcode::MEM_RAX, CELLS(3), asmcode::RDI);
  code.add(asmcode::MOV, asmcode::MEM_RAX, CELLS(4), asmcode::R8);
  code.add(asmcode::MOV, asmcode::MEM_RAX, CELLS(5), asmcode::R9);
  code.add(asmcode::MOV, asmcode::MEM_RAX, CELLS(6), asmcode::R12);
  code.add(asmcode::MOV, asmcode::MEM_RAX, CELLS(7), asmcode::R14);

  code.add(asmcode::MOV, asmcode::RDX, asmcode::R11); // save r11 in rdx

  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, CELLS(1)); // mem_rax now points to rdx = 2nd arg
  code.add(asmcode::SUB, asmcode::R11, asmcode::NUMBER, 2);

  //now we look for the last argument, which should be a list
  std::string repeat = label_to_string(label++);
  std::string done = label_to_string(label++);
  code.add(asmcode::LABEL, repeat);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 0);
  code.add(asmcode::JES, done); // last arg found, is in mem_rax
  code.add(asmcode::DEC, asmcode::R11);
  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::JMPS, repeat);
  code.add(asmcode::LABEL, done);

  std::string unravel = label_to_string(label++);
  std::string unraveled = label_to_string(label++);

  code.add(asmcode::MOV, asmcode::R11, asmcode::RDX); // get length again
  code.add(asmcode::SUB, asmcode::R11, asmcode::NUMBER, 2);
  // mem_rax now contains a list, let's check
  code.add(asmcode::MOV, asmcode::R15, asmcode::MEM_RAX);
  code.add(asmcode::CMP, asmcode::R15, asmcode::NUMBER, nil); // last element is nil, so that's fine
  code.add(asmcode::JE, unraveled);
  if (ops.safe_primitives)
    jump_if_arg_is_not_block(code, asmcode::R15, asmcode::RDX, error);
  code.add(asmcode::AND, asmcode::R15, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    jump_if_arg_does_not_point_to_pair(code, asmcode::R15, asmcode::RDX, error);

  code.add(asmcode::LABEL, unravel);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::MEM_R15, CELLS(1)); // get car of pair in rdx
  code.add(asmcode::MOV, asmcode::MEM_RAX, asmcode::RDX);
  code.add(asmcode::INC, asmcode::R11);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::MEM_R15, CELLS(2)); // get cdr of pair in rdx
  code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, nil); // last element is nil, so all elements are unpacked
  code.add(asmcode::JE, unraveled);
  if (ops.safe_primitives)
    jump_if_arg_is_not_block(code, asmcode::RDX, asmcode::R15, error);
  code.add(asmcode::AND, asmcode::RDX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    jump_if_arg_does_not_point_to_pair(code, asmcode::RDX, asmcode::R15, error);
  code.add(asmcode::MOV, asmcode::R15, asmcode::RDX);
  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::JMPS, unravel);
  code.add(asmcode::LABEL, unraveled);


  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, procedure_mask);
  code.add(asmcode::CMP, asmcode::RAX, asmcode::NUMBER, procedure_tag);
  code.add(asmcode::JE, proc);
  if (ops.safe_primitives)
    {
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R15, error);
    code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
    jump_if_arg_does_not_point_to_closure(code, asmcode::RCX, asmcode::R15, error);
    code.add(asmcode::OR, asmcode::RCX, asmcode::NUMBER, block_tag); // add block tag again
    }
  // if we are here, then the apply is on a closure in rcx

  //first we make a fake cps object
  // todo: move this to the context. We don't need to create one each apply. We could reserve memory for it in the context
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, (uint64_t)closure_tag << (uint64_t)block_shift);
  code.add(asmcode::OR, asmcode::RAX, asmcode::R15);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R15, asmcode::LABELADDRESS, "L_identity");
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::R15);
  code.add(asmcode::MOV, asmcode::R15, ALLOC);
  code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));

  //rcx is ok, it contains the closure
  code.add(asmcode::MOV, asmcode::RDX, asmcode::R15); // the cps object in rdx

  auto all_args_set_clos = label_to_string(label++);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 0);
  code.add(asmcode::JE, all_args_set_clos);
  code.add(asmcode::MOV, asmcode::RAX, GC_SAVE);
  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::RSI, asmcode::MEM_RAX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
  code.add(asmcode::JE, all_args_set_clos);

  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::RDI, asmcode::MEM_RAX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
  code.add(asmcode::JE, all_args_set_clos);

  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::R8, asmcode::MEM_RAX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 3);
  code.add(asmcode::JE, all_args_set_clos);

  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::R9, asmcode::MEM_RAX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 4);
  code.add(asmcode::JE, all_args_set_clos);

  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::R12, asmcode::MEM_RAX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 5);
  code.add(asmcode::JE, all_args_set_clos);

  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::R14, asmcode::MEM_RAX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 6);
  code.add(asmcode::JE, all_args_set_clos);

  code.add(asmcode::PUSH, asmcode::R11);
  code.add(asmcode::SUB, asmcode::R11, asmcode::NUMBER, 6);
  code.add(asmcode::MOV, asmcode::R15, LOCAL);
  /*
  We have to go from the end to the front, as we'll otherwise overwrite data we still need, as GC_SAVE and LOCAL are next to each other in memory
  */
  code.add(asmcode::SHL, asmcode::R11, asmcode::NUMBER, 3);
  code.add(asmcode::ADD, asmcode::R15, asmcode::R11);
  code.add(asmcode::ADD, asmcode::RAX, asmcode::R11);
  code.add(asmcode::SHR, asmcode::R11, asmcode::NUMBER, 3);
  code.add(asmcode::PUSH, asmcode::RCX);

  auto rep_arg_set_clos = label_to_string(label++);
  auto rep_arg_set_clos_done = label_to_string(label++);
  code.add(asmcode::LABEL, rep_arg_set_clos);
  code.add(asmcode::SUB, asmcode::R15, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::RCX, asmcode::MEM_RAX);
  code.add(asmcode::MOV, asmcode::MEM_R15, asmcode::RCX);
  code.add(asmcode::SUB, asmcode::RAX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::DEC, asmcode::R11);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 0);
  code.add(asmcode::JES, rep_arg_set_clos_done);
  code.add(asmcode::JMPS, rep_arg_set_clos);
  code.add(asmcode::LABEL, rep_arg_set_clos_done);
  code.add(asmcode::POP, asmcode::RCX);
  code.add(asmcode::POP, asmcode::R11);
  code.add(asmcode::LABEL, all_args_set_clos);
  code.add(asmcode::ADD, asmcode::R11, asmcode::NUMBER, 2);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOV, asmcode::R15, asmcode::MEM_RAX, CELLS(1));

  code.add(asmcode::PUSH, CONTINUE);
  code.add(asmcode::JMP, asmcode::R15); // call closure  
  code.add(asmcode::LABEL, "L_return_from_identity");

  code.add(asmcode::POP, CONTINUE);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, proc);
  // if we are here, then the apply is on a procedure in rcx

  auto all_args_set = label_to_string(label++);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 0);
  code.add(asmcode::JE, all_args_set);
  code.add(asmcode::MOV, asmcode::RAX, GC_SAVE);
  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::RCX, asmcode::MEM_RAX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
  code.add(asmcode::JE, all_args_set);
  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::RDX, asmcode::MEM_RAX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
  code.add(asmcode::JE, all_args_set);

  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::RSI, asmcode::MEM_RAX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 3);
  code.add(asmcode::JE, all_args_set);

  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::RDI, asmcode::MEM_RAX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 4);
  code.add(asmcode::JE, all_args_set);

  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::R8, asmcode::MEM_RAX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 5);
  code.add(asmcode::JE, all_args_set);

  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::R9, asmcode::MEM_RAX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 6);
  code.add(asmcode::JE, all_args_set);

  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::R12, asmcode::MEM_RAX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 7);
  code.add(asmcode::JE, all_args_set);

  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::R14, asmcode::MEM_RAX);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 8);
  code.add(asmcode::JE, all_args_set);

  code.add(asmcode::PUSH, asmcode::R11);
  code.add(asmcode::SUB, asmcode::R11, asmcode::NUMBER, 8);
  code.add(asmcode::MOV, asmcode::R15, LOCAL);
  code.add(asmcode::PUSH, asmcode::RCX);
  /*
  We have to go from the front to the end (as opposed to the case for closures above), as we'll otherwise overwrite data we still need, as GC_SAVE and LOCAL are next to each other in memory
  */
  auto rep_arg_set = label_to_string(label++);
  auto rep_arg_set_done = label_to_string(label++);
  code.add(asmcode::LABEL, rep_arg_set);
  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::RCX, asmcode::MEM_RAX);
  code.add(asmcode::MOV, asmcode::MEM_R15, asmcode::RCX);
  code.add(asmcode::ADD, asmcode::R15, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::DEC, asmcode::R11);
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 0);
  code.add(asmcode::JES, rep_arg_set_done);
  code.add(asmcode::JMPS, rep_arg_set);
  code.add(asmcode::LABEL, rep_arg_set_done);
  code.add(asmcode::POP, asmcode::RCX);
  code.add(asmcode::POP, asmcode::R11);
  code.add(asmcode::LABEL, all_args_set);
  code.add(asmcode::MOV, asmcode::RAX, GC_SAVE);
  code.add(asmcode::MOV, asmcode::R15, asmcode::MEM_RAX);
  code.add(asmcode::AND, asmcode::R15, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8); // get procedure address
  code.add(asmcode::JMP, asmcode::R15); // call primitive

  if (ops.safe_primitives)
    {
    error_label(code, error, re_apply_contract_violation);
    }
  }

void compile_quotient(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  std::string division_by_zero;
  std::string l1 = label_to_string(label++);
  std::string l2 = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    division_by_zero = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JL, error);
    }
  jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, l1);
  //rcx is a block
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RCX, CELLS(1));
  code.add(asmcode::CVTTSD2SI, asmcode::RCX, asmcode::XMM0);
  code.add(asmcode::SHL, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::LABEL, l1);

  jump_short_if_arg_is_not_block(code, asmcode::RDX, asmcode::R11, l2);
  //rdx is a block
  code.add(asmcode::AND, asmcode::RDX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RDX, asmcode::R11, error);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RDX, CELLS(1));
  code.add(asmcode::CVTTSD2SI, asmcode::RDX, asmcode::XMM0);
  code.add(asmcode::SHL, asmcode::RDX, asmcode::NUMBER, 1);
  code.add(asmcode::LABEL, l2);

  if (ops.safe_primitives)
    {
    code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::RDX);
    code.add(asmcode::JES, division_by_zero);
    }
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::RCX, asmcode::RDX);
  code.add(asmcode::XOR, asmcode::RDX, asmcode::RDX);
  code.add(asmcode::CQO);
  code.add(asmcode::IDIV, asmcode::RCX);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_quotient_contract_violation);
    error_label(code, division_by_zero, re_division_by_zero);
    }
  }

void compile_remainder(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  std::string division_by_zero;
  std::string l1 = label_to_string(label++);
  std::string l2 = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    division_by_zero = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JL, error);
    }
  jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, l1);
  //rcx is a block
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RCX, CELLS(1));
  code.add(asmcode::CVTTSD2SI, asmcode::RCX, asmcode::XMM0);
  code.add(asmcode::SHL, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::LABEL, l1);

  jump_short_if_arg_is_not_block(code, asmcode::RDX, asmcode::R11, l2);
  //rdx is a block
  code.add(asmcode::AND, asmcode::RDX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RDX, asmcode::R11, error);
    }
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RDX, CELLS(1));
  code.add(asmcode::CVTTSD2SI, asmcode::RDX, asmcode::XMM0);
  code.add(asmcode::SHL, asmcode::RDX, asmcode::NUMBER, 1);
  code.add(asmcode::LABEL, l2);

  if (ops.safe_primitives)
    {
    code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::RDX);
    code.add(asmcode::JES, division_by_zero);
    }
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::RCX, asmcode::RDX);
  code.add(asmcode::XOR, asmcode::RDX, asmcode::RDX);
  code.add(asmcode::CQO);
  code.add(asmcode::IDIV, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RDX);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, ~((uint64_t)1));
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_remainder_contract_violation);
    error_label(code, division_by_zero, re_division_by_zero);
    }
  }

void compile_make_port(asmcode& code, const compiler_options& ops)
  {
  /*
  rcx: boolean: input port?
  rdx: string:  filename
  rsi: fixnum:  filehandle
  rdi: string:  buffer
  r8:  fixnum:  index to buffer
  r9:  fixnum:  size of buffer
  extra storage: for input ports: contains the number of bytes actually read. is initialized equal to size of buffer.
  */
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 6);
    code.add(asmcode::JNE, error);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);
    code.add(asmcode::INC, asmcode::RAX);
    check_heap(code, re_make_port_heap_overflow);
    jump_if_arg_is_not_block(code, asmcode::RDX, asmcode::R11, error);
    code.add(asmcode::TEST, asmcode::RSI, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RDI, asmcode::R11, error);
    code.add(asmcode::TEST, asmcode::R8, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    code.add(asmcode::TEST, asmcode::R9, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    code.add(asmcode::AND, asmcode::RDX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
    code.add(asmcode::AND, asmcode::RDI, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
    jump_if_arg_does_not_point_to_string(code, asmcode::RDX, asmcode::R11, error);
    jump_if_arg_does_not_point_to_string(code, asmcode::RDI, asmcode::R11, error);
    code.add(asmcode::OR, asmcode::RDX, asmcode::NUMBER, block_tag);
    code.add(asmcode::OR, asmcode::RDI, asmcode::NUMBER, block_tag);
    }

  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 7);
  code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, (uint64_t)port_tag << (uint64_t)block_shift);
  code.add(asmcode::OR, asmcode::RAX, asmcode::R15);
  code.add(asmcode::MOV, asmcode::R15, ALLOC);
  code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::RCX);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(2), asmcode::RDX);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(3), asmcode::RSI);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(4), asmcode::RDI);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(5), asmcode::R8);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(6), asmcode::R9);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(7), asmcode::R9);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(8));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R15);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_make_port_contract_violation);
    }
  }

void compile_is_port(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  auto lab_false = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::RAX, lab_false);
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_port(code, asmcode::RCX, asmcode::R11, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_is_port_contract_violation);
    }
  }

void compile_is_input_port(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  auto lab_false = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::RAX, lab_false);
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_port(code, asmcode::RCX, asmcode::R11, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX, CELLS(1));
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_is_input_port_contract_violation);
    }
  }

void compile_is_output_port(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  auto lab_false = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::RAX, lab_false);
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_port(code, asmcode::RCX, asmcode::R11, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX, CELLS(1));
  code.add(asmcode::XOR, asmcode::RAX, asmcode::NUMBER, 8); // toggle 4-th bit to switch from #t to #f or vice versa
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_is_output_port_contract_violation);
    }
  }

namespace
  {
  int _my_close(int const fd)
    {
    if (fd < 0)
      return 0;
#ifdef _WIN32
    return _close(fd);
#else
    return close(fd);
#endif
    }

  int _my_read(int const fd, void * const buffer, unsigned const buffer_size)
    {
    if (fd < 0)
      return 0;
#ifdef _WIN32
    return _read(fd, buffer, buffer_size);
#else
    return ::read(fd, buffer, buffer_size);
#endif
    }

  int _my_write(int fd, const void *buffer, unsigned int count)
    {
    if (fd < 0)
      return 0;
#ifdef _WIN32
    return _write(fd, buffer, count);
#else
    return write(fd, buffer, count);
#endif
    }
  }

void compile_peek_char(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_port(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::MOV, asmcode::R11, asmcode::MEM_RCX, CELLS(5));
  code.add(asmcode::MOV, asmcode::R15, asmcode::MEM_RCX, CELLS(7)); // get the number of bytes read

  // rcx contains port
  // r11 contains index
  // r15 contains number of bytes read

  auto no_buffer_read = label_to_string(label++);
  auto eof_lab = label_to_string(label++);
  code.add(asmcode::CMP, asmcode::R11, asmcode::R15);
  code.add(asmcode::JL, no_buffer_read);
  // start reading buffer
  code.add(asmcode::MOV, asmcode::R11, asmcode::MEM_RCX, CELLS(4)); // pointer to string
  code.add(asmcode::AND, asmcode::R11, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);

#ifdef _WIN32
  code.add(asmcode::PUSH, asmcode::RCX);
  code.add(asmcode::PUSH, asmcode::RDX);
  code.add(asmcode::PUSH, asmcode::R8);

  code.add(asmcode::MOV, asmcode::R8, asmcode::MEM_RCX, CELLS(5)); // index
  code.add(asmcode::SAR, asmcode::R8, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::R11);
  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::RCX, asmcode::MEM_RCX, CELLS(3)); // filehandle
  code.add(asmcode::SAR, asmcode::RCX, asmcode::NUMBER, 1);

#else
  code.add(asmcode::PUSH, asmcode::RDI);
  code.add(asmcode::PUSH, asmcode::RSI);
  code.add(asmcode::PUSH, asmcode::RDX);

  code.add(asmcode::MOV, asmcode::RDI, asmcode::MEM_RCX, CELLS(3)); // filehandle
  code.add(asmcode::SAR, asmcode::RDI, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::MEM_RCX, CELLS(5)); // index
  code.add(asmcode::SAR, asmcode::RDX, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RSI, asmcode::R11);
  code.add(asmcode::ADD, asmcode::RSI, asmcode::NUMBER, CELLS(1));
#endif

  save_before_foreign_call(code);
  align_stack(code);
  code.add(asmcode::MOV, asmcode::R15, CONTEXT); // r15 should be saved by the callee but r10 not, so we save the context in r15
#ifdef _WIN32
  code.add(asmcode::SUB, asmcode::RSP, asmcode::NUMBER, 32);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&_my_read);
#else
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&_my_read);
#endif  
  code.add(asmcode::CALL, asmcode::R11);
  code.add(asmcode::MOV, CONTEXT, asmcode::R15); // now we restore the context
  restore_stack(code);
  restore_after_foreign_call(code);

#ifdef _WIN32
  code.add(asmcode::POP, asmcode::R8);
  code.add(asmcode::POP, asmcode::RDX);
  code.add(asmcode::POP, asmcode::RCX);
#else
  code.add(asmcode::POP, asmcode::RDX);
  code.add(asmcode::POP, asmcode::RSI);
  code.add(asmcode::POP, asmcode::RDI);
#endif
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::MEM_RCX, CELLS(7), asmcode::RAX); // store the number of bytes read
  code.add(asmcode::CMP, asmcode::RAX, asmcode::NUMBER, 0);
  code.add(asmcode::JES, eof_lab);
  code.add(asmcode::XOR, asmcode::R11, asmcode::R11);
  code.add(asmcode::MOV, asmcode::MEM_RCX, CELLS(5), asmcode::R11); //update index
  code.add(asmcode::LABEL, no_buffer_read);
  code.add(asmcode::MOV, asmcode::R15, asmcode::MEM_RCX, CELLS(4));
  code.add(asmcode::SAR, asmcode::R11, asmcode::NUMBER, 1);
  code.add(asmcode::AND, asmcode::R15, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8); // get address to string
  code.add(asmcode::ADD, asmcode::R11, asmcode::NUMBER, 8); // increase 8 for header
  code.add(asmcode::ADD, asmcode::R15, asmcode::R11);
  code.add(asmcode::MOV, asmcode::AL, asmcode::BYTE_MEM_R15);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 8);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, char_tag);

  //code.add(asmcode::SUB, asmcode::R11, asmcode::NUMBER, 7);
  //code.add(asmcode::SHL, asmcode::R11, asmcode::NUMBER, 1);
  //code.add(asmcode::MOV, asmcode::MEM_RCX, CELLS(5), asmcode::R11); //update index

  code.add(asmcode::JMP, CONTINUE);

  code.add(asmcode::LABEL, eof_lab);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, eof_tag);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_peek_char_contract_violation);
    }
  }

void compile_read_char(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_port(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::MOV, asmcode::R11, asmcode::MEM_RCX, CELLS(5));
  code.add(asmcode::MOV, asmcode::R15, asmcode::MEM_RCX, CELLS(7)); // get the number of bytes read

  // rcx contains port
  // r11 contains index
  // r15 contains number of bytes read

  auto no_buffer_read = label_to_string(label++);
  auto eof_lab = label_to_string(label++);
  code.add(asmcode::CMP, asmcode::R11, asmcode::R15);
  code.add(asmcode::JL, no_buffer_read);
  // start reading buffer
  code.add(asmcode::MOV, asmcode::R11, asmcode::MEM_RCX, CELLS(4)); // pointer to string
  code.add(asmcode::AND, asmcode::R11, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);

#ifdef _WIN32
  code.add(asmcode::PUSH, asmcode::RCX);
  code.add(asmcode::PUSH, asmcode::RDX);
  code.add(asmcode::PUSH, asmcode::R8);

  code.add(asmcode::MOV, asmcode::R8, asmcode::MEM_RCX, CELLS(5)); // index
  code.add(asmcode::SAR, asmcode::R8, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::R11);
  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::RCX, asmcode::MEM_RCX, CELLS(3)); // filehandle
  code.add(asmcode::SAR, asmcode::RCX, asmcode::NUMBER, 1);

#else
  code.add(asmcode::PUSH, asmcode::RDI);
  code.add(asmcode::PUSH, asmcode::RSI);
  code.add(asmcode::PUSH, asmcode::RDX);

  code.add(asmcode::MOV, asmcode::RDI, asmcode::MEM_RCX, CELLS(3)); // filehandle
  code.add(asmcode::SAR, asmcode::RDI, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::MEM_RCX, CELLS(5)); // index
  code.add(asmcode::SAR, asmcode::RDX, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RSI, asmcode::R11);
  code.add(asmcode::ADD, asmcode::RSI, asmcode::NUMBER, CELLS(1));
#endif

  save_before_foreign_call(code);
  align_stack(code);
  code.add(asmcode::MOV, asmcode::R15, CONTEXT); // r15 should be saved by the callee but r10 not, so we save the context in r15
#ifdef _WIN32
  code.add(asmcode::SUB, asmcode::RSP, asmcode::NUMBER, 32);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&_my_read);
#else
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&_my_read);
#endif  
  code.add(asmcode::CALL, asmcode::R11);
  code.add(asmcode::MOV, CONTEXT, asmcode::R15); // now we restore the context
  restore_stack(code);
  restore_after_foreign_call(code);

#ifdef _WIN32
  code.add(asmcode::POP, asmcode::R8);
  code.add(asmcode::POP, asmcode::RDX);
  code.add(asmcode::POP, asmcode::RCX);
#else
  code.add(asmcode::POP, asmcode::RDX);
  code.add(asmcode::POP, asmcode::RSI);
  code.add(asmcode::POP, asmcode::RDI);
#endif
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::MEM_RCX, CELLS(7), asmcode::RAX); // store the number of bytes read
  code.add(asmcode::CMP, asmcode::RAX, asmcode::NUMBER, 0);
  code.add(asmcode::JES, eof_lab);
  code.add(asmcode::XOR, asmcode::R11, asmcode::R11);
  code.add(asmcode::LABEL, no_buffer_read);
  code.add(asmcode::MOV, asmcode::R15, asmcode::MEM_RCX, CELLS(4));
  code.add(asmcode::SAR, asmcode::R11, asmcode::NUMBER, 1);
  code.add(asmcode::AND, asmcode::R15, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8); // get address to string
  code.add(asmcode::ADD, asmcode::R11, asmcode::NUMBER, 8); // increase 8 for header
  code.add(asmcode::ADD, asmcode::R15, asmcode::R11);
  code.add(asmcode::MOV, asmcode::AL, asmcode::BYTE_MEM_R15);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 8);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, char_tag);

  code.add(asmcode::SUB, asmcode::R11, asmcode::NUMBER, 7);
  code.add(asmcode::SHL, asmcode::R11, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::MEM_RCX, CELLS(5), asmcode::R11); //update index

  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, eof_lab);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, eof_tag);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_read_char_contract_violation);
    }
  }

void compile_write_string(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    jump_if_arg_is_not_block(code, asmcode::RDX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RDX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_string(code, asmcode::RCX, asmcode::R11, error);
    jump_if_arg_does_not_point_to_port(code, asmcode::RDX, asmcode::R11, error);
    }

  code.add(asmcode::PUSH, asmcode::RCX);

  string_length(code, asmcode::RCX);

  // r15 now contains the string length
  code.add(asmcode::POP, asmcode::RCX);
  code.add(asmcode::PUSH, asmcode::R15); // save string length
  code.add(asmcode::MOV, asmcode::R11, asmcode::R15);
  code.add(asmcode::SHL, asmcode::R11, asmcode::NUMBER, 1);
  code.add(asmcode::ADD, asmcode::R11, asmcode::MEM_RDX, CELLS(5)); // add the index to r11

  code.add(asmcode::MOV, asmcode::R15, asmcode::MEM_RDX, CELLS(6)); // get the size of the port buffer

  // rdx contains port
  // rcx contains input string
  // rax contains string
  // r11 contains index
  // r15 contains size

  auto noflush = label_to_string(label++);
  code.add(asmcode::CMP, asmcode::R11, asmcode::R15);
  /*
  two options:
  r11 is smaller than r15: this means the string fits in the buffer
  r11 is equal or larger than r15: the string does not fit in the buffer => we flush the buffer and also flush the string
  */
  code.add(asmcode::JL, noflush);
  // start flush
  code.add(asmcode::MOV, asmcode::R11, asmcode::MEM_RDX, CELLS(4)); // pointer to string
  code.add(asmcode::AND, asmcode::R11, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);

#ifdef _WIN32
  code.add(asmcode::PUSH, asmcode::RCX);
  code.add(asmcode::PUSH, asmcode::RDX);
  code.add(asmcode::PUSH, asmcode::R8);

  code.add(asmcode::MOV, asmcode::RCX, asmcode::MEM_RDX, CELLS(3)); // filehandle
  code.add(asmcode::SAR, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::R8, asmcode::MEM_RDX, CELLS(5)); // index
  code.add(asmcode::SAR, asmcode::R8, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::R11);
  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1));
#else
  code.add(asmcode::PUSH, asmcode::RDI);
  code.add(asmcode::PUSH, asmcode::RSI);
  code.add(asmcode::PUSH, asmcode::RDX);

  code.add(asmcode::MOV, asmcode::RDI, asmcode::MEM_RDX, CELLS(3)); // filehandle
  code.add(asmcode::SAR, asmcode::RDI, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::MEM_RDX, CELLS(5)); // index
  code.add(asmcode::SAR, asmcode::RDX, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RSI, asmcode::R11);
  code.add(asmcode::ADD, asmcode::RSI, asmcode::NUMBER, CELLS(1));
#endif

  save_before_foreign_call(code);
  align_stack(code);
  code.add(asmcode::MOV, asmcode::R15, CONTEXT); // r15 should be saved by the callee but r10 not, so we save the context in r15
#ifdef _WIN32
  code.add(asmcode::SUB, asmcode::RSP, asmcode::NUMBER, 32);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&_my_write);
#else
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&_my_write);
#endif  
  code.add(asmcode::CALL, asmcode::R11);
  code.add(asmcode::MOV, CONTEXT, asmcode::R15); // now we restore the context
  restore_stack(code);
  restore_after_foreign_call(code);

#ifdef _WIN32
  code.add(asmcode::POP, asmcode::R8);
  code.add(asmcode::POP, asmcode::RDX);
  code.add(asmcode::POP, asmcode::RCX);
#else
  code.add(asmcode::POP, asmcode::RDX);
  code.add(asmcode::POP, asmcode::RSI);
  code.add(asmcode::POP, asmcode::RDI);
#endif

  code.add(asmcode::XOR, asmcode::R11, asmcode::R11);
  code.add(asmcode::MOV, asmcode::MEM_RDX, CELLS(5), asmcode::R11);

  // now flush the input string
  code.add(asmcode::POP, asmcode::R15); // get the string length
#ifdef _WIN32
  code.add(asmcode::PUSH, asmcode::RCX);
  code.add(asmcode::PUSH, asmcode::RDX);
  code.add(asmcode::PUSH, asmcode::R8);
  code.add(asmcode::MOV, asmcode::R8, asmcode::R15); // string length  

  code.add(asmcode::MOV, asmcode::R15, asmcode::RCX); // pointer to string
  code.add(asmcode::MOV, asmcode::RCX, asmcode::MEM_RDX, CELLS(3)); // filehandle
  code.add(asmcode::SAR, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::R15);
  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1));



#else
  code.add(asmcode::PUSH, asmcode::RDI);
  code.add(asmcode::PUSH, asmcode::RSI);
  code.add(asmcode::PUSH, asmcode::RDX);

  code.add(asmcode::MOV, asmcode::RDI, asmcode::MEM_RDX, CELLS(3)); // filehandle
  code.add(asmcode::SAR, asmcode::RDI, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::R15); // string length
  code.add(asmcode::MOV, asmcode::RSI, asmcode::RCX);
  code.add(asmcode::ADD, asmcode::RSI, asmcode::NUMBER, CELLS(1));
#endif

  save_before_foreign_call(code);
  align_stack(code);
  code.add(asmcode::MOV, asmcode::R15, CONTEXT); // r15 should be saved by the callee but r10 not, so we save the context in r15
#ifdef _WIN32
  code.add(asmcode::SUB, asmcode::RSP, asmcode::NUMBER, 32);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&_my_write);
#else
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&_my_write);
#endif  
  code.add(asmcode::CALL, asmcode::R11);
  code.add(asmcode::MOV, CONTEXT, asmcode::R15); // now we restore the context
  restore_stack(code);
  restore_after_foreign_call(code);

#ifdef _WIN32
  code.add(asmcode::POP, asmcode::R8);
  code.add(asmcode::POP, asmcode::RDX);
  code.add(asmcode::POP, asmcode::RCX);
#else
  code.add(asmcode::POP, asmcode::RDX);
  code.add(asmcode::POP, asmcode::RSI);
  code.add(asmcode::POP, asmcode::RDI);
#endif

  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, skiwi_quiet_undefined);
  code.add(asmcode::JMP, CONTINUE); // done

  code.add(asmcode::LABEL, noflush);

  code.add(asmcode::MOV, asmcode::R15, asmcode::MEM_RDX, CELLS(5)); //get old index
  code.add(asmcode::SAR, asmcode::R15, asmcode::NUMBER, 1);
  // r11 contains the final index, we can already save it
  code.add(asmcode::MOV, asmcode::MEM_RDX, CELLS(5), asmcode::R11); //update index

  code.add(asmcode::POP, asmcode::R11); //string-length

  // so right now: r11 == string length and r15 == index in buffer  
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RDX, CELLS(4));
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8); // get address to string
  code.add(asmcode::ADD, asmcode::R15, asmcode::NUMBER, 8); // increase 8 for header
  code.add(asmcode::ADD, asmcode::RAX, asmcode::R15); // rax now points to position where we can add to the string
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, 8); // increase 8 for header

  auto repeat = label_to_string(label++);
  auto done = label_to_string(label++);

  code.add(asmcode::LABEL, repeat);
  code.add(asmcode::TEST, asmcode::R11, asmcode::R11);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::MEM_RCX);
  code.add(asmcode::MOV, asmcode::BYTE_MEM_RAX, asmcode::DL);
  code.add(asmcode::INC, asmcode::RCX);
  code.add(asmcode::INC, asmcode::RAX);
  code.add(asmcode::DEC, asmcode::R11);
  code.add(asmcode::JMPS, repeat);
  code.add(asmcode::LABEL, done);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, skiwi_quiet_undefined);

  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_write_string_contract_violation);
    }
  }

void compile_write_char(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNE, error);
    code.add(asmcode::CMP, asmcode::CL, asmcode::NUMBER, char_tag);
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RDX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RDX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_port(code, asmcode::RDX, asmcode::R11, error);
    }
  code.add(asmcode::MOV, asmcode::R11, asmcode::MEM_RDX, CELLS(5));
  code.add(asmcode::MOV, asmcode::R15, asmcode::MEM_RDX, CELLS(6));

  // rdx contains port
  // rcx contains character
  // rax contains string
  // r11 contains index
  // r15 contains size

  auto noflush = label_to_string(label++);
  code.add(asmcode::CMP, asmcode::R11, asmcode::R15);
  code.add(asmcode::JL, noflush);
  // start flush
  code.add(asmcode::MOV, asmcode::R11, asmcode::MEM_RDX, CELLS(4)); // pointer to string
  code.add(asmcode::AND, asmcode::R11, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);

#ifdef _WIN32
  code.add(asmcode::PUSH, asmcode::RCX);
  code.add(asmcode::PUSH, asmcode::RDX);
  code.add(asmcode::PUSH, asmcode::R8);

  code.add(asmcode::MOV, asmcode::RCX, asmcode::MEM_RDX, CELLS(3)); // filehandle
  code.add(asmcode::SAR, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::R8, asmcode::MEM_RDX, CELLS(5)); // index
  code.add(asmcode::SAR, asmcode::R8, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::R11);
  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1));
#else
  code.add(asmcode::PUSH, asmcode::RDI);
  code.add(asmcode::PUSH, asmcode::RSI);
  code.add(asmcode::PUSH, asmcode::RDX);

  code.add(asmcode::MOV, asmcode::RDI, asmcode::MEM_RDX, CELLS(3)); // filehandle
  code.add(asmcode::SAR, asmcode::RDI, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::MEM_RDX, CELLS(5)); // index
  code.add(asmcode::SAR, asmcode::RDX, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RSI, asmcode::R11);
  code.add(asmcode::ADD, asmcode::RSI, asmcode::NUMBER, CELLS(1));
#endif

  save_before_foreign_call(code);
  align_stack(code);
  code.add(asmcode::MOV, asmcode::R15, CONTEXT); // r15 should be saved by the callee but r10 not, so we save the context in r15
#ifdef _WIN32
  code.add(asmcode::SUB, asmcode::RSP, asmcode::NUMBER, 32);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&_my_write);
#else
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&_my_write);
#endif  
  code.add(asmcode::CALL, asmcode::R11);
  code.add(asmcode::MOV, CONTEXT, asmcode::R15); // now we restore the context
  restore_stack(code);
  restore_after_foreign_call(code);

#ifdef _WIN32
  code.add(asmcode::POP, asmcode::R8);
  code.add(asmcode::POP, asmcode::RDX);
  code.add(asmcode::POP, asmcode::RCX);
#else
  code.add(asmcode::POP, asmcode::RDX);
  code.add(asmcode::POP, asmcode::RSI);
  code.add(asmcode::POP, asmcode::RDI);
#endif

  code.add(asmcode::XOR, asmcode::R11, asmcode::R11);
  code.add(asmcode::LABEL, noflush);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RDX, CELLS(4));
  code.add(asmcode::SAR, asmcode::R11, asmcode::NUMBER, 1);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8); // get address to string
  code.add(asmcode::ADD, asmcode::R11, asmcode::NUMBER, 8); // increase 8 for header
  code.add(asmcode::ADD, asmcode::RAX, asmcode::R11);
  code.add(asmcode::SHR, asmcode::RCX, asmcode::NUMBER, 8); // get char
  code.add(asmcode::MOV, asmcode::BYTE_MEM_RAX, asmcode::CL); // write char

  code.add(asmcode::SUB, asmcode::R11, asmcode::NUMBER, 7);
  code.add(asmcode::SHL, asmcode::R11, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::MEM_RDX, CELLS(5), asmcode::R11); //update index

  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, skiwi_quiet_undefined);

  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_write_char_contract_violation);
    }
  }

void compile_flush_output_port(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_port(code, asmcode::RCX, asmcode::R11, error);
    }
  /*
  Windows:
  rcx: filehandle
  rdx: buffer
  r8: buffersize

  Linux:
  rdi: filehandle
  rsi: buffer
  rdx: buffersize
  */
  auto skip = label_to_string(label++);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::MEM_RAX, CELLS(3)); // check filehandle valid?
  code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 0);
  code.add(asmcode::JL, skip);
  code.add(asmcode::MOV, asmcode::R11, asmcode::MEM_RAX, CELLS(4)); // pointer to string
  code.add(asmcode::AND, asmcode::R11, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
#ifdef _WIN32
  code.add(asmcode::PUSH, asmcode::RDX);
  code.add(asmcode::PUSH, asmcode::R8);

  code.add(asmcode::MOV, asmcode::RCX, asmcode::MEM_RAX, CELLS(3)); // filehandle
  code.add(asmcode::SAR, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::R8, asmcode::MEM_RAX, CELLS(5)); // index
  code.add(asmcode::SAR, asmcode::R8, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::MEM_RAX, CELLS(5), asmcode::NUMBER, 0); // set index to zero
  code.add(asmcode::MOV, asmcode::RDX, asmcode::R11);
  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1));
#else
  code.add(asmcode::PUSH, asmcode::RDI);
  code.add(asmcode::PUSH, asmcode::RSI);
  code.add(asmcode::PUSH, asmcode::RDX);

  code.add(asmcode::MOV, asmcode::RDI, asmcode::MEM_RAX, CELLS(3)); // filehandle
  code.add(asmcode::SAR, asmcode::RDI, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::MEM_RAX, CELLS(5)); // index
  code.add(asmcode::SAR, asmcode::RDX, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::MEM_RAX, CELLS(5), asmcode::NUMBER, 0);  // set index to zero
  code.add(asmcode::MOV, asmcode::RSI, asmcode::R11);
  code.add(asmcode::ADD, asmcode::RSI, asmcode::NUMBER, CELLS(1));
#endif

  save_before_foreign_call(code);
  align_stack(code);
  code.add(asmcode::MOV, asmcode::R15, CONTEXT); // r15 should be saved by the callee but r10 not, so we save the context in r15
#ifdef _WIN32
  code.add(asmcode::SUB, asmcode::RSP, asmcode::NUMBER, 32);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&_my_write);
#else
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&_my_write);
#endif  
  code.add(asmcode::CALL, asmcode::R11);
  code.add(asmcode::MOV, CONTEXT, asmcode::R15); // now we restore the context
  restore_stack(code);
  restore_after_foreign_call(code);

#ifdef _WIN32
  code.add(asmcode::POP, asmcode::R8);
  code.add(asmcode::POP, asmcode::RDX);
#else
  code.add(asmcode::POP, asmcode::RDX);
  code.add(asmcode::POP, asmcode::RSI);
  code.add(asmcode::POP, asmcode::RDI);
#endif

  code.add(asmcode::LABEL, skip);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, skiwi_quiet_undefined);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_flush_output_port_contract_violation);
    }
  }

void compile_close_file(asmcode& code, const compiler_options& ops)
  {
  // rcx : filehandle  
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    }

  code.add(asmcode::SAR, asmcode::RCX, asmcode::NUMBER, 1); // rcx now contains the filehandle

  /*
  Windows:
  rcx
  rdx
  r8

  Linux:
  rdi
  rsi
  rdx
  */
#ifndef WIN32
  code.add(asmcode::PUSH, asmcode::RDI);
  code.add(asmcode::MOV, asmcode::RDI, asmcode::RCX);
#endif

  save_before_foreign_call(code);
  align_stack(code);
  code.add(asmcode::MOV, asmcode::R15, CONTEXT); // r15 should be saved by the callee but r10 not, so we save the context in r15
#ifdef _WIN32
  code.add(asmcode::SUB, asmcode::RSP, asmcode::NUMBER, 32);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&_my_close);
#else
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&_my_close);
#endif  
  code.add(asmcode::CALL, asmcode::R11);
  code.add(asmcode::MOV, CONTEXT, asmcode::R15); // now we restore the context
  restore_stack(code);
  restore_after_foreign_call(code);

#ifndef WIN32
  code.add(asmcode::POP, asmcode::RDI);
#endif
  code.add(asmcode::SAL, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_close_file_contract_violation);
    }
  }

void compile_open_file(asmcode& code, const compiler_options& ops)
  {
  // rcx : filename
  // rdx : boolean: true = input file, false = output file
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_string(code, asmcode::RCX, asmcode::R11, error);
    }

  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1)); // rcx now points to string representing filename

#ifdef _WIN32
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, _O_CREAT | O_WRONLY | O_TRUNC | O_BINARY);
#else
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, O_CREAT | O_WRONLY | O_TRUNC);
#endif
  code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, bool_t);
  auto is_output = label_to_string(label++);
  code.add(asmcode::JNES, is_output);
#ifdef _WIN32
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, O_RDONLY | O_BINARY);
#else
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, O_RDONLY);
#endif
  code.add(asmcode::LABEL, is_output);
  /*
  Windows:
  rcx
  rdx
  r8

  Linux:
  rdi
  rsi
  rdx
  */
#ifdef _WIN32
  code.add(asmcode::PUSH, asmcode::R8);

  code.add(asmcode::MOV, asmcode::R8, asmcode::NUMBER, _S_IREAD | _S_IWRITE);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::R11);

#elif defined(unix)
  code.add(asmcode::PUSH, asmcode::RDI);
  code.add(asmcode::PUSH, asmcode::RSI);

  code.add(asmcode::MOV, asmcode::RDX, asmcode::NUMBER, __S_IREAD | __S_IWRITE);
  code.add(asmcode::MOV, asmcode::RDI, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::RSI, asmcode::R11);
#elif defined(__APPLE__)
      code.add(asmcode::PUSH, asmcode::RDI);
      code.add(asmcode::PUSH, asmcode::RSI);

      code.add(asmcode::MOV, asmcode::RDX, asmcode::NUMBER, S_IREAD | S_IWRITE);
      code.add(asmcode::MOV, asmcode::RDI, asmcode::RCX);
      code.add(asmcode::MOV, asmcode::RSI, asmcode::R11);
#endif

  save_before_foreign_call(code);
  align_stack(code);
  code.add(asmcode::MOV, asmcode::R15, CONTEXT); // r15 should be saved by the callee but r10 not, so we save the context in r15
#ifdef _WIN32
  code.add(asmcode::SUB, asmcode::RSP, asmcode::NUMBER, 32);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&_open);
#else
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&open);
#endif  
  code.add(asmcode::CALL, asmcode::R11);
  code.add(asmcode::MOV, CONTEXT, asmcode::R15); // now we restore the context
  restore_stack(code);
  restore_after_foreign_call(code);

#ifdef _WIN32
  code.add(asmcode::POP, asmcode::R8);
#else
  code.add(asmcode::POP, asmcode::RSI);
  code.add(asmcode::POP, asmcode::RDI);
#endif
  code.add(asmcode::SAL, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_open_file_contract_violation);
    }
  }

void compile_str2num(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNE, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_string(code, asmcode::RCX, asmcode::R11, error);
    }

  copy_string_to_buffer(code);

  std::string empty_string = label_to_string(label++);
  std::string rax_is_integer = label_to_string(label++);
  // first check whether the string has zero length
  code.add(asmcode::MOV, asmcode::RCX, BUFFER);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
  code.add(asmcode::TEST, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::JE, empty_string);
  code.add(asmcode::SAR, asmcode::RDX, asmcode::NUMBER, 1);
  /*
  Windows:
  rcx
  rdx
  r8

  Linux:
  rdi
  rsi
  rdx
  */
#ifdef _WIN32
  code.add(asmcode::PUSH, asmcode::R8);

  code.add(asmcode::PUSH, asmcode::RAX); // endptr for strtoll

  code.add(asmcode::MOV, asmcode::R8, asmcode::RDX);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::RSP);
  code.add(asmcode::MOV, asmcode::RCX, BUFFER);

#else
  code.add(asmcode::PUSH, asmcode::RDI);
  code.add(asmcode::PUSH, asmcode::RSI);
  code.add(asmcode::PUSH, asmcode::RAX); // endptr for strtoll

  code.add(asmcode::MOV, asmcode::RSI, asmcode::RSP);
  code.add(asmcode::MOV, asmcode::RDI, BUFFER);
#endif

  save_before_foreign_call(code);
  align_stack(code);
  code.add(asmcode::MOV, asmcode::R15, CONTEXT); // r15 should be saved by the callee but r10 not, so we save the context in r15
#ifdef _WIN32
  code.add(asmcode::SUB, asmcode::RSP, asmcode::NUMBER, 32);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&strtoll);
#else
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&strtoll);
#endif  
  code.add(asmcode::CALL, asmcode::R11);
  code.add(asmcode::MOV, CONTEXT, asmcode::R15); // now we restore the context
  restore_stack(code);
  restore_after_foreign_call(code);

  // check endptr
  code.add(asmcode::POP, asmcode::R11);

#ifdef _WIN32
  code.add(asmcode::POP, asmcode::R8);
#else
  code.add(asmcode::POP, asmcode::RSI);
  code.add(asmcode::POP, asmcode::RDI);
#endif

  code.add(asmcode::MOV, asmcode::RCX, asmcode::MEM_R11);
  code.add(asmcode::TEST, asmcode::CL, asmcode::CL);
  code.add(asmcode::JE, rax_is_integer);

  // now try as a float
#ifdef _WIN32
  code.add(asmcode::PUSH, asmcode::RAX); // endptr for strtoll

  code.add(asmcode::MOV, asmcode::RDX, asmcode::RSP);
  code.add(asmcode::MOV, asmcode::RCX, BUFFER);

#else
  code.add(asmcode::PUSH, asmcode::RDI);
  code.add(asmcode::PUSH, asmcode::RSI);
  code.add(asmcode::PUSH, asmcode::RAX); // endptr for strtoll

  code.add(asmcode::MOV, asmcode::RSI, asmcode::RSP);
  code.add(asmcode::MOV, asmcode::RDI, BUFFER);
#endif

  save_before_foreign_call(code);
  align_stack(code);
  code.add(asmcode::MOV, asmcode::R15, CONTEXT); // r15 should be saved by the callee but r10 not, so we save the context in r15
#ifdef _WIN32
  code.add(asmcode::SUB, asmcode::RSP, asmcode::NUMBER, 32);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&strtod);
#else
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&strtod);
#endif  
  code.add(asmcode::CALL, asmcode::R11);
  code.add(asmcode::MOV, CONTEXT, asmcode::R15); // now we restore the context
  restore_stack(code);
  restore_after_foreign_call(code);

  // check endptr
  code.add(asmcode::POP, asmcode::R11);

#ifndef WIN32
  code.add(asmcode::POP, asmcode::RSI);
  code.add(asmcode::POP, asmcode::RDI);
#endif

  code.add(asmcode::MOV, asmcode::RCX, asmcode::MEM_R11);
  code.add(asmcode::TEST, asmcode::CL, asmcode::CL);
  code.add(asmcode::JNE, empty_string);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::R11, ALLOC);
  code.add(asmcode::OR, asmcode::R11, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOVQ, asmcode::RAX, asmcode::XMM0);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::RAX);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);

  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, rax_is_integer);
  code.add(asmcode::SAL, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::JMP, CONTINUE);

  code.add(asmcode::LABEL, empty_string);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_str2num_contract_violation);
    }
  }

void compile_num2str(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  std::string flonum = label_to_string(label++);
  std::string cont = label_to_string(label++);
  std::string oct = label_to_string(label++);
  std::string hex = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNE, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    }
  code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::JNE, flonum);
  code.add(asmcode::SAR, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, 16);
  code.add(asmcode::JES, oct);
  code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, 32);
  code.add(asmcode::JES, hex);
  code.add(asmcode::MOV, asmcode::R15, DCVT);
  code.add(asmcode::JMP, cont);
  code.add(asmcode::LABEL, oct);
  code.add(asmcode::MOV, asmcode::R15, OCVT);
  code.add(asmcode::JMP, cont);
  code.add(asmcode::LABEL, hex);
  code.add(asmcode::MOV, asmcode::R15, XCVT);
  code.add(asmcode::JMP, cont);
  code.add(asmcode::LABEL, flonum);
  if (ops.safe_primitives)
    {
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::MOV, asmcode::RCX, asmcode::MEM_RCX, CELLS(1));  
  code.add(asmcode::MOV, asmcode::R15, GCVT);

  /*
  Windows:
  rcx
  rdx
  r8

  Linux:
  rdi
  rsi
  rdx
  */
#ifdef _WIN32
  code.add(asmcode::PUSH, asmcode::R8);

  code.add(asmcode::MOV, asmcode::R8, asmcode::RCX);
  code.add(asmcode::MOVQ, asmcode::XMM2, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::R15);
  code.add(asmcode::MOV, asmcode::RCX, BUFFER);

#else
  code.add(asmcode::PUSH, asmcode::RDI);
  code.add(asmcode::PUSH, asmcode::RSI);

  code.add(asmcode::MOVQ, asmcode::XMM0, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::RSI, asmcode::R15);
  code.add(asmcode::MOV, asmcode::RDI, BUFFER);
#endif

  save_before_foreign_call(code);
  align_stack(code);
  code.add(asmcode::MOV, asmcode::R15, CONTEXT); // r15 should be saved by the callee but r10 not, so we save the context in r15
#ifdef _WIN32
  code.add(asmcode::SUB, asmcode::RSP, asmcode::NUMBER, 32);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&sprintf);
#else
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&sprintf);
#endif  
  code.add(asmcode::CALL, asmcode::R11);
  code.add(asmcode::MOV, CONTEXT, asmcode::R15); // now we restore the context
  restore_stack(code);
  restore_after_foreign_call(code);

#ifdef _WIN32
  code.add(asmcode::POP, asmcode::R8);
#else
  code.add(asmcode::POP, asmcode::RSI);
  code.add(asmcode::POP, asmcode::RDI);
#endif

  allocate_buffer_as_string(code);

  code.add(asmcode::JMP, CONTINUE);


  code.add(asmcode::LABEL, cont); // rcx contains number, r15 contains format

  /*
  Windows:
  rcx
  rdx
  r8

  Linux:
  rdi
  rsi
  rdx
  */
#ifdef _WIN32
  code.add(asmcode::PUSH, asmcode::R8);

  code.add(asmcode::MOV, asmcode::R8, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::R15);
  code.add(asmcode::MOV, asmcode::RCX, BUFFER);

#else
  code.add(asmcode::PUSH, asmcode::RDI);
  code.add(asmcode::PUSH, asmcode::RSI);

  code.add(asmcode::MOV, asmcode::RDX, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::RSI, asmcode::R15);
  code.add(asmcode::MOV, asmcode::RDI, BUFFER);
#endif

  save_before_foreign_call(code);
  align_stack(code);
  code.add(asmcode::MOV, asmcode::R15, CONTEXT); // r15 should be saved by the callee but r10 not, so we save the context in r15
#ifdef _WIN32
  code.add(asmcode::SUB, asmcode::RSP, asmcode::NUMBER, 32);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&sprintf);
#else
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&sprintf);
#endif  
  code.add(asmcode::CALL, asmcode::R11);
  code.add(asmcode::MOV, CONTEXT, asmcode::R15); // now we restore the context
  restore_stack(code);
  restore_after_foreign_call(code);

#ifdef _WIN32
  code.add(asmcode::POP, asmcode::R8);
#else
  code.add(asmcode::POP, asmcode::RSI);
  code.add(asmcode::POP, asmcode::RDI);
#endif

  allocate_buffer_as_string(code);

  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_num2str_contract_violation);
    }
  }

void compile_ieee754_sign(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX, CELLS(1));
  code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 63);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_ieee754_sign_contract_violation);
    }
  }

void compile_ieee754_exponent(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::R11, error);
    }
  //  ($inline "mov rax, [rax + CELLS(1)]; sar rax, 51; and rax, 0xfff; or rax, 1" x))
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX, CELLS(1));
  code.add(asmcode::SAR, asmcode::RAX, asmcode::NUMBER, 52); // mantissa is 52
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0x7ff); // remove sign bit
  code.add(asmcode::SAL, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_ieee754_exponent_contract_violation);
    }
  }

void compile_ieee754_mantissa(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::R11, error);
    }
  // ($inline "mov rax, [rax + CELLS(1)]; mov r11, 0x000fffffffffffff; and rax, r11; INT2FIX rax" x))
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX, CELLS(1));
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, 0x000fffffffffffff);
  code.add(asmcode::AND, asmcode::RAX, asmcode::R11);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_ieee754_mantissa_contract_violation);
    }
  }

void compile_ieee754_sin(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  std::string is_fixnum = label_to_string(label++);
  std::string save_flonum = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, is_fixnum);
  if (ops.safe_primitives)
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::R11, error);

  code.add(asmcode::FLD, asmcode::MEM_RCX, CELLS(1));
  code.add(asmcode::JMPS, save_flonum);
  code.add(asmcode::LABEL, is_fixnum);
  code.add(asmcode::SAR, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::PUSH, asmcode::RCX);
  code.add(asmcode::FILD, asmcode::MEM_RSP);
  code.add(asmcode::POP, asmcode::RCX);
  code.add(asmcode::LABEL, save_flonum);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::R11, ALLOC);
  code.add(asmcode::OR, asmcode::R11, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::FSIN);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);

  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_ieee754_sin_contract_violation);
    }
  }

void compile_ieee754_cos(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  std::string is_fixnum = label_to_string(label++);
  std::string save_flonum = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, is_fixnum);
  if (ops.safe_primitives)
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::R11, error);

  code.add(asmcode::FLD, asmcode::MEM_RCX, CELLS(1));
  code.add(asmcode::JMPS, save_flonum);
  code.add(asmcode::LABEL, is_fixnum);
  code.add(asmcode::SAR, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::PUSH, asmcode::RCX);
  code.add(asmcode::FILD, asmcode::MEM_RSP);
  code.add(asmcode::POP, asmcode::RCX);
  code.add(asmcode::LABEL, save_flonum);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::R11, ALLOC);
  code.add(asmcode::OR, asmcode::R11, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::FCOS);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);

  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_ieee754_cos_contract_violation);
    }
  }

void compile_ieee754_tan(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  std::string is_fixnum = label_to_string(label++);
  std::string save_flonum = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, is_fixnum);
  if (ops.safe_primitives)
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::R11, error);

  code.add(asmcode::FLD, asmcode::MEM_RCX, CELLS(1));
  code.add(asmcode::JMPS, save_flonum);
  code.add(asmcode::LABEL, is_fixnum);
  code.add(asmcode::SAR, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::PUSH, asmcode::RCX);
  code.add(asmcode::FILD, asmcode::MEM_RSP);
  code.add(asmcode::POP, asmcode::RCX);
  code.add(asmcode::LABEL, save_flonum);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::R11, ALLOC);
  code.add(asmcode::OR, asmcode::R11, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::FPTAN);
  code.add(asmcode::FSTP, asmcode::ST0);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);

  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_ieee754_tan_contract_violation);
    }
  }

void compile_ieee754_asin(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  std::string is_fixnum = label_to_string(label++);
  std::string save_flonum = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, is_fixnum);
  if (ops.safe_primitives)
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::R11, error);

  code.add(asmcode::FLD, asmcode::MEM_RCX, CELLS(1));
  code.add(asmcode::JMPS, save_flonum);
  code.add(asmcode::LABEL, is_fixnum);
  code.add(asmcode::SAR, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::PUSH, asmcode::RCX);
  code.add(asmcode::FILD, asmcode::MEM_RSP);
  code.add(asmcode::POP, asmcode::RCX);
  code.add(asmcode::LABEL, save_flonum);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::R11, ALLOC);
  code.add(asmcode::OR, asmcode::R11, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);

  code.add(asmcode::FLD, asmcode::ST0);
  code.add(asmcode::FMUL, asmcode::ST0, asmcode::ST0);
  code.add(asmcode::FLD1);
  //code.add(asmcode::FSUBR, asmcode::ST1, asmcode::ST0);
  //code.add(asmcode::FXCH);
  code.add(asmcode::FSUBRP);
  code.add(asmcode::FSQRT);
  code.add(asmcode::FPATAN);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);

  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_ieee754_asin_contract_violation);
    }
  }

void compile_ieee754_acos(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  std::string is_fixnum = label_to_string(label++);
  std::string save_flonum = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, is_fixnum);
  if (ops.safe_primitives)
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::R11, error);

  code.add(asmcode::FLD, asmcode::MEM_RCX, CELLS(1));
  code.add(asmcode::JMPS, save_flonum);
  code.add(asmcode::LABEL, is_fixnum);
  code.add(asmcode::SAR, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::PUSH, asmcode::RCX);
  code.add(asmcode::FILD, asmcode::MEM_RSP);
  code.add(asmcode::POP, asmcode::RCX);
  code.add(asmcode::LABEL, save_flonum);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::R11, ALLOC);
  code.add(asmcode::OR, asmcode::R11, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::FLD, asmcode::ST0);
  code.add(asmcode::FMUL, asmcode::ST0, asmcode::ST0);
  code.add(asmcode::FLD1);
  code.add(asmcode::FSUBRP);
  code.add(asmcode::FSQRT);
  code.add(asmcode::FXCH, asmcode::ST1);
  code.add(asmcode::FPATAN);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);

  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_ieee754_acos_contract_violation);
    }
  }

void compile_ieee754_atan1(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  std::string is_fixnum = label_to_string(label++);
  std::string save_flonum = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, is_fixnum);
  if (ops.safe_primitives)
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::R11, error);

  code.add(asmcode::FLD, asmcode::MEM_RCX, CELLS(1));
  code.add(asmcode::JMPS, save_flonum);
  code.add(asmcode::LABEL, is_fixnum);
  code.add(asmcode::SAR, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::PUSH, asmcode::RCX);
  code.add(asmcode::FILD, asmcode::MEM_RSP);
  code.add(asmcode::POP, asmcode::RCX);
  code.add(asmcode::LABEL, save_flonum);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::R11, ALLOC);
  code.add(asmcode::OR, asmcode::R11, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::FLD1);
  code.add(asmcode::FPATAN);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);

  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_ieee754_atan1_contract_violation);
    }
  }

void compile_ieee754_log(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  std::string is_fixnum = label_to_string(label++);
  std::string save_flonum = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::FLDLN2);
  code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, is_fixnum);
  if (ops.safe_primitives)
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::R11, error);

  code.add(asmcode::FLD, asmcode::MEM_RCX, CELLS(1));
  code.add(asmcode::JMPS, save_flonum);
  code.add(asmcode::LABEL, is_fixnum);
  code.add(asmcode::SAR, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::PUSH, asmcode::RCX);
  code.add(asmcode::FILD, asmcode::MEM_RSP);
  code.add(asmcode::POP, asmcode::RCX);
  code.add(asmcode::LABEL, save_flonum);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::R11, ALLOC);
  code.add(asmcode::OR, asmcode::R11, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);

  code.add(asmcode::FYL2X);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);

  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_ieee754_log_contract_violation);
    }
  }

void compile_ieee754_round(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  std::string is_fixnum = label_to_string(label++);
  std::string save_flonum = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, is_fixnum);
  if (ops.safe_primitives)
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::R11, error);

  code.add(asmcode::FLD, asmcode::MEM_RCX, CELLS(1));
  code.add(asmcode::JMPS, save_flonum);
  code.add(asmcode::LABEL, is_fixnum);
  code.add(asmcode::SAR, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::PUSH, asmcode::RCX);
  code.add(asmcode::FILD, asmcode::MEM_RSP);
  code.add(asmcode::POP, asmcode::RCX);
  code.add(asmcode::LABEL, save_flonum);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::R11, ALLOC);
  code.add(asmcode::OR, asmcode::R11, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::FRNDINT);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);

  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_ieee754_round_contract_violation);
    }
  }

void compile_ieee754_truncate(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  std::string is_fixnum = label_to_string(label++);
  std::string save_fixnum = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, is_fixnum);
  if (ops.safe_primitives)
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::R11, error);

  code.add(asmcode::MOVQ, asmcode::XMM0, asmcode::MEM_RCX, CELLS(1));
  code.add(asmcode::JMPS, save_fixnum);
  code.add(asmcode::LABEL, is_fixnum);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, save_fixnum);
  code.add(asmcode::CVTTSD2SI, asmcode::RAX, asmcode::XMM0);
  code.add(asmcode::SAL, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_ieee754_round_contract_violation);
    }
  }

void compile_ieee754_sqrt(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  std::string is_fixnum = label_to_string(label++);
  std::string save_flonum = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, is_fixnum);
  if (ops.safe_primitives)
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::R11, error);

  code.add(asmcode::MOVQ, asmcode::XMM0, asmcode::MEM_RCX, CELLS(1));
  code.add(asmcode::JMPS, save_flonum);
  code.add(asmcode::LABEL, is_fixnum);
  code.add(asmcode::SAR, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::CVTSI2SD, asmcode::XMM0, asmcode::RCX);
  code.add(asmcode::LABEL, save_flonum);
  code.add(asmcode::SQRTPD, asmcode::XMM0, asmcode::XMM0);
  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::R11, ALLOC);
  code.add(asmcode::OR, asmcode::R11, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOVQ, MEM_ALLOC, CELLS(1), asmcode::XMM0);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);

  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_ieee754_sqrt_contract_violation);
    }
  }

void compile_ieee754_pi(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 0);
    code.add(asmcode::JNES, error);
    }
  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::R11, ALLOC);
  code.add(asmcode::OR, asmcode::R11, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::FLDPI);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);

  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_ieee754_pi_contract_violation);
    }
  }

void compile_fixnum_expt(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    }
  code.add(asmcode::SAR, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::SAR, asmcode::RDX, asmcode::NUMBER, 1);
  if (ops.safe_primitives)
    {
    code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, 0);
    code.add(asmcode::JLS, error);
    }
  std::string done = label_to_string(label++);
  std::string rep = label_to_string(label++);
  std::string zero = label_to_string(label++);
  code.add(asmcode::CMP, asmcode::RDX, asmcode::NUMBER, 0);
  code.add(asmcode::JE, zero);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::R15, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::RCX, asmcode::RDX);
  code.add(asmcode::DEC, asmcode::RCX);
  code.add(asmcode::JES, done);
  code.add(asmcode::LABEL, rep);
  code.add(asmcode::IMUL, asmcode::R15);
  code.add(asmcode::DEC, asmcode::RCX);
  code.add(asmcode::JES, done);
  code.add(asmcode::JMPS, rep);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::SAL, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, zero);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 2);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_fixnum_expt_contract_violation);
    }
  }

void compile_flonum_expt(asmcode& code, const compiler_options& ops)
  {
  // only for positive exponents
  std::string error;
  std::string rcx_is_fixnum = label_to_string(label++);
  std::string rcx_stored = label_to_string(label++);
  std::string rdx_is_fixnum = label_to_string(label++);
  std::string rdx_stored = label_to_string(label++);
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNE, error);
    }
  code.add(asmcode::TEST, asmcode::RDX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, rdx_is_fixnum);
  if (ops.safe_primitives)
    jump_if_arg_is_not_block(code, asmcode::RDX, asmcode::R11, error);
  code.add(asmcode::AND, asmcode::RDX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RDX, asmcode::R11, error);

  code.add(asmcode::FLD, asmcode::MEM_RDX, CELLS(1));
  code.add(asmcode::JMPS, rdx_stored);
  code.add(asmcode::LABEL, rdx_is_fixnum);
  code.add(asmcode::SAR, asmcode::RDX, asmcode::NUMBER, 1);
  code.add(asmcode::PUSH, asmcode::RDX);
  code.add(asmcode::FILD, asmcode::MEM_RSP);
  code.add(asmcode::POP, asmcode::RDX);
  code.add(asmcode::LABEL, rdx_stored);

  code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::JES, rcx_is_fixnum);
  if (ops.safe_primitives)
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    jump_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::R11, error);

  code.add(asmcode::FLD, asmcode::MEM_RCX, CELLS(1));
  code.add(asmcode::JMPS, rcx_stored);
  code.add(asmcode::LABEL, rcx_is_fixnum);
  code.add(asmcode::SAR, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::PUSH, asmcode::RCX);
  code.add(asmcode::FILD, asmcode::MEM_RSP);
  code.add(asmcode::POP, asmcode::RCX);
  code.add(asmcode::LABEL, rcx_stored);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::R11, ALLOC);
  code.add(asmcode::OR, asmcode::R11, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);

  code.add(asmcode::FYL2X);
  code.add(asmcode::FLD, asmcode::ST0);
  code.add(asmcode::FISTPQ, MEM_ALLOC, CELLS(1));
  code.add(asmcode::FILD, MEM_ALLOC, CELLS(1));
  code.add(asmcode::FSUB);
  code.add(asmcode::F2XM1);
  code.add(asmcode::FLD1);
  code.add(asmcode::FADD);
  code.add(asmcode::FILD, MEM_ALLOC, CELLS(1));
  code.add(asmcode::FXCH);
  code.add(asmcode::FSCALE);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(2)); // popping to some unused memory location
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_flonum_expt_contract_violation);
    }
  }

void compile_fixnum_to_flonum(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    code.add(asmcode::TEST, asmcode::RCX, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    if (ops.safe_flonums)
      {
      code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 2);
      check_heap(code, re_fixnum_to_flonum_heap_overflow);
      }
    }
  code.add(asmcode::SAR, asmcode::RCX, asmcode::NUMBER, 1);
  code.add(asmcode::CVTSI2SD, asmcode::XMM0, asmcode::RCX);
  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::R11, ALLOC);
  code.add(asmcode::OR, asmcode::R11, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOVQ, MEM_ALLOC, CELLS(1), asmcode::XMM0);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_fixnum_to_flonum_contract_violation);
    }
  }
void compile_flonum_to_fixnum(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNES, error);
    jump_short_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX, CELLS(1));
  code.add(asmcode::MOVQ, asmcode::XMM0, asmcode::RAX);
  code.add(asmcode::CVTTSD2SI, asmcode::RAX, asmcode::XMM0);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_flonum_to_fixnum_contract_violation);
    }
  }

void compile_compare_strings(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 3);
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    jump_if_arg_is_not_block(code, asmcode::RDX, asmcode::R11, error);
    code.add(asmcode::TEST, asmcode::RSI, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RDX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_string(code, asmcode::RCX, asmcode::R11, error);
    jump_short_if_arg_does_not_point_to_string(code, asmcode::RDX, asmcode::R11, error);
    }
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::SAR, asmcode::RSI, asmcode::NUMBER, 1);
  auto repeat = label_to_string(label++);
  auto equal = label_to_string(label++);
  auto less = label_to_string(label++);
  auto greater = label_to_string(label++);
  code.add(asmcode::LABEL, repeat);
  code.add(asmcode::TEST, asmcode::RSI, asmcode::RSI);
  code.add(asmcode::JES, equal);
  code.add(asmcode::MOV, asmcode::AL, asmcode::BYTE_MEM_RDX);
  code.add(asmcode::CMP, asmcode::BYTE_MEM_RCX, asmcode::AL);
  code.add(asmcode::JLS, less);
  code.add(asmcode::JGS, greater);
  code.add(asmcode::INC, asmcode::RCX);
  code.add(asmcode::INC, asmcode::RDX);
  code.add(asmcode::DEC, asmcode::RSI);
  code.add(asmcode::JMPS, repeat);
  code.add(asmcode::LABEL, equal);
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, less);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 0xfffffffffffffffe);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, greater);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 2);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_compare_strings_contract_violation);
    }
  }


void compile_compare_strings_ci(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 3);
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    jump_if_arg_is_not_block(code, asmcode::RDX, asmcode::R11, error);
    code.add(asmcode::TEST, asmcode::RSI, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RDX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_string(code, asmcode::RCX, asmcode::R11, error);
    jump_short_if_arg_does_not_point_to_string(code, asmcode::RDX, asmcode::R11, error);
    }
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::SAR, asmcode::RSI, asmcode::NUMBER, 1);
  auto repeat = label_to_string(label++);
  auto equal = label_to_string(label++);
  auto less = label_to_string(label++);
  auto greater = label_to_string(label++);
  auto local_equal = label_to_string(label++);
  auto test_bl = label_to_string(label++);
  auto test_again = label_to_string(label++);
  /*
  A = 65
  Z = 90
  a = 97
  z = 122
  */
  code.add(asmcode::PUSH, asmcode::RBX);
  code.add(asmcode::LABEL, repeat);
  code.add(asmcode::TEST, asmcode::RSI, asmcode::RSI);
  code.add(asmcode::JES, equal);
  code.add(asmcode::MOV, asmcode::AL, asmcode::BYTE_MEM_RCX);
  code.add(asmcode::MOV, asmcode::BL, asmcode::BYTE_MEM_RDX);
  code.add(asmcode::CMP, asmcode::AL, asmcode::BL);
  code.add(asmcode::JES, local_equal);
  code.add(asmcode::CMP, asmcode::AL, asmcode::NUMBER, 65);
  code.add(asmcode::JLS, test_bl);
  code.add(asmcode::CMP, asmcode::AL, asmcode::NUMBER, 90);
  code.add(asmcode::JGS, test_bl);
  code.add(asmcode::ADD, asmcode::AL, asmcode::NUMBER, 32);
  code.add(asmcode::LABEL, test_bl);
  code.add(asmcode::CMP, asmcode::BL, asmcode::NUMBER, 65);
  code.add(asmcode::JLS, test_again);
  code.add(asmcode::CMP, asmcode::BL, asmcode::NUMBER, 90);
  code.add(asmcode::JGS, test_again);
  code.add(asmcode::ADD, asmcode::BL, asmcode::NUMBER, 32);
  code.add(asmcode::LABEL, test_again);
  code.add(asmcode::CMP, asmcode::AL, asmcode::BL);
  code.add(asmcode::JLS, less);
  code.add(asmcode::JGS, greater);
  code.add(asmcode::LABEL, local_equal);
  code.add(asmcode::INC, asmcode::RCX);
  code.add(asmcode::INC, asmcode::RDX);
  code.add(asmcode::DEC, asmcode::RSI);
  code.add(asmcode::JMPS, repeat);
  code.add(asmcode::LABEL, equal);
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::POP, asmcode::RBX);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, less);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 0xfffffffffffffffe);
  code.add(asmcode::POP, asmcode::RBX);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, greater);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 2);
  code.add(asmcode::POP, asmcode::RBX);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_compare_strings_ci_contract_violation);
    }
  }

void compile_make_promise(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    if (ops.safe_promises)
      {
      code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 2);
      check_heap(code, re_make_promise_heap_overflow);
      }
    }
  if (ops.safe_primitives)
    {
    code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
    jump_if_arg_does_not_point_to_closure(code, asmcode::RCX, asmcode::R11, error);
    code.add(asmcode::OR, asmcode::RCX, asmcode::NUMBER, block_tag);
    }

  uint64_t header = make_block_header(1, T_PROMISE);
  code.add(asmcode::MOV, asmcode::R11, ALLOC);
  code.add(asmcode::OR, asmcode::R11, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::RCX);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R11);

  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_make_promise_contract_violation);
    }
  }

void compile_undefined(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, skiwi_undefined);
  code.add(asmcode::JMP, CONTINUE);
  }

void compile_skiwi_quiet_undefined(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, skiwi_quiet_undefined);
  code.add(asmcode::JMP, CONTINUE);
  }

void compile_load(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_string(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));

  /*
Windows:
rcx
Linux:
rdi
*/
#ifndef WIN32
  code.add(asmcode::PUSH, asmcode::RDI);
  code.add(asmcode::MOV, asmcode::RDI, asmcode::RCX);
#endif

  code.add(asmcode::MOV, ALLOC_SAVED, ALLOC); // this line is here so that our foreign calls can access free heap memory
  save_before_foreign_call(code);
  align_stack(code);
  code.add(asmcode::MOV, asmcode::R15, CONTEXT); // r15 should be saved by the callee but r10 not, so we save the context in r15
#ifdef _WIN32
  code.add(asmcode::SUB, asmcode::RSP, asmcode::NUMBER, 32);
#else
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
#endif  
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&c_prim_load);
  code.add(asmcode::CALL, asmcode::R11);
  code.add(asmcode::MOV, CONTEXT, asmcode::R15); // now we restore the context
  restore_stack(code);
  restore_after_foreign_call(code);
  code.add(asmcode::MOV, ALLOC, ALLOC_SAVED); // foreign calls should have updated free heap memory if they used some
#ifndef WIN32
  code.add(asmcode::POP, asmcode::RDI);
#endif
  // now check whether rax contains an error: if so we jump to ERROR
  code.add(asmcode::MOV, asmcode::RCX, asmcode::RAX);
  code.add(asmcode::AND, asmcode::CL, asmcode::NUMBER, error_mask);
  code.add(asmcode::CMP, asmcode::CL, asmcode::NUMBER, error_tag);
  std::string error_in_load = label_to_string(label++);
  code.add(asmcode::JE, error_in_load);
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_load_contract_violation);
    }
  code.add(asmcode::LABEL, error_in_load);
  code.add(asmcode::JMP, ERROR);
  }


void compile_eval(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_string(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));

  /*
Windows:
rcx
Linux:
rdi
*/
#ifndef WIN32
  code.add(asmcode::PUSH, asmcode::RDI);
  code.add(asmcode::MOV, asmcode::RDI, asmcode::RCX);
#endif

  code.add(asmcode::MOV, ALLOC_SAVED, ALLOC); // this line is here so that our foreign calls can access free heap memory
  save_before_foreign_call(code);
  align_stack(code);
  code.add(asmcode::MOV, asmcode::R15, CONTEXT); // r15 should be saved by the callee but r10 not, so we save the context in r15
#ifdef _WIN32
  code.add(asmcode::SUB, asmcode::RSP, asmcode::NUMBER, 32);
#else
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
#endif  
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&c_prim_eval);
  code.add(asmcode::CALL, asmcode::R11);
  code.add(asmcode::MOV, CONTEXT, asmcode::R15); // now we restore the context
  restore_stack(code);
  restore_after_foreign_call(code);
  code.add(asmcode::MOV, ALLOC, ALLOC_SAVED); // foreign calls should have updated free heap memory if they used some
#ifndef WIN32
  code.add(asmcode::POP, asmcode::RDI);
#endif
  code.add(asmcode::JMP, CONTINUE);
  if (ops.safe_primitives)
    {
    error_label(code, error, re_eval_contract_violation);
    }
  }

namespace
  {
  const char* _my_getenv(const char* name)
    {
    static std::string env_value;
#ifdef _WIN32
    std::string s(name);
    std::wstring ws = convert_string_to_wstring(s);
    wchar_t* path = _wgetenv(ws.c_str());
    if (!path)
      return nullptr;
    std::wstring wresult(path);
    env_value = convert_wstring_to_string(wresult);
    return env_value.c_str();
#else
    return ::getenv(name);
#endif
    }

  int _my_putenv(const char* name, const char* value)
    {
#ifdef _WIN32
    std::string sname(name);
    std::string svalue(value);
    std::wstring wname = convert_string_to_wstring(sname);
    std::wstring wvalue = convert_string_to_wstring(svalue);
    return (int)_wputenv_s(wname.c_str(), wvalue.c_str());
#else
    return setenv(name, value, 1);
#endif
    }

  int _my_file_exists(const char* filename)
    {
#ifdef _WIN32
    int res = 0;
    std::string sname(filename);
    std::wstring wname = convert_string_to_wstring(sname);
    std::ifstream f(wname);
    if (f.is_open())
      {
      res = 1;
      f.close();
      }
    return res;
#else
    int res = 0;
    std::ifstream f(filename);
    if (f.is_open())
      {
      res = 1;
      f.close();
      }
    return res;
#endif
    }

  uint64_t _current_seconds()
    {
    uint64_t secondsUTC = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return secondsUTC;
    }

  uint64_t _current_milliseconds()
    {
    uint64_t millisecondsUTC = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return millisecondsUTC;
    }
  }

void compile_current_seconds(ASM::asmcode& code, const compiler_options& /*options*/)
  {
  save_before_foreign_call(code);
  align_stack(code);
  code.add(asmcode::MOV, asmcode::R15, CONTEXT); // r15 should be saved by the callee but r10 not, so we save the context in r15
#ifdef _WIN32
  code.add(asmcode::SUB, asmcode::RSP, asmcode::NUMBER, 32);
#else
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
#endif  
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&_current_seconds);
  code.add(asmcode::CALL, asmcode::R11);
  code.add(asmcode::MOV, CONTEXT, asmcode::R15); // now we restore the context
  restore_stack(code);
  restore_after_foreign_call(code);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::JMP, CONTINUE);
  }

void compile_current_milliseconds(ASM::asmcode& code, const compiler_options& /*options*/)
  {
  save_before_foreign_call(code);
  align_stack(code);
  code.add(asmcode::MOV, asmcode::R15, CONTEXT); // r15 should be saved by the callee but r10 not, so we save the context in r15
#ifdef _WIN32
  code.add(asmcode::SUB, asmcode::RSP, asmcode::NUMBER, 32);
#else
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
#endif  
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&_current_milliseconds);
  code.add(asmcode::CALL, asmcode::R11);
  code.add(asmcode::MOV, CONTEXT, asmcode::R15); // now we restore the context
  restore_stack(code);
  restore_after_foreign_call(code);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::JMP, CONTINUE);
  }

void compile_getenv(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_string(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));

  /*
Windows:
rcx
Linux:
rdi
*/
#ifndef WIN32
  code.add(asmcode::PUSH, asmcode::RDI);
  code.add(asmcode::MOV, asmcode::RDI, asmcode::RCX);
#endif

  save_before_foreign_call(code);
  align_stack(code);
  code.add(asmcode::MOV, asmcode::R15, CONTEXT); // r15 should be saved by the callee but r10 not, so we save the context in r15
#ifdef _WIN32
  code.add(asmcode::SUB, asmcode::RSP, asmcode::NUMBER, 32);
#else
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
#endif  
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&_my_getenv);
  code.add(asmcode::CALL, asmcode::R11);
  code.add(asmcode::MOV, CONTEXT, asmcode::R15); // now we restore the context
  restore_stack(code);
  restore_after_foreign_call(code);
#ifndef WIN32
  code.add(asmcode::POP, asmcode::RDI);
#endif
  std::string false_label = label_to_string(label++);
  code.add(asmcode::CMP, asmcode::RAX, asmcode::NUMBER, 0);
  code.add(asmcode::JE, false_label);
  code.add(asmcode::MOV, asmcode::RCX, asmcode::RAX); // rcx points to string now
  code.add(asmcode::PUSH, asmcode::RCX);
  raw_string_length(code, asmcode::RCX); //r15 contains string length
  code.add(asmcode::POP, asmcode::RCX);
  if (ops.safe_primitives)
    {
    code.add(asmcode::MOV, asmcode::R11, asmcode::R15); // save length in r11
    code.add(asmcode::MOV, asmcode::RAX, asmcode::R15); // put length in rax    
    code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 3);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, 2);
    check_heap(code, re_getenv_heap_overflow);
    code.add(asmcode::MOV, asmcode::R15, asmcode::R11); // length again in r15
    }

  code.add(asmcode::MOV, asmcode::RAX, asmcode::R15);
  code.add(asmcode::SHR, asmcode::R15, asmcode::NUMBER, 3);
  code.add(asmcode::INC, asmcode::R15);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)string_tag << (uint64_t)block_shift);
  code.add(asmcode::OR, asmcode::R15, asmcode::R11);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::R15);
  code.add(asmcode::MOV, asmcode::R15, ALLOC);
  code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));

  auto repeat = label_to_string(label++);
  auto done = label_to_string(label++);

  code.add(asmcode::MOV, asmcode::R11, asmcode::RAX);
  code.add(asmcode::LABEL, repeat);
  code.add(asmcode::TEST, asmcode::R11, asmcode::R11);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::AL, asmcode::BYTE_MEM_RCX);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, asmcode::AL);
  code.add(asmcode::DEC, asmcode::R11);
  code.add(asmcode::INC, ALLOC);
  code.add(asmcode::INC, asmcode::RCX);
  code.add(asmcode::JMPS, repeat);
  code.add(asmcode::LABEL, done);

  code.add(asmcode::MOV, BYTE_MEM_ALLOC, asmcode::NUMBER, 0);
  code.add(asmcode::INC, ALLOC);

  auto repeat2 = label_to_string(label++);
  auto done2 = label_to_string(label++);
  code.add(asmcode::LABEL, repeat2);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 7);
  code.add(asmcode::AND, asmcode::RAX, ALLOC);
  code.add(asmcode::TEST, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::JES, done2);
  code.add(asmcode::MOV, BYTE_MEM_ALLOC, asmcode::NUMBER, 0);
  code.add(asmcode::INC, ALLOC);
  code.add(asmcode::JMPS, repeat2);
  code.add(asmcode::LABEL, done2);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R15);
  code.add(asmcode::JMP, CONTINUE);

  code.add(asmcode::LABEL, false_label);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_getenv_contract_violation);
    }
  }

void compile_putenv(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 2);
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    jump_if_arg_is_not_block(code, asmcode::RDX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RDX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_string(code, asmcode::RCX, asmcode::R11, error);
    jump_if_arg_does_not_point_to_string(code, asmcode::RDX, asmcode::R11, error);
    }
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::ADD, asmcode::RDX, asmcode::NUMBER, CELLS(1));
  /*
Windows:
rcx:
rdx:

Linux:
rdi:
rsi:
*/

#ifndef WIN32
  code.add(asmcode::PUSH, asmcode::RDI);
  code.add(asmcode::PUSH, asmcode::RSI);
  code.add(asmcode::MOV, asmcode::RDI, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::RSI, asmcode::RDX);
#endif

  save_before_foreign_call(code);
  align_stack(code);
  code.add(asmcode::MOV, asmcode::R15, CONTEXT); // r15 should be saved by the callee but r10 not, so we save the context in r15
#ifdef _WIN32
  code.add(asmcode::SUB, asmcode::RSP, asmcode::NUMBER, 32);
#else
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
#endif  
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&_my_putenv);
  code.add(asmcode::CALL, asmcode::R11);
  code.add(asmcode::MOV, CONTEXT, asmcode::R15); // now we restore the context
  restore_stack(code);
  restore_after_foreign_call(code);
#ifndef WIN32
  code.add(asmcode::POP, asmcode::RSI);
  code.add(asmcode::POP, asmcode::RDI);
#endif

  std::string success = label_to_string(label++);
  code.add(asmcode::TEST, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::JES, success);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, success);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_putenv_contract_violation);
    }
  }


void compile_file_exists(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    code.add(asmcode::CMP, asmcode::R11, asmcode::NUMBER, 1);
    code.add(asmcode::JNE, error);
    jump_if_arg_is_not_block(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::AND, asmcode::RCX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_if_arg_does_not_point_to_string(code, asmcode::RCX, asmcode::R11, error);
    }
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));
  /*
Windows:
rcx:
Linux:
rdi:
*/

#ifndef WIN32
  code.add(asmcode::PUSH, asmcode::RDI);
  code.add(asmcode::MOV, asmcode::RDI, asmcode::RCX);
#endif

  save_before_foreign_call(code);
  align_stack(code);
  code.add(asmcode::MOV, asmcode::R15, CONTEXT); // r15 should be saved by the callee but r10 not, so we save the context in r15
#ifdef _WIN32
  code.add(asmcode::SUB, asmcode::RSP, asmcode::NUMBER, 32);
#else
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
#endif  
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&_my_file_exists);
  code.add(asmcode::CALL, asmcode::R11);
  code.add(asmcode::MOV, CONTEXT, asmcode::R15); // now we restore the context
  restore_stack(code);
  restore_after_foreign_call(code);
#ifndef WIN32
  code.add(asmcode::POP, asmcode::RDI);
#endif

  std::string does_not_exists = label_to_string(label++);
  code.add(asmcode::TEST, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::JES, does_not_exists);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMP, CONTINUE);
  code.add(asmcode::LABEL, does_not_exists);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMP, CONTINUE);

  if (ops.safe_primitives)
    {
    error_label(code, error, re_file_exists_contract_violation);
    }
  }

SKIWI_END
