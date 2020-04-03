#include "asm_aux.h"
#include "types.h"
#include "context.h"
#include "context_defs.h"
#include "globals.h"
#include <sstream>

SKIWI_BEGIN

asmcode::operand get_mem_operand(asmcode::operand reg)
  {
  switch (reg)
    {
    case asmcode::RAX: return asmcode::MEM_RAX;
    case asmcode::RBX: return asmcode::MEM_RBX;
    case asmcode::RCX: return asmcode::MEM_RCX;
    case asmcode::RDX: return asmcode::MEM_RDX;
    case asmcode::RSI: return asmcode::MEM_RSI;
    case asmcode::RDI: return asmcode::MEM_RDI;
    case asmcode::R8: return asmcode::MEM_R8;
    case asmcode::R9: return asmcode::MEM_R9;
    case asmcode::R10: return asmcode::MEM_R10;
    case asmcode::R11: return asmcode::MEM_R11;
    case asmcode::R12: return asmcode::MEM_R12;
    case asmcode::R13: return asmcode::MEM_R13;
    case asmcode::R14: return asmcode::MEM_R14;
    case asmcode::R15: return asmcode::MEM_R15;
    default: throw std::runtime_error("not implemented");
    }
  }

asmcode::operand get_byte_mem_operand(asmcode::operand reg)
  {
  switch (reg)
    {
    case asmcode::RAX: return asmcode::BYTE_MEM_RAX;
    case asmcode::RBX: return asmcode::BYTE_MEM_RBX;
    case asmcode::RCX: return asmcode::BYTE_MEM_RCX;
    case asmcode::RDX: return asmcode::BYTE_MEM_RDX;
    case asmcode::RSI: return asmcode::BYTE_MEM_RSI;
    case asmcode::RDI: return asmcode::BYTE_MEM_RDI;
    case asmcode::R8: return asmcode::BYTE_MEM_R8;
    case asmcode::R9: return asmcode::BYTE_MEM_R9;
    case asmcode::R10: return asmcode::BYTE_MEM_R10;
    case asmcode::R11: return asmcode::BYTE_MEM_R11;
    case asmcode::R12: return asmcode::BYTE_MEM_R12;
    case asmcode::R13: return asmcode::BYTE_MEM_R13;
    case asmcode::R14: return asmcode::BYTE_MEM_R14;
    case asmcode::R15: return asmcode::BYTE_MEM_R15;
    default: throw std::runtime_error("not implemented");
    }
  }


std::string label_to_string(uint64_t lab)
  {
  std::stringstream str;
  str << "L_" << lab;
  return str.str();
  }


void store_registers(asmcode& code)
  {
  /*
  linux: r12, r13, r14, r15, rbx, rsp, rbp should be preserved
  windows: r12, r13, r14, r15, rbx, rsp, rbp, rdi, rsi
  */
  code.add(asmcode::MOV, RBX_STORE, asmcode::RBX);
  code.add(asmcode::MOV, RDI_STORE, asmcode::RDI);
  code.add(asmcode::MOV, RSI_STORE, asmcode::RSI);
  code.add(asmcode::MOV, RSP_STORE, asmcode::RSP);
  code.add(asmcode::MOV, RBP_STORE, asmcode::RBP);
  code.add(asmcode::MOV, R12_STORE, asmcode::R12);
  code.add(asmcode::MOV, R13_STORE, asmcode::R13);
  code.add(asmcode::MOV, R14_STORE, asmcode::R14);
  code.add(asmcode::MOV, R15_STORE, asmcode::R15);
  }

