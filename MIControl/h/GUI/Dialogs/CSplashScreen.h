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

#pragma once

#include <Splash/splash.h>

using HSCREENDC = CAutoHandle<HDC, nullptr, [](HDC hDC) -> BOOL { ReleaseDC(GetDesktopWindow(), hDC); return true; }>;

class CSplashScreen: public CWindow
{
	CBitmap			m_bmSplash;
	UINT			m_uRes = 0;
	int16_t			m_bAlpha = 255;
	SIZE			m_sz;
	bool			m_bUseSourceAlpha = false;
	HSCREENDC		m_hScreenDC = GetDC(GetDesktopWindow());

	BOOL			OnCreate(LPCREATESTRUCT lpcs) override;
	void			OnTimer(DWORD_PTR idTimer, FARPROC pTimerProc) override;
	VOID			OnPaint(HDC hDC, BOOL fErase, PRECT prc) override;
	VOID			OnEraseBg(HDC hDC) override;
	VOID			OnSize(INT dwState, USHORT cx, USHORT cy);
	VOID			OnSizing(INT dwState, USHORT cx, USHORT cy);

	RECT&			AdjustRect(const POINT& relpt, RECT& rc) const;
	RECT			DPIRect(LONG _left, LONG _top, const SIZE & sz) const;

public:
	CSplashScreen(HINSTANCE hInstance);
	void		SetSplash(UINT res);
	void		SetSplash(CSplashImagePtr pImage);
};