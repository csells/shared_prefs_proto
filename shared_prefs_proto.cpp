// shared_prefs_proto.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include "pch.h"
#include "SharedPreferences.h"
#include <iostream>
#include <sstream>
#include "shared_prefs_proto.h"

#define E(e, a) expect(TEXT(__FILE__), __LINE__, e, a)

template<class T> bool areEqual(const T lhs, const T rhs) { return lhs == rhs; }
template<> bool areEqual(const wchar_t* lhs, const wchar_t* rhs) { return wcscmp(lhs, rhs) == 0; }

template<class T> void expect(wstring file, int line, T expected, T actual) {
  wostringstream fileline;
  fileline << file.substr(file.find_last_of(L'\\') + 1) << "(" << line << ")";
  if (areEqual<T>(expected, actual)) {
    wcout << "test passed @ " << fileline.str() << endl;
  }
  else {
    wcerr << "test failed @ " << fileline.str() << ": expected '" << expected << "' vs. actual '" << actual << "'" << endl;
  }
}

// from https://github.com/flutter/plugins/blob/master/packages/shared_preferences/test/shared_preferences_test.dart
void testSharedPrefs() {
  SharedPreferences prefs;

  // where are we read/writing preferences?
  wcout << L"reading/writing preferences from: " << prefs.getPrefsFileName() << endl;

  // set some values
  prefs.setString(L"s1", L"hello, world");
  prefs.setBool(L"b1", true);
  prefs.setInt(L"i1", 42);
  prefs.setDouble(L"d1", 3.14);

  auto sl1a = vector<wstring>() = { L"foo", L"bar" };
  prefs.setStringList(L"sl1", sl1a);

  // get some values
  auto s1 = prefs.getString(L"s1");
  E(L"hello, world", s1.c_str());

  auto b1 = prefs.getBool(L"b1");
  E(true, b1);

  auto i1 = prefs.getInt(L"i1");
  E(42, i1);

  auto d1 = prefs.getDouble(L"d1");
  E(3.14, d1);

  auto sl1b = prefs.getStringList(L"sl1");
  E(sl1a.size(), sl1b.size());
  for (auto i = 0; i != sl1a.size(); ++i) {
    E(sl1a[i].c_str(), sl1b[i].c_str());
  }

  // get the list of keys
  vector<wstring> keys = prefs.getKeys();
  E(true, find(keys.begin(), keys.end(), L"s1") != keys.end());
  E(true, find(keys.begin(), keys.end(), L"b1") != keys.end());
  E(true, find(keys.begin(), keys.end(), L"i1") != keys.end());
  E(true, find(keys.begin(), keys.end(), L"d1") != keys.end());
  E(true, find(keys.begin(), keys.end(), L"sl1") != keys.end());

  // get the values as a variant
  auto v1 = prefs.get(L"s1");
  E(L"hello, world", get<wstring>(v1).c_str());

  auto v2 = prefs.get(L"b1");
  E(true, get<bool>(v2));

  auto v3 = prefs.get(L"i1");
  E(42, get<int>(v3));

  auto v4 = prefs.get(L"d1");
  E(3.14, get<double>(v4));

  auto v5 = prefs.get(L"sl1");
  auto sl2 = get<vector<wstring>>(v5);
  E(sl1a.size(), sl2.size());
  for (auto i = 0; i != sl2.size(); ++i) {
    E(sl2[i].c_str(), sl1a[i].c_str());
  }

  // get all of the values as variants
  auto all = prefs.getAll();
  E((size_t)5, all.size());
  for (auto const&[key, val] : all) {
    if (key.compare(L"s1") == 0) {
      E(L"hello, world", get<wstring>(val).c_str());
    }
    else if (key.compare(L"b1") == 0) {
      E(true, get<bool>(val));
    }
    else if (key.compare(L"i1") == 0) {
      E(42, get<int>(val));
    }
    else if (key.compare(L"d1") == 0) {
      E(3.14, get<double>(val));
    }
    else if (key.compare(L"sl1") == 0) {
      auto sl3 = get<vector<wstring>>(val);
      E(sl1a.size(), sl3.size());
      for (auto i = 0; i != sl3.size(); ++i) {
        E(sl3[i].c_str(), sl1a[i].c_str());
      }
    }
    else throw "oops";
  }

  // remove some values
  prefs.remove(L"s1");
  prefs.remove(L"b1");
  prefs.remove(L"i1");
  prefs.remove(L"d1");
  prefs.remove(L"sl1");

  // get some default values
  auto s1b = prefs.getString(L"s1", L"goodnight moon");
  E(L"goodnight moon", s1b.c_str());

  auto b1b = prefs.getBool(L"d1");
  E(false, b1b);

  auto i1b = prefs.getInt(L"i1");
  E(0, i1b);

  auto d1b = prefs.getDouble(L"d1");
  E(0.0, d1b);

  auto sl1c = prefs.getStringList(L"sl1");
  E((size_t)0, sl1c.size());

  auto containsKey = prefs.containsKey(L"key1");
  E(false, containsKey);

  prefs.setString(L"foo", L"bar");
  auto containsKey2 = prefs.containsKey(L"foo");
  E(true, containsKey2);
  prefs.clear();
  auto containsKey3 = prefs.containsKey(L"foo");
  E(false, containsKey3);
}