void load_registers(asmcode& code)
  {
  /*
  linux: r12, r13, r14, r15, rbx, rsp, rbp should be preserved
  windows: r12, r13, r14, r15, rbx, rsp, rbp, rdi, rsi
  */
  code.add(asmcode::MOV, asmcode::RBX, RBX_STORE);
  code.add(asmcode::MOV, asmcode::RDI, RDI_STORE);
  code.add(asmcode::MOV, asmcode::RSI, RSI_STORE);
  code.add(asmcode::MOV, asmcode::RSP, RSP_STORE);
  code.add(asmcode::MOV, asmcode::RBP, RBP_STORE);
  code.add(asmcode::MOV, asmcode::R12, R12_STORE);
  code.add(asmcode::MOV, asmcode::R13, R13_STORE);
  code.add(asmcode::MOV, asmcode::R14, R14_STORE);
  code.add(asmcode::MOV, asmcode::R15, R15_STORE);
  }

int64_t int2fixnum(int64_t i)
  {
  return (i << fixnum_shift) | fixnum_tag;
  }

int64_t fixnum2int(int64_t i)
  {
  return i >> fixnum_shift;
  }

void error_label(asmcode& code, const std::string& label_name, runtime_error re)
  {
  code.add(asmcode::LABEL, label_name);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 8);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, (uint64_t)re);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 8);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, error_tag);
  code.add(asmcode::JMP, ERROR);
  }

namespace
  {

  void _jump_if_arg_is_not_block(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name, bool jmp_short)
    {
    code.add(asmcode::MOV, free_reg, arg);
    code.add(asmcode::AND, free_reg, asmcode::NUMBER, block_mask);
    code.add(asmcode::CMP, free_reg, asmcode::NUMBER, block_tag);
    if (jmp_short)
      code.add(asmcode::JNES, label_name);
    else
      code.add(asmcode::JNE, label_name);
    }

  void _jump_if_arg_does_not_point_to_type(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name, bool jmp_short, uint64_t typ)
    {
    code.add(asmcode::MOV, free_reg, get_mem_operand(arg));
    code.add(asmcode::SHR, free_reg, asmcode::NUMBER, block_shift);
    code.add(asmcode::AND, free_reg, asmcode::NUMBER, block_header_mask);
    code.add(asmcode::CMP, free_reg, asmcode::NUMBER, typ);
    if (jmp_short)
      code.add(asmcode::JNES, label_name);
    else
      code.add(asmcode::JNE, label_name);
    }

  void _jump_if_arg_does_not_point_to_flonum(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name, bool jmp_short)
    {
    _jump_if_arg_does_not_point_to_type(code, arg, free_reg, label_name, jmp_short, flonum_tag);
    }

  void _jump_if_arg_does_not_point_to_closure(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name, bool jmp_short)
    {
    _jump_if_arg_does_not_point_to_type(code, arg, free_reg, label_name, jmp_short, closure_tag);
    }

  void _jump_if_arg_does_not_point_to_vector(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name, bool jmp_short)
    {
    _jump_if_arg_does_not_point_to_type(code, arg, free_reg, label_name, jmp_short, vector_tag);
    }

  void _jump_if_arg_does_not_point_to_symbol(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name, bool jmp_short)
    {
    _jump_if_arg_does_not_point_to_type(code, arg, free_reg, label_name, jmp_short, symbol_tag);
    }

  void _jump_if_arg_does_not_point_to_promise(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name, bool jmp_short)
    {
    _jump_if_arg_does_not_point_to_type(code, arg, free_reg, label_name, jmp_short, promise_tag);
    }

  void _jump_if_arg_does_not_point_to_string(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name, bool jmp_short)
    {
    _jump_if_arg_does_not_point_to_type(code, arg, free_reg, label_name, jmp_short, string_tag);
    }

  void _jump_if_arg_does_not_point_to_pair(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name, bool jmp_short)
    {
    _jump_if_arg_does_not_point_to_type(code, arg, free_reg, label_name, jmp_short, pair_tag);
    }

  void _jump_if_arg_does_not_point_to_port(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name, bool jmp_short)
    {
    _jump_if_arg_does_not_point_to_type(code, arg, free_reg, label_name, jmp_short, port_tag);
    }
  }

void jump_short_if_arg_is_not_block(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name)
  {
  _jump_if_arg_is_not_block(code, arg, free_reg, label_name, true);
  }

