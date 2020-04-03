#pragma once

//https://stackoverflow.com/questions/143174/how-do-i-get-the-directory-that-a-program-is-running-from

#include <string>
#include <fstream>
#include <sstream>

#ifdef _WIN32


#include <windows.h>

inline std::wstring get_folder(const std::wstring& wpath)
  {  
  auto pos1 = wpath.find_last_of('/');
  auto pos2 = wpath.find_last_of('\\');
  if (pos1 == std::wstring::npos && pos2 == std::wstring::npos)
    return L"./";
  if (pos1 == std::wstring::npos)
    return wpath.substr(0, pos2 + 1);
  if (pos2 == std::wstring::npos)
    return wpath.substr(0, pos1 + 1);
  return wpath.substr(0, (pos1 > pos2 ? pos1 : pos2) + 1);
  }

inline std::wstring get_exe_path()
  {
  wchar_t result[MAX_PATH];
  return std::wstring(result, GetModuleFileNameW(NULL, result, MAX_PATH));
  }

inline std::string read_file_in_exe_path(const std::string& relative_path)
  {
  std::wstring wrelative(relative_path.begin(), relative_path.end());
  std::wstring filepath = get_folder(get_exe_path()) + wrelative;
  std::ifstream f{ filepath };
  if (f.is_open())
    {
    std::stringstream ss;
    ss << f.rdbuf();    
    f.close();
    return ss.str();
    }
  return "";
  }

#else

#include <limits.h>
#include <unistd.h>

inline std::string get_folder(const std::string& path)
  {
  auto pos1 = path.find_last_of('/');
  auto pos2 = path.find_last_of('\\');
  if (pos1 == std::wstring::npos && pos2 == std::wstring::npos)
    return L".";
  if (pos1 == std::wstring::npos)
    return path.substr(0, pos2 + 1);
  if (pos2 == std::wstring::npos)
    return path.substr(0, pos1 + 1);
  return path.substr(0, (pos1 > pos2 ? pos1 : pos2) + 1);

  }
inline std::string get_exe_path()
  {
  char result[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
  return std::string(result, (count > 0) ? count : 0);
  }

inline std::string read_file_in_exe_path(const std::string& relative_path)
  {
  std::string filepath = get_folder(get_exe_path()) + relative_path;
  std::ifstream f{ filepath };
  if (f.is_open())
    {
    std::stringstream ss;
    ss << f.rdbuf();
    f.close();
    return ss.str();
    }
  return "";
  }

#endif