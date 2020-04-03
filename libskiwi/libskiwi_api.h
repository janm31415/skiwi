#pragma once

#ifdef _WIN32
#if defined(libskiwi_EXPORTS)
#  define SKIWI_SCHEME_API __declspec(dllexport)
#else
#  define SKIWI_SCHEME_API __declspec(dllimport)
#endif
#else
#define SKIWI_SCHEME_API
#endif