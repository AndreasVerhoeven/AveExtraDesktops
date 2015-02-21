// AveShellBrowser.cpp : Implementation of CAveShellBrowser

#include "stdafx.h"
#include "AveShellBrowser.h"


#include <vector>

// CAveShellBrowser

#include "AveRegFuncs.h"

#ifndef LVS_EX_TRANSPARENTSHADOWTEXT
#define LVS_EX_TRANSPARENTSHADOWTEXT 0x00800000
#endif

#ifndef LVS_EX_TRANSPARENTBKGND
#define LVS_EX_TRANSPARENTBKGND 0x00400000
#endif


BOOL CALLBACK GetMonitorsRectsMonitorEnumProc(
  HMONITOR hMonitor,  // handle to display monitor
  HDC hdcMonitor,     // handle to monitor DC
  LPRECT lprcMonitor, // monitor intersection rectangle
  LPARAM dwData       // data
)
{
	std::vector<CRect>* rects = (std::vector<CRect>*)dwData;
	if(rects != NULL && hMonitor != NULL)
	{
		MONITORINFO info = {0};
		info.cbSize = sizeof(info);
		GetMonitorInfo(hMonitor, &info);

		if(lprcMonitor->left == 0 && lprcMonitor->top == 0 )
		{
			// the default monitor always starts at (0,0)
			rects->insert(rects->begin(), info.rcWork);
		}
		else
		{
			rects->push_back(info.rcWork);
		}
	}

	return TRUE;
}

std::vector<CRect> GetMonitorsRects()
{
	std::vector<CRect> rects;
	EnumDisplayMonitors(NULL, NULL, GetMonitorsRectsMonitorEnumProc, (LPARAM)&rects);

	return rects;
}



HRESULT CAveShellBrowser::InternalSubclassDefView()
{
	WCHAR className[100] = {0};
	GetClassName(defView, className, 100);
	if(defView != NULL)
	{
		hasSubclassed = SetWindowSubclass(defView, DefViewSubclassProc, 1, (DWORD_PTR)this);
	}

	return S_OK;
}

HRESULT CAveShellBrowser::InternalSubclassListView()
{
	HWND lv = FindWindowEx(defView, NULL, L"SysListView32", NULL);

	WCHAR className[100] = {0};
	GetClassName(lv, className, 100);
	if(lv != NULL)
	{
		hasSubclassedListView = SetWindowSubclass(lv, ListViewSubclassProc, 1, (DWORD_PTR)this);
	}

	return S_OK;
}

LRESULT CAveShellBrowser::OnListViewCustomDraw(LPNMLVCUSTOMDRAW customDraw, BOOL& handled)
{
	if(NULL == customDraw)
		return 0;

	DWORD dwStage = customDraw->nmcd.dwDrawStage;
	
	if(CDDS_PREPAINT == dwStage)
	{
		handled = TRUE;
		return CDRF_NOTIFYITEMDRAW | CDRF_NEWFONT | CDRF_DOERASE | CDRF_NOTIFYPOSTPAINT;
	}
	else if(CDDS_ITEMPREPAINT == dwStage)
	{
		bool isSelected = (customDraw->nmcd.uItemState & CDIS_FOCUS) || (customDraw->nmcd.uItemState & CDIS_SELECTED);
		customDraw->clrText = GetSysColor(isSelected ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT);

		handled = TRUE;
		return CDRF_NOTIFYITEMDRAW | CDRF_NEWFONT | CDRF_DOERASE | CDRF_NOTIFYPOSTPAINT;
	}
	else if(CDDS_ITEMPOSTPAINT == dwStage)
	{
		return 0;
		/*
		CRect textRect;
		CString text;
		CListViewCtrl lv;
		lv.Attach(customDraw->nmcd.hdr.hwndFrom);
		lv.GetItemRect((int)customDraw->nmcd.dwItemSpec, &textRect, LVIR_LABEL);
		lv.GetItemText((int)customDraw->nmcd.dwItemSpec, 0, text);
		int selIndex = lv.GetSelectedIndex();
		bool selected = selIndex == customDraw->nmcd.dwItemSpec;
		CDCHandle dc = customDraw->nmcd.hdc;
		//dc.FillSolidRect(&textRect, RGB(255, 255, 255));
		DWORD flags = DT_NOPREFIX | DT_WORDBREAK | DT_LEFT;
		if(!selected)
			flags |= DT_END_ELLIPSIS;
		dc.DrawShadowText(text, -1, &textRect, flags, RGB(255,255,255), RGB(0,0,0), 1, 1);
		lv.Detach();
		handled = TRUE;
		return 0;
		*/
	}

	return 0;
}


