#include "macro_data.h"
#include "asm/vm.h"
SKIWI_BEGIN

macro_data create_macro_data()
  {
  macro_data md;
  return md;
  }

void destroy_macro_data(macro_data& md)
  {
#ifdef _SKIWI_FOR_ARM
    for (auto& f : md.compiled_macros)
      ASM::free_bytecode((void*)f.first, f.second);
    md.compiled_macros.clear();
#else
  for (auto& f : md.compiled_macros)
    ASM::free_assembled_function(f.first, f.second);
  md.compiled_macros.clear();
#endif
  }

SKIWI_END
