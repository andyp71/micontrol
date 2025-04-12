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

CSplashScreen::CSplashScreen(HINSTANCE hInstance)
    : CWindow(hInstance, TEXT("SysSplashClass32"))
    , m_sz{}
{
    Register(CS_DBLCLKS, NULL, LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_BTNFACE + 1));
}

BOOL CSplashScreen::OnCreate(LPCREATESTRUCT lpcs)
{
    SetWindowLongPtr(*this, GWL_EXSTYLE, (GetWindowLongPtr(*this, GWL_EXSTYLE)) | WS_EX_LAYERED | WS_EX_NOACTIVATE);
    SetWindowLongPtr(*this, GWL_STYLE, WS_POPUP);
    m_sz.cx = lpcs->cx;
    m_sz.cy = lpcs->cy;
    return TRUE;
}

void CSplashScreen::OnTimer(DWORD_PTR idTimer, FARPROC pTimerProc)
{
    switch (idTimer)
    {
    case 100:
        {
            m_bAlpha -= 4;
            if (m_bAlpha >= 0)
            {
                if (!m_bUseSourceAlpha)
                    SetLayeredWindowAttributes(*this, 0, LOBYTE(m_bAlpha), LWA_ALPHA);

                InvalidateRect(*this, nullptr, TRUE);
            }
            else
            {
                KillTimer(*this,idTimer);
                Visible(FALSE);
            }

            break;
        }
    case 101:
        {
            SetTimer(*this, 100, 10, nullptr);
            KillTimer(*this, idTimer);
            break;
        }
    }
}

RECT& CSplashScreen::AdjustRect(const POINT& relpt, RECT& rc) const
{
    POINT realPt{};
    RECT rcArea{};
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcArea, false);
    SIZE szArea = { .cx = rcArea.right - rcArea.left, .cy = rcArea.bottom - rcArea.top };
    realPt.x = MulDiv(relpt.x, szArea.cx, SPLASH_POS_WIDTH);
    realPt.y = MulDiv(relpt.y, szArea.cy, SPLASH_POS_HEIGHT);
    
    rc.right = realPt.x + (rc.right - rc.left);
    rc.bottom = realPt.y + (rc.bottom - rc.top);
    rc.left = realPt.x;
    rc.top = realPt.y;

    return rc;
}

RECT CSplashScreen::DPIRect(LONG _left, LONG _top, const SIZE &sz) const
{
    constexpr auto _defaultDPI = 96.0;
    auto dpi = GetDpiForWindow(*this) / _defaultDPI;
    RECT rc{}, rcArea{};
    rc.left = static_cast<decltype(rc.left)>(round(_left * dpi));
    rc.top = static_cast<decltype(rc.left)>(round(_top * dpi));
    rc.right = rc.left + static_cast<decltype(rc.left)>(round(sz.cx * dpi));
    rc.bottom = rc.top + static_cast<decltype(rc.left)>(round(sz.cy * dpi));

    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcArea, false);
    if (rc.right > rcArea.right)
    {
        auto uOffset = rc.right - rcArea.right;
        rc.right -= uOffset;
        rc.left -= uOffset;
    }

    if (rc.bottom > rcArea.bottom)
    {
        auto uOffset = rc.bottom - rcArea.bottom;
        rc.bottom -= uOffset;
        rc.top -= uOffset;
    }

    return rc;
}

