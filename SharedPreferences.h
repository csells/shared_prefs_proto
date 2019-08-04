#pragma once
#include "pch.h"
#include <sstream>
#include <vector>
#include <variant>
#include <map>
#include "basic_debugbuf.h"
#include "VersionInfo.h"

using namespace std;

class SharedPreferences {
private:
  inline static const wchar_t* sectionName = L"flutter";
  inline static const wchar_t* missingUuid = L"f042b00f-adcd-4462-94ba-fae5265a541c";
  inline static const wchar_t* listSizeEnd = L".size";
  inline static const int MAX_VALUE_LENGTH = 1024;
  const wstring prefsFileName = getPrefsFileName();

  static wstring getListSizeKey(const wstring& key) {
    return (wostringstream() << key << listSizeEnd).str();
  }

  static wstring getListIndexKey(const wstring& key, int index) {
    return (wostringstream() << key << L"[" << index << L"]").str();
  }

  static bool endsWith(wstring const& s, wstring const& end) {
    if (s.length() < end.length()) return false;
    return s.compare(s.length() - end.length(), end.length(), end) == 0;
  }

  template <typename T>
  bool try_parse(const wstring& s, T& r) {
    wstringstream ss(s.c_str());
    return !((ss >> r).fail() || !(ss >> ws).eof());
  }

  template <>
  bool try_parse(const wstring& s, bool& r) {
    int i;
    if (try_parse(s, i) && ((i == 0) || (i == 1))) { r = (i == 1); return true; }
    return false;
  }

public:
  typedef variant<wstring, bool, int, double, vector<wstring>> AnyPrefValue;

  static wstring getPrefsFileName() {
    wostringstream ss;

    // TODO: update this to use Windows resources, i.e. company name and app name
    PWSTR localAppDataFolder = NULL;
    SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, 0, &localAppDataFolder);
    ss << localAppDataFolder;
    CoTaskMemFree(localAppDataFolder);

    wstring modulePath;
    modulePath.reserve(MAX_PATH);
    GetModuleFileNameW(0, &modulePath[0], modulePath.capacity());

    wstring moduleFileName;
    moduleFileName.reserve(_MAX_FNAME);
    _wsplitpath_s(modulePath.c_str(), 0, 0, 0, 0, &moduleFileName[0], moduleFileName.capacity(), 0, 0);
    ss << L'\\' << moduleFileName.c_str();

    // make sure the folder exists for the preferences file
    CreateDirectoryW(ss.str().c_str(), NULL);

    ss << L"\\prefs.ini";
    return ss.str();
  }

  AnyPrefValue get(const wstring& key, wstring defval = L"") {
    // is it a list?
    auto sl = getStringList(key, vector<wstring> { missingUuid });
    if (!(sl.size() == 1 && sl[0].compare(missingUuid) == 0)) return sl;

    // if the key is missing completely, return the default value
    auto s = getString(key, missingUuid);
    if (s.compare(missingUuid) == 0) return defval;

    // is it a bool?
    bool b;
    if (try_parse(s, b)) return b;

    // is it an int?
    int i;
    if (try_parse(s, i)) return i;

    // is it a double?
    double d;
    if (try_parse(s, d)) return d;

    // otherwise, it's a string
    return s;
  }

  wstring getString(const wstring& key, const wstring& defval = L"") {
    wstring value;
    value.reserve(MAX_VALUE_LENGTH);

    auto size = GetPrivateProfileStringW(sectionName, key.c_str(), defval.empty() ? 0 : defval.c_str(), &value[0], value.capacity(), prefsFileName.c_str());
    if (size == 0) {
      wostringstream err;
      err << L"GetPrivateProfileStringW failed. sectionName='" << sectionName << "' key='" << key << L"'. " << WindowsUtility::getLastErrorMessage();
      throw err.str();
    }

    return value;
  }

  bool getBool(const wstring& key, bool defval = false) {
    return stoi(getString(key, to_wstring(defval))) != 0;
  }

  int getInt(const wstring& key, int defval = 0) {
    return stoi(getString(key, to_wstring(defval)));
  }

  double getDouble(const wstring& key, double defval = 0.0) {
    return stod(getString(key, to_wstring(defval)));
  }

  vector<wstring> getStringList(const wstring& key, const vector<wstring>& defvals = vector<wstring>()) {
    vector<wstring> values;

    int size = getInt(getListSizeKey(key), 0);
    if (size == 0) {
      for (auto& defval : defvals) {
        values.push_back(defval);
      }
    }
    else {
      for (auto i = 0; i != size; ++i) {
        values.push_back(getString(getListIndexKey(key, i).c_str()));
      }
    }

    return values;
  }

  map<wstring, AnyPrefValue> getAll() {
    map<wstring, AnyPrefValue> all;
    for (auto& key : getKeys()) all[key] = get(key);
    return all;
  }

  void setString(const wstring& key, const wstring& value) {
    auto success = WritePrivateProfileStringW(sectionName, key.c_str(), value.empty() ? 0 : value.c_str(), prefsFileName.c_str());
    if (!success) {
      wostringstream err;
      err << L"WritePrivateProfileStringW failed. sectionName='" << sectionName << "' key='" << key << L"', value='" << value << L"'. " << WindowsUtility::getLastErrorMessage();
      throw err.str();
    }
  }

  void setBool(const wstring& key, bool value) {
    setString(key, to_wstring(value));
  }

  void setInt(const wstring& key, int value) {
    setString(key, to_wstring(value));
  }

  void setDouble(const wstring& key, double value) {
    setString(key, to_wstring(value));
  }

  void setStringList(const wstring& key, const vector<wstring>& values) {
    setInt(getListSizeKey(key), values.size());
    for (auto i = 0; i != values.size(); ++i) {
      setString(getListIndexKey(key, i), values[i]);
    }
  }

  void remove(const wstring& key) {
    // check if we're removing a list
    wstring listSizeKey = getListSizeKey(key);
    auto size = getInt(listSizeKey);
    if (size != 0) {
      setString(listSizeKey, L"");
      for (auto i = 0; i != size; i++) {
        setString(getListIndexKey(key, i), L"");
      }
    }
    else {
      setString(key, L"");
    }
  }

  bool containsKey(const wstring& key) {
    // if our default UUID is returned to us, the key isn't set in our preferences
    return wcscmp(getString(key, missingUuid).c_str(), missingUuid) != 0;
  }

  vector<wstring> getKeys() {
    vector<wstring> keys;
    wstring section;
    section.reserve(32767);
    auto size = GetPrivateProfileSectionW(sectionName, &section[0], section.capacity(), prefsFileName.c_str());
    if (size > 0) {
      // traverse double-null terminated string
      for (auto pszz = &section[0]; *pszz; pszz += lstrlen(pszz) + 1) {
        auto pszzend = pszz + lstrlen(pszz);
        auto peq = find(pszz, pszzend, L'=');
        auto key = wstring(pszz, peq);

        // look for list keys
        if (endsWith(key, listSizeEnd)) key.erase(key.length() - lstrlen(listSizeEnd));
        if (endsWith(key, L"]")) continue;

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
