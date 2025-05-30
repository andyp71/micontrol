

#define				IMGLOADER_ALLOWED_FILE_SIZE				( 64 * ( 1 << 20 ) )		// 64 MB

class CImgLoader
{
	IStream				*m_pStream;
	Gdiplus::Bitmap		*m_pgpBmp;

	HGLOBAL				StreamFromFile(LPCTSTR lpszFilename);
	HGLOBAL				StreamFromResource(HINSTANCE hInstance, LPCTSTR lpszResName, LPCTSTR lpszResType);
	BOOL				LoadToStream(HGLOBAL hData);
//	IWICBitmapSource *	BitmapFromStream(BOOL fPreAlpha, BOOL fNoAlpha);

protected:
	virtual CLSID		DecoderCLSID() PURE;
	int					GetEncoderClsid(LPCTSTR format, CLSID* pClsid);
	Gdiplus::Bitmap *	Handle();

public:
	CImgLoader(HINSTANCE hInstance, LPCTSTR lpszResName, LPCTSTR lpszResType);
	CImgLoader(LPCTSTR lpszFilename);
	CImgLoader(CBitmap & bmp);

	virtual				~CImgLoader();
	BOOL				Bitmap(CBitmap & bmp, BOOL fPreAlpha = FALSE);
	HBITMAP				Bitmap(SIZE & sz);
	BOOL				FixedBitmap(CBitmap & bmp);
};

class CPngLoader: public CImgLoader
{
	CLSID				DecoderCLSID();

public:
	CPngLoader(HINSTANCE hInstance, LPCTSTR lpszResName, LPCTSTR lpszResType);
	CPngLoader(LPCTSTR lpszFilename);
	CPngLoader(CBitmap & bmp);
	BOOL				SavePng(LPCTSTR pszFilename);
};