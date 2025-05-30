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

#include "Controls.h"

std::map<UINT, HDC>			l_mDesktopDC;
std::map<UINT, INT_PTR>		l_mDesktopDCRef;

HDC & __get_thread_dc()
{
    return l_mDesktopDC[ GetCurrentThreadId() ];
}

INT_PTR & __get_thread_dc_ref()
{
    return l_mDesktopDCRef[ GetCurrentThreadId() ];
}

VOID __init_desktop_dc_allocator()
{
    HDC & hDC = __get_thread_dc();

    if ( !hDC )
    {
        hDC = GetDC( GetDesktopWindow() );
    }
}

VOID __free_desktop_dc_allocator()
{
    HDC & hDC = __get_thread_dc();
    if ( hDC )
    {
        // ReleaseDC( GetDesktopWindow(), hDC );
    }
    // hDC = nullptr;
}

HDC __alloc_desktop_dc()
{
    INT_PTR & ref = __get_thread_dc_ref();
    if ( !ref )
    {
        __init_desktop_dc_allocator();
    }

    ref++;
    return __get_thread_dc();
}

VOID __free_desktop_dc()
{
    INT_PTR & ref = __get_thread_dc_ref();

    if ( ref > 0 )
    {
        ref--;
        if ( ref <= 0 )
        {
            __free_desktop_dc_allocator();
        }
    }
}

CBitmap::CBitmap()
{
    m_buffer.hDC = m_buffer.hDCDesktop = NULL;
    m_buffer.hBitmap = m_buffer.hDefaultBitmap = NULL;
    m_buffer.pvBuffer = NULL;
    m_buffer.sz.cx = m_buffer.sz.cy = 0;
    m_buffer.uSize = 0;
    m_buffer.fLocked = FALSE;
}

CBitmap::CBitmap(INT cx, INT cy)
{
    m_buffer.fLocked = FALSE;
    if ( cx <= 0 || cy <= 0 )
        throw std::exception("Invalid size");

    Create(cx, cy);
}

CBitmap::CBitmap(CBitmap&& source) noexcept
{
    m_buffer = source.m_buffer;
    source.m_buffer.fLocked = false;
    source.m_buffer.hBitmap = source.m_buffer.hDefaultBitmap = nullptr;
    source.m_buffer.hDC = source.m_buffer.hDCDesktop = nullptr;
    source.m_buffer.pvBuffer = nullptr;
    source.m_buffer.sz = {};
    source.m_buffer.uSize = 0;
}

CBitmap& CBitmap::operator=(const CBitmap& source)
{
    Create(source.m_buffer.sz.cx, source.m_buffer.sz.cy);
    CopyMemory(m_buffer.pvBuffer, source.m_buffer.pvBuffer, source.m_buffer.uSize);
    return *this;
}

CBitmap::~CBitmap()
{
    Destroy();
}

VOID CBitmap::Create(INT cx, INT cy)
{
    Destroy();
    
    if ( !cx || !cy )
        return;

    m_buffer.hDCDesktop = __alloc_desktop_dc();

    if ( !m_buffer.hDCDesktop )
    {
        char sError[1024] = { 0 };
        _sprintf_p(sError, sizeof(sError), "Lock DC desktop failed: %d", GetLastError());
        throw std::exception( sError );
    }

    m_buffer.hDC = CreateCompatibleDC( m_buffer.hDCDesktop );

    if ( !m_buffer.hDC )
    {
        char sError[1024] = { 0 };
        _sprintf_p(sError, sizeof(sError), "DC creation failed: %d", GetLastError());
        throw std::exception( sError );
    }
    
    UINT uSize = m_buffer.uSize = sizeof(RGBQUAD)*cx*cy;

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(bmi);
    bmi.bmiHeader.biWidth = cx;
    bmi.bmiHeader.biHeight = -cy;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = uSize;

    m_buffer.hBitmap = CreateDIBSection( m_buffer.hDCDesktop, &bmi, DIB_RGB_COLORS, (PVOID*) &m_buffer.pvBuffer, NULL, 0 );

    if ( !m_buffer.hBitmap )
    {
        char sError[1024] = { 0 };
        _sprintf_p(sError, sizeof(sError), "Bitmap creation failed: %d (%d:%d)", GetLastError(), cx, cy);
        throw std::exception( sError );
    }

#ifndef BITMAP_USE_LOCKDC
    m_buffer.hDefaultBitmap = (HBITMAP) SelectObject(m_buffer.hDC, m_buffer.hBitmap);
#endif

    m_buffer.sz.cx = cx;
    m_buffer.sz.cy = cy;
    __free_desktop_dc();
    m_buffer.hDCDesktop = nullptr;
    GdiFlush();
}

