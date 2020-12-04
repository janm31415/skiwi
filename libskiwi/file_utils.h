#pragma once

#include "utf8.h"
#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#ifdef _WIN32
#include <algorithm>
#include <sys/types.h>
#endif

namespace jtk
  {
  std::wstring convert_string_to_wstring(const std::string& str);
  std::string convert_wstring_to_string(const std::wstring& str);
  bool valid_utf8_file(const std::wstring& filename);
  bool valid_utf8_file(const std::string& filename);
  bool is_directory(const std::string& directory);
  bool file_exists(const std::string& filename);
  std::vector<std::string> get_files_from_directory(const std::string& d, bool include_subfolders);
  std::vector<std::string> get_subdirectories_from_directory(const std::string& d, bool include_subfolders);
  std::vector<std::string> get_list_from_directory(const std::string& d, bool include_subfolders);

  std::string get_executable_path();
  std::string get_cwd();
  /*
  Everything is assumed to be in utf8 encoding
  */
  std::string get_extension(const std::string& filename);
  std::string remove_extension(const std::string& filename);
  std::string get_folder(const std::string& path);
  std::string get_filename(const std::string& path);
  std::string getenv(const std::string& name);
  void putenv(const std::string& name, const std::string& value);  

  long long file_size(const std::string& filename);

  } // namespace jtk


#ifdef _WIN32

/*
 * Dirent interface for Microsoft Visual Studio
 * Version 1.21
 *
 * Copyright (C) 2006-2012 Toni Ronkko
 * This file is part of dirent.  Dirent may be freely distributed
 * under the MIT license.  For all details and documentation, see
 * https://github.com/tronkko/dirent
 */

 /*
   * Define architecture flags so we don't need to include windows.h.
   * Avoiding windows.h makes it simpler to use windows sockets in conjunction
   * with dirent.h.
   */
#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && defined(_M_IX86)
#   define _X86_
#endif
#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && defined(_M_AMD64)
#define _AMD64_
#endif

#include <stdio.h>
#include <stdarg.h>
#include <windef.h>
#include <winbase.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

   /* Indicates that d_type field is available in dirent structure */
#define _DIRENT_HAVE_D_TYPE

/* Indicates that d_namlen field is available in dirent structure */
#define _DIRENT_HAVE_D_NAMLEN

/* Entries missing from MSVC 6.0 */
#if !defined(FILE_ATTRIBUTE_DEVICE)
#   define FILE_ATTRIBUTE_DEVICE 0x40
#endif

/* File type and permission flags for stat(), general mask */
#if !defined(S_IFMT)
#   define S_IFMT _S_IFMT
#endif

/* Directory bit */
#if !defined(S_IFDIR)
#   define S_IFDIR _S_IFDIR
#endif

/* Character device bit */
#if !defined(S_IFCHR)
#   define S_IFCHR _S_IFCHR
#endif

/* Pipe bit */
#if !defined(S_IFFIFO)
#   define S_IFFIFO _S_IFFIFO
#endif

/* Regular file bit */
#if !defined(S_IFREG)
#   define S_IFREG _S_IFREG
#endif

/* Read permission */
#if !defined(S_IREAD)
#   define S_IREAD _S_IREAD
#endif

/* Write permission */
#if !defined(S_IWRITE)
#   define S_IWRITE _S_IWRITE
#endif

/* Execute permission */
#if !defined(S_IEXEC)
#   define S_IEXEC _S_IEXEC
#endif

/* Pipe */
#if !defined(S_IFIFO)
#   define S_IFIFO _S_IFIFO
#endif

/* Block device */
#if !defined(S_IFBLK)
#   define S_IFBLK 0
#endif

/* Link */
#if !defined(S_IFLNK)
#   define S_IFLNK 0
#endif

/* Socket */
#if !defined(S_IFSOCK)
#   define S_IFSOCK 0
#endif

/* Read user permission */
#if !defined(S_IRUSR)
#   define S_IRUSR S_IREAD
#endif

/* Write user permission */
#if !defined(S_IWUSR)
#   define S_IWUSR S_IWRITE
#endif

/* Execute user permission */
#if !defined(S_IXUSR)
#   define S_IXUSR 0
#endif

