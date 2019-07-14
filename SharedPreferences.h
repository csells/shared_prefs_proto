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
};