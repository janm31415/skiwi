#pragma once

#ifdef WIN32
#if defined(asm_EXPORTS)
#define SKIWI_ASSEMBLER_API __declspec(dllexport)
#else
#define SKIWI_ASSEMBLER_API __declspec(dllimport)
#endif
#else
#define SKIWI_ASSEMBLER_API
#endif