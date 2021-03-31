///////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////

#include "VMTests.h"
#include <stdint.h>
#include <iostream>
#include "asm/asmcode.h"
#include "asm/vm.h"
#include "test_assert.h"

ASM_BEGIN

namespace
  {
  void test_vm_mov_bytecode()
    {
    asmcode code;
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);

    uint64_t size;
    uint8_t* f = (uint8_t*)vm_bytecode(size, code);
    TEST_EQ(4, size);
    TEST_EQ((int)asmcode::MOV, (int)f[0]);
    TEST_EQ((int)asmcode::RAX, (int)f[1]);
    TEST_EQ((int)asmcode::RCX, (int)f[2]);
    TEST_EQ(0, (int)f[3]);

    asmcode::operation op;
    asmcode::operand operand1;
    asmcode::operand operand2;
    uint64_t operand1_mem;
    uint64_t operand2_mem;
    uint64_t sz = disassemble_bytecode(op, operand1, operand2, operand1_mem, operand2_mem, f);
    TEST_EQ(4, sz);
    TEST_EQ(asmcode::MOV, op);
    TEST_EQ(asmcode::RAX, operand1);
    TEST_EQ(asmcode::RCX, operand2);
    TEST_EQ(0, operand1_mem);
    TEST_EQ(0, operand2_mem);

    free_bytecode(f, size);
    }

  void test_vm_mov_bytecode_2()
    {
    asmcode code;
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 0x6F77206F6C6C6548);

    uint64_t size;
    uint8_t* f = (uint8_t*)vm_bytecode(size, code);
    TEST_EQ(12, size);
    TEST_EQ((int)asmcode::MOV, (int)f[0]);
    TEST_EQ((int)asmcode::RAX, (int)f[1]);
    TEST_EQ((int)asmcode::NUMBER, (int)f[2]);
    TEST_EQ(4 << 4, (int)f[3]);
    TEST_EQ(0x48, f[4]);
    TEST_EQ(0x65, f[5]);
    TEST_EQ(0x6C, f[6]);
    TEST_EQ(0x6C, f[7]);
    TEST_EQ(0x6F, f[8]);
    TEST_EQ(0x20, f[9]);
    TEST_EQ(0x77, f[10]);
    TEST_EQ(0x6F, f[11]);

    asmcode::operation op;
    asmcode::operand operand1;
    asmcode::operand operand2;
    uint64_t operand1_mem;
    uint64_t operand2_mem;
    uint64_t sz = disassemble_bytecode(op, operand1, operand2, operand1_mem, operand2_mem, f);
    TEST_EQ(12, sz);
    TEST_EQ(asmcode::MOV, op);
    TEST_EQ(asmcode::RAX, operand1);
    TEST_EQ(asmcode::NUMBER, operand2);
    TEST_EQ(0, operand1_mem);
    TEST_EQ(0x6F77206F6C6C6548, operand2_mem);

    free_bytecode(f, size);
    }

  void test_vm_nop_bytecode()
    {
    asmcode code;
    code.add(asmcode::NOP);

    uint64_t size;
    uint8_t* f = (uint8_t*)vm_bytecode(size, code);
    TEST_EQ(1, size);
    TEST_EQ((int)asmcode::NOP, (int)f[0]);

    asmcode::operation op;
    asmcode::operand operand1;
    asmcode::operand operand2;
    uint64_t operand1_mem;
    uint64_t operand2_mem;
    uint64_t sz = disassemble_bytecode(op, operand1, operand2, operand1_mem, operand2_mem, f);
    TEST_EQ(1, sz);
    TEST_EQ(asmcode::NOP, op);
    
    free_bytecode(f, size);
    }

  void test_vm_ret()
    {
    asmcode code;
    code.add(asmcode::RET);
    uint64_t size;
    uint8_t* f = (uint8_t*)vm_bytecode(size, code);
    registers reg;

    try {
      run_bytecode(f, size, reg);
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }

    free_bytecode(f, size);
    }

  void test_vm_mov()
    {
    asmcode code;
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 10);
    code.add(asmcode::RET);
    uint64_t size;
    uint8_t* f = (uint8_t*)vm_bytecode(size, code);
    registers reg;

    try {
      run_bytecode(f, size, reg);
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }

    TEST_EQ(10, reg.rax);

    free_bytecode(f, size);
    }

  void test_vm_mov_2()
    {
    asmcode code;
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, -10);
    code.add(asmcode::RET);
    uint64_t size;
    uint8_t* f = (uint8_t*)vm_bytecode(size, code);
    registers reg;

    try {
      run_bytecode(f, size, reg);
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }

    TEST_EQ(-10, reg.rax);

    free_bytecode(f, size);
    }

  void test_vm_add()
    {
    asmcode code;
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 10);
    code.add(asmcode::MOV, asmcode::RCX, asmcode::NUMBER, 12);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::RET);
    uint64_t size;
    uint8_t* f = (uint8_t*)vm_bytecode(size, code);
    registers reg;

    try {
      run_bytecode(f, size, reg);
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    TEST_EQ(22, reg.rax);
    TEST_EQ(12, reg.rcx);
    free_bytecode(f, size);
    }

  void test_vm_call()
    {
    asmcode code;
    code.add(asmcode::CALL, "L_label");
    code.add(asmcode::RET);
    code.add(asmcode::LABEL, "L_label");
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 10);
    code.add(asmcode::MOV, asmcode::RCX, asmcode::NUMBER, 12);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::RET);
    uint64_t size;
    uint8_t* f = (uint8_t*)vm_bytecode(size, code);
    registers reg;

    try
      {
      run_bytecode(f, size, reg);
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    TEST_EQ(22, reg.rax);
    TEST_EQ(12, reg.rcx);
    free_bytecode(f, size);
    }

  void test_vm_call_2()
    {
    asmcode code;
    code.add(asmcode::CALL, "L_label");
    code.add(asmcode::RET);
    code.add(asmcode::LABEL, "L_label_2");
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 10);
    code.add(asmcode::MOV, asmcode::RCX, asmcode::NUMBER, 12);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::RET);
    code.add(asmcode::LABEL, "L_label");
    code.add(asmcode::CALL, "L_label_2");
    code.add(asmcode::RET);
    uint64_t size;
    uint8_t* f = (uint8_t*)vm_bytecode(size, code);
    registers reg;

    try
      {
      run_bytecode(f, size, reg);
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    TEST_EQ(22, reg.rax);
    TEST_EQ(12, reg.rcx);
    free_bytecode(f, size);
    }

  void test_vm_jmp()
    {
    asmcode code;
    code.add(asmcode::JMP, "L_label");
    code.add(asmcode::LABEL, "L_label");
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 10);
    code.add(asmcode::MOV, asmcode::RCX, asmcode::NUMBER, 12);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::RET);
    uint64_t size;
    uint8_t* f = (uint8_t*)vm_bytecode(size, code);
    registers reg;

    try
      {
      run_bytecode(f, size, reg);
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    TEST_EQ(22, reg.rax);
    TEST_EQ(12, reg.rcx);
    free_bytecode(f, size);
    }

  void test_vm_call_aligned()
    {
    asmcode code;   
    code.add(asmcode::CALL, "L_label");
    code.add(asmcode::RET);
    code.add(asmcode::LABEL_ALIGNED, "L_label");
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 10);
    code.add(asmcode::MOV, asmcode::RCX, asmcode::NUMBER, 12);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::RET);
    uint64_t size;
    uint8_t* f = (uint8_t*)vm_bytecode(size, code);
    registers reg;

    try
      {
      run_bytecode(f, size, reg);
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    TEST_EQ(22, reg.rax);
    TEST_EQ(12, reg.rcx);
    free_bytecode(f, size);
    }

  void test_vm_call_aligned_2()
    {
    asmcode code;
    code.add(asmcode::CALL, "L_label");
    code.add(asmcode::RET);
    code.add(asmcode::LABEL_ALIGNED, "L_label_2");
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 10);
    code.add(asmcode::MOV, asmcode::RCX, asmcode::NUMBER, 12);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::RET);
    code.add(asmcode::LABEL_ALIGNED, "L_label");
    code.add(asmcode::CALL, "L_label_2");
    code.add(asmcode::RET);
    uint64_t size;
    uint8_t* f = (uint8_t*)vm_bytecode(size, code);
    registers reg;

    try
      {
      run_bytecode(f, size, reg);
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    TEST_EQ(22, reg.rax);
    TEST_EQ(12, reg.rcx);
    free_bytecode(f, size);
    }

  void test_vm_jmp_aligned()
    {
    asmcode code;
    code.add(asmcode::JMP, "L_label");
    code.add(asmcode::LABEL_ALIGNED, "L_label");
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 10);
    code.add(asmcode::MOV, asmcode::RCX, asmcode::NUMBER, 12);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::RET);
    uint64_t size;
    uint8_t* f = (uint8_t*)vm_bytecode(size, code);
    registers reg;

    try
      {
      run_bytecode(f, size, reg);
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    TEST_EQ(22, reg.rax);
    TEST_EQ(12, reg.rcx);
    free_bytecode(f, size);
    }

  void test_vm_jmp_register()
    {
    asmcode code;
    code.add(asmcode::MOV, asmcode::RDX, asmcode::LABELADDRESS, "L_label");
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 10);
    code.add(asmcode::MOV, asmcode::RCX, asmcode::NUMBER, 12);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::JMP, asmcode::RDX);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 0);
    code.add(asmcode::MOV, asmcode::RCX, asmcode::NUMBER, 0);
    code.add(asmcode::LABEL, "L_label");
    code.add(asmcode::RET);
    uint64_t size;
    uint8_t* f = (uint8_t*)vm_bytecode(size, code);
    registers reg;

    try
      {
      run_bytecode(f, size, reg);
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    TEST_EQ(22, reg.rax);
    TEST_EQ(12, reg.rcx);
    free_bytecode(f, size);
    }

  void test_vm_shift()
    {
    asmcode code;
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 10);
    code.add(asmcode::MOV, asmcode::RBX, asmcode::NUMBER, 10);
    code.add(asmcode::MOV, asmcode::RCX, asmcode::NUMBER, 10);
    code.add(asmcode::MOV, asmcode::RDX, asmcode::NUMBER, 10);
    code.add(asmcode::MOV, asmcode::R8, asmcode::NUMBER, -10);
    code.add(asmcode::MOV, asmcode::R9, asmcode::NUMBER, -10);
    code.add(asmcode::MOV, asmcode::R10, asmcode::NUMBER, -10);
    code.add(asmcode::MOV, asmcode::R11, asmcode::NUMBER, -10);

    code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 1);
    code.add(asmcode::SAL, asmcode::RBX, asmcode::NUMBER, 1);
    code.add(asmcode::SHR, asmcode::RCX, asmcode::NUMBER, 1);
    code.add(asmcode::SAR, asmcode::RDX, asmcode::NUMBER, 1);

    code.add(asmcode::SHL, asmcode::R8, asmcode::NUMBER, 1);
    code.add(asmcode::SAL, asmcode::R9, asmcode::NUMBER, 1);
    code.add(asmcode::SHR, asmcode::R10, asmcode::NUMBER, 1);
    code.add(asmcode::SAR, asmcode::R11, asmcode::NUMBER, 1);

    code.add(asmcode::RET);
    uint64_t size;
    uint8_t* f = (uint8_t*)vm_bytecode(size, code);
    registers reg;

    try
      {
      run_bytecode(f, size, reg);
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    TEST_EQ(20, reg.rax);
    TEST_EQ(20, reg.rbx);
    TEST_EQ(5, reg.rcx);
    TEST_EQ(5, reg.rdx);

    TEST_EQ(-20, reg.r8);
    TEST_EQ(-20, reg.r9);
    TEST_EQ(9223372036854775803, reg.r10);
    TEST_EQ(-5, reg.r11);
    free_bytecode(f, size);
    }

  void test_vm_jls()
    {
    asmcode code;
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 10);
    code.add(asmcode::CMP, asmcode::RAX, asmcode::NUMBER, 5);
    code.add(asmcode::JLS, "L_label");
    code.add(asmcode::MOV, asmcode::RCX, asmcode::NUMBER, 3);
    code.add(asmcode::RET);
    code.add(asmcode::LABEL, "L_label");
    code.add(asmcode::MOV, asmcode::RCX, asmcode::NUMBER, 4);
    code.add(asmcode::RET);

    uint64_t size;
    uint8_t* f = (uint8_t*)vm_bytecode(size, code);
    registers reg;

    try
      {
      run_bytecode(f, size, reg);
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    TEST_EQ(3, reg.rcx);
    free_bytecode(f, size);
    }

  void test_vm_jls_2()
    {
    asmcode code;
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 10);
    code.add(asmcode::CMP, asmcode::RAX, asmcode::NUMBER, 20);
    code.add(asmcode::JLS, "L_label");
    code.add(asmcode::MOV, asmcode::RCX, asmcode::NUMBER, 3);
    code.add(asmcode::RET);
    code.add(asmcode::LABEL, "L_label");
    code.add(asmcode::MOV, asmcode::RCX, asmcode::NUMBER, 4);
    code.add(asmcode::RET);

    uint64_t size;
    uint8_t* f = (uint8_t*)vm_bytecode(size, code);
    registers reg;

    try
      {
      run_bytecode(f, size, reg);
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    TEST_EQ(4, reg.rcx);
    free_bytecode(f, size);
    }

  void test_vm_jles()
    {
    asmcode code;
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 10);
    code.add(asmcode::CMP, asmcode::RAX, asmcode::NUMBER, 10);
    code.add(asmcode::JLES, "L_label");
    code.add(asmcode::MOV, asmcode::RCX, asmcode::NUMBER, 3);
    code.add(asmcode::RET);
    code.add(asmcode::LABEL, "L_label");
    code.add(asmcode::MOV, asmcode::RCX, asmcode::NUMBER, 4);
    code.add(asmcode::RET);

    uint64_t size;
    uint8_t* f = (uint8_t*)vm_bytecode(size, code);
    registers reg;

    try
      {
      run_bytecode(f, size, reg);
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    TEST_EQ(4, reg.rcx);
    free_bytecode(f, size);
    }

  void test_vm_jges()
    {
    asmcode code;
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 10);
    code.add(asmcode::CMP, asmcode::RAX, asmcode::NUMBER, 10);
    code.add(asmcode::JGES, "L_label");
    code.add(asmcode::MOV, asmcode::RCX, asmcode::NUMBER, 3);
    code.add(asmcode::RET);
    code.add(asmcode::LABEL, "L_label");
    code.add(asmcode::MOV, asmcode::RCX, asmcode::NUMBER, 4);
    code.add(asmcode::RET);

    uint64_t size;
    uint8_t* f = (uint8_t*)vm_bytecode(size, code);
    registers reg;

    try
      {
      run_bytecode(f, size, reg);
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    TEST_EQ(4, reg.rcx);
    free_bytecode(f, size);
    }

  } // namespace

ASM_END

void run_all_vm_tests()
  {
  using namespace ASM;
  test_vm_mov_bytecode();
  test_vm_mov_bytecode_2();
  test_vm_nop_bytecode();
  test_vm_ret();
  test_vm_mov();
  test_vm_mov_2();
  test_vm_add();
  test_vm_call();
  test_vm_call_2();
  test_vm_jmp();
  test_vm_call_aligned();
  test_vm_call_aligned_2();
  test_vm_jmp_aligned();
  test_vm_jmp_register();
  test_vm_shift();
  test_vm_jls();
  test_vm_jls_2();
  test_vm_jles();  
  test_vm_jges();
  }