VOID CBitmap::Destroy()
{
    if ( m_buffer.uSize )
    {		
#ifndef BITMAP_USE_LOCKDC
        SelectObject(m_buffer.hDC, m_buffer.hDefaultBitmap);
#endif
        FreeDC();
        DeleteObject(m_buffer.hBitmap);
        DeleteDC(m_buffer.hDC);
        __free_desktop_dc();

        m_buffer.hDC = m_buffer.hDCDesktop = NULL;
        m_buffer.hBitmap = m_buffer.hDefaultBitmap = NULL;
        m_buffer.pvBuffer = NULL;
        m_buffer.sz.cx = m_buffer.sz.cy = 0;
        m_buffer.uSize = 0;
    }
}

VOID CBitmap::FreeDC()
{
    if ( m_buffer.fLocked )
    {
#ifdef BITMAP_USE_LOCKDC
        SelectObject(m_buffer.hDC, m_buffer.hDefaultBitmap);
#endif
        m_buffer.fLocked = FALSE;
    }

    GdiFlush();
}

HDC CBitmap::LockDC()
{
    if ( !m_buffer.fLocked )
    {
        m_buffer.fLocked = TRUE;
#ifdef BITMAP_USE_LOCKDC
        m_buffer.hDefaultBitmap = (HBITMAP) SelectObject(CheckDC(), m_buffer.hBitmap);
#endif
    }

    return m_buffer.hDC;
}

RGBQUAD * CBitmap::Bits()
{
    FreeDC();
    return m_buffer.pvBuffer;
}

CBitmap::operator HDC()
{
    return LockDC();
}

HBITMAP CBitmap::Bitmap()
{
    INT cx = m_buffer.sz.cx, cy = m_buffer.sz.cy, uSize = sizeof(RGBQUAD) * cx * cy;
    PVOID pvBuffer = NULL;

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(bmi);
    bmi.bmiHeader.biWidth = cx;
    bmi.bmiHeader.biHeight = -cy;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = uSize;

    HBITMAP hBitmap = CreateDIBSection( m_buffer.hDC, &bmi, DIB_RGB_COLORS, (PVOID*) &pvBuffer, NULL, 0 );
    if ( hBitmap )
    {
        CopyMemory(pvBuffer, Bits(), uSize);
        return hBitmap;
    }

    return NULL;
}

INT CBitmap::Width() const
{
    return m_buffer.sz.cx;
}

INT CBitmap::Height() const
{
    return m_buffer.sz.cy;
}

VOID CBitmap::PremultiplyAlpha()
{
    BOOL fLocked = m_buffer.fLocked;
    UINT uSize = m_buffer.uSize / sizeof( RGBQUAD );
    FreeDC();
    for(UINT i = 0; i < uSize; ++i)
    {
        RGBQUAD & w = m_buffer.pvBuffer[i];
        w.rgbBlue = (BYTE) ( int(w.rgbReserved) * int(w.rgbBlue) / 255 );
        w.rgbGreen = (BYTE) ( int(w.rgbReserved) * int(w.rgbGreen) / 255 );
        w.rgbRed = (BYTE) ( int(w.rgbReserved) * int(w.rgbRed) / 255 );
    }

    if ( fLocked )
        LockDC();
}


VOID CBitmap::RenderBitmap(CBitmap & bmSrc, int x, int y, int cx, int cy, int dest_x, int dest_y, BYTE bAlpha, BOOL fUseSourceAlpha)
{
    BLENDFUNCTION bf = { 0 };
    bf.AlphaFormat = ( fUseSourceAlpha ? AC_SRC_ALPHA : 0 );
    bf.BlendOp = AC_SRC_OVER;
    bf.SourceConstantAlpha = bAlpha;

    GdiAlphaBlend(*this, dest_x, dest_y, cx, cy, bmSrc, x, y, cx, cy, bf);
}


VOID CBitmap::RenderBitmap(HDC hDC, int x, int y, int cx, int cy, int dest_x, int dest_y, BYTE bAlpha, BOOL fUseSourceAlpha)
{
    BLENDFUNCTION bf = { 0 };
    bf.AlphaFormat = ( fUseSourceAlpha ? AC_SRC_ALPHA : 0 );
    bf.BlendOp = AC_SRC_OVER;
    bf.SourceConstantAlpha = bAlpha;

    GdiAlphaBlend(hDC, dest_x, dest_y, cx, cy, *this, x, y, cx, cy, bf);
}

HBRUSH CBitmap::CreateBrush()
{
    return CreatePatternBrush(m_buffer.hBitmap);
}

VOID CBitmap::CreateFromBitmap(HBITMAP hBmp)
{
    BITMAP obj = { 0 };
    
    if ( GetObject( hBmp, sizeof(obj), &obj ) )
    {
        Destroy();
        Create(obj.bmWidth, obj.bmHeight);

        HDC hDesktopDC = __alloc_desktop_dc();
        HDC hMyDC = CreateCompatibleDC( hDesktopDC );
        
        HGDIOBJ hOldBm = SelectObject(hMyDC, hBmp);
        BitBlt( LockDC(), 0, 0, obj.bmWidth, obj.bmHeight, hMyDC, 0, 0, SRCCOPY );
        SelectObject(hMyDC, hOldBm);
        DeleteDC( hMyDC );
        __free_desktop_dc();
    }
}

