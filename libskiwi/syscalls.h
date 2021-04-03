#pragma once

#include "namespace.h"
#include "compiler.h"
#include "libskiwi_api.h"

SKIWI_BEGIN

int _skiwi_close(int const fd);
int _skiwi_read(int const fd, void* const buffer, unsigned const buffer_size);
int _skiwi_write(int fd, const void* buffer, unsigned int count);
const char* _skiwi_getenv(const char* name);
int _skiwi_putenv(const char* name, const char* value);
int _skiwi_file_exists(const char* filename);
uint64_t _skiwi_current_seconds();
uint64_t _skiwi_current_milliseconds();

SKIWI_SCHEME_API void add_system_calls(std::map<std::string, external_function>& externals);

SKIWI_END