/* Read group permission */
#if !defined(S_IRGRP)
#   define S_IRGRP 0
#endif

/* Write group permission */
#if !defined(S_IWGRP)
#   define S_IWGRP 0
#endif

/* Execute group permission */
#if !defined(S_IXGRP)
#   define S_IXGRP 0
#endif

/* Read others permission */
#if !defined(S_IROTH)
#   define S_IROTH 0
#endif

/* Write others permission */
#if !defined(S_IWOTH)
#   define S_IWOTH 0
#endif

/* Execute others permission */
#if !defined(S_IXOTH)
#   define S_IXOTH 0
#endif

/* Maximum length of file name */
#if !defined(PATH_MAX)
#   define PATH_MAX MAX_PATH
#endif
#if !defined(FILENAME_MAX)
#   define FILENAME_MAX MAX_PATH
#endif
#if !defined(NAME_MAX)
#   define NAME_MAX FILENAME_MAX
#endif

/* File type flags for d_type */
#define DT_UNKNOWN 0
#define DT_REG S_IFREG
#define DT_DIR S_IFDIR
#define DT_FIFO S_IFIFO
#define DT_SOCK S_IFSOCK
#define DT_CHR S_IFCHR
#define DT_BLK S_IFBLK
#define DT_LNK S_IFLNK

/* Macros for converting between st_mode and d_type */
#define IFTODT(mode) ((mode) & S_IFMT)
#define DTTOIF(type) (type)

/*
 * File type macros.  Note that block devices, sockets and links cannot be
 * distinguished on Windows and the macros S_ISBLK, S_ISSOCK and S_ISLNK are
 * only defined for compatibility.  These macros should always return false
 * on Windows.
 */
#if !defined(S_ISFIFO)
#   define S_ISFIFO(mode) (((mode) & S_IFMT) == S_IFIFO)
#endif
#if !defined(S_ISDIR)
#   define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif
#if !defined(S_ISREG)
#   define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
#endif
#if !defined(S_ISLNK)
#   define S_ISLNK(mode) (((mode) & S_IFMT) == S_IFLNK)
#endif
#if !defined(S_ISSOCK)
#   define S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)
#endif
#if !defined(S_ISCHR)
#   define S_ISCHR(mode) (((mode) & S_IFMT) == S_IFCHR)
#endif
#if !defined(S_ISBLK)
#   define S_ISBLK(mode) (((mode) & S_IFMT) == S_IFBLK)
#endif

 /* Return the exact length of d_namlen without zero terminator */
#define _D_EXACT_NAMLEN(p) ((p)->d_namlen)

/* Return number of bytes needed to store d_namlen */
#define _D_ALLOC_NAMLEN(p) (PATH_MAX)


