#include "syscalls.h"

#include <stdint.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <fcntl.h>
#include <fstream>
#include <chrono>
#include "file_utils.h"
#include "c_prim_decl.h"

SKIWI_BEGIN

int _skiwi_close(int const fd)
  {
  if (fd < 0)
    return 0;
#ifdef _WIN32
  return _close(fd);
#else
  return close(fd);
#endif
  }

int _skiwi_read(int const fd, void* const buffer, unsigned const buffer_size)
  {
  if (fd < 0)
    return 0;
#ifdef _WIN32
  return _read(fd, buffer, buffer_size);
#else
  return ::read(fd, buffer, buffer_size);
#endif
  }

int _skiwi_write(int fd, const void* buffer, unsigned int count)
  {
  if (fd < 0)
    return 0;
#ifdef _WIN32
  return _write(fd, buffer, count);
#else
  return write(fd, buffer, count);
#endif
  }

const char* _skiwi_getenv(const char* name)
  {
  static std::string env_value;
#ifdef _WIN32
  std::string s(name);
  std::wstring ws = convert_string_to_wstring(s);
  wchar_t* path = _wgetenv(ws.c_str());
  if (!path)
    return nullptr;
  std::wstring wresult(path);
  env_value = convert_wstring_to_string(wresult);
  return env_value.c_str();
#else
  return ::getenv(name);
#endif
  }

int _skiwi_putenv(const char* name, const char* value)
  {
#ifdef _WIN32
  std::string sname(name);
  std::string svalue(value);
  std::wstring wname = convert_string_to_wstring(sname);
  std::wstring wvalue = convert_string_to_wstring(svalue);
  return (int)_wputenv_s(wname.c_str(), wvalue.c_str());
#else
  return setenv(name, value, 1);
#endif
  }

int _skiwi_file_exists(const char* filename)
  {
#ifdef _WIN32
  int res = 0;
  std::string sname(filename);
  std::wstring wname = convert_string_to_wstring(sname);
  std::ifstream f(wname);
  if (f.is_open())
    {
    res = 1;
    f.close();
    }
  return res;
#else
  int res = 0;
  std::ifstream f(filename);
  if (f.is_open())
    {
    res = 1;
    f.close();
    }
  return res;
#endif
  }

uint64_t _skiwi_current_seconds()
  {
  uint64_t secondsUTC = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  return secondsUTC;
  }

uint64_t _skiwi_current_milliseconds()
  {
  uint64_t millisecondsUTC = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  return millisecondsUTC;
  }