void jump_short_if_arg_does_not_point_to_flonum(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name)
  {
  _jump_if_arg_does_not_point_to_flonum(code, arg, free_reg, label_name, true);
  }

void jump_short_if_arg_does_not_point_to_closure(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name)
  {
  _jump_if_arg_does_not_point_to_closure(code, arg, free_reg, label_name, true);
  }

void jump_short_if_arg_does_not_point_to_vector(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name)
  {
  _jump_if_arg_does_not_point_to_vector(code, arg, free_reg, label_name, true);
  }

void jump_short_if_arg_does_not_point_to_symbol(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name)
  {
  _jump_if_arg_does_not_point_to_symbol(code, arg, free_reg, label_name, true);
  }

void jump_short_if_arg_does_not_point_to_promise(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name)
  {
  _jump_if_arg_does_not_point_to_promise(code, arg, free_reg, label_name, true);
  }

void jump_short_if_arg_does_not_point_to_string(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name)
  {
  _jump_if_arg_does_not_point_to_string(code, arg, free_reg, label_name, true);
  }

void jump_short_if_arg_does_not_point_to_pair(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name)
  {
  _jump_if_arg_does_not_point_to_pair(code, arg, free_reg, label_name, true);
  }

void jump_short_if_arg_does_not_point_to_port(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name)
  {
  _jump_if_arg_does_not_point_to_port(code, arg, free_reg, label_name, true);
  }

void jump_if_arg_is_not_block(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name)
  {
  _jump_if_arg_is_not_block(code, arg, free_reg, label_name, false);
  }

void jump_if_arg_does_not_point_to_flonum(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name)
  {
  _jump_if_arg_does_not_point_to_flonum(code, arg, free_reg, label_name, false);
  }

void jump_if_arg_does_not_point_to_closure(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name)
  {
  _jump_if_arg_does_not_point_to_closure(code, arg, free_reg, label_name, false);
  }

void jump_if_arg_does_not_point_to_vector(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name)
  {
  _jump_if_arg_does_not_point_to_vector(code, arg, free_reg, label_name, false);
  }

void jump_if_arg_does_not_point_to_symbol(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name)
  {
  _jump_if_arg_does_not_point_to_symbol(code, arg, free_reg, label_name, false);
  }

void jump_if_arg_does_not_point_to_promise(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name)
  {
  _jump_if_arg_does_not_point_to_promise(code, arg, free_reg, label_name, false);
  }

void jump_if_arg_does_not_point_to_string(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name)
  {
  _jump_if_arg_does_not_point_to_string(code, arg, free_reg, label_name, false);
  }

void jump_if_arg_does_not_point_to_pair(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name)
  {
  _jump_if_arg_does_not_point_to_pair(code, arg, free_reg, label_name, false);
  }

void jump_if_arg_does_not_point_to_port(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name)
  {
  _jump_if_arg_does_not_point_to_port(code, arg, free_reg, label_name, false);
  }

void check_heap(asmcode& code, runtime_error re)
  {
  // clobbers rax and r15
  auto heap_ok = label_to_string(label++);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::ADD, asmcode::RAX, ALLOC);
  code.add(asmcode::MOV, asmcode::R15, FROM_SPACE_END);
  code.add(asmcode::CMP, asmcode::RAX, asmcode::R15);
  code.add(asmcode::JLS, heap_ok);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, (uint64_t)re);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 8);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, error_tag);
  code.add(asmcode::JMP, ERROR);
  code.add(asmcode::LABEL, heap_ok);
  }

