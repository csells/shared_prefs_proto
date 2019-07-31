// shared_prefs_proto.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include "pch.h"
#include "SharedPreferences.h"
#include <iostream>
#include <sstream>
#include "shared_prefs_proto.h"

#define E(e, a) expect(TEXT(__FILE__), __LINE__, e, a)

template<class T> bool areEqual(T lhs, T rhs) { return lhs == rhs; }
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
	auto prefs = new SharedPreferences();

	// set some values
	prefs->setString(L"s1", L"hello, world");
	prefs->setBool(L"b1", true);
	prefs->setInt(L"i1", 42);
	prefs->setDouble(L"d1", 3.14);

	auto sl1a = vector<const wchar_t*>();
	sl1a.push_back(L"foo");
	sl1a.push_back(L"bar");
	prefs->setStringList(L"sl1", sl1a);

	// get some values
	auto s1 = prefs->getString(L"s1");
	E(L"hello, world", s1.c_str());

	auto b1 = prefs->getBool(L"b1");
	E(true, b1);

	auto i1 = prefs->getInt(L"i1");
	E(42, i1);

	auto d1 = prefs->getDouble(L"d1");
	E(3.14, d1);

	auto sl1b = prefs->getStringList(L"sl1");
	E(sl1a.size(), sl1b.size());
	for (auto i = 0; i != sl1a.size(); ++i) {
		E(sl1a[i], sl1b[i].c_str());
	}

	// get the list of keys
	vector<wstring> keys = prefs->getKeys();
	E(true, find(keys.begin(), keys.end(), L"s1") != keys.end());
	E(true, find(keys.begin(), keys.end(), L"b1") != keys.end());
	E(true, find(keys.begin(), keys.end(), L"i1") != keys.end());
	E(true, find(keys.begin(), keys.end(), L"d1") != keys.end());
	E(true, find(keys.begin(), keys.end(), L"sl1") != keys.end()); // TODO: make this work

	// remove some values
	prefs->remove(L"s1");
	prefs->remove(L"b1");
	prefs->remove(L"i1");
	prefs->remove(L"d1");
	prefs->remove(L"sl1");

	// get some default values
	auto s1b = prefs->getString(L"s1", L"goodnight moon");
	E(L"goodnight moon", s1b.c_str());

	auto b1b = prefs->getBool(L"d1");
	E(false, b1b);

	auto i1b = prefs->getInt(L"i1");
	E(0, i1b);

	auto d1b = prefs->getDouble(L"d1");
	E(0.0, d1b);

	auto sl1c = prefs->getStringList(L"sl1");
	E((size_t)0, sl1c.size());

	auto containsKey = prefs->containsKey(L"key1");
	E(false, containsKey);

	prefs->setString(L"foo", L"bar");
	prefs->clear();
	auto containsKey2 = prefs->containsKey(L"foo");
	E(false, containsKey2);
}

void testSimpleFilename() {
	auto fn1 = WindowsUtility::getSimpleFilename(L"COM1");
	E(L"_COM1", fn1.c_str());

	auto fn2 = WindowsUtility::getSimpleFilename(L"foo...   ");
	E(L"foo", fn2.c_str());

	auto fn3 = WindowsUtility::getSimpleFilename(L"<>:\"/\\?*b<>:\"/\\?*a<>:\"/\\?*r<>:\"/\\?*");
	E(L"bar", fn3.c_str());
}

int main() {
	testSharedPrefs();
	testSimpleFilename();
}
