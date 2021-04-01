#pragma once

#include "namespace.h"
#include "asm_api.h"
#include "asmcode.h"
#include "assembler.h"

ASM_BEGIN

ASSEMBLER_API void* vm_bytecode(uint64_t& size, first_pass_data& d, asmcode& code, const std::map<std::string, uint64_t>& externals);
ASSEMBLER_API void* vm_bytecode(uint64_t& size, first_pass_data& d, asmcode& code);
ASSEMBLER_API void* vm_bytecode(uint64_t& size, asmcode& code, const std::map<std::string, uint64_t>& externals);
ASSEMBLER_API void* vm_bytecode(uint64_t& size, asmcode& code);

ASSEMBLER_API void free_bytecode(void* f, uint64_t size);

ASSEMBLER_API uint64_t disassemble_bytecode(asmcode::operation& op,
                                        asmcode::operand& operand1,
                                        asmcode::operand& operand2,
                                        uint64_t& operand1_mem,
                                        uint64_t& operand2_mem,
                                        const uint8_t* bytecode);
  
#define carry_flag 1
#define overflow_flag 2048
#define sign_flag 64
#define zero_flag 128

#define operand_has_8bit_mem 128

struct registers
  {
  ASSEMBLER_API registers();

  uint64_t rax;
  uint64_t rbx;
  uint64_t rcx;
  uint64_t rdx;
  uint64_t rsi;
  uint64_t rdi;
  uint64_t rsp;
  uint64_t rbp;
  uint64_t r8;
  uint64_t r9;
  uint64_t r10;
  uint64_t r11;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;
  double xmm0;
  double xmm1;
  double xmm2;
  double xmm3;
  double xmm4;
  double xmm5;
  double xmm6;
  double xmm7;
  double xmm8;
  double xmm9;
  double xmm10;
  double xmm11;
  double xmm12;
  double xmm13;
  double xmm14;
  double xmm15;

  uint16_t eflags;
  uint64_t stack[256];
  };

ASSEMBLER_API void run_bytecode(const uint8_t* bytecode, uint64_t size, registers& regs);

ASM_END