void save_before_foreign_call(asmcode& code)
  {
  /*
  sub rsp, CELLS(8)
  mov [rsp + CELLS(0)], rbx
  mov [rsp + CELLS(1)], rcx
  mov [rsp + CELLS(2)], rdx
  mov [rsp + CELLS(3)], rsi
  mov [rsp + CELLS(4)], rdi
  mov [rsp + CELLS(5)], r8
  mov [rsp + CELLS(6)], r9
  mov [rsp + CELLS(7)], r10
  */
  code.add(asmcode::SUB, asmcode::RSP, asmcode::NUMBER, CELLS(8));
  code.add(asmcode::MOV, asmcode::MEM_RSP, CELLS(0), asmcode::RBX);
  code.add(asmcode::MOV, asmcode::MEM_RSP, CELLS(1), asmcode::RCX);
  code.add(asmcode::MOV, asmcode::MEM_RSP, CELLS(2), asmcode::RDX);
  code.add(asmcode::MOV, asmcode::MEM_RSP, CELLS(3), asmcode::RSI);
  code.add(asmcode::MOV, asmcode::MEM_RSP, CELLS(4), asmcode::RDI);
  code.add(asmcode::MOV, asmcode::MEM_RSP, CELLS(5), asmcode::R8);
  code.add(asmcode::MOV, asmcode::MEM_RSP, CELLS(6), asmcode::R9);
  code.add(asmcode::MOV, asmcode::MEM_RSP, CELLS(7), asmcode::R10);

  code.add(asmcode::MOV, STACK, STACK_REGISTER); // We save the current stack position in the context, so that any load or eval functions start from the correct stack position.
                                                 // This is important for the garbage collector.
  }

void restore_after_foreign_call(asmcode& code)
  {
  /*
  mov r10, [rsp + CELLS(7)]
  mov r9, [rsp + CELLS(6)]
  mov r8, [rsp + CELLS(5)]
  mov rdi, [rsp + CELLS(4)]
  mov rsi, [rsp + CELLS(3)]
  mov rdx, [rsp + CELLS(2)]
  mov rcx, [rsp + CELLS(1)]
  mov rbx, [rsp + CELLS(0)]
  add rsp, CELLS(8)
  */
  code.add(asmcode::MOV, asmcode::R10, asmcode::MEM_RSP, CELLS(7));
  code.add(asmcode::MOV, asmcode::R9, asmcode::MEM_RSP, CELLS(6));
  code.add(asmcode::MOV, asmcode::R8, asmcode::MEM_RSP, CELLS(5));
  code.add(asmcode::MOV, asmcode::RDI, asmcode::MEM_RSP, CELLS(4));
  code.add(asmcode::MOV, asmcode::RSI, asmcode::MEM_RSP, CELLS(3));
  code.add(asmcode::MOV, asmcode::RDX, asmcode::MEM_RSP, CELLS(2));
  code.add(asmcode::MOV, asmcode::RCX, asmcode::MEM_RSP, CELLS(1));
  code.add(asmcode::MOV, asmcode::RBX, asmcode::MEM_RSP, CELLS(0));
  code.add(asmcode::ADD, asmcode::RSP, asmcode::NUMBER, CELLS(8));

  code.add(asmcode::MOV, STACK_REGISTER, STACK); // We update the stack register with the current stack position. The external function might have called load or eval which
                                                 // might update the scheme stack. This is important for the garbage collector.
  }

void align_stack(asmcode& code)
  {
  /*
    mov qword [rsp_save], rsp
  and rsp, [rsp_alignment_mask]
  */
  code.add(asmcode::MOV, RSP_SAVE, asmcode::RSP);
  code.add(asmcode::AND, asmcode::RSP, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF0);
  }

void restore_stack(asmcode& code)
  {
  //mov rsp, qword [rsp_save]
  code.add(asmcode::MOV, asmcode::RSP, RSP_SAVE);
  }

void copy_string_to_buffer(asmcode& code)
  {
  // assumptions:
  // input: buffer is in BUFFER
  //        string is in rcx
  // output: result is in rax
  // clobbers r11 and r15


  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, block_size_mask);
  code.add(asmcode::AND, asmcode::RAX, asmcode::R11);
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::R11, BUFFER);
  std::string fill = label_to_string(label++);
  std::string done = label_to_string(label++);
  code.add(asmcode::LABEL, fill);
  code.add(asmcode::TEST, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::JES, done);
  code.add(asmcode::MOV, asmcode::R15, asmcode::MEM_RCX);
  code.add(asmcode::MOV, asmcode::MEM_R11, asmcode::R15);
  code.add(asmcode::ADD, asmcode::R11, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::ADD, asmcode::RCX, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::DEC, asmcode::RAX);
  code.add(asmcode::JMPS, fill);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::MOV, asmcode::MEM_R11, asmcode::NUMBER, 0);
  }