LRESULT CALLBACK CAveShellBrowser::ListViewSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR dwData)
{
	if(dwData != NULL)
	{
		CAveShellBrowser* self = reinterpret_cast<CAveShellBrowser*>(dwData);
		if(WM_XBUTTONDOWN == msg)
		{
			int fwButton = GET_XBUTTON_WPARAM (wParam);
			if(MK_XBUTTON1 == fwButton)
			{
				self->GoBack();
				return 0;
			}
			else if(MK_XBUTTON2 == fwButton)
			{
				self->GoForward();
				return 0;
			}

			
		}

		else if(WM_CHAR == msg)
		{
			// XXX magic
			if(VK_BACK == wParam)
			{
				if(GetKeyState(VK_SHIFT) < 0)
					self->GoForward();
				else
					self->GoBack();
				return 0;
			}
		}

		else if(LVM_FIRST + 193 == msg)
		{
			LRESULT res = DefSubclassProc(hwnd, msg, wParam, lParam);

			CListViewCtrl lv;
			lv.Attach(hwnd);

			int count = lv.GetItemCount();
			if(count > 0 && self->iconPositions.size() > 0)
			{
				for(int index = 0; index < count; ++index)
				{
					CString text;
					lv.GetItemText(index, 0, text);
					if(self->iconPositions.find(text) != self->iconPositions.end())
					{
						lv.SetItemPosition(index, self->iconPositions[text]);
						self->iconPositions.erase(text);
					}
				}
			}

			if(!(lv.GetStyle() & LVS_NOCOLUMNHEADER))
				lv.ModifyStyle(0, LVS_NOCOLUMNHEADER);

			lv.Detach();

			return res;
		}
		else if(LVM_SETITEMTEXT == msg)
		{
			LRESULT res = DefSubclassProc(hwnd, msg, wParam, lParam);

			int index = (int)wParam;

			CListViewCtrl lv;
			lv.Attach(hwnd);

			CString text;
			lv.GetItemText(index, 0, text);
			if(self->iconPositions.find(text) != self->iconPositions.end())
			{
				lv.SetItemPosition(index, self->iconPositions[text]);
				self->iconPositions.erase(text);
			}

			lv.Detach();

			return res;

		}
		if(LVM_SETITEM == msg)
		{
			LVITEM* item = (LVITEM*) lParam;
			LRESULT res = DefSubclassProc(hwnd, msg, wParam, lParam);

			if(NULL == item)
				return res;

			int index = item->iItem;

			CListViewCtrl lv;
			lv.Attach(hwnd);

			CString text;
			lv.GetItemText(index, 0, text);
			if(self->iconPositions.find(text) != self->iconPositions.end())
			{
				lv.SetItemPosition(index, self->iconPositions[text]);
				self->iconPositions.erase(text);
			}

			lv.Detach();


			return res;
		}
		/*else if(WM_RBUTTONDOWN == msg)
		{
			DWORD tick = GetTickCount();
			CRect rc;
			::GetClientRect(hwnd, &rc);
			CDC dc;
			dc.CreateCompatibleDC(NULL);
			CBitmap bmp;
			bmp.CreateCompatibleBitmap(dc, rc.Width(), rc.Height());
			dc.SelectBitmap(bmp);
			DWORD elapsedCreating = GetTickCount() - tick;
			PrintWindow(hwnd, dc, 0);

			DWORD elapsed = GetTickCount() - tick;
			if(NULL == dc)
				dc = NULL;
		}*/
	}
	return DefSubclassProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK CAveShellBrowser::DefViewSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR dwData)
{
	if(dwData != NULL)
	{
		CAveShellBrowser* self = reinterpret_cast<CAveShellBrowser*>(dwData);
		{
			if(WM_CHAR == msg)
			{
				// XXX magic
				if(VK_BACK == wParam)
				{
					self->GoBack();
					return 0;
				}
			}

			if(WM_CHILDACTIVATE == msg || 
				(WM_ACTIVATE == msg && (WA_ACTIVE == wParam || WA_CLICKACTIVE == wParam)))
			{
				HWND progman = FindWindow(L"progman", self->isInDebugHost ? L"FakeDesktop" : NULL);
				SendMessage(progman, self->notifyOfActivationMsg, 0L, (LPARAM)self->defView);
			}
			else if(WM_NOTIFY == msg)
			{
				LPNMHDR  notifyHeader = (LPNMHDR) lParam;
				if(NULL == notifyHeader)
					return 0;

				if(NM_CUSTOMDRAW == notifyHeader->code)
				{
					LPNMLVCUSTOMDRAW lpNMCustomDraw = (LPNMLVCUSTOMDRAW) lParam;

					
					BOOL handled = FALSE;
					LRESULT lRes = self->OnListViewCustomDraw(lpNMCustomDraw, handled);
					if(handled)
						return lRes;
				}
				else if(LVN_INSERTITEM  == notifyHeader->code)// ||	LVN_ITEMCHANGED == notifyHeader->code)
				{
					NMLISTVIEW* insertItem = (NMLISTVIEW*) lParam;

					BOOL handled = FALSE;
					LRESULT lRes = self->OnListViewInsertItem(insertItem, handled);
					if(handled)
						return lRes;
				}
			}
			else if(WM_DESTROY == msg || WM_QUIT == msg)
			{
				if(!self->isKilled)
					self->SaveState();
			}
		}

	}
	return DefSubclassProc(hwnd, msg, wParam, lParam);
}

