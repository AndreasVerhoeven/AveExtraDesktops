// AveMultiDesktopHost.cpp : Implementation of CAveMultiDesktopHost

#include "stdafx.h"
#include "AveMultiDesktopHost.h"

#include "AveShellBrowser.h"

HHOOK CAveMultiDesktopHost::hookHandle = NULL;

HRESULT CAveMultiDesktopHost::InternalSubclassProgman()
{
	HWND progman = FindWindow(L"progman", NULL);
	if(progman != NULL)
	{
		SetProp(progman, L"AveMultiDesktopPtr", (HANDLE)this);
		hasSubclassed = SetWindowSubclass(progman, ProgmanSubclassProc, 1, (DWORD_PTR)this);
		InternalHookMsgLoop();
	}

	return S_OK;
}

HRESULT CAveMultiDesktopHost::InternalHookMsgLoop()
{
	hookHandle = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, NULL, GetCurrentThreadId()); 
	return S_OK;
}

LRESULT CALLBACK CAveMultiDesktopHost::GetMsgProc(int code, WPARAM wParam, LPARAM lParam)
{
	if(HC_ACTION == code)
	{
		CAveMultiDesktopHost* host = reinterpret_cast<CAveMultiDesktopHost*>(GetProp(FindWindow(L"progman", NULL), L"AveMultiDesktopPtr"));	
		if(host != NULL)
		{
			CComPtr<IAveShellBrowser> currentBrowser = host->GetCurrentBrowser();
			if(currentBrowser != NULL)
			{
				MSG* msg = reinterpret_cast<MSG*>(lParam);
				HRESULT hRes = currentBrowser->OnMessage(msg);
				if(S_OK == hRes)
				{
					msg->message = WM_NULL;
				}
			}
		}
	}

	return CallNextHookEx(hookHandle, code, wParam, lParam);
}

void CAveMultiDesktopHost::ShowDesktop(AveDesktop& desktop)
{
	if(NULL == desktop.object->defView)
		desktop.object->Make(FindWindow(L"progman", NULL));

	desktop.object->Show();
}

void CAveMultiDesktopHost::HideDesktop(AveDesktop& desktop)
{
	desktop.object->Hide();

	CheckZOrderAndFix();
}

void CAveMultiDesktopHost::CheckZOrderAndFix()
{
	HWND topWindow = GetTopWindow(FindWindow(L"progman", NULL));
	if(NULL == topWindow)
	{
		curDesktop = -1;
		return;
	}

	while(!IsWindowVisible(topWindow) && (topWindow = GetNextWindow(topWindow, GW_HWNDNEXT)) != NULL);

	if(NULL == topWindow) 
	{
		curDesktop = -1;
		return;
	}

	for(size_t i = 0; i < desktops.size(); ++i)
	{
		AveDesktop& desktop = desktops[i];
		if(desktop.object->defView == topWindow)
		{
			curDesktop = (int)i;
			return;
		}
	}
	
	curDesktop = -1;
}

