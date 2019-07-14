#pragma once

#include "pch.h"
#include <iostream>
#include "basic_debugbuf.h"

using namespace std;

class SharedPreferences {
  wstring localAppData;

public:
  SharedPreferences() {
    PWSTR path = NULL;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &path))) {
      localAppData.append(path);
      CoTaskMemFree(path);

      dbg::wcout << L"localAppData= " << localAppData.c_str() << endl;
    }
  }

  void setString(const wchar_t* key, const wchar_t* value) {
    // TODO: write this to an ini file at the LocalAppData folder
  }

  wstring getString(const wchar_t* key) {
    return wstring(L"hello, world");
    // TODO: read this from an ini file at the LocalAppData folder
  }

};