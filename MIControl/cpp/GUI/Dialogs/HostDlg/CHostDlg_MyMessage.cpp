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

LRESULT CHostDlg::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandle)
{
    switch (uMsg)
    {
    case WM_CUSTOM_SHOW_SPLASH:
        {
            if (lParam)
            {
                if (wParam)	// on button release
                    OnMiButtonUp();
                else
                    OnMiButtonDown();
            }
            else
            {
                OnUpdateEvent();
                if (wParam)
                    OnSplashShow(LOWORD(wParam));
            }

            if (!wParam)
                break;

            fHandle = true;
            return 0;
        }
    case WM_CUSTOM_TRAY:
        {
            fHandle = true;
            switch (lParam)
            {
                case WM_LBUTTONDOWN: OnTrayLBtn(); break;
                case WM_RBUTTONUP: OnTrayRBtn(); break;
            }
            break;
        }
    default:
        {
            if (m_wmActivateMessage == uMsg)
                OnWmMyMessage(static_cast<UINT>(wParam));
        }
    }

    return LRESULT();
}

void CHostDlg::OnMiButtonDown()
{

}

void CHostDlg::OnMiButtonUp()
{
    CRegKey keyMI;
    if (ERROR_SUCCESS == keyMI.Open(HKEY_CURRENT_USER, TEXT("Software\\MIControl"), KEY_QUERY_VALUE))
    {
        ULONG l = 1000;
        tstring strCommand(l, 0);
        if (ERROR_SUCCESS == keyMI.QueryStringValue(nullptr, strCommand.data(), &l))
        {
            strCommand.resize(l);
            while (!strCommand.empty() && '\x00' == strCommand.back())
                strCommand.pop_back();

            if (!strCommand.empty())
            {
                l = 0;
                keyMI.QueryDWORDValue(TEXT("Elevated"), l);
                if (INT_PTR(
                    ShellExecute(*this, 
                        l ? TEXT("runas") : TEXT("open"), strCommand.c_str(), 
                        nullptr, nullptr, SW_SHOW)) > 32)
                    return;
            }
        }
    }

    Visible(TRUE);
    SetForegroundWindow(*this);
}

void CHostDlg::OnSplashShow(UINT res)
{
    if (!m_splash)
    {
        m_splash = std::make_unique<CSplashScreen>(GetInstance());
        m_splash->Create(nullptr);
    }

    if (!m_host.splash.pview)
        m_host.splash.pview = m_spmgr.GetDefault();

    CSplashImagePtr pImage;
    if (m_host.splash.pview)
        pImage = (*m_host.splash.pview)[res];

    if (!pImage)
        m_splash->SetSplash(res);
    else
        m_splash->SetSplash(pImage);
}

void CHostDlg::OnUpdateEvent()
{
    UpdateLvIcons();
}

void CHostDlg::OnTrayLBtn()
{
    if (!Enable())
        return;

    SetForegroundWindow(*this);
    Visible(!Visible());
}

void CHostDlg::OnTrayRBtn()
{
    SetForegroundWindow(*this);
    using HHMENU = CAutoHandle<HMENU, nullptr, DestroyMenu>;
    HHMENU hMenu = CreatePopupMenu();
    bool fCanQuit = true;
    std::for_each(m_mPage.begin(), m_mPage.end(), [&fCanQuit](const auto& it) { fCanQuit &= it.second->IsCanQuit(); });

    POINT pt{};
    AppendMenu(hMenu, MF_STRING, 1, TEXT("Show settings..."));
    AppendMenu(hMenu, MF_STRING, 2, TEXT("Force Full speed Perfomance"));
    AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(hMenu, MF_STRING | ((fCanQuit && Enable()) ? 0 : MF_DISABLED), 3, TEXT("Close MI Control"));
    GetCursorPos(&pt);

    switch (TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, 0, *this, nullptr))
    {
    case 1: Visible(TRUE); break;
    case 2: SendCommandPut(0x08, 0x04); break;
    case 3: DoClose(); break;
    }
}

void CHostDlg::OnWmMyMessage(UINT pid)
{
    Visible(true);
    SetForegroundWindow(*this);
}