LRESULT CALLBACK CAveMultiDesktopHost::ProgmanSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR dwData)
{
	if(dwData != NULL)
	{
		CAveMultiDesktopHost* host = reinterpret_cast<CAveMultiDesktopHost*>(dwData);

		// undocumented message used by some ShellViews
		if(WM_USER + 7 == uMsg)
		{
			CComPtr<IAveShellBrowser> currentBrowser = host->GetCurrentBrowser();
			return (LRESULT)(IShellBrowser*)currentBrowser.p;
		}
		else if(WM_HOTKEY == uMsg)
		{
			int index =0;
			int idHotKey = (int) wParam; 
			for(DesktopList::iterator iter = host->desktops.begin(); iter != host->desktops.end(); ++iter, ++index)
			{
				AveDesktop& desktop = *iter;
				if(desktop.object->hotkeyAtom == idHotKey)
				{
					if(NULL == desktop.object->defView || !IsWindowVisible(desktop.object->defView) ||
						index != host->curDesktop)
					{
						host->ShowDesktop(desktop);
					}
					else
					{
						host->HideDesktop(desktop);
					}
				}
			}
		}
		else if(host->notifyOfActivationMsg == uMsg)
		{
			HWND defView = (HWND)lParam;
			for(size_t i = 0; i < host->desktops.size(); ++i)
			{
				if(host->desktops[i].object->defView == defView)
				{
					host->curDesktop = static_cast<int>(i);
					break;
				}
			}
			return 0;
		}
		else if(WM_SETTINGCHANGE == uMsg)
		{
			if(SPI_SETDESKWALLPAPER == wParam)
			{
				PostMessage(hwnd, host->notifyOfWallpaperReload, 1, 0);
			}
		}
		else if(WM_SYSCOLORCHANGE == uMsg)
		{
			PostMessage(hwnd, host->notifyOfWallpaperReload, 2, 0);
		}

		else if(host->notifyOfWallpaperReload == uMsg)
		{
			for(DesktopList::iterator iter = host->desktops.begin(); iter != host->desktops.end(); ++iter)
			{
				AveDesktop& desktop = *iter;
				desktop.object->SetWallpaperToListView((int)wParam);
			}
		}

		// For accelerator messages
		CComPtr<IAveShellBrowser> currentBrowser = host->GetCurrentBrowser();
		if(currentBrowser != NULL)
		{
			MSG msg = {hwnd, uMsg, wParam, lParam, GetTickCount(), GetMessagePos()};
			HRESULT hRes = currentBrowser->OnMessage(&msg);
			if(S_OK == hRes)
				return hRes;
		}
	}

	return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}


void CAveMultiDesktopHost::SaveAll()
{
	for(size_t i = 0; i < desktops.size(); ++i)
	{
		desktops[i].object->SaveState();
	}
}

void CAveMultiDesktopHost::LoadAll()
{
	const WCHAR* totalKeyName = L"SOFTWARE\\AveSoftware\\MultiDesktop\\Items";

	HKEY key = NULL;
	LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, totalKeyName, 0L, KEY_ENUMERATE_SUB_KEYS,  &key);
	if(key != NULL)
	{
		DWORD index = 0;
		do
		{
			WCHAR subKeyName[255] = {0};
			DWORD subKeyBufferLength = sizeof(subKeyName) / sizeof(subKeyName[0]);
			result = RegEnumKeyEx(key, index, subKeyName, &subKeyBufferLength, 0, NULL, NULL, NULL);
			index++;
			if(result != ERROR_NO_MORE_ITEMS)
			{
				LoadItem(subKeyName);
			}

		}while(result != ERROR_NO_MORE_ITEMS);

		RegCloseKey(key);
	}
}

void CAveMultiDesktopHost::LoadItem(CString guid)
{
	if(!hasSubclassed)
		InternalSubclassProgman();

	CComObject<CAveShellBrowser>* browser = NULL;
	CComObject<CAveShellBrowser>::CreateInstance(&browser);
	if(browser != NULL)
	{
		HWND hwnd = FindWindow(L"progman", NULL);

		AveDesktop desktop;
		desktop.object = browser;
		browser->QueryInterface(IID_IAveShellBrowser, (void**)&desktop.browser);
		size_t index = desktops.size();
		desktops.push_back(desktop);

		browser->LoadFromSettings(guid, FindWindow(L"progman", NULL));
		//browser->Make(hwnd);
		//browser->Show();
		//browser->SaveState();		
	}
}

int CAveMultiDesktopHost::InternalNewItem(CPidl pidl)
{
	if(!hasSubclassed)
		InternalSubclassProgman();

	CComObject<CAveShellBrowser>* browser = NULL;
	CComObject<CAveShellBrowser>::CreateInstance(&browser);
	if(browser != NULL)
	{
		HWND hwnd = FindWindow(L"progman", NULL);

		AveDesktop desktop;
		desktop.object = browser;
		browser->QueryInterface(IID_IAveShellBrowser, (void**)&desktop.browser);
		size_t index = desktops.size();
		desktops.push_back(desktop);

		browser->hotkeyVK = 0;
		browser->InitGUID();
		browser->pidl = pidl;
		//browser->Make(hwnd);
		//browser->Show();
		browser->SaveState();

		return (int)index;
		
	}
	return -1;
}