LRESULT CAveShellBrowser::OnListViewInsertItem(NMLISTVIEW* insertItem, BOOL& handled)
{
	if(NULL == insertItem)
		return 0;
	
	CListViewCtrl lv;
	lv.Attach(insertItem->hdr.hwndFrom);

	CString text;
	lv.GetItemText(insertItem->iItem, 0, text);
	if(iconPositions.find(text) != iconPositions.end())
	{
		lv.SetItemPosition(insertItem->iItem, iconPositions[text]);
		iconPositions.erase(text);
	}

	lv.Detach();

	return 0;
}

void CAveShellBrowser::SetWallpaperToListView(int operation)
{
	// operation = 0 -> always set
	// operation = 1 -> system wallpaper
	// operation = 2 -> system color change

	if(NULL == defView)
		return;

	HWND lv = FindWindowEx(defView, NULL, L"SysListView32", NULL);
	if(NULL == lv)
		return;

	if(operation != 0)
	{
		if(isVista)
			return;

		CString userWallpaper = AveRegFuncs::ReadString(HKEY_CURRENT_USER, L"Control Panel\\Desktop", L"Wallpaper", L"");
		if(1 == operation && userWallpaper.GetLength() > 0)
		{
			WCHAR wallPath[MAX_PATH] = {0};
			wcscpy_s(wallPath, MAX_PATH, userWallpaper);

			LVBKIMAGE lvImg = {0};
			lvImg.ulFlags = LVBKIF_SOURCE_URL | LVBKIF_STYLE_TILE;
			lvImg.pszImage = wallPath;
			ListView_SetBkImage(lv, &lvImg);
		}
		else if(2 == operation && userWallpaper.GetLength() == 0)
		{
			ListView_SetBkColor(lv, GetSysColor(COLOR_DESKTOP));
		}

		return;
	}

	if(wallpaper.GetLength() > 0)
	{
		WCHAR wallPath[MAX_PATH] = {0};
		wcscpy_s(wallPath, MAX_PATH, wallpaper);
		LVBKIMAGE lvImg = {0};
		lvImg.ulFlags = LVBKIF_SOURCE_URL | LVBKIF_STYLE_TILE;
		lvImg.pszImage = wallPath;
		ListView_SetBkImage(lv, &lvImg);
	}
	else if(isVista)
	{
		LVBKIMAGE lvImg = {0};
		lvImg.ulFlags = LVBKIF_SOURCE_NONE;
		ListView_SetBkImage(lv, &lvImg);
	}
	else if(!isVista)
	{
		CString userWallpaper = AveRegFuncs::ReadString(HKEY_CURRENT_USER, L"Control Panel\\Desktop", L"Wallpaper", L"");
		if(userWallpaper.GetLength() == 0)
		{
			ListView_SetBkColor(lv, GetSysColor(COLOR_DESKTOP));
		}
		else
		{
			WCHAR wallPath[MAX_PATH] = {0};
			wcscpy_s(wallPath, MAX_PATH, userWallpaper);

			LVBKIMAGE lvImg = {0};
			lvImg.ulFlags = LVBKIF_SOURCE_URL | LVBKIF_STYLE_TILE;
			lvImg.pszImage = wallPath;
			ListView_SetBkImage(lv, &lvImg);
		}
	}
}