#ifdef __cplusplus
extern "C" {
#endif


  /* Wide-character version */
  struct _wdirent {
    /* Always zero */
    long d_ino;

    /* Structure size */
    unsigned short d_reclen;

    /* Length of name without \0 */
    size_t d_namlen;

    /* File type */
    int d_type;

    /* File name */
    wchar_t d_name[PATH_MAX];
    };
  typedef struct _wdirent _wdirent;

  struct _WDIR {
    /* Current directory entry */
    struct _wdirent ent;

    /* Private file data */
    WIN32_FIND_DATAW data;

    /* True if data is valid */
    int cached;

    /* Win32 search handle */
    HANDLE handle;

    /* Initial directory name */
    wchar_t *patt;
    };
  typedef struct _WDIR _WDIR;

  static _WDIR *_wopendir(const wchar_t *dirname);
  static struct _wdirent *_wreaddir(_WDIR *dirp);
  static int _wclosedir(_WDIR *dirp);
  static void _wrewinddir(_WDIR* dirp);


  /* For compatibility with Symbian */
#define wdirent _wdirent
#define WDIR _WDIR
#define wopendir _wopendir
#define wreaddir _wreaddir
#define wclosedir _wclosedir
#define wrewinddir _wrewinddir


/* Multi-byte character versions */
  struct dirent {
    /* Always zero */
    long d_ino;

    /* Structure size */
    unsigned short d_reclen;

    /* Length of name without \0 */
    size_t d_namlen;

    /* File type */
    int d_type;

    /* File name */
    char d_name[PATH_MAX];
    };
  typedef struct dirent dirent;

  struct DIR {
    struct dirent ent;
    struct _WDIR *wdirp;
    };
  typedef struct DIR DIR;

  /* Internal utility functions */
  static WIN32_FIND_DATAW *dirent_first(_WDIR *dirp);
  static WIN32_FIND_DATAW *dirent_next(_WDIR *dirp);

  
  static void dirent_set_errno(int error);

  /*
   * Open directory stream DIRNAME for read and return a pointer to the
   * internal working area that is used to retrieve individual directory
   * entries.
   */
  static _WDIR*
    _wopendir(
      const wchar_t *dirname)
    {
    _WDIR *dirp = NULL;
    int error;

    /* Must have directory name */
    if (dirname == NULL || dirname[0] == '\0') {
      dirent_set_errno(ENOENT);
      return NULL;
      }

    /* Allocate new _WDIR structure */
    dirp = (_WDIR*)malloc(sizeof(struct _WDIR));
    if (dirp != NULL) {
      DWORD n;

      /* Reset _WDIR structure */
      dirp->handle = INVALID_HANDLE_VALUE;
      dirp->patt = NULL;
      dirp->cached = 0;

      /* Compute the length of full path plus zero terminator */
      n = GetFullPathNameW(dirname, 0, NULL, NULL);

      /* Allocate room for absolute directory name and search pattern */
      dirp->patt = (wchar_t*)malloc(sizeof(wchar_t) * n + 16);
      if (dirp->patt) {

        /*
         * Convert relative directory name to an absolute one.  This
         * allows rewinddir() to function correctly even when current
         * working directory is changed between opendir() and rewinddir().
         */
        n = GetFullPathNameW(dirname, n, dirp->patt, NULL);
        if (n > 0) {
          wchar_t *p;

          /* Append search pattern \* to the directory name */
          p = dirp->patt + n;
          if (dirp->patt < p) {
            switch (p[-1]) {
              case '\\':
              case '/':
              case ':':
                /* Directory ends in path separator, e.g. c:\temp\ */
                /*NOP*/;
                break;

              default:
                /* Directory name doesn't end in path separator */
                *p++ = '\\';
              }
            }
          *p++ = '*';
          *p = '\0';

          /* Open directory stream and retrieve the first entry */
          if (dirent_first(dirp)) {
            /* Directory stream opened successfully */
            error = 0;
            }
          else {
            /* Cannot retrieve first entry */
            error = 1;
            dirent_set_errno(ENOENT);
            }

          }
        else {
          /* Cannot retrieve full path name */
          dirent_set_errno(ENOENT);
          error = 1;
          }

        }
      else {
        /* Cannot allocate memory for search pattern */
        error = 1;
        }

      }
    else {
      /* Cannot allocate _WDIR structure */
      error = 1;
      }

    /* Clean up in case of error */
    if (error  &&  dirp) {
      _wclosedir(dirp);
      dirp = NULL;
      }

    return dirp;
    }

  /*
   * Read next directory entry.  The directory entry is returned in dirent
   * structure in the d_name field.  Individual directory entries returned by
   * this function include regular files, sub-directories, pseudo-directories
   * "." and ".." as well as volume labels, hidden files and system files.
   */
  static struct _wdirent*
    _wreaddir(
      _WDIR *dirp)
    {
    WIN32_FIND_DATAW *datap;
    struct _wdirent *entp;

    /* Read next directory entry */
    datap = dirent_next(dirp);
    if (datap) {
      size_t n;
      DWORD attr;

      /* Pointer to directory entry to return */
      entp = &dirp->ent;

      /*
       * Copy file name as wide-character string.  If the file name is too
       * long to fit in to the destination buffer, then truncate file name
       * to PATH_MAX characters and zero-terminate the buffer.
       */
      n = 0;
      while (n + 1 < PATH_MAX  &&  datap->cFileName[n] != 0) {
        entp->d_name[n] = datap->cFileName[n];
        n++;
        }
      dirp->ent.d_name[n] = 0;

      /* Length of file name excluding zero terminator */
      entp->d_namlen = n;

      /* File type */
      attr = datap->dwFileAttributes;
      if ((attr & FILE_ATTRIBUTE_DEVICE) != 0) {
        entp->d_type = DT_CHR;
        }
      else if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        entp->d_type = DT_DIR;
        }
      else {
        entp->d_type = DT_REG;
        }

      /* Reset dummy fields */
      entp->d_ino = 0;
      entp->d_reclen = sizeof(struct _wdirent);

      }
    else {

      /* Last directory entry read */
      entp = NULL;

      }

    return entp;
    }

  /*
   * Close directory stream opened by opendir() function.  This invalidates the
   * DIR structure as well as any directory entry read previously by
   * _wreaddir().
   */
  static int
    _wclosedir(
      _WDIR *dirp)
    {
    int ok;
    if (dirp) {

      /* Release search handle */
      if (dirp->handle != INVALID_HANDLE_VALUE) {
        FindClose(dirp->handle);
        dirp->handle = INVALID_HANDLE_VALUE;
        }

      /* Release search pattern */
      if (dirp->patt) {
        free(dirp->patt);
        dirp->patt = NULL;
        }

      /* Release directory structure */
      free(dirp);
      ok = /*success*/0;

      }
    else {
      /* Invalid directory stream */
      dirent_set_errno(EBADF);
      ok = /*failure*/-1;
      }
    return ok;
    }

  /*
   * Rewind directory stream such that _wreaddir() returns the very first
   * file name again.
   */

   /* Get first directory entry (internal) */
  static WIN32_FIND_DATAW*
    dirent_first(
      _WDIR *dirp)
    {
    WIN32_FIND_DATAW *datap;

    /* Open directory and retrieve the first entry */
    dirp->handle = FindFirstFileW(dirp->patt, &dirp->data);
    if (dirp->handle != INVALID_HANDLE_VALUE) {

      /* a directory entry is now waiting in memory */
      datap = &dirp->data;
      dirp->cached = 1;

      }
    else {

      /* Failed to re-open directory: no directory entry in memory */
      dirp->cached = 0;
      datap = NULL;

      }
    return datap;
    }

  /* Get next directory entry (internal) */
  static WIN32_FIND_DATAW*
    dirent_next(
      _WDIR *dirp)
    {
    WIN32_FIND_DATAW *p;

    /* Get next directory entry */
    if (dirp->cached != 0) {

      /* A valid directory entry already in memory */
      p = &dirp->data;
      dirp->cached = 0;

      }
    else if (dirp->handle != INVALID_HANDLE_VALUE) {

      /* Get the next directory entry from stream */
      if (FindNextFileW(dirp->handle, &dirp->data) != FALSE) {
        /* Got a file */
        p = &dirp->data;
        }
      else {
        /* The very last entry has been processed or an error occured */
        FindClose(dirp->handle);
        dirp->handle = INVALID_HANDLE_VALUE;
        p = NULL;
        }

      }
    else {

      /* End of directory stream reached */
      p = NULL;

      }

    return p;
    }
   

      /* Set errno variable */
  static void
    dirent_set_errno(
      int error)
    {
#if defined(_MSC_VER)  &&  _MSC_VER >= 1400

    /* Microsoft Visual Studio 2005 and later */
    _set_errno(error);

#else

    /* Non-Microsoft compiler or older Microsoft compiler */
    errno = error;

#endif
    }


