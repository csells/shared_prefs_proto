#pragma once

#include "pch.h"
#include <sstream>
#include "basic_debugbuf.h"

using namespace std;

class SharedPreferences {
  const wstring prefsFileName = getPrefsFileName();

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
    GetModuleFileNameW(NULL, &modulePath[0], MAX_PATH);

    wstring moduleFileName;
    moduleFileName.reserve(_MAX_FNAME);
    _wsplitpath_s(modulePath.c_str(), NULL, 0, NULL, 0, &moduleFileName[0], _MAX_FNAME, NULL, 0);
    ss << L'\\' << moduleFileName.c_str() << L"\\prefs.ini";

    return ss.str();
  }

public:
  void setString(const wchar_t* key, const wchar_t* value) {
    // TODO: write this to an ini file at the LocalAppData folder
  }

  wstring getString(const wchar_t* key) {
    return wstring(L"hello, world");
    // TODO: read this from an ini file at the LocalAppData folder
  }

};