void CAveShellBrowser::Make(HWND parent)
{
	if(!CheckTimeBomb())
		return;

	if(visitedPidls.size() == 0)
		visitedPidls.push_back(pidl);

	isKilled = FALSE;

	hwnd = parent;
	HRESULT hRes = S_OK;
	CComPtr<IShellFolder> desktop;
	SHGetDesktopFolder(&desktop);

	CComPtr<IShellFolder> shellFolder;	
	if(browseToPidl != NULL)
		desktop->BindToObject(browseToPidl, NULL, IID_IShellFolder, (void**)&shellFolder);
	else
	{
		if(pidl != NULL)
			desktop->BindToObject(pidl, NULL, IID_IShellFolder, (void**)&shellFolder);
		else
			shellFolder = desktop;
	}

	if(NULL == shellFolder)
		shellFolder = desktop;
	
	if(NULL == shellFolder)
		return;

	hRes = shellFolder->CreateViewObject(NULL, IID_IShellView, (void**)&view);
	if(NULL == view)
		return;

	CRect rc;
	::GetClientRect(parent, &rc);

	CRect rcFinal;
	std::vector<CRect> rects = GetMonitorsRects();
	for(size_t i = 0; i < rects.size(); ++i)
	{
		CRect& r = rects[i];
		CPoint topLeft = r.TopLeft();
		CPoint bottomRight = r.BottomRight();
		ScreenToClient(parent, &topLeft);
		ScreenToClient(parent, &bottomRight);
		r = CRect(topLeft, bottomRight);

		rcFinal.UnionRect(&rcFinal, &r);
	}

	//ReadRegSettings();

	hRes = view->CreateViewWindow(NULL, &folderSettings, (IShellBrowser*)this, rcFinal, &defView);

	view->UIActivate(SVUIA_INPLACEACTIVATE);
	::SetWindowPos(defView, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_HIDEWINDOW);

	if(defView != NULL)
	{
		HWND lv = FindWindowEx(defView, NULL, L"SysListView32", NULL);
		if(lv != NULL)
		{

			if(isVista)
				ListView_SetExtendedListViewStyleEx(lv, LVS_EX_TRANSPARENTBKGND |LVS_EX_TRANSPARENTSHADOWTEXT,
														LVS_EX_TRANSPARENTBKGND |LVS_EX_TRANSPARENTSHADOWTEXT);

			LONG lvStyle = GetWindowLong(lv, GWL_STYLE);
			lvStyle |= LVS_ALIGNLEFT;
			
			//lvStyle &= ~LVS_NOCOLUMNHEADER;
			//lvStyle &= ~LVS_NOSORTHEADER;
			SetWindowLong(lv, GWL_STYLE, lvStyle);

			LONG lvExStyle = GetWindowLong(lv, GWL_EXSTYLE);
			//lvExStyle |= LVS_EX_TRANSPARENTSHADOWTEXT;
			//lvExStyle |= LVS_EX_TRANSPARENTBKGND;
			if(!isVista)
				lvExStyle &= ~WS_EX_CLIENTEDGE;
			SetWindowLong(lv, GWL_EXSTYLE, lvExStyle);


			SetWallpaperToListView(0);
			
			
			if(rects.size() > 0)
				ListView_SetWorkAreas(lv, rects.size(), &rects[0]);
		}

		

		_ReadAndSetIconPos();
		InternalSubclassListView();
		InternalSubclassDefView();
	}
}