#ifdef __cplusplus
  }
#endif

#elif defined(unix) //#ifdef _WIN32
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <linux/limits.h>
#elif defined(__APPLE__)
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <mach-o/dyld.h>
#endif


namespace jtk
  {
  inline std::string get_executable_path()
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

  inline std::string get_cwd()
    {
#ifdef _WIN32
    wchar_t buf[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, buf);
    std::wstring wbuf(buf);
    std::replace(wbuf.begin(), wbuf.end(), '\\', '/'); // replace all '\\' by '/'
    return jtk::convert_wstring_to_string(wbuf);
#else
    char buf[PATH_MAX];
    getcwd(buf, sizeof(buf));
    return std::string(buf);
#endif
    }

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
    std::string fn = convert_wstring_to_string(filename);
    std::ifstream ifs(fn);
#endif
    if (!ifs)
      return false;

    std::istreambuf_iterator<char> it(ifs.rdbuf());
    std::istreambuf_iterator<char> eos;

    return utf8::is_valid(it, eos);
    }

  inline bool valid_utf8_file(const std::string& filename)
    {
#ifdef _WIN32
    std::wstring wfn = convert_string_to_wstring(filename);
    std::ifstream ifs(wfn);
#else
    std::ifstream ifs(filename);
#endif
    if (!ifs)
      return false;

    std::istreambuf_iterator<char> it(ifs.rdbuf());
    std::istreambuf_iterator<char> eos;

    return utf8::is_valid(it, eos);
    }

  inline bool is_directory(const std::string& directory)
    {
#ifdef _WIN32
    std::wstring wdirectory = convert_string_to_wstring(directory);
    if (wdirectory.length() >= PATH_MAX)
      return false;
    bool result = false;
    _WDIR* dir = wopendir(wdirectory.c_str());
    if (dir)
      {
      _wdirent* ent = wreaddir(dir);
      if (ent)
        result = true;
      wclosedir(dir);
      }
    return result;
#else
    if (directory.length() >= PATH_MAX)
      return false;
    bool result = false;
    DIR* dir = opendir(directory.c_str());
    if (dir)
      {
      dirent* ent = readdir(dir);
      if (ent)
        result = true;
      closedir(dir);
      }
    return result;
#endif  
    }

  inline bool file_exists(const std::string& filename)
    {
#ifdef _WIN32
    std::wstring wfilename = convert_string_to_wstring(filename);
    std::ifstream f;
    f.open(wfilename, std::ifstream::in);
    if (f.fail())
      return false;
    f.close();
    return true;
#else
    if (is_directory(filename)) // This should not be necessary, but otherwise crash with gcc on Ubuntu
      return false;
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
#endif
    }

  inline std::vector<std::string> get_files_from_directory(const std::string& d, bool include_subfolders)
    {
    std::string directory(d);
    if (!directory.empty() && !(directory.back() == '/' || directory.back() == '\\'))
      directory.push_back('/');
    std::vector<std::string> files;
#ifdef _WIN32
    std::wstring wdirectory = convert_string_to_wstring(directory);
    _WDIR* dir = wopendir(wdirectory.c_str());
    _wdirent* ent = nullptr;
    if (dir)
      ent = wreaddir(dir);
#else
    DIR* dir = opendir(directory.c_str());
    dirent* ent = nullptr;
    if (dir)
      ent = readdir(dir);
#endif
    if (!dir)
      return files;
    while (ent)
      {
      if (ent->d_type == DT_REG || ent->d_type == DT_LNK) // a file
        {
#ifdef _WIN32
        files.push_back(directory + convert_wstring_to_string(std::wstring(ent->d_name)));
#else
        files.push_back(directory + std::string(ent->d_name));
#endif
        }
      else if (include_subfolders && ent->d_type == DT_DIR) // a directory
        {
#ifdef _WIN32
        std::string n = convert_wstring_to_string(std::wstring(ent->d_name));
#else
        std::string n(ent->d_name);
#endif
        if (n.front() != '.')
          {
          std::string path = directory + n;
          auto files_sub = get_files_from_directory(path, include_subfolders);
          files.insert(files.end(), files_sub.begin(), files_sub.end());
          }
        }

#ifdef _WIN32
      ent = wreaddir(dir);
#else
      ent = readdir(dir);
#endif
      }
#ifdef _WIN32
    wclosedir(dir);
#else
    closedir(dir);
#endif
    return files;
    }

  inline std::vector<std::string> get_subdirectories_from_directory(const std::string& d, bool include_subfolders)
    {
    std::string directory(d);
    if (!directory.empty() && !(directory.back() == '/' || directory.back() == '\\'))
      directory.push_back('/');
    std::vector<std::string> files;
#ifdef _WIN32
    std::wstring wdirectory = convert_string_to_wstring(directory);
    _WDIR* dir = wopendir(wdirectory.c_str());
    _wdirent* ent = nullptr;
    if (dir)
      ent = wreaddir(dir);
#else
    DIR* dir = opendir(directory.c_str());
    dirent* ent = nullptr;
    if (dir)
      ent = readdir(dir);
#endif
    if (!dir)
      return files;
    while (ent)
      {
      if (ent->d_type == DT_DIR) // a directory
        {
#ifdef _WIN32
        std::string n = convert_wstring_to_string(std::wstring(ent->d_name));
#else
        std::string n(ent->d_name);
#endif
        if (n.front() != '.')
          {
          files.push_back(directory + n);
          if (include_subfolders)
            {
            auto files_sub = get_subdirectories_from_directory(files.back(), include_subfolders);
            files.insert(files.end(), files_sub.begin(), files_sub.end());
            }
          }
        }

#ifdef _WIN32
      ent = wreaddir(dir);
#else
      ent = readdir(dir);
#endif
      }
#ifdef _WIN32
    wclosedir(dir);
#else
    closedir(dir);
#endif
    return files;
    }

  inline std::vector<std::string> get_list_from_directory(const std::string& d, bool include_subfolders)
    {
    std::string directory(d);
    if (!directory.empty() && !(directory.back() == '/' || directory.back() == '\\'))
      directory.push_back('/');
    std::vector<std::string> files;
#ifdef _WIN32
    std::wstring wdirectory = convert_string_to_wstring(directory);
    _WDIR* dir = wopendir(wdirectory.c_str());
    _wdirent* ent = nullptr;
    if (dir)
      ent = wreaddir(dir);
#else
    DIR* dir = opendir(directory.c_str());
    dirent* ent = nullptr;
    if (dir)
      ent = readdir(dir);
#endif
    if (!dir)
      return files;
    while (ent)
      {
      if (ent->d_type == DT_REG || ent->d_type == DT_LNK) // a file
        {
#ifdef _WIN32
        files.push_back(directory + convert_wstring_to_string(std::wstring(ent->d_name)));
#else
        files.push_back(directory + std::string(ent->d_name));
#endif
        }
      else if (ent->d_type == DT_DIR) // a directory
        {
#ifdef _WIN32
        std::string n = convert_wstring_to_string(std::wstring(ent->d_name));
#else
        std::string n(ent->d_name);
#endif

        if (n.front() != '.')
          {
          files.push_back(directory + n);
          if (include_subfolders)
            {
            auto files_sub = get_list_from_directory(files.back(), include_subfolders);
            files.insert(files.end(), files_sub.begin(), files_sub.end());
            }
          }
        }

#ifdef _WIN32
      ent = wreaddir(dir);
#else
      ent = readdir(dir);
#endif
      }
#ifdef _WIN32
    wclosedir(dir);
#else
    closedir(dir);
#endif
    return files;
    }

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
    
  inline std::string getenv(const std::string& name)
    {
  #ifdef _WIN32
    std::wstring ws = jtk::convert_string_to_wstring(name);
    wchar_t* path = _wgetenv(ws.c_str());
    if (!path)
      return nullptr;
    std::wstring wresult(path);
    std::string out = jtk::convert_wstring_to_string(wresult);
  #else
    std::string out(::getenv(name.c_str()));
  #endif
    return out;
    }

  inline void putenv(const std::string& name, const std::string& value)
    {
#ifdef _WIN32
    std::wstring wvalue = jtk::convert_string_to_wstring(value);
    std::wstring wname = jtk::convert_string_to_wstring(name);
    _wputenv_s(wname.c_str(), wvalue.c_str());
#else
    ::setenv(name.c_str(), value.c_str(), 1);
#endif
    }

#ifdef _WIN32
  inline long long file_size(const std::string& filename)
    {
    struct _stat64 st;

    std::wstring wfilename = convert_string_to_wstring(filename);

    if (_wstat64(wfilename.c_str(), &st) == 0)
      return st.st_size;

    return -1;
    }
#else
  inline long long file_size(const std::string& filename)
    {
    struct stat st;

    if (stat(filename.c_str(), &st) == 0)
      return st.st_size;

    return -1;
    }
#endif

  } // namespace jtk
