// AveShellBrowser.h : Declaration of the CAveShellBrowser

#pragma once
#include "resource.h"       // main symbols

#include "MultiDesktop.h"

#include <map>
#include <vector>

#include "pidl.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CAveShellBrowser

class ATL_NO_VTABLE CAveShellBrowser :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CAveShellBrowser, &CLSID_AveShellBrowser>,
	public IAveShellBrowser,
	public IShellBrowser,
	public ICommDlgBrowser,
	public IServiceProvider
{
	std::map<CString, CPoint> iconPositions;
public:
	CString guidAsString;

	CPidl pidl;
	CPidl browseToPidl;
	CComPtr<IShellView> view;
	HWND hwnd;
	HWND defView;
	BOOL hasSubclassed;
	BOOL hasSubclassedListView;
	BOOL isVisible;
	BOOL doInternalBrowsing;

	std::vector<CPidl> visitedPidls;
	size_t currentVisitedPidl;

	BOOL isChangingStyles;

	FOLDERSETTINGS folderSettings;

	UINT notifyOfActivationMsg;

	BOOL isInDebugHost;

	BOOL isKilled;

	BOOL isVista;

	CString wallpaper;

	HRESULT InternalSubclassDefView();
	HRESULT InternalSubclassListView();
	LRESULT OnListViewCustomDraw(LPNMLVCUSTOMDRAW customDraw, BOOL& handled);
	LRESULT OnListViewInsertItem(NMLISTVIEW* insertItem, BOOL& handled);

	static LRESULT CALLBACK DefViewSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR dwData);
	static LRESULT CALLBACK ListViewSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR dwData);

	CAveShellBrowser()
	{
		currentVisitedPidl = 0;
		WCHAR path[MAX_PATH] = {0};
		GetModuleFileName(NULL, path, MAX_PATH);
		const WCHAR* fileName = PathFindFileName(path);
		isInDebugHost = _wcsicmp(fileName, L"AveEffectDebugHost.exe") == 0 ||  _wcsicmp(fileName, L"AveDebugHost.exe") == 0;

		notifyOfActivationMsg = RegisterWindowMessage(L"AveNotifyOfActivation");

		hwnd = NULL;
		defView = NULL;
		hasSubclassed = FALSE;
		hasSubclassedListView = FALSE;
		isChangingStyles = FALSE;

		hotkeyVK = 0;
		hotkeyModifiers = 0;
		hotkeyAtom = NULL;
		isKilled = FALSE;
		isVisible = FALSE;
		doInternalBrowsing = FALSE;

		OSVERSIONINFO info = {0};
		info.dwOSVersionInfoSize = sizeof(info);
		GetVersionEx(&info);
		isVista = info.dwMajorVersion >= 6;

		folderSettings.ViewMode = FVM_ICON;
		folderSettings.fFlags = FWF_NOCOLUMNHEADER | FWF_TRANSPARENT;

		if(!isVista)
			folderSettings.fFlags = FWF_NOWEBVIEW;
	}

	// IServiceProvider
	virtual /* [local] */ HRESULT STDMETHODCALLTYPE QueryService( 
                /* [in] */ REFGUID guidService,
                /* [in] */ REFIID riid,
                /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		if(!doInternalBrowsing)
			return E_FAIL;

		*ppvObject = NULL;
		if(riid == IID_IShellBrowser)
		{
			AddRef();
			*ppvObject = (IShellBrowser*)this;
			return S_OK;
		}

		return E_FAIL;
	}

	UINT hotkeyVK;
	UINT hotkeyModifiers;
	ATOM hotkeyAtom;

	void InitGUID()
	{
		GUID guid = {0};
		CoCreateGuid(&guid);
		LPOLESTR str = NULL;
		StringFromCLSID(guid, &str);
		guidAsString = str;
		CoTaskMemFree(str);
	}

	void RegisterHotkey(HWND hwnd)
	{
		if(NULL == hotkeyAtom)
			hotkeyAtom  = GlobalAddAtom(guidAsString);

		RegisterHotKey(hwnd, hotkeyAtom, hotkeyModifiers, hotkeyVK);
	}

	void UnregisterHotkey(HWND hwnd)
	{
		UnregisterHotKey(hwnd, hotkeyAtom);
	}

	CString GetRegKeyName()
	{
		CString regKey = L"SOFTWARE\\AveSoftware\\MultiDesktop\\Items\\";
		regKey += guidAsString;

		return regKey;
	}

	void LoadFromSettings(CString guid, HWND parent);

DECLARE_REGISTRY_RESOURCEID(IDR_AVESHELLBROWSER)


BEGIN_COM_MAP(CAveShellBrowser)
	COM_INTERFACE_ENTRY(IAveShellBrowser)
	COM_INTERFACE_ENTRY(IShellBrowser)
	//COM_INTERFACE_ENTRY(ICommDlgBrowser)
	COM_INTERFACE_ENTRY(IOleWindow)
	COM_INTERFACE_ENTRY(IServiceProvider)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}


