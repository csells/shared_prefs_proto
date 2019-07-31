#pragma once
#include "pch.h"
#include <sstream>

using namespace std;

class WindowsUtility {
public:
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
};

// e.g.
//auto filename = L"C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\Common7\\IDE\\devenv.exe";
//VersionInfo ver(filename);
//wcout << "devenv ProductName= '" << ver.getProductName() << "'" << endl;
class VersionInfo {
  wstring filename;

public:
  VersionInfo(const wchar_t* filename) {
    this->filename = filename;
  }

  wstring getProductName() {
    DWORD dwLen, dwUseless;
    LPTSTR lpVI;
    std::wstring productName;
    UINT verMajor;

    dwLen = GetFileVersionInfoSizeExW(FILE_VER_GET_LOCALISED, filename.c_str(), &dwUseless);
    if (dwLen == 0)
      throw "oops";

    lpVI = (LPTSTR)GlobalAlloc(GPTR, dwLen);
    if (lpVI == 0) throw "oops";

    DWORD dwBufSize;
    VS_FIXEDFILEINFO* lpFFI;
    BOOL bRet = FALSE;
    WORD* langInfo;
    UINT cbLang;
    TCHAR tszVerStrName[128];
    LPVOID lpt;
    UINT cbBufSize;

    GetFileVersionInfoExW(FILE_VER_GET_LOCALISED, filename.c_str(), NULL, dwLen, lpVI);

    if (VerQueryValue(lpVI, L"\\",
      (LPVOID *)&lpFFI, (UINT *)&dwBufSize))

    {
      //We now have the VS_FIXEDFILEINFO in lpFFI
      verMajor = HIWORD(lpFFI->dwFileVersionMS);
    }
    //Get the Product Name.
    //First, to get string information, we need to get
    //language information.
    VerQueryValueW(lpVI, L"\\VarFileInfo\\Translation",
      (LPVOID*)&langInfo, &cbLang);
    //Prepare the label -- default lang is bytes 0 & 1
    //of langInfo
    wsprintf(tszVerStrName, L"\\StringFileInfo\\%04x%04x\\%s",
      langInfo[0], langInfo[1], L"ProductName");
    //Get the string from the resource data
    if (VerQueryValueW(lpVI, tszVerStrName, &lpt, &cbBufSize))
      productName.assign((LPTSTR)lpt);    //*must* save this
  //Cleanup
    GlobalFree((HGLOBAL)lpVI);

    return productName;
  }
};
