#include "StdAfx.h"
#include "ConfigureDlg.h"

#include "AveMultiDesktopHost.h"
#include "AveRegFuncs.h"

CConfigureDlg::CConfigureDlg(CAveMultiDesktopHost* host) : host(host)
{
	doNotUpdateDesktopFromControls = false;
	transformedNewItem = false;
}

CConfigureDlg::~CConfigureDlg(void)
{
}

void CConfigureDlg::ClearControls()
{
	doNotUpdateDesktopFromControls = true;

	SetStaticWallpaperFromFile(L"");
	SetDlgItemText(IDC_FOLDER, L"");
	CheckDlgButton(IDC_SCROLLBARS, FALSE);
	CheckDlgButton(IDC_BROWSING, FALSE);
	hotkey.SetHotKey(0, 0);
	
	doNotUpdateDesktopFromControls= false;
}

void CConfigureDlg::ToggleControls(BOOL enable)
{
	GetDlgItem(IDC_LBL1).EnableWindow(enable);
	GetDlgItem(IDC_FOLDER).EnableWindow(enable);
	GetDlgItem(IDC_BROWSEFOLDER).EnableWindow(enable);
	
	if(HasSelectedNewDesktopItem())
		enable = FALSE;

	GetDlgItem(IDC_LBL2).EnableWindow(enable);
	GetDlgItem(IDC_REMOVEDESKTOP).EnableWindow(enable);
	GetDlgItem(IDC_SCROLLBARS).EnableWindow(enable);
	GetDlgItem(IDC_BROWSING).EnableWindow(enable);
	GetDlgItem(IDC_HOTKEY).EnableWindow(enable);
	GetDlgItem(IDC_WALLPAPERFRAME).EnableWindow(enable);
	GetDlgItem(IDC_CHANGEWALLPAPER).EnableWindow(enable);
	GetDlgItem(IDC_CLEARWALLPAPER).EnableWindow(enable);
	GetDlgItem(IDC_WALLPAPER).EnableWindow(enable);

	// unsupported now
	GetDlgItem(IDC_SCROLLBARS).EnableWindow(FALSE);
	//GetDlgItem(IDC_BROWSING).EnableWindow(FALSE);
}
LRESULT CConfigureDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(USE_TIMEBOMB)
	{
		SYSTEMTIME bomb = {0};
		bomb.wYear =  TIMEBOMB_DATE_YEAR;
		bomb.wMonth =  TIMEBOMB_DATE_MONTH;
		bomb.wDay =  TIMEBOMB_DATE_DAY;

		if(!CheckTimeBomb())
		{
			SetDlgItemText(IDC_EXPIRE1, L"This application has expired!");
			GetDlgItem(IDC_EXPIRE1).ShowWindow(SW_SHOW);
		}
		else
		{
			WCHAR str[1024] = {0};
			GetDateFormat(LOCALE_USER_DEFAULT, 0, &bomb, L"dddd d MMMM, yyyy", str, 1024);
			SetDlgItemText(IDC_EXPIRE2, str);
	
			GetDlgItem(IDC_EXPIRE1).ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EXPIRE2).ShowWindow(SW_SHOW);
		}
	}

	ToggleControls(FALSE);

	balloonOverWindow	= NULL;
	balloonVisible		= FALSE;

	desktops		= GetDlgItem(IDC_DESKTOPS);
	hotkey			= GetDlgItem(IDC_HOTKEY);
	wallpaperStatic	= GetDlgItem(IDC_WALLPAPER);

	INITCOMMONCONTROLSEX initCC = {0};
	initCC.dwSize = sizeof(initCC);
	InitCommonControlsEx(&initCC);

	balloon.m_hWnd = CreateWindowEx(WS_EX_TOOLWINDOW,
        TOOLTIPS_CLASS,
        NULL,
        WS_POPUP | TTS_NOPREFIX | TTS_BALLOON,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        m_hWnd,
        NULL,
        _AtlBaseModule.GetModuleInstance(),
        NULL
        );

	//balloon.Create(TOOLTIPS_CLASS, m_hWnd, NULL, L"", WS_EX_TOOLWINDOW, WS_POPUP | TTS_NOPREFIX | TTS_BALLOON);
	if(balloon.IsWindow())
		balloon.SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOMOVE);

	wallpaperStatic.SetWindowPos(NULL, 0, 0, 120, 90, SWP_NOMOVE | SWP_NOZORDER);
	wallpaperStatic.ModifyStyle(0, SS_REALSIZECONTROL, 0);

	CString newItemText;
	newItemText.LoadString(IDS_NEWITEMTEXT);

	for(CAveMultiDesktopHost::DesktopList::iterator iter = host->desktops.begin();
			iter != host->desktops.end();
			++iter
			)
	{
		AveDesktop& desktop = *iter;
		desktops.AddString(desktop.object->pidl.GetFriendlyName().c_str());
	}

	desktops.AddString(newItemText);

	return 0;
}