void CAveShellBrowser::RemoveSubclassing()
{
	if(hasSubclassed)
	{
		RemoveWindowSubclass(defView, DefViewSubclassProc, 1);
		hasSubclassed = FALSE;
	}

	if(hasSubclassedListView)
	{
		RemoveWindowSubclass(FindWindowEx(defView, NULL, L"SysListView32", NULL), ListViewSubclassProc, 1);
		hasSubclassedListView = FALSE;
	}
}

void CAveShellBrowser::KillView(CComPtr<IShellView>& view)
{
	if(NULL == view)
		return;

	view->UIActivate(SVUIA_DEACTIVATE);
	view->DestroyViewWindow();
	view.Release();
	
}

void CAveShellBrowser::Kill()
{
	browseToPidl = CPidl();
	isKilled = TRUE;
	RemoveSubclassing();
	KillView(view);

	defView = NULL;
}

void CAveShellBrowser::Show()
{
	if(NULL == view)
		return;

	if(!CheckTimeBomb())
		return;

	isVisible = TRUE;

	::SetWindowPos(defView, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
	//view->UIActivate(SVUIA_ACTIVATE_NOFOCUS);
}

void CAveShellBrowser::Hide()
{
	if(NULL == view)
		return;

	isVisible = FALSE;

	::SetWindowPos(defView, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_HIDEWINDOW);
}


void CAveShellBrowser::LoadFromSettings(CString guid, HWND parent)
{
	guidAsString = guid;
	ReadRegSettings();

	if(hotkeyVK != 0)
		RegisterHotkey(parent);

	if(isVisible)
	{
		Make(parent);
		Show();
	}
}

void CAveShellBrowser::ReadRegSettings()
{
	CString regKey = GetRegKeyName();

	AveRegFuncs::ReadBinary(HKEY_CURRENT_USER, regKey, L"folderSettings", &folderSettings, sizeof(folderSettings));

	isVisible		= (BOOL) AveRegFuncs::ReadInt(HKEY_CURRENT_USER, regKey, L"isVisible", FALSE);
	hotkeyVK		= AveRegFuncs::ReadInt(HKEY_CURRENT_USER, regKey, L"hotkeyVK", 0);
	hotkeyModifiers	= AveRegFuncs::ReadInt(HKEY_CURRENT_USER, regKey, L"hotkeyModifiers", 0);
	CString pidlStr	= AveRegFuncs::ReadString(HKEY_CURRENT_USER, regKey, L"pidl", L"");
	wallpaper		= AveRegFuncs::ReadString(HKEY_CURRENT_USER, regKey, L"wallpaper", L"");
	doInternalBrowsing = (BOOL)AveRegFuncs::ReadInt(HKEY_CURRENT_USER, regKey, L"doInternalBrowsing", 0);

	pidl.FromString((LPCTSTR)pidlStr);

	/*
	DWORD type = REG_BINARY;
	DWORD size = sizeof(folderSettings);
	SRegGetValue(HKEY_CURRENT_USER, regKey,
			L"folderSettings", RRF_RT_REG_BINARY, &type,(LPVOID)&folderSettings, &size);

	DWORD dwordVal = 0;
	size = sizeof(dwordVal);
	type = REG_DWORD;

	RegGetValue(HKEY_CURRENT_USER, regKey,
			L"isVisible", RRF_RT_REG_DWORD, &type,(LPVOID)&dwordVal, &size);
	isVisible = dwordVal != 0;

	RegGetValue(HKEY_CURRENT_USER, regKey,
			L"hotkeyVK", RRF_RT_REG_DWORD, &type,(LPVOID)&dwordVal, &size);
	hotkeyVK = dwordVal;

	RegGetValue(HKEY_CURRENT_USER, regKey,
			L"hotkeyModifiers", RRF_RT_REG_DWORD, &type,(LPVOID)&dwordVal, &size);
	hotkeyModifiers = dwordVal;

	type = REG_SZ;
	size = 0;
	RegGetValue(HKEY_CURRENT_USER, regKey,
			L"pidl", RRF_RT_REG_SZ, &type,NULL, &size);

	WCHAR* wPidl = new WCHAR[size/2];
	memset(wPidl, 0, size);

	RegGetValue(HKEY_CURRENT_USER, regKey,
			L"pidl", RRF_RT_REG_SZ, &type,wPidl, &size);

	pidl.FromString(wPidl);

	delete[] wPidl;
	*/
}

STDMETHODIMP CAveShellBrowser::OnMessage(MSG* pMsg)
{
	if(NULL == pMsg)
		return E_INVALIDARG;

	if(NULL == view)
		return S_FALSE;

	return view->TranslateAcceleratorW(pMsg);
}

STDMETHODIMP CAveShellBrowser::Stop(void)
{
	if(hasSubclassed)
	{
		RemoveWindowSubclass(defView, DefViewSubclassProc, 1);
		hasSubclassed = FALSE;
	}

	if(hasSubclassedListView)
	{
		RemoveWindowSubclass(FindWindowEx(defView, NULL, L"SysListView32", NULL), ListViewSubclassProc, 1);
		hasSubclassedListView = FALSE;
	}

	if(view != NULL)
	{	
		SaveState();
		view->UIActivate(SVUIA_DEACTIVATE);
		//PostMessage(defView, WM_QUIT, 0, 0);
		view->DestroyViewWindow();
		ULONG ref = view.p->Release();
		view.p = NULL;
	}

	return S_OK;
}

STDMETHODIMP CAveShellBrowser::SaveState(void)
{
	FOLDERSETTINGS fs = {0};

	CString regKey = GetRegKeyName();

	std::wstring pidlStr = pidl.ToString();

	AveRegFuncs::WriteString(HKEY_CURRENT_USER, regKey, L"wallpaper", wallpaper);
	AveRegFuncs::WriteString(HKEY_CURRENT_USER, regKey, L"pidl", pidlStr.c_str());
	AveRegFuncs::WriteInt(HKEY_CURRENT_USER, regKey, L"isVisible", (int)isVisible);
	AveRegFuncs::WriteInt(HKEY_CURRENT_USER, regKey, L"hotkeyVK", (int)hotkeyVK);
	AveRegFuncs::WriteInt(HKEY_CURRENT_USER, regKey, L"hotkeyModifiers", (int)hotkeyModifiers);
	AveRegFuncs::WriteInt(HKEY_CURRENT_USER, regKey, L"doInternalBrowsing", (int)doInternalBrowsing);
	
	/*
	RegSetKeyValue(HKEY_CURRENT_USER, regKey,
				L"pidl", REG_SZ, pidlStr.c_str(), (DWORD)(sizeof(WCHAR) * (pidlStr.size() + 1)));


	DWORD dwordVal = (DWORD)isVisible;
	RegSetKeyValue(HKEY_CURRENT_USER, regKey, L"isVisible", REG_DWORD, &dwordVal, sizeof(DWORD));

	dwordVal = (DWORD)hotkeyVK;
	RegSetKeyValue(HKEY_CURRENT_USER, regKey, L"hotkeyVK", REG_DWORD, &dwordVal, sizeof(DWORD));

	dwordVal = (DWORD)hotkeyModifiers;
	RegSetKeyValue(HKEY_CURRENT_USER, regKey, L"hotkeyModifiers", REG_DWORD, &dwordVal, sizeof(DWORD));
	*/

	if(view != NULL)
	{

		CComPtr<IStream> stream;
		CreateStreamOnHGlobal(NULL, TRUE, &stream);
		if(stream != NULL)
		{
			_SaveIconPosToStream(stream);
			HGLOBAL global = NULL;
			GetHGlobalFromStream(stream, &global);
			LPVOID data = GlobalLock(global);

			DWORD len = (DWORD)GlobalSize(global);

			/*RegSetKeyValue(HKEY_CURRENT_USER, regKey,
				L"iconPositions", REG_BINARY, data, len);
				*/

			AveRegFuncs::WriteHGlobal(HKEY_CURRENT_USER, regKey, L"iconPositions", global);

			GlobalUnlock(global);
		}

		view->GetCurrentInfo(&fs);
		//RegSetKeyValue(HKEY_CURRENT_USER, regKey,
		//		L"folderSettings", REG_BINARY, (LPCVOID)&fs, sizeof(fs));
		AveRegFuncs::WriteBinary (HKEY_CURRENT_USER, regKey, L"folderSettings", &fs, sizeof(fs));

		view->SaveViewState();
	}

	return S_OK;
}

HRESULT CAveShellBrowser::_ReadAndSetIconPos()
{
	HWND hwndLv = FindWindowEx(defView, NULL, L"SysListView32", NULL);
	if(NULL == hwndLv)
		return E_FAIL;

	CListViewCtrl lv;
	lv.Attach(hwndLv);

	CString regKey = GetRegKeyName();
/*
	DWORD type = REG_BINARY;
	DWORD size = 0;
	RegGetValue(HKEY_CURRENT_USER, regKey,
			L"iconPositions", RRF_RT_REG_BINARY, &type, NULL, &size);

	if(size <= 0)
		return S_OK;

	HGLOBAL global =  GlobalAlloc(GMEM_MOVEABLE | GMEM_DISCARDABLE, (SIZE_T)size);
	if(NULL == global)
		return S_OK;

	LPVOID data = GlobalLock(global);

	RegGetValue(HKEY_CURRENT_USER, regKey,
			L"iconPositions", RRF_RT_REG_BINARY, &type, data, &size);

	GlobalUnlock(global);
	*/

	HGLOBAL global = AveRegFuncs::ReadHGlobal(HKEY_CURRENT_USER, regKey, L"iconPositions");
	if(NULL == global)
		return S_OK;

	CComPtr<IStream> stream;
	CreateStreamOnHGlobal(global, TRUE, &stream);
	if(NULL == stream)
		return S_OK;


	int itemCount = lv.GetItemCount();


	DWORD numRead;

	int count = 0;
	stream->Read(&count, sizeof(count), &numRead);
	for(int i = 0; i < count; ++i)
	{
		int len = 0;
		stream->Read(&len, sizeof(len), &numRead);
		WCHAR* title = new WCHAR[len];
		memset(title, 0, sizeof(WCHAR) * len);
		stream->Read(title, len * sizeof(WCHAR), &numRead);
		CPoint pos;
		stream->Read(&pos.x, sizeof(pos.x), &numRead);
		stream->Read(&pos.y, sizeof(pos.y), &numRead);

		int index = -1;
		LVFINDINFO info = {0};
		info.flags |= LVFI_STRING;
		info.psz = title;
		index = lv.FindItem(&info, -1);
		if(index != -1)
		{
			lv.SetItemPosition(index, pos);
		}
		else
		{
			iconPositions.insert(std::pair<CString, CPoint>(title, pos));
		}

		delete[] title;
	}

	lv.Detach();

	return S_OK;
}

HRESULT CAveShellBrowser::_SaveIconPosToStream(CComPtr<IStream> stream)
{
	if(NULL == stream)
		return E_POINTER;

	HWND hwndLv = FindWindowEx(defView, NULL, L"SysListView32", NULL);
	if(NULL == hwndLv)
		return E_FAIL;

	DWORD numWritten = 0;
	CListViewCtrl lv;
	lv.Attach(hwndLv);

	int count = lv.GetItemCount();
	stream->Write(&count, sizeof(int), &numWritten);
	for(int i = 0; i < count; ++i)
	{
		CString title;
		CPoint pos;
		lv.GetItemText(i, 0, title);
		lv.GetItemPosition(i, &pos);

		int len = title.GetLength()+1;
		stream->Write(&len, sizeof(len), &numWritten);
		stream->Write((LPCTSTR)title, len*2, &numWritten);
		stream->Write(&pos.x, sizeof(pos.x), &numWritten);
		stream->Write(&pos.y, sizeof(pos.y), &numWritten);
	}

	lv.Detach();

	return S_OK;
}

CComPtr<IStream> CAveShellBrowser::_GetSettingsStream(const WCHAR* name, DWORD grfMode)
{

	CString regKey = GetRegKeyName();
	const WCHAR* value = name;

	HKEY hkey = NULL;
	LSTATUS res = RegOpenKey(HKEY_CURRENT_USER, regKey, &hkey);
	if(ERROR_SUCCESS != res || NULL == hkey)
	{
		RegCreateKey(HKEY_CURRENT_USER, regKey, &hkey);
	}

	if(hkey != NULL)
		RegCloseKey(hkey);
		
	return SHOpenRegStream2(HKEY_CURRENT_USER, regKey, value, grfMode);
}

STDMETHODIMP CAveShellBrowser::GetShellViewWindow(HWND* pHwnd)
{
	if(NULL == pHwnd)
		return E_POINTER;

	*pHwnd = defView;

	return S_OK;
}