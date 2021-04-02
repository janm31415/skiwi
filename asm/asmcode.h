#pragma once

#include <list>
#include <vector>
#include <string>
#include <stdint.h>
#include <ostream>
#include "namespace.h"
#include "asm_api.h"

ASM_BEGIN

class asmcode
  {
  public:
    enum operation
      {
      ADD,
      ADDSD,      
      AND,
      CALL,
      CALLEXTERNAL,
      CMP,
      CMPEQPD,
      CMPLTPD,
      CMPLEPD,
      COMMENT,
      CQO,      
      CVTSI2SD,      
      CVTTSD2SI,
      DEC,
      DIV,
      DIVSD,  
      EXTERN,
      F2XM1,
      FADD,                  
      FILD,      
      FISTPQ,      
      FLD,
      FLD1,
      FLDLN2,      
      FLDPI,
      FMUL,      
      FSIN,
      FCOS,
      FPATAN,      
      FPTAN,
      FRNDINT,
      FSCALE,
      FSQRT,
      FSTP,      
      FSUB,      
      FSUBRP,
      FXCH,
      FYL2X,
      GLOBAL,
      IDIV,
      IMUL,
      INC,
      JMP,
      JA,
      JB,
      JE,
      JL,
      JLE,
      JG,
      JGE,
      JNE,
      JMPS, // JMP short
      JAS, // JA short
      JBS, // JB short
      JES, // JE short
      JLS, // JL short
      JLES, // JLE short
      JGS, // JG short
      JGES, // JGE short
      JNES, // JNE short      
      LABEL,
      LABEL_ALIGNED,
      MOV,      
      MOVQ,      
      MOVMSKPD,      
      MOVSD,
      MOVZX,
      MUL,
      MULSD,      
      NEG,
      NOP,
      OR,
      POP,
      PUSH,
      RET,
      SAL,
      SAR,      
      SETE,
      SETNE,
      SETL,
      SETG,
      SETLE,
      SETGE,
      SHL,      
      SHR,
      SQRTPD,      
      SUB,
      SUBSD,      
      TEST,
      UCOMISD,      
      XOR,
      XORPD
      };

    enum operand
      {
      EMPTY,
      AL,
      AH,
      BL,
      BH,
      CL,
      CH,
      DL,
      DH,   
      RAX,
      RBX,
      RCX,
      RDX,
      RSI,
      RDI,
      RSP,
      RBP,
      R8,
      R9,
      R10,
      R11,
      R12,
      R13,
      R14,
      R15,  
      MEM_RAX,
      MEM_RBX,
      MEM_RCX,
      MEM_RDX,
      MEM_RDI,
      MEM_RSI,
      MEM_RSP,
      MEM_RBP,
      MEM_R8,
      MEM_R9,
      MEM_R10,
      MEM_R11,
      MEM_R12,
      MEM_R13,
      MEM_R14,
      MEM_R15,
      BYTE_MEM_RAX,
      BYTE_MEM_RBX,
      BYTE_MEM_RCX,
      BYTE_MEM_RDX,
      BYTE_MEM_RDI,
      BYTE_MEM_RSI,
      BYTE_MEM_RSP,
      BYTE_MEM_RBP,
      BYTE_MEM_R8,
      BYTE_MEM_R9,
      BYTE_MEM_R10,
      BYTE_MEM_R11,
      BYTE_MEM_R12,
      BYTE_MEM_R13,
      BYTE_MEM_R14,
      BYTE_MEM_R15,
      NUMBER,
      ST0,
      ST1,
      ST2,
      ST3,
      ST4,
      ST5,
      ST6,
      ST7,    
      LABELADDRESS,
      XMM0,
      XMM1,
      XMM2,
      XMM3,
      XMM4,
      XMM5,
      XMM6,
      XMM7,
      XMM8,
      XMM9,
      XMM10,
      XMM11,
      XMM12,
      XMM13,
      XMM14,
      XMM15 
      };   

    struct instruction
      {
      operation oper;
      operand operand1, operand2;
      uint64_t operand1_mem, operand2_mem;
      std::string text;

      ASSEMBLER_API instruction();
      ASSEMBLER_API instruction(const std::string& text);
      ASSEMBLER_API instruction(operation op);
      ASSEMBLER_API instruction(operation op, operand op1);
      ASSEMBLER_API instruction(operation op, operand op1, operand op2);
      ASSEMBLER_API instruction(operation op, operand op1, uint64_t op1_mem);
      ASSEMBLER_API instruction(operation op, operand op1, uint64_t op1_mem, operand op2);
      ASSEMBLER_API instruction(operation op, operand op1, uint64_t op1_mem, operand op2, uint64_t op2_mem);
      ASSEMBLER_API instruction(operation op, operand op1, operand op2, uint64_t op2_mem);      
      ASSEMBLER_API instruction(operation op, const std::string& text);
      ASSEMBLER_API instruction(operation op, operand op1, const std::string& text);
      ASSEMBLER_API instruction(operation op, operand op1, operand op2, const std::string& text);
      ASSEMBLER_API instruction(operation op, operand op1, uint64_t op1_mem, const std::string& text);
      ASSEMBLER_API instruction(operation op, operand op1, uint64_t op1_mem, operand op2, const std::string& text);
      ASSEMBLER_API instruction(operation op, operand op1, uint64_t op1_mem, operand op2, uint64_t op2_mem, const std::string& text);
      ASSEMBLER_API instruction(operation op, operand op1, operand op2, uint64_t op2_mem, const std::string& text);

      ASSEMBLER_API void stream(std::ostream& out) const;
      ASSEMBLER_API uint64_t fill_opcode(uint8_t* opcode_stream) const;
      };

    ASSEMBLER_API asmcode();
    ASSEMBLER_API ~asmcode();

    template <class... T>
    void add(T&&... val)
      {
      instruction ins(std::forward<T>(val)...);
      (*instructions_list_stack.back()).push_back(ins);
      }

    ASSEMBLER_API void push();
    ASSEMBLER_API void pop();

    ASSEMBLER_API const std::list<std::vector<instruction>>& get_instructions_list() const;
    ASSEMBLER_API std::list<std::vector<instruction>>& get_instructions_list();

    ASSEMBLER_API void stream(std::ostream& out) const;

    ASSEMBLER_API void clear();

    ASSEMBLER_API static std::string operation_to_string(operation op);
    ASSEMBLER_API static std::string operand_to_string(operand op);

  private:
    std::list<std::vector<instruction>> instructions_list;
    std::vector<std::list<std::vector<instruction>>::iterator> instructions_list_stack;


  };

ASM_END