BOOL CConfigureDlg::HasSelectedNewDesktopItem()
{
	return desktops.IsWindow() && desktops.GetCurSel() == desktops.GetCount()-1;
}

LRESULT CConfigureDlg::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DestroyWindow();
	return 0;
}

LRESULT CConfigureDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	host->SaveAll();

	if(balloon.IsWindow())
		balloon.DestroyWindow();

	return 0;
}

LRESULT CConfigureDlg::OnLeftButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SetCurrentBalloonRegKeyToNeverShow();
	return 0;
}

LRESULT CConfigureDlg::OnMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(!balloon.IsWindow() || !balloon.IsWindowVisible())
		return 0;

		TOOLINFO ti = {0};
		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = TTF_TRACK | TTF_TRANSPARENT;
		ti.hwnd = m_hWnd;
		ti.hinst = _AtlBaseModule.GetModuleInstance();
		ti.uId = (UINT_PTR) 0; // uid
		::GetClientRect(balloonOverWindow, &ti.rect);

		CPoint pt;
		::ClientToScreen(balloonOverWindow, &pt);
		pt.Offset(10, 10);
		LRESULT res = ::SendMessage(balloon, TTM_TRACKPOSITION, 0, MAKELPARAM(pt.x, pt.y));

	return 0;
}

void CConfigureDlg::SetCurrentBalloonRegKeyToNeverShow()
{
	if(balloonVisible && currentBalloonName.GetLength() > 0)
	{
		AveRegFuncs::WriteInt(HKEY_CURRENT_USER, L"Software\\AveSoftware\\MultiDesktop", currentBalloonName, 1);
		HideBalloon();
	}
}

void CConfigureDlg::HideBalloon()
{
	if(!balloon.IsWindow())
		return;

	balloonVisible = FALSE;
	currentBalloonName = L"";

	TOOLINFO ti = {0};
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_TRACK | TTF_TRANSPARENT;
    ti.hwnd = m_hWnd;
    ti.hinst = _AtlBaseModule.GetModuleInstance();
    ti.uId = (UINT_PTR) 0; // uid
	::GetClientRect(balloonOverWindow, &ti.rect);

	::SendMessage(balloon, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)&ti);
}

