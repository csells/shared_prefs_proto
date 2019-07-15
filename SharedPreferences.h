#pragma once

#include "pch.h"
#include <sstream>
#include "basic_debugbuf.h"

using namespace std;

class SharedPreferences {
  const wstring prefsFileName = getPrefsFileName();
  const wchar_t* sectionName = L"preferences";

public:
  SharedPreferences() {
    dbg::wcout << L"SharedPreferences.prefsFileName= '" << prefsFileName.c_str() << "'" << endl;
  }

private:
  static wstring getPrefsFileName() {
    wostringstream ss;

    PWSTR localAppDataFolder = NULL;
    SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &localAppDataFolder);
    ss << localAppDataFolder;
    CoTaskMemFree(localAppDataFolder);

    wstring modulePath;
    modulePath.reserve(MAX_PATH);
    GetModuleFileNameW(NULL, &modulePath[0], modulePath.capacity());

    wstring moduleFileName;
    moduleFileName.reserve(_MAX_FNAME);
    _wsplitpath_s(modulePath.c_str(), NULL, 0, NULL, 0, &moduleFileName[0], moduleFileName.capacity(), NULL, 0);
    ss << L'\\' << moduleFileName.c_str();

    // make sure the folder exists for the preferences file
    CreateDirectoryW(ss.str().c_str(), NULL);

    ss << L"\\prefs.ini";
    return ss.str();
  }

  static wstring getLastErrorMessage() {
    DWORD dw = GetLastError();

    wchar_t* msgBuf;
    FormatMessageW(
      FORMAT_MESSAGE_ALLOCATE_BUFFER |
      FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      dw,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR)&msgBuf,
      0,
      NULL);

    wostringstream ss;
    ss << msgBuf << L"(" << dw << L")";
    LocalFree(msgBuf);
    return ss.str();
  }

public:
  void setString(const wchar_t* key, const wchar_t* value) {
    // TODO: write this to an ini file at the LocalAppData folder
    auto success = WritePrivateProfileStringW(sectionName, key, value, prefsFileName.c_str());
    if (!success) {
      wostringstream err;
      err << L"setString failed. key= '" << key << L"', value= '" << value << L"'. " << getLastErrorMessage();
      throw err.str();
    }
  }

  wstring getString(const wchar_t* key, const wchar_t* defaultValue = NULL) {
    wstring value;
    value.reserve(256);
    auto success = GetPrivateProfileStringW(sectionName, key, defaultValue, &value[0], value.capacity(), prefsFileName.c_str());
    if (!success) {
      wostringstream err;
      err << L"getString failed. key= '" << key << L"', defaultValue= '" << defaultValue << L"'. " << getLastErrorMessage();
      throw err.str();
    }

    return value;
  }

};