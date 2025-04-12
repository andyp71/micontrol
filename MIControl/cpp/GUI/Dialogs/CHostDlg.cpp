/*
	MI Control Utility — v1.2.5 [public version]
	Copyright (C) 2025 ALXR aka loginsin
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "precomp.h"
#include <shellapi.h>
#include <Common.h>
#include <Uxtheme.h>
#include <dwmapi.h>

#pragma comment(lib, "Dwmapi.lib")

CHostDlg::CHostDlg(HINSTANCE hInstance)
    :
    CDialog(hInstance, 450, 250, TEXT("MI Control v1.2.5.2"), FALSE, FALSE, FALSE, TRUE)
    , m_lvSelector(hInstance, 0x13, 7, 7, 100, -7, WS_VISIBLE | LVS_REPORT | LVS_NOSORTHEADER | LVS_SHOWSELALWAYS | LVS_SINGLESEL | LVS_NOCOLUMNHEADER | WS_TABSTOP, WS_EX_CLIENTEDGE)
    , m_host{ .status = m_status, .splash{ .splash_mgr = m_spmgr } }
{
    pushctl(m_lvSelector);

    m_dtlEv[22] = std::bind(&CHostDlg::OnEventWorkload, this, std::placeholders::_1);
    m_dtlEv[33] = std::bind(&CHostDlg::OnEventMiclock, this, std::placeholders::_1);

    m_getEv[0x08] = std::bind(&CHostDlg::OnWorkloadModeGet, this, std::placeholders::_1);
    m_getEv[0x0A] = std::bind(&CHostDlg::OnMicLockGet, this, std::placeholders::_1);
    m_getEv[0x10] = std::bind(&CHostDlg::OnPowerModeGet, this, std::placeholders::_1);
    m_getEv[0xA0] = std::bind(&CHostDlg::OnCameraModeGet, this, std::placeholders::_1);
}

BOOL CHostDlg::OnInit(LPARAM lParam)
{
    m_hMutex = CreateMutex(nullptr, true, MI_CONTROL_MUTEX);
    if (!m_hMutex || ERROR_ALREADY_EXISTS == GetLastError())
    {
        PostMessage(HWND_BROADCAST, m_wmActivateMessage, GetCurrentProcessId(), 0);
        Close(0);
        return FALSE;
    }

    RECT rc({ 0 });
    auto hIcon = LoadIcon(GetInstance(), MAKEINTRESOURCE(IDI_BIG));
    NOTIFYICONDATA ntf({ 0 });
    ntf.cbSize = sizeof(ntf);
    ntf.uFlags = NIM_ADD | NIF_TIP | NIF_ICON | NIF_MESSAGE;
    ntf.hIcon = hIcon;
    ntf.hWnd = *this;
    ntf.uID = 1;
    ntf.uCallbackMessage = WM_CUSTOM_TRAY;
    lstrcpy(ntf.szTip, TEXT("MI Control"));

    Shell_NotifyIcon(NIM_ADD, &ntf);

    wSendMessage(WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    wSendMessage(WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

    GetClientRect(m_lvSelector, &rc);
    SetTimer(*this, 100, 250, nullptr);

    if (!CheckServicePresence())
    {
        if (Confirm(TEXT("Unable to read service status.\n\tSeems to be it is not installed.\n")
            TEXT("Do you want to install it now?\n\n")
            TEXT("Note: This action requires Administrative rights!")))
            ShellExecute(*this, TEXT("runas"), ImagePath().c_str(), TEXT("/install"), nullptr, SW_SHOW);
    }

    CBitmap bm;
    MergeIcons(bm, 
        { 
            IDI_FIRE, 
            IDI_MICROPHONE, 
            IDI_MOUSE,
            IDI_BATTERY,
            IDI_RATE,
            IDI_TOGGLE1,
            IDI_COLORS,
            IDI_SERVICE,
            IDI_LAPTOP,
            IDI_STARS,
            IDI_CONVERSATION }
    );
    
    auto hbm = bm.Bitmap();

    SetWindowTheme(m_lvSelector, TEXT("EXPLORER"), NULL);

    // this will be deleted with listview
    HIMAGELIST himl = ImageList_Create(bm.Height(), bm.Height(), ILC_COLOR32, bm.Width() / bm.Height(), 1);
    ImageList_Add(himl, hbm, nullptr);
    m_uIconsCount = ImageList_GetImageCount(himl) / 3;
    DeleteBitmap(hbm);

    ListView_SetImageList(m_lvSelector, himl, LVSIL_NORMAL);
    ListView_SetImageList(m_lvSelector, himl, LVSIL_SMALL);

    m_lvSelector.ExtendedStyle(LVS_EX_FULLROWSELECT);
    m_lvSelector.AddColumn(TEXT(""), rc.right);

    InitPages();
    SetFocus(m_lvSelector);
    UpdateLvIcons();

    return TRUE;
}



BOOL CHostDlg::OnClose()
{
    Visible(FALSE);
    return FALSE;
}

VOID CHostDlg::OnCancel()
{
    if (OnClose())
        Close(0);
}