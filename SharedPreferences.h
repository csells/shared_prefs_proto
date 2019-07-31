#pragma once
#include "pch.h"
#include <sstream>
#include <vector>
#include "basic_debugbuf.h"
#include "VersionInfo.h"

using namespace std;

class SharedPreferences {
  const wchar_t* sectionName = L"preferences";
  const wchar_t* containsUuid = L"f042b00f-adcd-4462-94ba-fae5265a541c";
  const int MAX_VALUE_LENGTH = 1024;
  const wstring prefsFileName = getPrefsFileName();

public:
  SharedPreferences() {
    dbg::wcout << L"SharedPreferences.prefsFileName= '" << prefsFileName.c_str() << "'" << endl;
  }

private:
  static wstring getPrefsFileName() {
    wostringstream ss;

    // TODO: update this to use Windows resources, i.e. company name and app name
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

  static wstring getListSizeKey(const wchar_t* key) {
    return (wostringstream() << key << L".size").str();
  }

  static wstring getListIndexKey(const wchar_t* key, int index) {
    return (wostringstream() << key << L"[" << index << L"]").str();
  }

public:
  wstring getString(const wchar_t* key, const wchar_t* defval = 0) {
    wstring value;
    value.reserve(MAX_VALUE_LENGTH);

    auto size = GetPrivateProfileStringW(sectionName, key, defval, &value[0], value.capacity(), prefsFileName.c_str());
    if (size == 0) {
      wostringstream err;
      err << L"GetPrivateProfileStringW failed. sectionName='" << sectionName << "' key='" << key << L"'. " << WindowsUtility::getLastErrorMessage();
      throw err.str();
    }

    return value;
  }

  bool getBool(const wchar_t* key, bool defval = false) {
    return stoi(getString(key, to_wstring(defval).c_str())) != 0;
  }

  int getInt(const wchar_t* key, int defval = 0) {
    return stoi(getString(key, to_wstring(defval).c_str()));
  }

  double getDouble(const wchar_t* key, double defval = 0.0) {
    return stod(getString(key, to_wstring(defval).c_str()));
  }

  vector<wstring> getStringList(const wchar_t* key, vector<const wchar_t*> defvals = vector<const wchar_t*>()) {
    vector<wstring> values;

    int size = getInt(getListSizeKey(key).c_str());
    if (size == 0) {
      for (auto pdefval = defvals.begin(); pdefval != defvals.end(); ++pdefval) {
        values.push_back(*pdefval);
      }
    }
    else {
      for (auto i = 0; i != size; ++i) {
        values.push_back(getString(getListIndexKey(key, i).c_str()));
      }
    }

    return values;
  }

  void setString(const wchar_t* key, const wchar_t* value) {
    auto success = WritePrivateProfileStringW(sectionName, key, value, prefsFileName.c_str());
    if (!success) {
      wostringstream err;
      err << L"WritePrivateProfileStringW failed. sectionName='" << sectionName << "' key='" << key << L"', value='" << value << L"'. " << WindowsUtility::getLastErrorMessage();
      throw err.str();
    }
  }

  void setBool(const wchar_t* key, bool value) {
    setString(key, to_wstring(value).c_str());
  }

  void setInt(const wchar_t* key, int value) {
    setString(key, to_wstring(value).c_str());
  }

  void setDouble(const wchar_t* key, double value) {
    setString(key, to_wstring(value).c_str());
  }

  void setStringList(const wchar_t* key, vector<const wchar_t*> values) {
    setInt(getListSizeKey(key).c_str(), values.size());
    for (auto i = 0; i != values.size(); ++i) {
      setString(getListIndexKey(key, i).c_str(), values[i]);
    }
  }

  void remove(const wchar_t* key) {
    // check if we're removing a list
    wstring listSizeKey = getListSizeKey(key);
    auto size = getInt(listSizeKey.c_str());
    if (size != 0) {
      setString(listSizeKey.c_str(), 0);
      for (auto i = 0; i != size; i++) {
        setString(getListIndexKey(key, i).c_str(), 0);
      }
    }
    else {
      setString(key, 0);
    }
  }

  bool containsKey(const wchar_t* key) {
    // if our default UUID is returned to us, the key isn't set in our preferences
    return wcscmp(getString(key, containsUuid).c_str(), containsUuid) != 0;
  }

  vector<wstring> getKeys() {
    vector<wstring> keys;

    wstring section;
    section.reserve(32767);
    auto size = GetPrivateProfileSectionW(sectionName, &section[0], section.capacity(), prefsFileName.c_str());
    if (size != 0) {
      // traverse double-null terminated string
      for (const wchar_t* pszz = &section[0]; *pszz; pszz += lstrlen(pszz) + 1) {
        auto pszzend = pszz + lstrlen(pszz);
        auto peq = find(pszz, pszzend, L'=');
        auto key = wstring(pszz, peq);
        keys.push_back(key);
      }
    }

    return keys;
  }

  void clear() {
    auto success = WritePrivateProfileSectionW(sectionName, 0, prefsFileName.c_str());
    if (!success) {
      wostringstream err;
      err << L"WritePrivateProfileSectionW failed. sectionName= '" << sectionName << "'. " << WindowsUtility::getLastErrorMessage();
      throw err.str();
    }
  }
};