VOID CBitmap::RenderText(INT x, INT y, INT cx, INT cy, LPCTSTR pszText, HFONT hFont, BOOL bTransparent, COLORREF crTextColor, COLORREF crBackColor, DWORD dwFlags)
{
    if (!m_buffer.uSize) return;		// return if bitmap was not created

    INT iright = x+cx,ibottom = y+cy;
    BOOL fLocked = m_buffer.fLocked;

    int bkMode = GetBkMode( *this );
    COLORREF clrText = GetTextColor( *this );
    COLORREF clrBg = GetBkColor( *this );
    HGDIOBJ hfntOld = SelectObject( *this, hFont );

    SetBkMode( *this,  bTransparent ? TRANSPARENT : OPAQUE );
    SetTextColor( *this, crTextColor );
    SetBkColor( *this, crBackColor );
    RECT rc = { x, y, x + cx, y + cy };

    DrawText( *this, pszText, lstrlen(pszText), &rc, dwFlags);

    SetBkMode( *this,  bkMode );
    SetTextColor( *this, clrText );
    SetBkColor( *this, clrBg );
    SelectObject( *this, hfntOld );

    if ( !fLocked )
        this->FreeDC();
}

#ifdef UNICODE
VOID CBitmap::RenderTextA(INT x, INT y, INT cx, INT cy, LPCSTR pszText, HFONT hFont, BOOL bTransparent, COLORREF crTextColor, COLORREF crBackColor, DWORD dwFlags)
{
    if (!m_buffer.uSize) return;		// return if bitmap was not created

    BOOL fLocked = m_buffer.fLocked;

    INT iright = x+cx,ibottom = y+cy;

    int bkMode = GetBkMode( *this );
    COLORREF clrText = GetTextColor( *this );
    COLORREF clrBg = GetBkColor( *this );
    HGDIOBJ hfntOld = SelectObject( *this, hFont );

    SetBkMode( *this,  bTransparent ? TRANSPARENT : OPAQUE );
    SetTextColor( *this, crTextColor );
    SetBkColor( *this, crBackColor );
    RECT rc = { x, y, x + cx, y + cy };

    DrawTextA( *this, pszText, lstrlenA(pszText), &rc, dwFlags);

    SetBkMode( *this,  bkMode );
    SetTextColor( *this, clrText );
    SetBkColor( *this, clrBg );
    SelectObject( *this, hfntOld );

    if ( !fLocked )
        this->FreeDC();
}
#endif

VOID CBitmap::CopyRect(CBitmap & bmSrc, UINT xSrc, UINT ySrc, UINT cxSrc, UINT cySrc, UINT xDst, UINT yDst)
{
    if ( !bmSrc.m_buffer.uSize )
        return;

    RECT rcDst =
    {
        static_cast<LONG>( xDst ),
        static_cast<LONG>( yDst ),
        static_cast<LONG>( xDst + cxSrc ),
        static_cast<LONG>( yDst + cySrc )
    };

    RECT rcSrc =
    {
        static_cast<LONG>( xSrc ),
        static_cast<LONG>( ySrc ),
        static_cast<LONG>( xSrc + cxSrc ),
        static_cast<LONG>( ySrc + cySrc )
    };

    rcDst.left = min( this->m_buffer.sz.cx, rcDst.left );
    rcDst.top = min( this->m_buffer.sz.cy, rcDst.top );
    rcDst.right = min( this->m_buffer.sz.cx, rcDst.right );
    rcDst.bottom = min( this->m_buffer.sz.cy, rcDst.bottom );

    if ( rcDst.left == rcDst.right || rcDst.top == rcDst.bottom )
        return;

    rcSrc.left = min( bmSrc.m_buffer.sz.cx, rcSrc.left );
    rcSrc.top = min( bmSrc.m_buffer.sz.cy, rcSrc.top );
    rcSrc.right = min( bmSrc.m_buffer.sz.cx, rcSrc.right );
    rcSrc.bottom = min( bmSrc.m_buffer.sz.cy, rcSrc.bottom );

    if ( rcSrc.left == rcSrc.right || rcSrc.top == rcSrc.bottom )
        return;

    const UINT yFrom = rcDst.top, yTo = rcDst.bottom;
    const UINT xSize = rcDst.right - rcDst.left;

    RGBQUAD * qSrc = bmSrc.m_buffer.pvBuffer + bmSrc.m_buffer.sz.cx * rcSrc.top + rcSrc.left;
    RGBQUAD * qDst = this->m_buffer.pvBuffer + this->m_buffer.sz.cx * rcDst.top + rcDst.left;

    for(UINT y = yFrom; y < yTo; ++y)
    {
        MoveMemory(qDst, qSrc, xSize * sizeof(RGBQUAD));
        qSrc += bmSrc.m_buffer.sz.cx;
        qDst += this->m_buffer.sz.cx;
    }
}
