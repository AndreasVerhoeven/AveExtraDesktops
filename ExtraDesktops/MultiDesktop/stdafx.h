// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0501 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit


#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <atlapp.h>
#include <atlmisc.h>
#include <atlctrls.h>
#include <atldlgs.h>

#include <shlobj.h>

#pragma comment(lib, "shell32.lib")


#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

#include <math.h>

using namespace ATL;

#include "AveRegFuncs.h"

BOOL inline ShouldNotRunInExplorer()
{
	return AveRegFuncs::ReadInt(HKEY_CURRENT_USER, L"Software\\AveSoftware\\MultiDesktop", L"DoNotLoadInExplorer", 0) != 0;
}

BOOL inline IsRunningInExplorer()
{
	WCHAR path[MAX_PATH] = {0};
	GetModuleFileName(NULL, path, MAX_PATH);
	const WCHAR* fileName = PathFindFileName(path);
	return _wcsicmp(fileName, L"explorer.exe") == 0 || _wcsicmp(fileName, L"verclsid.exe") == 0;
}

BOOL inline IsRunningInDebugHost()
{
	WCHAR path[MAX_PATH] = {0};
	GetModuleFileName(NULL, path, MAX_PATH);
	const WCHAR* fileName = PathFindFileName(path);
	return _wcsicmp(fileName, L"AveEffectDebugHost.exe") == 0 || _wcsicmp(fileName, L"AveDebugHost.exe") == 0;
}

#define USE_TIMEBOMB		FALSE
#define TIMEBOMB_DATE_YEAR	2020
#define TIMEBOMB_DATE_MONTH	9
#define TIMEBOMB_DATE_DAY	28
inline __forceinline BOOL CheckTimeBomb()
{
	if(!USE_TIMEBOMB)
		return TRUE;

	SYSTEMTIME st = {0};
	SYSTEMTIME bomb = {0};
	bomb.wYear = TIMEBOMB_DATE_YEAR;
	bomb.wMonth = TIMEBOMB_DATE_MONTH;
	bomb.wDay = TIMEBOMB_DATE_DAY;
	FILETIME now = {0}, bombTime = {0};
	GetSystemTime(&st);
	SystemTimeToFileTime(&st, &now);
	SystemTimeToFileTime(&bomb, &bombTime);

	return CompareFileTime(&now, &bombTime) <= 0;
}