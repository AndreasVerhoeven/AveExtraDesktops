// MultiDesktop.cpp : Implementation of DLL Exports.


#include "stdafx.h"
#include "resource.h"
#include "MultiDesktop.h"
#include "dlldatax.h"


class CMultiDesktopModule : public CAtlDllModuleT< CMultiDesktopModule >
{
public :
	DECLARE_LIBID(LIBID_MultiDesktopLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_MULTIDESKTOP, "{CE51223B-0E2C-4CE8-A59C-95F88C2EA3B6}")
};

CMultiDesktopModule _AtlModule;


#ifdef _MANAGED
#pragma managed(push, off)
#endif

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
#ifdef _MERGE_PROXYSTUB
    if (!PrxDllMain(hInstance, dwReason, lpReserved))
        return FALSE;
#endif
	hInstance;
    return _AtlModule.DllMain(dwReason, lpReserved); 
}

#ifdef _MANAGED
#pragma managed(pop)
#endif




// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
#ifdef _MERGE_PROXYSTUB
    HRESULT hr = PrxDllCanUnloadNow();
    if (hr != S_OK)
        return hr;
#endif
    return _AtlModule.DllCanUnloadNow();
}

BOOL isInitialized = FALSE;

// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	if(ppv != NULL)
		*ppv = NULL;

	if(!CheckTimeBomb())
		return E_FAIL;

	if((!IsRunningInExplorer() || ShouldNotRunInExplorer()) && !IsRunningInDebugHost())
		return E_FAIL;

	if(!isInitialized)
	{
		isInitialized = TRUE;
		CComPtr<IAveMultiDesktopHost> host;
		host.CoCreateInstance(CLSID_AveMultiDesktopHost);
		CComQIPtr<IShellExtInit> shellExtInit(host);
		if(shellExtInit != NULL)
			shellExtInit->Initialize(NULL, NULL, NULL);

	}

#ifdef _MERGE_PROXYSTUB
    if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
        return S_OK;
#endif
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib
    HRESULT hr = _AtlModule.DllRegisterServer();
#ifdef _MERGE_PROXYSTUB
    if (FAILED(hr))
        return hr;
    hr = PrxDllRegisterServer();
#endif
	return hr;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();
#ifdef _MERGE_PROXYSTUB
    if (FAILED(hr))
        return hr;
    hr = PrxDllRegisterServer();
    if (FAILED(hr))
        return hr;
    hr = PrxDllUnregisterServer();
#endif
	return hr;
}

