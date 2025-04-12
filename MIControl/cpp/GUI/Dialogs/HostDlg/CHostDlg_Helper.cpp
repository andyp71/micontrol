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

CBitmap& CHostDlg::MergeIcons(CBitmap& dest, std::initializer_list<UINT> icons)
{
    std::vector<CIconBits> vIcons;
    SIZE szIcon{};
    for (const auto& uid : icons)
    {
        auto hIcon = LoadIcon(GetInstance(), MAKEINTRESOURCE(uid));
        vIcons.emplace_back(hIcon);
        DestroyIcon(hIcon);
        auto& ico = vIcons.back();
        szIcon.cx = std::max(ico.Size().cx, szIcon.cx);
        szIcon.cy = std::max(ico.Size().cy, szIcon.cy);
    }

    SIZE szMerged = { .cx = szIcon.cx * static_cast<INT>(vIcons.size()), .cy = szIcon.cy };
    CBitmap 
        rgb(szMerged.cx, szMerged.cy), 
        red, green;

    CBitmap bmIco;
    int cx = 0;
    for (auto& ico : vIcons)
    {
        bmIco.Create(ico.Size().cx, ico.Size().cy);
        CopyMemory(bmIco.Bits(), ico.Bits(), bmIco.Width() * bmIco.Height() * sizeof(RGBQUAD));

        rgb.RenderBitmap(
            bmIco,
            0, 0, bmIco.Width(), bmIco.Height(),
            cx + std::max<LONG>(0, szIcon.cx - bmIco.Width()) / 2,
            std::max<LONG>(0, (szIcon.cy - bmIco.Height())) / 2
        );

        cx += szIcon.cx;
    }

    red = green = rgb;

    auto count = rgb.Width() * rgb.Height();
    auto r = rgb.Bits(), rred = red.Bits(), rgreen = green.Bits();
    for (decltype(count)i = 0;i < count;++i)
    {
        rgreen[i].rgbReserved = rred[i].rgbReserved = r[i].rgbReserved = 255 - r[i].rgbGreen;
        rred[i].rgbRed = 68; rred[i].rgbGreen = 210; rred[i].rgbBlue = 118;
        rgreen[i].rgbRed = 210; rgreen[i].rgbGreen = 68; rgreen[i].rgbBlue = 118;
    }

    dest.Create(rgb.Width() + red.Width() + green.Width(), rgb.Height());
    dest.CopyRect(rgb, 0, 0, rgb.Width(), rgb.Height(), 0, 0);
    dest.CopyRect(red, 0, 0, red.Width(), red.Height(), rgb.Width(), 0);
    dest.CopyRect(green, 0, 0, green.Width(), green.Height(), rgb.Width() + red.Width(), 0);
    return dest;
}

void CHostDlg::SelectPage(Page pageId)
{
    union
    {
        RECT rc;
        POINT pt[2];
    } rc = {}, rcLv{};

    GetWindowRect(*this, &rc.rc);
    GetWindowRect(m_lvSelector, &rcLv.rc);
    ScreenToClient(*this, &rcLv.pt[0]);
    ScreenToClient(*this, &rcLv.pt[1]);
    GetClientRect(*this, &rc.rc);
    POINT ptLeft = { rcLv.rc.right + rcLv.rc.left, rcLv.rc.top };
    POINT ptRight = { rc.rc.right - rcLv.rc.left, rcLv.rc.bottom };

    for (auto& page : m_mPage)
    {
        bool fSelected = (pageId == page.first);
        page.second->Visible(fSelected);

        if (fSelected)
        {
            auto& hPage = *page.second.get();
            hPage.OnShow();
            SetWindowPos(
                hPage, 
                nullptr, 
                ptLeft.x, ptLeft.y, ptRight.x - ptLeft.x, ptRight.y - ptLeft.y, 
                SWP_FRAMECHANGED | SWP_SHOWWINDOW
            );
        }
    }
}

void CHostDlg::InitPages()
{
    m_lvSelector.ResetContent();
    Add<CPagePerfomanceDlg>(Page::pagePerfomance, TEXT("Perfomance"))->Create(this);
    Add<CPageMicDlg>(Page::pageMicrophone, TEXT("Microphone"))->Create(this);
    Add<CPageTouchpadDlg>(Page::pageTouchpad, TEXT("Touchpad"))->Create(this);
    Add<CPageChargeProtectDlg>(Page::pageChargeProtection, TEXT("Charge protection"))->Create(this);
    Add<CPageRefreshRateDlg>(Page::pageDisplayRate, TEXT("Display refresh rate"))->Create(this);
    Add<CPageMiButtonDlg>(Page::pageMiButton, TEXT("MI Button"))->Create(this);
    Add<CPageICCProfileDlg>(Page::pageICCProfile, TEXT("Color profiles"))->Create(this);
    Add<CPageServiceStatusDlg>(Page::pageServiceStatus, TEXT("Service status"))->Create(this);
    Add<CPageSysInfoDlg>(Page::pageSysInfo, TEXT("Device info"))->Create(this);
    Add<CPageSplashSkinDlg>(Page::pageSplashEdit, TEXT("Splash icons"))->Create(this);
    Add<CPageAboutDlg>(Page::pageAbout, TEXT("About utility"))->Create(this);

    SelectPage(pagePerfomance);
    m_lvSelector.CurSel(0);
}

void CHostDlg::DoClose()
{
    NOTIFYICONDATA ntf({ 0 });
    ntf.cbSize = sizeof(ntf);
    ntf.uID = 1;
    ntf.hWnd = *this;

    Shell_NotifyIcon(NIM_DELETE, &ntf);
    Close(0);
}

void CHostDlg::UpdateLvIcons()
{
    auto count = static_cast<int>(m_lvSelector.Count());
    for (decltype(count)i = 0;i < count;++i)
    {
        auto id = m_lvSelector.Param(i);
        auto it = m_mPage.find(static_cast<Page>(id));
        if (it == m_mPage.end())
            continue;

        COLORREF clr = 0;
        UINT iconId = 0;
        if (!it->second->GetLineColor(clr, iconId))
            continue;

        iconId = m_uIconsCount * iconId + i;
        m_lvSelector.SetItemImage(i, iconId);
    }

    InvalidateRect(m_lvSelector, nullptr, true);
}
