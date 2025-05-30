

class CIconBits
{
	std::vector<RGBQUAD>	m_pvBits;
	SIZE					m_sz;
	BOOL					GetMaskBits(HBITMAP hBitmap, std::vector<BYTE> & bits);

public:
	CIconBits();
	CIconBits(HICON hIcon);
	CIconBits(const CIconBits & bits);

	~CIconBits();

	CIconBits & operator=(const CIconBits & bits);
	
	BOOL					Create(SIZE sz);
	BOOL					ImportIcon(HBITMAP hBmp);
	BOOL					ImportIcon(HICON hIcon);
	HICON					ExportIcon();
	RGBQUAD	*				Bits();
	SIZE					Size() const;
	LONG					Width() const;
	LONG					Height() const;
	BOOL					Save(LPCTSTR pszFile);
};