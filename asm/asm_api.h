#pragma once

#if defined(asm_EXPORTS)
#ifdef WIN32
  #define SKIWI_ASSEMBLER_API __declspec(dllexport)
#else
  #define SKIWI_ASSEMBLER_API 
#endif
#else
#ifdef WIN32
  #define SKIWI_ASSEMBLER_API __declspec(dllimport)
#else
  #define SKIWI_ASSEMBLER_API
#endif
#endif