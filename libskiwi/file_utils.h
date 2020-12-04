#pragma once

#include "namespace.h"
#include <string>

SKIWI_BEGIN

std::wstring convert_string_to_wstring(const std::string& str);
std::string convert_wstring_to_string(const std::wstring& str);

std::string get_executable_path();
std::string get_cwd();
/*
Everything is assumed to be in utf8 encoding
*/
std::string get_folder(const std::string& path);
std::string get_filename(const std::string& path);
std::string getenv(const std::string& name);
void putenv(const std::string& name, const std::string& value);

SKIWI_END
