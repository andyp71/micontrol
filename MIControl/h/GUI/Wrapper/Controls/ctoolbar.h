
class CToolbar: public CControl
{
	HIMAGELIST			m_himl;
	BOOL				AddCustomButton(int iBitmap, int idCommand, BYTE fState, BYTE fStyle, LPARAM dwData, INT_PTR iString);
	BOOL				AddCustomButton(int iBitmap, int idCommand, BYTE fState, BYTE fStyle, LPARAM dwData, LPCTSTR iString);
	BOOL				AddCustomButton(int iIndex, int iBitmap, int idCommand, BYTE fState, BYTE fStyle, LPARAM dwData, INT_PTR iString);
	SIZE				m_szButton;

public:
	CToolbar(HINSTANCE hInstance, USHORT uId, 
		INT x, INT y, INT cx, INT cy,
		DWORD dwStyle, DWORD dwStyleEx = 0, WORD wAnchor = 0);
	~CToolbar();

	BOOL				AdjustButtonsSize();
	BOOL				SetButtonsSize(USHORT cx, USHORT cy);
	HBITMAP				SetImageList(HBITMAP hBitmap, COLORREF clrMask = 0);
	BOOL				AddCheck(int iBitmap, int idCommand, LPCTSTR pszText = NULL, BOOL fChecked = FALSE, BOOL fEnabled = TRUE, LPARAM lParam = 0);
	BOOL				AddRadio(int iBitmap, int idCommand, LPCTSTR pszText = NULL, BOOL fChecked = FALSE, BOOL fEnabled = TRUE, LPARAM lParam = 0);
	BOOL				AddButton(int iBitmap, int idCommand, LPCTSTR pszText = NULL, BOOL fChecked = FALSE, BOOL fEnabled = TRUE, LPARAM lParam = 0);
	BOOL				AddDropDownButton(int iBitmap, int idCommand, LPCTSTR pszText = NULL, BOOL fChecked = FALSE, BOOL fEnabled = TRUE, LPARAM lParam = 0);
	BOOL				AddSeparator();
	void				EnableAll(BOOL fEnabled, ...);
	void				CheckAll(BOOL fChecked, ...);
	BOOL				Enabled(int idCommand);
	BOOL				Checked(int idCommand);
	void				Enabled(int idCommand, BOOL fEnable);
	void				Checked(int idCommand, BOOL fCheck);
};