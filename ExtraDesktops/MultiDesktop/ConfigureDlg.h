#pragma once
#include "atlwin.h"

#include "resource.h"

class CAveMultiDesktopHost;

class CConfigureDlg :
	public CDialogImpl<CConfigureDlg>
{
protected:
	CAveMultiDesktopHost* host;
public:
	CConfigureDlg(CAveMultiDesktopHost* host);
	~CConfigureDlg(void);

	CListBox desktops;
	CHotKeyCtrl hotkey;

	void ToggleControls(BOOL enable);
	void ClearControls();

	BOOL HasSelectedNewDesktopItem();
	bool doNotUpdateDesktopFromControls;
	bool transformedNewItem;

	CStatic wallpaperStatic;

	CWindow balloon;
	HWND balloonOverWindow;
	BOOL balloonVisible;
	CString currentBalloonName;
	
	void SetCurrentBalloonRegKeyToNeverShow();
	void HideBalloon();
	void ShowBalloon(CString name, CString title, CString text, HWND balloonOverWindow);
	void SetStaticWallpaperFromFile(CString file);

	static int CALLBACK CConfigureDlg::BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

	enum { IDD = IDD_CONFIGDLG };

	BEGIN_MSG_MAP(CConfigureDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)		
		MESSAGE_HANDLER(WM_MOVE, OnMove)	
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLeftButtonDown)

		COMMAND_HANDLER(IDC_DESKTOPS, LBN_SELCHANGE, OnLbnSelchangeDesktops)
		COMMAND_HANDLER(IDC_BROWSEFOLDER, BN_CLICKED, OnBnClickedBrowsefolder)
		COMMAND_HANDLER(IDC_HOTKEY, EN_CHANGE, OnHotKeyChange)
		COMMAND_HANDLER(IDC_REMOVEDESKTOP, BN_CLICKED, OnBnClickedRemovedesktop)
		COMMAND_HANDLER(IDC_TOPLOGO, STN_CLICKED, OnStnClickedToplogo)
		COMMAND_HANDLER(IDC_CHANGEWALLPAPER, BN_CLICKED, OnBnClickedChangewallpaper)
		COMMAND_HANDLER(IDC_CLEARWALLPAPER, BN_CLICKED, OnBnClickedClearwallpaper)
		COMMAND_HANDLER(IDC_BROWSING, BN_CLICKED, OnBnClickedBrowsing)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLeftButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLbnSelchangeDesktops(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBrowsefolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnHotKeyChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedRemovedesktop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStnClickedToplogo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedChangewallpaper(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedClearwallpaper(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBrowsing(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