void testSimpleFilename() {
  // illegal file name
  auto fn1 = WindowsUtility::getSimpleFilename(L"COM1");
  E(L"_COM1", fn1.c_str());

  // trailing dots and spaces
  auto fn2 = WindowsUtility::getSimpleFilename(L"foo...   ");
  E(L"foo", fn2.c_str());

  // illegal characters
  auto fn3 = WindowsUtility::getSimpleFilename(L"<>:\"/\\?*b<>:\"/\\?*a<>:\"/\\?*r<>:\"/\\?*");
  E(L"bar", fn3.c_str());

  // empty
  auto fn4 = WindowsUtility::getSimpleFilename(L"<>:\"/\\?*");
  E(L"", fn4.c_str());
}

void dumpVersionInfo(const VersionInfo& ver) {
  wcout << L"Version info for file: " << ver.getFilename() << endl;
  wcout << "  Comments: '" << ver.getComments() << "'" << endl;
  wcout << "  CompanyName: '" << ver.getCompanyName() << "'" << endl;
  wcout << "  FileDescription: '" << ver.getFileDescription() << "'" << endl;
  wcout << "  FileVersion: '" << ver.getFileVersion() << "'" << endl;
  wcout << "  InternalName: '" << ver.getInternalName() << "'" << endl;
  wcout << "  LegalCopyright: '" << ver.getLegalCopyright() << "'" << endl;
  wcout << "  LegalTrademarks: '" << ver.getLegalTrademarks() << "'" << endl;
  wcout << "  OriginalFilename: '" << ver.getOriginalFilename() << "'" << endl;
  wcout << "  PrivateBuild: '" << ver.getPrivateBuild() << "'" << endl;
  wcout << "  ProductName: '" << ver.getProductName() << "'" << endl;
  wcout << "  ProductVersion: '" << ver.getProductVersion() << "'" << endl;
  wcout << "  SpecialBuild: '" << ver.getSpecialBuild() << "'" << endl;
}

void testVersionInfo() {
  auto filename = L"C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\Common7\\IDE\\devenv.exe";
  VersionInfo ver1(filename);
  dumpVersionInfo(ver1);

  try {
    VersionInfo ver2;
    dumpVersionInfo(ver2);
  }
  catch (const wstring& err) {
    wcout << "Can't load VERSIONINFO: " << err << endl;
  }
}

int main() {
  testSharedPrefs();
  testSimpleFilename();
  testVersionInfo();
}