void CConfigureDlg::ShowBalloon(CString name, CString title, CString text, HWND balloonOverWindow)
{
	if(!balloon.IsWindow())
		return;

	if(AveRegFuncs::ReadInt(HKEY_CURRENT_USER, L"SOFTWARE\\AveSoftware\\MultiDesktop\\", name, 0) != 0)
	{
		HideBalloon();
		return;
	}

	currentBalloonName = name;

	balloonVisible = TRUE;

	this->balloonOverWindow = balloonOverWindow;

	TOOLINFO ti = {0};
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_TRACK | TTF_TRANSPARENT;
    ti.hwnd = m_hWnd;
    ti.hinst = _AtlBaseModule.GetModuleInstance();
    ti.uId =  (UINT_PTR)0; // uid
    ti.lpszText = (LPWSTR)(const WCHAR*)text;
        // ToolTip control will cover the whole window
	::GetClientRect(balloonOverWindow, &ti.rect);
   
	::SendMessage(balloon, TTM_DELTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);

    ::SendMessage(balloon, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
	::SendMessage(balloon,  TTM_SETTITLE, (WPARAM)1, (LPARAM)(LPCTSTR)title);


	CPoint pt;
	::ClientToScreen(balloonOverWindow, &pt);
	pt.Offset(10, 10);
	::SendMessage(balloon, TTM_TRACKPOSITION, 0, MAKELPARAM(pt.x, pt.y));
	::SendMessage(balloon, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&ti);
}

LRESULT CConfigureDlg::OnLbnSelchangeDesktops(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(HasSelectedNewDesktopItem())
	{
		transformedNewItem = true;
		ClearControls();
		ToggleControls(TRUE);

		CString title, text;
		title.LoadStringW(IDS_NEWDESKTOPTT_TITLE);
		text.LoadStringW(IDS_NEWDESKTOPTT_TEXT);
		ShowBalloon(L"ShowNewDesktopBalloon", title, text, GetDlgItem(IDC_BROWSEFOLDER));

		return 0;
	}
	else
	{
		HideBalloon();
	}

	transformedNewItem = false;

	int curSel = desktops.GetCurSel();
	if(curSel >= 0 && curSel < (int)host->desktops.size())
	{
		ToggleControls(TRUE);

		AveDesktop& desktop = host->desktops[curSel];
		hotkey.SetHotKey(desktop.object->hotkeyVK, desktop.object->hotkeyModifiers);
		SetDlgItemText(IDC_FOLDER, desktop.object->pidl.GetFullParseDisplayName().c_str());
		SetStaticWallpaperFromFile(desktop.object->wallpaper);
		CheckDlgButton(IDC_BROWSING, desktop.object->doInternalBrowsing);
	}
	else
	{
		ClearControls();
		ToggleControls(FALSE);
	}

	return 0;
}

int CALLBACK CConfigureDlg::BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	CConfigureDlg* dlg = reinterpret_cast<CConfigureDlg*>(lpData);
	if(BFFM_INITIALIZED == uMsg)
	{
		if(!dlg->HasSelectedNewDesktopItem())
		{
			CPidl& pidl = dlg->host->desktops[dlg->desktops.GetCurSel()].object->pidl;
			::SendMessage(hwnd, BFFM_SETSELECTION, FALSE, (LPARAM)(LPITEMIDLIST)pidl);
		}
	}

	return 0;
}

LRESULT CConfigureDlg::OnBnClickedBrowsefolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CString browseDlgTitle;
	browseDlgTitle.LoadStringW(IDS_BROWSEFORFOLDERTITLE);
	WCHAR displayName[MAX_PATH] = {0};
	BROWSEINFO bi = {0};
	bi.hwndOwner = GetActiveWindow();
	bi.lpszTitle = browseDlgTitle;
	bi.pszDisplayName = displayName;
	bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_BROWSEINCLUDEFILES;
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = (LPARAM)this;
	CPidl pidl;
	pidl.SetFromPIDL(SHBrowseForFolder(&bi));
	if(NULL == pidl)
		return 0;

	if(HasSelectedNewDesktopItem())
	{
		int index = host->InternalNewItem(pidl);
		if(index != -1)
		{
			transformedNewItem = false;
			AveDesktop& desktop = host->desktops[index];
			int newIndex = desktops.InsertString(desktops.GetCount() - 1, desktop.object->pidl.GetFriendlyName().c_str());
			desktops.SetCurSel(newIndex);

			ToggleControls(TRUE);

			WORD vk = 0;
			WORD mod = 0;
			hotkey.GetHotKey(vk, mod);
			desktop.object->hotkeyVK = vk;
			desktop.object->hotkeyModifiers	= mod;
			if(desktop.object->hotkeyVK != 0)
				desktop.object->RegisterHotkey(FindWindow(L"progman", NULL));

			SetCurrentBalloonRegKeyToNeverShow();

			CString title, text;
			title.LoadStringW(IDS_NEWSETTINGSTT_TITLE);
			text.LoadStringW(IDS_NEWSETTINGSTT_TEXT);
			ShowBalloon(L"ShowNewSettingsBalloon", title, text, GetDlgItem(IDC_HOTKEY));
		}
	}
	else
	{
		AveDesktop& desktop = host->desktops[desktops.GetCurSel()];
		BOOL wasVisible = FALSE;
		if(desktop.object->defView != NULL)
		{
			wasVisible = ::IsWindowVisible(desktop.object->defView);
			desktop.object->Kill();
		}
		
		desktop.object->pidl = pidl;
		if(wasVisible)
		{
			desktop.object->Make(FindWindow(L"progman", NULL));
			desktop.object->Show();
		}
	}

	SetDlgItemText(IDC_FOLDER, pidl.GetFullParseDisplayName().c_str());

	

	return 0;
}


