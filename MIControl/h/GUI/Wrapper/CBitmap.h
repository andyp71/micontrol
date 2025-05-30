#pragma once

#define XCHGCOLORREF(_x)              ((((_x)&0xFF00FF00L)|(((_x)&0x00FF0000L)>>16)|(((_x)&0x000000FFL)<<16)))
#define ALPHA_COLOR(front,back,alpha)	((alpha)*((front)-(back))/255+(back))
#define ALPHA_SUM(front,back)			((back) + (front) * (255 - (back)) / 255)

// #define		BITMAP_USE_LOCKDC

class CBitmap
{
	struct
	{
		HDC						hDCDesktop;
		HDC						hDC;
		RGBQUAD *				pvBuffer;
		HBITMAP					hBitmap;
		HBITMAP					hDefaultBitmap;
		SIZE					sz;
		UINT					uSize;
		// std::vector<RGBQUAD*>	vLines;
		BOOL					fLocked;
	} m_buffer;

	// shadow buffer
	PBYTE					*m_bmAlphaMask;
	PBYTE					*m_bmAlphaRestored;
	BOOL					m_fShadow;

public:
	CBitmap();
	CBitmap(CBitmap&& source) noexcept;
	CBitmap(INT cx, INT cy);
	~CBitmap();

	VOID						Create(INT cx, INT cy);
	VOID						Destroy();
	RGBQUAD *					Bits();
	// RGBQUAD *					operator[](int cy);
	CBitmap &					operator=(const CBitmap & bm);
	operator HDC();
	VOID						FreeDC();
	HDC							LockDC();
	HBITMAP						Bitmap();
	INT							Width() const;
	INT							Height() const;

	HBRUSH						CreateBrush();

	VOID						CreateFromBitmap(HBITMAP hBmp);
	VOID						PremultiplyAlpha();
	VOID						RenderBitmap(CBitmap & bmSrc, int x, int y, int cx, int cy, int dest_x, int dest_y, BYTE bAlpha = 255, BOOL fUseSourceAlpha = FALSE);
	VOID						RenderBitmap(HDC hDC, int x, int y, int cx, int cy, int dest_x, int dest_y, BYTE bAlpha = 255, BOOL fUseSourceAlpha = FALSE);
	VOID						RenderText(INT x, INT y, INT cx, INT cy, LPCTSTR pszText, HFONT hFont, BOOL bTransparent, COLORREF crTextColor, COLORREF crBackColor, DWORD dwFlags);
	VOID						CopyRect(CBitmap & bmSrc, UINT xSrc, UINT ySrc, UINT cxSrc, UINT cySrc, UINT xDst, UINT yDst);
#ifdef UNICODE
	VOID						RenderTextA(INT x, INT y, INT cx, INT cy, LPCSTR pszText, HFONT hFont, BOOL bTransparent, COLORREF crTextColor, COLORREF crBackColor, DWORD dwFlags);
#else
#define RenderTextA				RenderText
#endif

};