void allocate_buffer_as_string(asmcode& code)
  {
  // assumptions:
  // input: buffer is in BUFFER
  //        size of string is in rax
  // output: result is in rax
  // clobbers r11 and r15
  // does not check the heap
  code.add(asmcode::PUSH, asmcode::RCX);
  code.add(asmcode::MOV, asmcode::R11, BUFFER);
  code.add(asmcode::ADD, asmcode::R11, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::MEM_R11, asmcode::NUMBER, 0); // make closing zeros at end of string in buffer
  code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::INC, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::RCX, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R15, asmcode::NUMBER, (uint64_t)string_tag << (uint64_t)block_shift);
  code.add(asmcode::OR, asmcode::RAX, asmcode::R15);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R15, ALLOC);
  code.add(asmcode::OR, asmcode::R15, asmcode::NUMBER, block_tag);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(1));

  std::string fill = label_to_string(label++);
  std::string done = label_to_string(label++);

  code.add(asmcode::MOV, asmcode::R11, BUFFER);
  

  code.add(asmcode::LABEL, fill);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_R11);
  code.add(asmcode::MOV, asmcode::MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::ADD, asmcode::R11, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::ADD, asmcode::ALLOC, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::DEC, asmcode::RCX);
  code.add(asmcode::TEST, asmcode::RCX, asmcode::RCX);
  code.add(asmcode::JES, done);
  code.add(asmcode::JMPS, fill);
  code.add(asmcode::LABEL, done);

  code.add(asmcode::POP, asmcode::RCX);

  code.add(asmcode::MOV, asmcode::RAX, asmcode::R15);
  }

void raw_string_length(asmcode& code, asmcode::operand string_reg)
  {
  /*
  input is a const char* or a char*, not a scheme string
  clobbers rax, r11, r15, and string_reg
  result is in r15
  string_reg points to the string
  */
  std::string stringlengthdone = label_to_string(label++);
  std::string repeat = label_to_string(label++);

  code.add(asmcode::XOR, asmcode::R15, asmcode::R15);
  code.add(asmcode::LABEL, repeat);
  code.add(asmcode::MOV, asmcode::AL, get_byte_mem_operand(string_reg));
  code.add(asmcode::CMP, asmcode::AL, asmcode::NUMBER, 0);
  code.add(asmcode::JES, stringlengthdone);
  code.add(asmcode::INC, asmcode::R15);
  code.add(asmcode::INC, string_reg);
  code.add(asmcode::JMPS, repeat);

  code.add(asmcode::LABEL, stringlengthdone);
  // r15 now contains the string length
  }

