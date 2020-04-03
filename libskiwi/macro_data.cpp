#include "macro_data.h"

SKIWI_BEGIN

macro_data create_macro_data()
  {
  macro_data md;
  return md;
  }

void destroy_macro_data(macro_data& md)
  {
  for (auto& f : md.compiled_macros)
    free_assembled_function(f.first, f.second);
  md.compiled_macros.clear();
  }

SKIWI_END