public:

	void Make(HWND parent);
	void Show();
	void Hide();
	void Kill();
	void RemoveSubclassing();
	void KillView(CComPtr<IShellView>& view);
	void ReadRegSettings();

	void SetWallpaperToListView(int operation);

	void _Make(HWND parent, LPITEMIDLIST pidl);
	CComPtr<IStream> _GetSettingsStream(const WCHAR* name, DWORD grfMode);
	HRESULT _SaveIconPosToStream(CComPtr<IStream> stream);
	HRESULT _ReadAndSetIconPos();


	  STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode){return E_NOTIMPL;}	
	// *** ICommDlgBrowser methods ***
    STDMETHOD(OnDefaultCommand) (THIS_ struct IShellView * ppshv)
	{	//handle double click and ENTER key if needed
		return E_NOTIMPL; 
	}
    STDMETHOD(OnStateChange) (THIS_ struct IShellView * ppshv,ULONG uChange)
	{	//handle selection, rename, focus if needed
		return S_OK; 
	}
    STDMETHOD(IncludeObject) (THIS_ struct IShellView * ppshv,LPCITEMIDLIST pidl)
	{	//filter files if needed
		return S_OK;
	}
    // *** IShellBrowser methods *** (same as IOleInPlaceFrame)
    STDMETHOD(InsertMenusSB) (HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths) {return E_NOTIMPL;}
    STDMETHOD(SetMenuSB) (HMENU hmenuShared, HOLEMENU holemenuReserved,HWND hwndActiveObject){return E_NOTIMPL;}
    STDMETHOD(RemoveMenusSB) (HMENU hmenuShared){return E_NOTIMPL;}
    STDMETHOD(SetStatusTextSB) (LPCOLESTR lpszStatusText){return E_NOTIMPL;}
    STDMETHOD(EnableModelessSB) (BOOL fEnable){return E_NOTIMPL;}
	STDMETHOD(BrowseObject)(LPCITEMIDLIST pidl, UINT wFlags)
	{
		if(!doInternalBrowsing)
			return E_NOTIMPL;

		LPITEMIDLIST browseTo = ILCloneFull(pidl);
		CPidl tmpPidl;
		tmpPidl.SetFromPIDL(browseTo);
		for(size_t i = currentVisitedPidl+1; i < visitedPidls.size(); ++i)
		{
			visitedPidls.erase(visitedPidls.begin() + i);
			i--;
		}

		visitedPidls.push_back(tmpPidl);
		currentVisitedPidl++;

		HRESULT hRes = InternalBrowseObject(pidl, wFlags);
		

		return hRes;
	}

	HRESULT InternalBrowseObject(LPCITEMIDLIST pidl, UINT wFlags)
	{
		LPITEMIDLIST browseTo = ILCloneFull(pidl);
		browseToPidl.SetFromPIDL(browseTo);
		CComPtr<IShellView> oldView = view;
		SaveState();
		view.Release();
		RemoveSubclassing();

		HWND progman = FindWindow(L"progman", isInDebugHost ? L"FakeDesktop" : NULL);

		Make(progman);
		Show();
		KillView(oldView);
		oldView.Release();

		if(defView)
		{
			HWND lv = FindWindowEx(defView, NULL, L"SysListView32", NULL);
			if(lv)
			{
				::SetFocus(lv);
			}

			if(view)
				view->UIActivate(SVUIA_INPLACEACTIVATE);
		}

		return S_OK;
	}
	
	HRESULT GoForward()
	{
		if(currentVisitedPidl >= visitedPidls.size()-1)
			return E_FAIL;

		currentVisitedPidl++;
		HRESULT hRes = InternalBrowseObject(visitedPidls[currentVisitedPidl], 0);
		
		return hRes;
	}

	HRESULT GoBack()
	{
		if(currentVisitedPidl == 0)
			return E_FAIL;

		currentVisitedPidl--;
		HRESULT hRes = InternalBrowseObject(visitedPidls[currentVisitedPidl], 0);

		return hRes;
	}

	STDMETHOD(GetViewStateStream)(DWORD grfMode,LPSTREAM  *ppStrm)
	{
		if(NULL == ppStrm)
			return E_INVALIDARG;
		
		*ppStrm = _GetSettingsStream(L"viewState", grfMode);
		return *ppStrm ? S_OK : E_FAIL;
	}

	STDMETHOD(OnViewWindowActive)(struct IShellView *ppshv){return E_NOTIMPL;}
	STDMETHOD(SetToolbarItems)(LPTBBUTTON lpButtons, UINT nButtons,UINT uFlags){return E_NOTIMPL;}
	STDMETHOD(TranslateAcceleratorSB) (LPMSG lpmsg, WORD wID)
	{

		return E_NOTIMPL; 
	}

	//IOleWindow
	STDMETHOD(GetWindow)(HWND * lphwnd)
	{ 
		*lphwnd = hwnd; 
		return S_OK; 
	}
	//IShellBrowser
	STDMETHOD(QueryActiveShellView)(struct IShellView ** ppshv)
	{
		view.CopyTo(ppshv);
		//*ppshv = view;
		return S_OK; 
	}
	STDMETHOD(GetControlWindow)(UINT id, HWND * lphwnd)
	{
		if(lphwnd == NULL)
			return E_POINTER;
		return E_NOTIMPL;
	}

	STDMETHOD(SendControlMsg)(UINT id, UINT uMsg, WPARAM wParam,LPARAM lParam, LRESULT *pret)
	{
		if(pret == NULL)
			return E_POINTER;
	
		return E_NOTIMPL;
	}

	STDMETHOD(OnMessage)(MSG* pMsg);
	STDMETHOD(Stop)(void);
	STDMETHOD(SaveState)(void);
	STDMETHOD(GetShellViewWindow)(HWND* pHwnd);
};

OBJECT_ENTRY_AUTO(__uuidof(AveShellBrowser), CAveShellBrowser)
