#pragma once
#include "namespace.h"
#include <fstream>
#include <string>
#include "utf8.h"

JAM_BEGIN

  std::wstring convert_string_to_wstring(const std::string& str);
  std::string convert_wstring_to_string(const std::wstring& str);
  bool valid_utf8_file(const std::wstring& filename);

  inline std::wstring convert_string_to_wstring(const std::string& str)
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

  inline std::string convert_wstring_to_string(const std::wstring& str)
    {
    std::string out;
    out.reserve(str.size());
    utf8::utf16to8(str.begin(), str.end(), std::back_inserter(out));
    return out;
    }

  inline bool valid_utf8_file(const std::wstring& filename)
    {
  #ifdef _WIN32
    std::ifstream ifs(filename);
  #else
    std::ifstream ifs(convert_wstring_to_string(filename));
  #endif
    if (!ifs)
      return false;

    std::istreambuf_iterator<char> it(ifs.rdbuf());
    std::istreambuf_iterator<char> eos;

    return utf8::is_valid(it, eos);
    }

  JAM_END