HRESULT CAveMultiDesktopHost::InternalCreateNewDesktop()
{
	HRESULT hRes = S_OK;

	if(!hasSubclassed)
		InternalSubclassProgman();

	CComObject<CAveShellBrowser>* browser = NULL;
	CComObject<CAveShellBrowser>::CreateInstance(&browser);
	if(browser != NULL)
	{
		HWND hwnd = FindWindow(L"progman", NULL);

		AveDesktop desktop;
		desktop.object = browser;
		browser->QueryInterface(IID_IAveShellBrowser, (void**)&desktop.browser);
		size_t index = desktops.size();
		desktops.push_back(desktop);
		//curDesktop = static_cast<int>(index);

		browser->hotkeyVK = VK_F11;
		browser->InitGUID();
		browser->RegisterHotkey(hwnd);
		browser->pidl.SetFromSpecialFolderLocation(CSIDL_MYDOCUMENTS);
		browser->Make(hwnd);
		//browser->Show();
		browser->SaveState();
		
	}

	return hRes;
}

// CAveMultiDesktopHost

HRESULT CAveMultiDesktopHost::InternalStart()
{
	if(isRunning)
		return S_OK;

	HRESULT hRes = S_OK;

	// we addref ourself, so we won't get unloaded
	AddRef();

	isRunning = TRUE;

	if(!hasSubclassed)
		InternalSubclassProgman();

	LoadAll();

	return hRes;
}

HRESULT CAveMultiDesktopHost::InternalStop()
{
	HRESULT hRes = S_OK;

	isRunning = FALSE;

	SaveAll();
	
	for(DesktopList::iterator iter = desktops.begin(); iter != desktops.end(); ++iter)
	{
		AveDesktop& desktop = *iter;
		desktop.browser->Stop();
		desktop.browser.Release();
	}

	desktops.clear();

	return hRes;
}

STDMETHODIMP CAveMultiDesktopHost::Initialize(LPCITEMIDLIST pidlFolder,LPDATAOBJECT pDataObj,HKEY hProgID)
{
	//if(!hasInitialized)
		//InternalInitialize(g_NotificationWindow);

	if(!isRunning)
		InternalStart();

	if(NULL == pidlFolder)
		return E_INVALIDARG;

	if(0 == pidlFolder->mkid.cb)
		return S_OK;

	return E_INVALIDARG;
}

HRESULT CAveMultiDesktopHost::QueryContextMenu(HMENU hmenu, UINT uMenuIndex, UINT uidFirstCmd,UINT uidLastCmd, UINT uFlags )
{
  // our item is not the default one, thus do not display when only the default item is requested.
  if(uFlags & CMF_DEFAULTONLY)
    return MAKE_HRESULT ( SEVERITY_SUCCESS, FACILITY_NULL, 0 );
 
	CString configDlgMenuText;
	configDlgMenuText.LoadString(IDS_CONFIGMENUITEMTEXT);
	InsertMenu ( hmenu, uMenuIndex, MF_BYPOSITION, uidFirstCmd, configDlgMenuText);
 

 
  return MAKE_HRESULT ( SEVERITY_SUCCESS, FACILITY_NULL, 2 );
}


HRESULT CAveMultiDesktopHost::GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT* pwReserved,LPSTR pszName, UINT cchMax)
{
	return E_NOTIMPL;
}

HRESULT CAveMultiDesktopHost::InvokeCommand(LPCMINVOKECOMMANDINFO pCmdInfo)
{
// If lpVerb really points to a string, ignore this function call and bail out.
	if(0 != HIWORD(pCmdInfo->lpVerb))
		return E_INVALIDARG;
 
	switch(LOWORD(pCmdInfo->lpVerb))
	{
		case 0:
			if(NULL == dlg.m_hWnd)
			{
				dlg.Create(NULL);
				dlg.ShowWindow(SW_SHOW);
			}
			else
			{
				if(dlg.IsIconic())
					dlg.ShowWindow(SW_RESTORE);
				dlg.BringWindowToTop();
			}

			/*if(isRunning)
			{
				InternalStop();
			}
			else
			{
				InternalStart();
			}*/
			
			return S_OK;

		default:
			return E_INVALIDARG;
	}
}
