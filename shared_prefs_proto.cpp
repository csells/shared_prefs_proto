// shared_prefs_proto.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include "pch.h"
#include "SharedPreferences.h"
#include <iostream>
#include <sstream>

#define E(e, v) expect(TEXT(__FILE__), __LINE__, e, v)

void expect(wstring file, int line, const wchar_t* expected, const wchar_t* actual) {
  wostringstream fileline;
  fileline << file.substr(file.find_last_of(L'\\')+1) << "(" << line << ")";
  if (wcscmp(expected, actual) == 0) {
    wcout << "test passed @ " << fileline.str() << endl;
  }
  else {
    wcerr << "test failed @ " << fileline.str() << ": expected '" << expected << "' vs. actual '" << actual << "'" << endl;
  }
}

int main() {
  auto prefs = new SharedPreferences();

  // from https://github.com/flutter/plugins/blob/master/packages/shared_preferences/test/shared_preferences_test.dart
  prefs->setString(L"s1", L"hello, world");
  auto value = prefs->getString(L"s1");
  E(L"hello, world", value.c_str());
}
