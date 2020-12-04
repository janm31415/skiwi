#include "file_utils.h"

#include "utf8.h"
#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#if defined(_WIN32)
#include <algorithm>
#include <sys/types.h>
#include <windows.h>
#elif defined(unix)
#include <unistd.h>
#include <linux/limits.h>
#elif defined(__APPLE__)
#include <unistd.h>
#include <mach-o/dyld.h>
#endif

SKIWI_BEGIN

std::string get_executable_path()
  {
#ifdef _WIN32
  typedef std::vector<wchar_t> char_vector;
  typedef std::vector<wchar_t>::size_type size_type;
  char_vector buf(1024, 0);
  size_type size = buf.size();
  bool havePath = false;
  bool shouldContinue = true;
  do
    {
    DWORD result = GetModuleFileNameW(nullptr, &buf[0], (DWORD)size);
    DWORD lastError = GetLastError();
    if (result == 0)
      {
      shouldContinue = false;
      }
    else if (result < size)
      {
      havePath = true;
      shouldContinue = false;
      }
    else if (
      result == size
      && (lastError == ERROR_INSUFFICIENT_BUFFER || lastError == ERROR_SUCCESS)
      )
      {
      size *= 2;
      buf.resize(size);
      }
    else
      {
      shouldContinue = false;
      }
    } while (shouldContinue);
    if (!havePath)
      {
      return std::string("");
      }
    std::wstring wret = &buf[0];
    std::replace(wret.begin(), wret.end(), '\\', '/'); // replace all '\\' by '/'
    return convert_wstring_to_string(wret);
#elif defined(unix)
  char result[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
  return std::string(result, (count > 0) ? count : 0);
#elif defined(__APPLE__)
  char path[1024];
  uint32_t size = sizeof(path);
  if (_NSGetExecutablePath(path, &size) == 0)
    return std::string(path);
  else
    return std::string();
#else
  return std::string();
#endif
  }

std::string get_cwd()
  {
#ifdef _WIN32
  wchar_t buf[MAX_PATH];
  GetCurrentDirectoryW(MAX_PATH, buf);
  std::wstring wbuf(buf);
  std::replace(wbuf.begin(), wbuf.end(), '\\', '/'); // replace all '\\' by '/'
  return convert_wstring_to_string(wbuf);
#else
  char buf[PATH_MAX];
  getcwd(buf, sizeof(buf));
  return std::string(buf);
#endif
  }

std::wstring convert_string_to_wstring(const std::string& str)
  {
  std::wstring out;
  out.reserve(str.size());
  auto it = str.begin();
  auto it_end = str.end();

  for (; it != it_end;)
    {
    uint32_t cp = 0;
    utf8::internal::utf_error err_code = utf8::internal::validate_next(it, it_end, cp);
    if (err_code == utf8::internal::UTF8_OK)
      {
      out.push_back((wchar_t)cp);
      }
    else
      {
      out.push_back(*it);
      ++it;
      }
    }
  return out;
  }

std::string convert_wstring_to_string(const std::wstring& str)
  {
  std::string out;
  out.reserve(str.size());
  utf8::utf16to8(str.begin(), str.end(), std::back_inserter(out));
  return out;
  }

std::string get_folder(const std::string& path)
  {
  std::wstring wpath = convert_string_to_wstring(path);
  auto pos1 = wpath.find_last_of('/');
  auto pos2 = wpath.find_last_of('\\');
  if (pos1 == std::wstring::npos && pos2 == std::wstring::npos)
    return "";
  if (pos1 == std::wstring::npos)
    return convert_wstring_to_string(wpath.substr(0, pos2 + 1));
  if (pos2 == std::wstring::npos)
    return convert_wstring_to_string(wpath.substr(0, pos1 + 1));
  return convert_wstring_to_string(wpath.substr(0, (pos1 > pos2 ? pos1 : pos2) + 1));
  }

std::string get_filename(const std::string& path)
  {
  std::wstring wpath = convert_string_to_wstring(path);
  auto pos1 = wpath.find_last_of('/');
  auto pos2 = wpath.find_last_of('\\');
  if (pos1 == std::wstring::npos && pos2 == std::wstring::npos)
    return path;
  if (pos1 == std::wstring::npos)
    return convert_wstring_to_string(wpath.substr(pos2 + 1));
  if (pos2 == std::wstring::npos)
    return convert_wstring_to_string(wpath.substr(pos1 + 1));
  return convert_wstring_to_string(wpath.substr((pos1 > pos2 ? pos1 : pos2) + 1));
  }

std::string getenv(const std::string& name)
  {
#ifdef _WIN32
  std::wstring ws = convert_string_to_wstring(name);
  wchar_t* path = _wgetenv(ws.c_str());
  if (!path)
    return nullptr;
  std::wstring wresult(path);
  std::string out = convert_wstring_to_string(wresult);
#else
  std::string out(::getenv(name.c_str()));
#endif
  return out;
  }

void putenv(const std::string& name, const std::string& value)
  {
#ifdef _WIN32
  std::wstring wvalue = convert_string_to_wstring(value);
  std::wstring wname = convert_string_to_wstring(name);
  _wputenv_s(wname.c_str(), wvalue.c_str());
#else
  ::setenv(name.c_str(), value.c_str(), 1);
#endif
  }
SKIWI_END
