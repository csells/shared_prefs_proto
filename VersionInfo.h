#pragma once
#include "pch.h"
#include "WindowsUtility.h"
#include <sstream>
#include <iomanip>

using namespace std;

// from https://www.codeguru.com/cpp/w-p/win32/versioning/article.php/c4539/Versioning-in-Windows.htm
// e.g.
// auto filename = L"C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\Common7\\IDE\\devenv.exe";
// VersionInfo ver(filename);
// wcout << "ProductName= '" << ver.getProductName() << "'" << endl;
class VersionInfo {
private:
	BYTE* pVI;

	wstring getStringFileInfo(const wstring& label) {
		// First, to get string information, we need to get language information
		WORD* langInfo;
		UINT cbLang;
		auto success = VerQueryValueW(pVI, L"\\VarFileInfo\\Translation", reinterpret_cast<LPVOID*>(&langInfo), &cbLang);
		if (!success) {
			wostringstream err;
			err << L"VerQueryValueW failed: \\VarFileInfo\\Translation. '" << WindowsUtility::getLastErrorMessage();
			throw err.str();
		}

		// Prepare the label -- default lang is bytes 0 & 1 of langInfo
		wostringstream oss;
		oss << L"\\StringFileInfo\\" << setfill(L'0') << setw(4) << setbase(16) << langInfo[0] << setw(4) << langInfo[1] << L'\\' << label;

		// Get the string from the resource data
		LPVOID lpt;
		UINT cbBufSize;
		success = VerQueryValueW(pVI, oss.str().c_str(), &lpt, &cbBufSize);
		if (!success) {
			// special case "resource type not found"
			if (GetLastError() == ERROR_RESOURCE_TYPE_NOT_FOUND) return wstring();

			wostringstream err;
			err << L"VerQueryValueW failed: " << oss.str() << ". '" << WindowsUtility::getLastErrorMessage();
			throw err.str();
		}

		return wstring((LPTSTR)lpt);
	}

public:
	VersionInfo(const wstring& filename) {
		DWORD useless;
		auto size = GetFileVersionInfoSizeExW(FILE_VER_GET_LOCALISED, filename.c_str(), &useless);
		if (size == 0) {
			wostringstream err;
			err << L"GetFileVersionInfoSizeExW failed. fileName= '" << filename << "'. " << WindowsUtility::getLastErrorMessage();
			throw err.str();
		}

		pVI = new BYTE[size];
		auto success = GetFileVersionInfoExW(FILE_VER_GET_LOCALISED, filename.c_str(), 0, size, pVI);
		if (!success) {
			wostringstream err;
			err << L"GetFileVersionInfoExW failed. fileName= '" << filename << "'. " << WindowsUtility::getLastErrorMessage();
			throw err.str();
		}
	}

	~VersionInfo() {
		delete[] pVI;
	}

	wstring getComments() { return getStringFileInfo(L"Comments"); }
	wstring getCompanyName() { return getStringFileInfo(L"CompanyName"); }
	wstring getFileDescription() { return getStringFileInfo(L"FileDescription"); }
	wstring getFileVersion() { return getStringFileInfo(L"FileVersion"); }
	wstring getInternalName() { return getStringFileInfo(L"InternalName"); }
	wstring getLegalCopyright() { return getStringFileInfo(L"LegalCopyright"); }
	wstring getLegalTrademarks() { return getStringFileInfo(L"LegalTrademarks"); }
	wstring getOriginalFilename() { return getStringFileInfo(L"OriginalFilename"); }
	wstring getPrivateBuild() { return getStringFileInfo(L"PrivateBuild"); }
	wstring getProductName() { return getStringFileInfo(L"ProductName"); }
	wstring getProductVersion() { return getStringFileInfo(L"ProductVersion"); }
	wstring getSpecialBuild() { return getStringFileInfo(L"SpecialBuild"); }
};