LRESULT CConfigureDlg::OnHotKeyChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(HasSelectedNewDesktopItem() || transformedNewItem)
		return 0;

	SetCurrentBalloonRegKeyToNeverShow();

	AveDesktop& desktop = host->desktops[desktops.GetCurSel()];

	if(desktop.object->hotkeyAtom != NULL && desktop.object->hotkeyVK != 0)
		desktop.object->UnregisterHotkey(FindWindow(L"progman", NULL));

	WORD vk = 0;
	WORD mod = 0;
	hotkey.GetHotKey(vk, mod);
	desktop.object->hotkeyVK = vk;
	desktop.object->hotkeyModifiers	= mod;
	if(desktop.object->hotkeyVK != 0)
		desktop.object->RegisterHotkey(FindWindow(L"progman", NULL));

	return 0;
}

LRESULT CConfigureDlg::OnBnClickedRemovedesktop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int curSel = desktops.GetCurSel();
	if(curSel >= 0 && curSel < (int)host->desktops.size())
	{
		AveDesktop &desktop = host->desktops[curSel];
		desktop.object->Hide();
		desktop.object->Kill();
		host->CheckZOrderAndFix();
		CString regKey = desktop.object->GetRegKeyName();
		if(regKey.GetLength() > 0)
			SHDeleteKey(HKEY_CURRENT_USER, regKey);

		desktops.DeleteString(curSel);

		host->desktops.erase(host->desktops.begin() + curSel);
		if(host->curDesktop == curSel)
			host->curDesktop = -1;
	}

	return 0;
}

LRESULT CConfigureDlg::OnStnClickedToplogo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ShellExecute(NULL, L"open", L"http://www.extradesktops.com", NULL, NULL,
		SW_SHOW);

	return 0;
}

void CConfigureDlg::SetStaticWallpaperFromFile(CString file)
{
	if(file.GetLength() == 0)
	{
		CBitmap bmp = wallpaperStatic.SetBitmap(NULL);
	}
	else
	{
		CBitmap hBmp;
		Gdiplus::Bitmap bmp(file);
		bmp.GetHBITMAP(NULL, &hBmp.m_hBitmap);
		CBitmap tmpBmp = wallpaperStatic.SetBitmap(hBmp);
	}

}

LRESULT CConfigureDlg::OnBnClickedChangewallpaper(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int curSel = desktops.GetCurSel();
	if(curSel >= 0 && curSel < (int)host->desktops.size())
	{
		AveDesktop &desktop = host->desktops[curSel];

		CFileDialog dlg(TRUE, L"*.jpg;*.bmp;*.png;*.gif", desktop.object->wallpaper,4|2,
						L"Image Files\0*.jpg;*.bmp;*.png;*.gif\0All Files\0*.*\0");

		if(dlg.DoModal())
		{
			desktop.object->wallpaper = dlg.m_szFileName;
			desktop.object->SetWallpaperToListView(0);
			SetStaticWallpaperFromFile(dlg.m_szFileName);
		}
	}

	return 0;
}

LRESULT CConfigureDlg::OnBnClickedClearwallpaper(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int curSel = desktops.GetCurSel();
	if(curSel >= 0 && curSel < (int)host->desktops.size())
	{
		AveDesktop &desktop = host->desktops[curSel];
		desktop.object->wallpaper = L"";
		desktop.object->SetWallpaperToListView(0);
		SetStaticWallpaperFromFile(L"");
	}

	return 0;
}

LRESULT CConfigureDlg::OnBnClickedBrowsing(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int curSel = desktops.GetCurSel();
	if(curSel >= 0 && curSel < (int)host->desktops.size())
	{
		AveDesktop &desktop = host->desktops[curSel];
		desktop.object->doInternalBrowsing = IsDlgButtonChecked(IDC_BROWSING);
	}

	return 0;
}