void add_system_calls(std::map<std::string, external_function>& externals)
  {
  external_function ef;
  ef.name = "_write";
#ifdef _WIN32
  ef.address = (uint64_t)&_write;
#else
  ef.address = (uint64_t)&write;
#endif
  ef.return_type = external_function::T_INT64;
  ef.arguments.push_back(external_function::T_INT64);
  ef.arguments.push_back(external_function::T_CHAR_POINTER);
  ef.arguments.push_back(external_function::T_INT64);
  externals[ef.name] = ef;

  ef.arguments.clear();
  ef.name = "_open";
#ifdef _WIN32
  ef.address = (uint64_t)&_open;
#else
  ef.address = (uint64_t)&open;
#endif
  ef.return_type = external_function::T_INT64;
  ef.arguments.push_back(external_function::T_CHAR_POINTER);
  ef.arguments.push_back(external_function::T_INT64);
  ef.arguments.push_back(external_function::T_INT64);
  externals[ef.name] = ef;

  ef.arguments.clear();
  ef.name = "strtoll";
  ef.address = (uint64_t)&strtoll;
  ef.return_type = external_function::T_INT64;
  ef.arguments.push_back(external_function::T_CHAR_POINTER);
  ef.arguments.push_back(external_function::T_INT64);
  ef.arguments.push_back(external_function::T_INT64);
  externals[ef.name] = ef;

  ef.arguments.clear();
  ef.name = "strtod";
  ef.address = (uint64_t)&strtod;
  ef.return_type = external_function::T_DOUBLE;
  ef.arguments.push_back(external_function::T_CHAR_POINTER);
  ef.arguments.push_back(external_function::T_INT64);
  externals[ef.name] = ef;
  
  ef.arguments.clear();
  ef.name = "sprintf";
  ef.address = (uint64_t)&sprintf;
  ef.return_type = external_function::T_INT64;
  ef.arguments.push_back(external_function::T_CHAR_POINTER);
  ef.arguments.push_back(external_function::T_CHAR_POINTER);
  ef.arguments.push_back(external_function::T_CHAR_POINTER);
  externals[ef.name] = ef;

  ef.arguments.clear();
  ef.name = "_skiwi_close";
  ef.address = (uint64_t)&_skiwi_close;
  ef.return_type = external_function::T_INT64;
  ef.arguments.push_back(external_function::T_INT64);
  externals[ef.name] = ef;

  ef.arguments.clear();
  ef.name = "_skiwi_read";
  ef.address = (uint64_t)&_skiwi_read;
  ef.return_type = external_function::T_INT64;
  ef.arguments.push_back(external_function::T_INT64);
  ef.arguments.push_back(external_function::T_CHAR_POINTER);
  ef.arguments.push_back(external_function::T_INT64);
  externals[ef.name] = ef;

  ef.arguments.clear();
  ef.name = "_skiwi_write";
  ef.address = (uint64_t)&_skiwi_write;
  ef.return_type = external_function::T_INT64;
  ef.arguments.push_back(external_function::T_INT64);
  ef.arguments.push_back(external_function::T_CHAR_POINTER);
  ef.arguments.push_back(external_function::T_INT64);
  externals[ef.name] = ef;

  ef.arguments.clear();
  ef.name = "_skiwi_getenv";
  ef.address = (uint64_t)&_skiwi_getenv;
  ef.return_type = external_function::T_CHAR_POINTER;
  ef.arguments.push_back(external_function::T_CHAR_POINTER);
  externals[ef.name] = ef;

  ef.arguments.clear();
  ef.name = "_skiwi_putenv";
  ef.address = (uint64_t)&_skiwi_putenv;
  ef.return_type = external_function::T_INT64;
  ef.arguments.push_back(external_function::T_CHAR_POINTER);
  ef.arguments.push_back(external_function::T_CHAR_POINTER);
  externals[ef.name] = ef;

  ef.arguments.clear();
  ef.name = "_skiwi_file_exists";
  ef.address = (uint64_t)&_skiwi_file_exists;
  ef.return_type = external_function::T_INT64;
  ef.arguments.push_back(external_function::T_CHAR_POINTER);
  externals[ef.name] = ef;

  ef.arguments.clear();
  ef.name = "_skiwi_current_seconds";
  ef.address = (uint64_t)&_skiwi_current_seconds;
  ef.return_type = external_function::T_INT64;
  externals[ef.name] = ef;

  ef.arguments.clear();
  ef.name = "_skiwi_current_milliseconds";
  ef.address = (uint64_t)&_skiwi_current_milliseconds;
  ef.return_type = external_function::T_INT64;
  externals[ef.name] = ef;

  ef.arguments.clear();
  ef.name = "c_prim_load";
  ef.address = (uint64_t)&c_prim_load;
  ef.return_type = external_function::T_INT64;
  ef.arguments.push_back(external_function::T_CHAR_POINTER);
  externals[ef.name] = ef;

  ef.arguments.clear();
  ef.name = "c_prim_eval";
  ef.address = (uint64_t)&c_prim_eval;
  ef.return_type = external_function::T_INT64;
  ef.arguments.push_back(external_function::T_CHAR_POINTER);
  externals[ef.name] = ef;
  }

SKIWI_END