void string_length(asmcode& code, asmcode::operand string_reg)
  {
  /*
  input points to a scheme string (thus not char* or const char*, see string_length_raw)
  clobbers rax, r11, r15, and string_reg
  result is in r15
  string_reg points to the string
  */
  std::string stringlengthdone = label_to_string(label++);

  code.add(asmcode::MOV, asmcode::R15, get_mem_operand(string_reg));
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, block_size_mask);
  code.add(asmcode::AND, asmcode::R15, asmcode::R11);
  code.add(asmcode::SHL, asmcode::R15, asmcode::NUMBER, 3);
  code.add(asmcode::ADD, string_reg, asmcode::R15);
  code.add(asmcode::SUB, asmcode::R15, asmcode::NUMBER, CELLS(1));
  code.add(asmcode::MOV, asmcode::AL, get_byte_mem_operand(string_reg));
  code.add(asmcode::CMP, asmcode::AL, asmcode::NUMBER, 0);
  code.add(asmcode::JES, stringlengthdone);
  code.add(asmcode::INC, asmcode::R15);
  code.add(asmcode::INC, string_reg);

  code.add(asmcode::MOV, asmcode::AL, get_byte_mem_operand(string_reg));
  code.add(asmcode::CMP, asmcode::AL, asmcode::NUMBER, 0);
  code.add(asmcode::JES, stringlengthdone);
  code.add(asmcode::INC, asmcode::R15);
  code.add(asmcode::INC, string_reg);

  code.add(asmcode::MOV, asmcode::AL, get_byte_mem_operand(string_reg));
  code.add(asmcode::CMP, asmcode::AL, asmcode::NUMBER, 0);
  code.add(asmcode::JES, stringlengthdone);
  code.add(asmcode::INC, asmcode::R15);
  code.add(asmcode::INC, string_reg);

  code.add(asmcode::MOV, asmcode::AL, get_byte_mem_operand(string_reg));
  code.add(asmcode::CMP, asmcode::AL, asmcode::NUMBER, 0);
  code.add(asmcode::JES, stringlengthdone);
  code.add(asmcode::INC, asmcode::R15);
  code.add(asmcode::INC, string_reg);

  code.add(asmcode::MOV, asmcode::AL, get_byte_mem_operand(string_reg));
  code.add(asmcode::CMP, asmcode::AL, asmcode::NUMBER, 0);
  code.add(asmcode::JES, stringlengthdone);
  code.add(asmcode::INC, asmcode::R15);
  code.add(asmcode::INC, string_reg);

  code.add(asmcode::MOV, asmcode::AL, get_byte_mem_operand(string_reg));
  code.add(asmcode::CMP, asmcode::AL, asmcode::NUMBER, 0);
  code.add(asmcode::JES, stringlengthdone);
  code.add(asmcode::INC, asmcode::R15);
  code.add(asmcode::INC, string_reg);

  code.add(asmcode::MOV, asmcode::AL, get_byte_mem_operand(string_reg));
  code.add(asmcode::CMP, asmcode::AL, asmcode::NUMBER, 0);
  code.add(asmcode::JES, stringlengthdone);
  code.add(asmcode::INC, asmcode::R15);
  code.add(asmcode::INC, string_reg);

  code.add(asmcode::LABEL, stringlengthdone);
  // r15 now contains the string length
  }

namespace
  {
  void _break()
    {
    printf("break here\n");
    }
  }

void break_point(asmcode& code)
  {
  code.add(asmcode::PUSH, asmcode::R15);
  code.add(asmcode::PUSH, asmcode::R11);
  code.add(asmcode::PUSH, asmcode::RAX);
  save_before_foreign_call(code);
  align_stack(code);
  code.add(asmcode::MOV, asmcode::R15, CONTEXT); // r15 should be saved by the callee but r10 not, so we save the context in r15
#ifdef _WIN32
  code.add(asmcode::SUB, asmcode::RSP, asmcode::NUMBER, 32);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&_break);
#else
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, (uint64_t)&_my_read);
#endif  
  code.add(asmcode::CALL, asmcode::R11);
  code.add(asmcode::MOV, CONTEXT, asmcode::R15); // now we restore the context
  restore_stack(code);
  restore_after_foreign_call(code);
  code.add(asmcode::POP, asmcode::RAX);
  code.add(asmcode::POP, asmcode::R11);
  code.add(asmcode::POP, asmcode::R15);
  }

void push(asmcode& code, asmcode::operand reg)
  {
  code.add(asmcode::MOV, STACK_REGISTER_MEM, reg);
  code.add(asmcode::ADD, STACK_REGISTER, asmcode::NUMBER, 8);
  }

void pop(asmcode& code, asmcode::operand reg)
  {
  code.add(asmcode::SUB, STACK_REGISTER, asmcode::NUMBER, 8);
  code.add(asmcode::MOV, reg, STACK_REGISTER_MEM);
  }

SKIWI_END
