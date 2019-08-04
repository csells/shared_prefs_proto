#pragma once
#include "pch.h"
#include <sstream>
#include <iomanip>
#include <regex>

using namespace std;

class WindowsUtility {
public:
  static wstring getLastErrorMessage() {
    DWORD dw = GetLastError();
    wchar_t* msgBuf;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&msgBuf), 0, 0);
    wostringstream oss;
    oss << msgBuf << L"(0x" << setw(4) << setfill(L'0') << setbase(16) << dw << L")";
    LocalFree(msgBuf);
    return oss.str();
  }

  // from https://stackoverflow.com/questions/1976007/what-characters-are-forbidden-in-windows-and-linux-directory-names
  static wstring getSimpleFilename(const wstring& baseFilenameNoExt) {
    // keep just these characters which we know are simple and safe: 'A-Za-z0-9-_ '
    wstring filename = regex_replace(baseFilenameNoExt, wregex(L"[^A-Za-z0-9-_ ]"), L"");

    // muck with file names which we know are illegal
    const vector<wstring> badFilenames = { L"CON", L"PRN", L"AUX", L"NUL", L"COM1", L"COM2", L"COM3", L"COM4", L"COM5", L"COM6", L"COM7", L"COM8", L"COM9", L"LPT1", L"LPT2", L"LPT3", L"LPT4", L"LPT5", L"LPT6", L"LPT7", L"LPT8", L"LPT9", };
    for (auto& badfn : badFilenames) {
      if (filename.compare(badfn) == 0) filename.insert(filename.begin(), L'_');
    }

    // trim trailing spaces and dots
    for (auto pch = filename.rbegin(); pch != filename.rend(); ++pch) {
      if ((*pch == L' ') || (*pch == L'.')) *pch = 0;
    }

    return filename;
  }

  static void createDirectory(const wstring& dirname) {
    auto success = CreateDirectoryW(dirname.c_str(), 0);
    if (!success) {
      // special case if the directory already exists
      if (GetLastError() == ERROR_ALREADY_EXISTS) return;

      wostringstream err;
      err << L"CreateDirectoryW failed. directory='" << dirname.c_str() << L"'. " << WindowsUtility::getLastErrorMessage();
      throw err.str();
    }
  }

};

