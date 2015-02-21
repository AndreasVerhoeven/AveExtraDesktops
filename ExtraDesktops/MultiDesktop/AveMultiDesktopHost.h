// AveMultiDesktopHost.h : Declaration of the CAveMultiDesktopHost

#pragma once
#include "resource.h"       // main symbols

#include "MultiDesktop.h"

#include "ConfigureDlg.h"

#include "AveShellBrowser.h"

#include "Pidl.h"


struct AveDesktop
{
	CComObject<CAveShellBrowser>* object;
	CComPtr<IAveShellBrowser> browser;
};


#include <vector>

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CAveMultiDesktopHost

class ATL_NO_VTABLE CAveMultiDesktopHost :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CAveMultiDesktopHost, &CLSID_AveMultiDesktopHost>,
	public IAveMultiDesktopHost,
	public IShellExtInit,
	public IContextMenu
{
public:

	BOOL isRunning;

	CConfigureDlg dlg;

	typedef std::vector<AveDesktop> DesktopList;
	DesktopList desktops;
	int curDesktop;

	UINT notifyOfActivationMsg;
	UINT notifyOfWallpaperReload;

	CAveMultiDesktopHost() :
		dlg(this)
	{
		notifyOfActivationMsg = RegisterWindowMessage(L"AveNotifyOfActivation");
		notifyOfWallpaperReload = RegisterWindowMessage(L"AveNotifyOfWallpaperReload");
		hasSubclassed	= FALSE;
		isRunning		= FALSE;
		curDesktop = -1;
	}

	CComPtr<IAveShellBrowser> GetCurrentBrowser()
	{
		if(curDesktop >= 0 && static_cast<size_t>(curDesktop) < desktops.size())
			return desktops[curDesktop].browser;
		else
			return NULL;
	}

	void ShowDesktop(AveDesktop& desktop);
	void HideDesktop(AveDesktop& desktop);
	void CheckZOrderAndFix();

	BOOL hasSubclassed;
	static HHOOK hookHandle;

DECLARE_REGISTRY_RESOURCEID(IDR_AVEMULTIDESKTOPHOST)


BEGIN_COM_MAP(CAveMultiDesktopHost)
	COM_INTERFACE_ENTRY(IAveMultiDesktopHost)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY(IContextMenu)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	HRESULT InternalStart();
	HRESULT InternalStop();

	HRESULT InternalCreateNewDesktop();

	HRESULT InternalSubclassProgman();
	HRESULT InternalHookMsgLoop();

	static LRESULT CALLBACK GetMsgProc(int code, WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK ProgmanSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR dwData);

	void SaveAll();
	void LoadItem(CString guid);
	void LoadAll();

public:

	int InternalNewItem(CPidl pidl);

	// IShellExtInit
	STDMETHODIMP Initialize(LPCITEMIDLIST, LPDATAOBJECT, HKEY);

	// IContextMenu
	STDMETHODIMP GetCommandString(UINT_PTR, UINT, UINT*, LPSTR, UINT);
	STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO);
	STDMETHODIMP QueryContextMenu(HMENU, UINT, UINT, UINT, UINT);

	DECLARE_CLASSFACTORY_SINGLETON(CAveMultiDesktopHost);
};

OBJECT_ENTRY_AUTO(__uuidof(AveMultiDesktopHost), CAveMultiDesktopHost)
DECLARE_CLASSFACTORY_SINGLETON(CAveMultiDesktopHost);
