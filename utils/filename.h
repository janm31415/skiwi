#pragma once
#include "namespace.h"
#include <string>
#include "encoding.h"

JAM_BEGIN
  /*
  Everything is assumed to be in utf8 encoding
  */
  std::string get_extension(const std::string& filename);
  std::string remove_extension(const std::string& filename);
  std::string get_folder(const std::string& path);
  std::string get_filename(const std::string& path);

  inline std::string get_extension(const std::string& filename)
    {
    std::wstring wfilename = convert_string_to_wstring(filename);
    auto ext_ind = wfilename.find_last_of('.');
    std::wstring ext;
    if (ext_ind != std::wstring::npos)
      ext = wfilename.substr(ext_ind + 1);
    return convert_wstring_to_string(ext);
    }

  inline std::string remove_extension(const std::string& filename)
    {
    std::wstring wfilename = convert_string_to_wstring(filename);
    auto ext_ind = wfilename.find_last_of('.');
    if (ext_ind == std::wstring::npos)
      return filename;
    return convert_wstring_to_string(wfilename.substr(0, ext_ind));
    }

  inline std::string get_folder(const std::string& path)
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

  inline std::string get_filename(const std::string& path)
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

  JAM_END
