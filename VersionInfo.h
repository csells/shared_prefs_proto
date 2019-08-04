#pragma once
#include "pch.h"
#include "WindowsUtility.h"
#include <sstream>
#include <iomanip>

using namespace std;

// from https://www.codeguru.com/cpp/w-p/win32/versioning/article.php/c4539/Versioning-in-Windows.htm
// e.g.
// VersionInfo ver; // use the current running exe
// wcout << "ProductName= '" << ver.getProductName() << "'" << endl;
class VersionInfo {
private:
	BYTE* pVI;
  wstring filename;

	wstring getStringFileInfo(const wstring& label) const {
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
	VersionInfo(const wstring& fname = L"") {
    filename = fname; // keep our own copy

    // if we don't get a filename passed in, use the filename of the running exe
    if (filename.empty()) {
      filename.reserve(MAX_PATH);
      GetModuleFileNameW(0, &filename[0], MAX_PATH);
    }

    // allocate the memory
		DWORD useless;
		auto size = GetFileVersionInfoSizeExW(FILE_VER_GET_LOCALISED, filename.c_str(), &useless);
		if (size == 0) {
			wostringstream err;
			err << L"GetFileVersionInfoSizeExW failed. fileName= '" << filename << "'. " << WindowsUtility::getLastErrorMessage();
			throw err.str();
		}

    // get the data
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

  wstring getFilename() const { return filename; }
	wstring getComments() const { return getStringFileInfo(L"Comments"); }
	wstring getCompanyName() const { return getStringFileInfo(L"CompanyName"); }
	wstring getFileDescription() const { return getStringFileInfo(L"FileDescription"); }
	wstring getFileVersion() const { return getStringFileInfo(L"FileVersion"); }
	wstring getInternalName() const { return getStringFileInfo(L"InternalName"); }
	wstring getLegalCopyright() const { return getStringFileInfo(L"LegalCopyright"); }
	wstring getLegalTrademarks() const { return getStringFileInfo(L"LegalTrademarks"); }
	wstring getOriginalFilename() const { return getStringFileInfo(L"OriginalFilename"); }
	wstring getPrivateBuild() const { return getStringFileInfo(L"PrivateBuild"); }
	wstring getProductName() const { return getStringFileInfo(L"ProductName"); }
	wstring getProductVersion() const { return getStringFileInfo(L"ProductVersion"); }
	wstring getSpecialBuild() const { return getStringFileInfo(L"SpecialBuild"); }
};
