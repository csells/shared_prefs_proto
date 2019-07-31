#pragma once
#include "pch.h"
#include <sstream>
#include <iomanip>
#include <regex>

using namespace std;

class WindowsUtility {
public:
	static wstring getLastErrorMessage() {
		DWORD dw = GetLastError();
		wchar_t* msgBuf;
		FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msgBuf, 0, 0);
		wostringstream oss;
		oss << msgBuf << L"(0x" << setw(4) << setfill(L'0') << setbase(16) << dw << L")";
		LocalFree(msgBuf);
		return oss.str();
	}

	// from https://stackoverflow.com/questions/1976007/what-characters-are-forbidden-in-windows-and-linux-directory-names
	static wstring getSimpleFilename(const wchar_t* baseFilenameNoExt) {
		// keep just these characters which we know are simple and safe: 'A-Za-z0-9-_ '
		wstring filename = regex_replace(baseFilenameNoExt, wregex(L"[^A-Za-z0-9-_ ]"), L"");

		// muck with file names which we know are illegal
		const vector<wstring> badFilenames = { L"CON", L"PRN", L"AUX", L"NUL", L"COM1", L"COM2", L"COM3", L"COM4", L"COM5", L"COM6", L"COM7", L"COM8", L"COM9", L"LPT1", L"LPT2", L"LPT3", L"LPT4", L"LPT5", L"LPT6", L"LPT7", L"LPT8", L"LPT9", };
		for (auto pbadfn = badFilenames.begin(); pbadfn != badFilenames.end(); ++pbadfn) {
			if (filename.compare(*pbadfn) == 0) filename.insert(filename.begin(), L'_');
		}

		// trim trailing spaces and dots
		for (auto pch = filename.rbegin(); pch != filename.rend(); ++pch) {
			if ((*pch == L' ') || (*pch == L'.')) *pch = 0;
		}

		return filename;
	}
};

// from https://www.codeguru.com/cpp/w-p/win32/versioning/article.php/c4539/Versioning-in-Windows.htm
// e.g.
// auto filename = L"C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\Common7\\IDE\\devenv.exe";
// VersionInfo ver(filename);
// wcout << "ProductName= '" << ver.getProductName() << "'" << endl;
// wcout << "CompanyName= '" << ver.getCompanyName() << "'" << endl;
// wcout << "Comments= '" << ver.getComments() << "'" << endl;
// wcout << "FileDescription= '" << ver.getFileDescription() << "'" << endl;
// wcout << "FileVersion= '" << ver.getFileVersion() << "'" << endl;
// wcout << "InternalName= '" << ver.getInternalName() << "'" << endl;
// wcout << "LegalCopyright= '" << ver.getLegalCopyright() << "'" << endl;
// wcout << "LegalTrademarks= '" << ver.getLegalTrademarks() << "'" << endl;
// wcout << "OriginalFilename= '" << ver.getOriginalFilename() << "'" << endl;
// wcout << "PrivateBuild= '" << ver.getPrivateBuild() << "'" << endl;
// wcout << "ProductVersion= '" << ver.getProductVersion() << "'" << endl;
// wcout << "ProductSpecialBuild= '" << ver.getSpecialBuild() << "'" << endl;
class VersionInfo {
private:
	BYTE* pVI;

	wstring getStringFileInfo(const wchar_t* label) {
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
	VersionInfo(const wchar_t* filename) {
		DWORD useless;
		auto size = GetFileVersionInfoSizeExW(FILE_VER_GET_LOCALISED, filename, &useless);
		if (size == 0) {
			wostringstream err;
			err << L"GetFileVersionInfoSizeExW failed. fileName= '" << filename << "'. " << WindowsUtility::getLastErrorMessage();
			throw err.str();
		}

		pVI = new BYTE[size];
		auto success = GetFileVersionInfoExW(FILE_VER_GET_LOCALISED, filename, 0, size, pVI);
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