void CSplashScreen::SetSplash(UINT res)
{
    CPngLoader png(GetInstance(), MAKEINTRESOURCE(res), TEXT("PNG"));
    png.FixedBitmap(m_bmSplash);
    KillTimer(*this, 100);
    KillTimer(*this, 101);
    m_bUseSourceAlpha = false;

    Visible(FALSE);

    m_bAlpha = 255;
    SetLayeredWindowAttributes(*this, 0, LOBYTE(m_bAlpha), LWA_ALPHA);
    SetTimer(*this, 101, IDB_WRONG_CHARGER == res ? 5000 : 400, nullptr);

    auto rc = DPIRect(100, 100, { m_bmSplash.Width(),m_bmSplash.Height() });
    OnSize(0, LOWORD(rc.right - rc.left), LOWORD(rc.bottom - rc.top));
    SetWindowPos(*this, HWND_TOPMOST, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

void CSplashScreen::SetSplash(CSplashImagePtr pImage)
{
    if (pImage->IsNull())
        return;

    KillTimer(*this, 100);
    KillTimer(*this, 101);
    Visible(FALSE);

    m_bUseSourceAlpha = pImage->Info().flags.bAlphaSrc;
    m_bAlpha = pImage->Info().bInitialAlpha;

    const auto& pt = pImage->Info().ptScreen;
    const SIZE sz = { pImage->Bitmap().Width() , pImage->Bitmap().Height() };
    RECT rc = { .left = pt.x, .top = pt.y, .right = pt.x + sz.cx, .bottom = pt.y + sz.cy };

    if (pImage->Info().flags.bUseDPI)
        rc = DPIRect(pt.x, pt.y, sz);

    AdjustRect(pImage->Info().ptScreen, rc);

    OffsetRect(&rc, -(rc.right - rc.left) / 2, -(rc.bottom - rc.top) / 2);

    if (m_bUseSourceAlpha)
    {
        // we're unable to use StretchBlt, because UpdateLayeredWindow will be used
        // so use AlphaBlend instead at initial time
        CBitmap bmOrigin(sz.cx, sz.cy);
        m_bmSplash.Create(rc.right - rc.left, rc.bottom - rc.top);
        BLENDFUNCTION bf = { 0 };
        bf.AlphaFormat = AC_SRC_ALPHA;
        bf.BlendOp = AC_SRC_OVER;
        bf.SourceConstantAlpha = LOBYTE(m_bAlpha);
        CopyMemory(bmOrigin.Bits(), pImage->Bitmap().Bits(), pImage->Bitmap().datasize());
        GdiAlphaBlend(m_bmSplash, 0, 0, m_bmSplash.Width(), m_bmSplash.Height(), bmOrigin, 0, 0, bmOrigin.Width(), bmOrigin.Height(), bf);

        // blink attribute to allow use UpdateLayeredWindow()
        SetWindowLongPtr(*this, GWL_EXSTYLE, (GetWindowLongPtr(*this, GWL_EXSTYLE)) & ~(WS_EX_LAYERED | WS_EX_NOACTIVATE));
        SetWindowLongPtr(*this, GWL_EXSTYLE, (GetWindowLongPtr(*this, GWL_EXSTYLE)) | WS_EX_LAYERED | WS_EX_NOACTIVATE);
        m_bmSplash.PremultiplyAlpha();
    }
    else
    {
        m_bmSplash.Create(pImage->Bitmap().Width(), pImage->Bitmap().Height());
        CopyMemory(m_bmSplash.Bits(), pImage->Bitmap().Bits(), pImage->Bitmap().datasize());
        SetLayeredWindowAttributes(*this, 0, m_bUseSourceAlpha ? 255 : LOBYTE(m_bAlpha), LWA_ALPHA);
    }

    SetTimer(*this, 101, pImage->Info().uTimeToShow, nullptr);

    OnSize(0, LOWORD(rc.right - rc.left), LOWORD(rc.bottom - rc.top));
    SetWindowPos(*this, HWND_TOPMOST, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE | SWP_SHOWWINDOW);
    InvalidateRect(*this, nullptr, false);
}

VOID CSplashScreen::OnPaint(HDC hDC, BOOL fErase, PRECT prc)
{
    OnEraseBg(hDC);
}

VOID CSplashScreen::OnEraseBg(HDC hDC)
{
    if (m_bUseSourceAlpha && GetDeviceCaps(hDC, BITSPIXEL) >= 32)
    {
        BLENDFUNCTION blend{};
        POINT ptDest{}, ptSrc{};
        SIZE sz{ .cx = m_sz.cx, .cy = m_sz.cy };
        blend.BlendOp = 0;
        blend.BlendFlags = 0;
        blend.SourceConstantAlpha = LOBYTE(m_bAlpha);
        blend.AlphaFormat = AC_SRC_ALPHA;
        UpdateLayeredWindow(*this, nullptr, nullptr, &sz, m_bmSplash, &ptSrc, 0, &blend, ULW_ALPHA);
    }
    else
    {
        SetStretchBltMode(hDC, HALFTONE);
        StretchBlt(hDC, 0, 0, m_sz.cx, m_sz.cy, m_bmSplash, 0, 0, m_bmSplash.Width(), m_bmSplash.Height(), SRCCOPY);
    }
    m_bmSplash.FreeDC();
}

VOID CSplashScreen::OnSize(INT dwState, USHORT cx, USHORT cy)
{
    m_sz.cx = cx;
    m_sz.cy = cy;
    InvalidateRect(*this, nullptr, true);
}

VOID CSplashScreen::OnSizing(INT dwState, USHORT cx, USHORT cy)
{
    OnSize(dwState, cx, cy);
}