
#include "CDialogTemplate.h"

#define		SPINVAL(x)				( dSendMessage((x), UDM_GETPOS) )
#define		SPINSET(x, a)			( dSendMessage((x), UDM_SETPOS, 0, MAKELONG((a), 0)) )
#define		SPINRANGE(x, a, b)		( dSendMessage((x), UDM_SETRANGE, 0, MAKELONG((a), (b))) )
#define		SPINERR(x)				( 0 != ( HIWORD(SPINVAL(x) ) ) )

#define			WM_GETCUSTOMINSTANCE	( WM_USER + 0x2122 )
#define			WM_GETCUSTOMMENU		( WM_USER + 0x2123 )
#define			MM_CALCSCROLL		0x3F
#define			DLGPT(x)		( 3 * (x) / 2 )

typedef enum _tagMOUSEBUTTON
{
	mbLeft,
	mbRight,
	mbMiddle,
	mbX
} MOUSEBUTTON, *PMOUSEBUTTON;

typedef enum _tagMOUSEACTION
{
	maDown,
	maUp,
	maDblClick,
	maMove
} MOUSEACTION, *PMOUSEACTION;

typedef enum _tagFRAME_TYPE
{
	ftWindow,
	ftMDI,
	ftMDIChild
} FRAME_TYPE, *PFRAME_TYPE;

typedef enum _tagWMGR_MINMAX
{
	mmMaxSize,
	mmMaxPos,
	mmMinTrackSize,
	mmMaxTrackSize
} WMGR_MINMAX, *PWMGR_MINMAX;

class CWindow;
class CDialog;
class CPropSheet;

class CControl
{
	friend				CDialog;

	HWND				m_hWnd;
	HWND				m_hParent;
	HINSTANCE			m_hInstance;

	struct
	{
		USHORT			uId;
		tstring			sClass;
		tstring			sTitle;
		POINT			pt;
		SIZE			sz;
		RECT			rc;
		DWORD			dwStyle;
		DWORD			dwStyleEx;
		WORD			wAnchor;
		RECT			rcAnchor;
		BOOL			fAutoSized;
	} m_pre;

	VOID				InitCtl();

protected:
	virtual VOID			PostCreate();
	HWND					Parent();
	virtual const PVOID		CreateStruct() const;

public:
	CControl(HINSTANCE hInstance);
	CControl();
	CControl(HINSTANCE hInstance, USHORT uId, 
		LPCTSTR pszClass, LPCTSTR pszTitle, 
		INT x, INT y, INT cx, INT cy,
		DWORD dwStyle, DWORD dwStyleEx = 0, WORD wAnchor = 0);
	virtual ~CControl();

	VOID				PostInit(HINSTANCE hInstance, USHORT uId, 
		LPCTSTR pszClass, LPCTSTR pszTitle, 
		INT x, INT y, INT cx, INT cy,
		DWORD dwStyle, DWORD dwStyleEx = 0, WORD wAnchor = 0);

	HWND				Create(HWND hParent, USHORT uId, 
							int x, int y, int cx, int cy,
							LPCTSTR pszClass, LPCTSTR pszTitle,
							DWORD dwStyle, DWORD dwStyleEx = 0);

	HWND				Create(CWindow * pWnd, BOOL fPreCreate = FALSE);

	INT_PTR				cSendMessage(UINT uMsg, WPARAM wParam = NULL, LPARAM lParam = NULL);
	const INT_PTR		cSendMessage( UINT uMsg, WPARAM wParam = NULL, LPARAM lParam = NULL ) const;
	operator			HWND();
	
	VOID				UpdateSize(INT cx, INT cy);

	VOID				Enable(BOOL fEnable);
	BOOL				Enable();
	VOID				Visible(BOOL fVisible);
	BOOL				Visible();

	tstring				Text();
	VOID				Text(tstring sText);
};

class CDialog;
class CMDIClient;

class CWindow
{
	friend					CDialog;
	friend					CPropSheet;

	CPropSheet				*m_pPropSheet;
	CWindow					*m_pParent;
	BOOL					m_fGlobalAccel;
	const BOOL				m_fDialogMode;
	FRAME_TYPE				m_ftWindowType;
	BOOL					m_fModal;
	BOOL					m_fWindowModal;
	UINT_PTR				m_uWindowModalExitCode;
	BOOL					m_fSelfHandle;
	BOOL					m_fExternal;
	BOOL					m_fInFocus;
	const HINSTANCE			m_hInstance;
	HWND					m_hWnd;
	HWND					m_hPropSheet;
	tstring					m_sClass;
	tstring					m_sCaption;
	const LONG_PTR			m_idRes;
	LPARAM					m_lDlgParam;

	HBRUSH					m_hbgBrush;
	std::vector<CControl*>	m_vCtl;
	CControl				m_cDefaultOk;
	CControl				m_cDefaultCancel;
	CMDIClient				*m_pMDI, *m_pMDIParent;
	std::map<WMGR_MINMAX, POINT>	m_mmInfo;

	static INT_PTR CALLBACK SysDialogProc(HWND, UINT, WPARAM, LPARAM);
	LRESULT				WindowProc(UINT, WPARAM, LPARAM);
	LRESULT				WindowProcDefault(UINT, WPARAM, LPARAM);

	BOOL				IsWindowModal();
	BOOL				InternalOnCreate(LPCREATESTRUCT lpcs);
	BOOL				InternalOnInit();
	VOID				InternalOnPaint(WPARAM, LPARAM);
	VOID				InternalOnClose();
	VOID				InternalOnSize(WPARAM wParam, LPARAM lParam);
	VOID				InternalOnSizing(WPARAM wParam, LPARAM lParam);
	INT_PTR				InternalOnNotify(LPNMHDR lpnm);
	VOID				InternalOnCommand(USHORT uCmd, USHORT uId, HWND hCtl);
	tstring				ShellFileDialog(BOOL fOpenFile, LPCTSTR pszDefaultFilename, std::vector<tstring> & vExtensions);
	CWindow(HINSTANCE, LONG_PTR pszResource);

protected:
	CWindow(CREATESTRUCT lpcs, HWND hWnd);
	static LRESULT CALLBACK SysWindowProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT		CreateExternal(HWND hWnd, LPCREATESTRUCT lpcs, CWindow * This);
	ATOM				Register(DWORD style, HICON hIcon = NULL, HCURSOR hCursor = NULL, HBRUSH hBg = NULL);
	static ATOM			Register(HINSTANCE hInstance, DWORD style, WNDPROC lpfnCustomProc, tstring sClass, HICON hIcon = NULL, HCURSOR hCursor = NULL, HBRUSH hBg = NULL);
	virtual	BOOL		OnInit(LPARAM lParam);
	virtual BOOL		OnCreate(LPCREATESTRUCT lpcs);
	virtual LRESULT		OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & fHandle);
	virtual VOID		OnDestroy();
	virtual VOID		OnPaint(HDC hDC, BOOL fErase, PRECT prc);
	virtual VOID		OnEraseBg(HDC hDC);
	virtual BOOL		OnClose();
	virtual VOID		OnSize(INT dwState, USHORT cx, USHORT cy);
	virtual VOID		OnSizing(INT dwState, USHORT cx, USHORT cy);
	virtual INT_PTR		OnNotify(LPNMHDR lpnm);
	virtual VOID		OnButton(USHORT uId);
	virtual VOID		OnOK();
	virtual VOID		OnCancel();
	virtual VOID		OnCommand(USHORT uCmd, USHORT uId, HWND hCtl);
	virtual VOID		OnTimer(DWORD_PTR idTimer, FARPROC pTimerProc);

	INT_PTR				dSendMessage(USHORT uId, UINT uMsg, WPARAM wParam = NULL, LPARAM lParam = NULL);
	INT_PTR				wSendMessage(UINT uMsg, WPARAM wParam = NULL, LPARAM lParam = NULL);

	HFONT				SetWndFont(HFONT hFont);

	VOID				pushctl( CControl & ctl );

	HWND				InternalCreate(HACCEL hAccel, DWORD dwStyle, DWORD dwStyleEx, LPCTSTR pszCaption, HWND hParent, USHORT uId);
	HWND				InternalCreate(HACCEL hAccel, DWORD dwStyle, DWORD dwStyleEx, LPCTSTR pszCaption, int x, int y, int cx, int cy, HWND hParent, USHORT uId);

	VOID				Close(INT_PTR uExitCode = 0);
	HINSTANCE			GetInstance();

	INT					MsgBox(LPCTSTR pszText, LPCTSTR pszCaption, UINT uType);
	VOID				ShowMessage(LPCTSTR pszText);
	VOID				ShowMessageF(LPCTSTR pszText, ...);
	VOID				ShowAlert(LPCTSTR pszText);
	VOID				ShowAlertF(LPCTSTR pszText, ...);
	VOID				ShowError(LPCTSTR pszText);
	VOID				ShowErrorF(LPCTSTR pszText, ...);
	BOOL				Confirm(LPCTSTR pszText, BOOL fDefaultNo = FALSE);
	BOOL				ConfirmF(LPCTSTR pszText, BOOL fDefaultNo, ...);

	tstring				OpenFile(LPCTSTR pszDefaultFilename, ...);
	tstring				SaveFile(LPCTSTR pszDefailtFilename, ...);

public:
	CWindow(HINSTANCE, tstring sClass, BOOL fAddDefaultButtons = FALSE);
	virtual				~CWindow();
	
	virtual LRESULT		ShowCustom( CWindow * pParent = NULL, LPARAM lParam = 0,
									LPCTSTR pszCaption = NULL,
									DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION | WS_DLGFRAME | WS_SYSMENU | WS_CLIPCHILDREN,
									DWORD dwStyleEx = 0, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int cx = CW_USEDEFAULT, int cy = CW_USEDEFAULT );

	virtual LRESULT		ShowCustomAccel( HACCEL hAccel, CWindow * pParent = NULL, LPARAM lParam = 0,
									LPCTSTR pszCaption = NULL,
									DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION | WS_DLGFRAME | WS_SYSMENU | WS_CLIPCHILDREN,
									DWORD dwStyleEx = 0, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int cx = CW_USEDEFAULT, int cy = CW_USEDEFAULT );

	virtual LRESULT		ShowCustomAccel( LPCTSTR pszAccelName, CWindow * pParent = NULL, LPARAM lParam = 0,
										 LPCTSTR pszCaption = NULL,
										 DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION | WS_DLGFRAME | WS_SYSMENU | WS_CLIPCHILDREN,
										 DWORD dwStyleEx = 0, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int cx = CW_USEDEFAULT, int cy = CW_USEDEFAULT );

	virtual LRESULT		ShowCustomAccel( UINT uAccelResId, CWindow * pParent = NULL, LPARAM lParam = 0,
										 LPCTSTR pszCaption = NULL,
										 DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION | WS_DLGFRAME | WS_SYSMENU | WS_CLIPCHILDREN,
										 DWORD dwStyleEx = 0, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int cx = CW_USEDEFAULT, int cy = CW_USEDEFAULT );

	virtual LRESULT		ShowAccel( HACCEL hAccel, CWindow * pParent = NULL, LPARAM lParam = 0 );
	virtual HWND		CreateAccel( HACCEL hAccel, CWindow * pParent = NULL, LPARAM lParam = 0 );
	virtual LRESULT		ShowAccel( LPCTSTR pszAccelName, CWindow * pParent = NULL, LPARAM lParam = 0 );
	virtual HWND		CreateAccel( LPCTSTR pszAccelName, CWindow * pParent = NULL, LPARAM lParam = 0 );
	virtual LRESULT		ShowAccel( UINT uAccelResId, CWindow * pParent = NULL, LPARAM lParam = 0 );
	virtual HWND		CreateAccel( UINT uAccelResId, CWindow * pParent = NULL, LPARAM lParam = 0 );


	virtual LRESULT		Show(CWindow * pParent = NULL, LPARAM lParam = 0);
	virtual HWND		Create(CWindow * pParent = NULL, LPARAM lParam = 0);
	UINT_PTR			MessageLoop();

	HWND				Create(DWORD dwStyle, DWORD dwStyleEx, LPCTSTR pszCaption, CWindow * pParent, USHORT uId);
	HWND				CreateAccel( HACCEL hAccel, DWORD dwStyle, DWORD dwStyleEx, LPCTSTR pszCaption, CWindow * pParent, USHORT uId );
	HWND				CreateAccel( LPCTSTR pszAccelName, DWORD dwStyle, DWORD dwStyleEx, LPCTSTR pszCaption, CWindow * pParent, USHORT uId );
	HWND				CreateAccel( UINT uAccelRes, DWORD dwStyle, DWORD dwStyleEx, LPCTSTR pszCaption, CWindow * pParent, USHORT uId );
	HWND				Create(DWORD dwStyle, DWORD dwStyleEx, LPCTSTR pszCaption, int x, int y, int cx, int cy, CWindow * pParent, USHORT uId);
	HWND				CreateAccel( HACCEL hAccel, DWORD dwStyle, DWORD dwStyleEx, LPCTSTR pszCaption, int x, int y, int cx, int cy, CWindow * pParent, USHORT uId );
	HWND				CreateAccel( LPCTSTR pszAccelName, DWORD dwStyle, DWORD dwStyleEx, LPCTSTR pszCaption, int x, int y, int cx, int cy, CWindow * pParent, USHORT uId );
	HWND				CreateAccel( UINT uAccelRes, DWORD dwStyle, DWORD dwStyleEx, LPCTSTR pszCaption, int x, int y, int cx, int cy, CWindow * pParent, USHORT uId );

	CWindow *			Parent();
	operator			HWND();
	operator			HWND() const;
	VOID				Visible(BOOL fVisible);
	BOOL				Visible();
	VOID				Enable(BOOL fEnable);
	BOOL				Enable();

	VOID				EnableAll(BOOL fEnable, ...);
	VOID				ShowAll(BOOL fVisible, ...);
	HWND				Ctl(INT id);
	VOID				DialogStyleWindow( BOOL fDialogStyle );
	BOOL				IsDialogStyleWindow();
	VOID				TitleText( const tstring & sText );
	tstring				TitleText() const;
};

class CDialog: public CWindow
{
	CDialogTemplate			m_dt;

	BOOL					m_fTemplateInstalled;
	BOOL					m_fAddedDefaultButtons;

	CControl				m_cOk;
	CControl				m_cCancel;

	VOID					InitFromTemplate();

	friend					CPropSheet;

public:
	CDialog(HINSTANCE hInstance, LPCTSTR pszResource);
	CDialog(HINSTANCE hInstance, DWORD dwStyle, DWORD dwExStyle, 
		INT x, INT y, INT cx, INT cy, LPCTSTR pszTitle, USHORT uMenuId, LPCTSTR pszClassName = NULL);
	CDialog(HINSTANCE hInstance, INT cx, INT cy, LPCTSTR pszTitle, BOOL fAddDefaultButtons = FALSE, BOOL fSizeable = FALSE, BOOL fUseIcon = FALSE, BOOL fCentered = FALSE);

	VOID					AddDefaultButtons();

	LRESULT		Show(CWindow * pParent = NULL, LPARAM lParam = 0);
	HWND		Create(CWindow * pParent = NULL, LPARAM lParam = 0);
	HWND		CreateAccel( HACCEL hAccel, CWindow * pParent = NULL, LPARAM lParam = 0 );
	HWND		CreateAccel( LPCTSTR pszAccelName, CWindow * pParent = NULL, LPARAM lParam = 0 );
	HWND		CreateAccel( UINT uAccelResId, CWindow * pParent = NULL, LPARAM lParam = 0 );
};

template<class T> class CCustomControl: public CWindow
{
	static LRESULT CALLBACK SysWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch(uMsg)
		{
		case WM_NCCREATE:
				return CWindow::CreateExternal(hWnd, (LPCREATESTRUCT) lParam, new T(*((LPCREATESTRUCT) lParam), hWnd));
		default:
				return CWindow::SysWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}

	virtual VOID	OnCancel() { return; }	// do nothing

protected:
	CCustomControl(CREATESTRUCT & cs, HWND hWnd)
		: CWindow(cs, hWnd) { }

	static VOID Register(tstring sClassName, HINSTANCE hInstance, DWORD style, HICON hIcon = NULL, HCURSOR hCursor = NULL, HBRUSH hBg = NULL)
	{
		CWindow::Register(hInstance, style, CCustomControl<T>::SysWindowProc, sClassName, hIcon, hCursor, hBg);
	}
};

class CEditControl: public CControl
{

public:
	CEditControl(HINSTANCE hInstance, USHORT uId, LPCTSTR pszText, INT x, INT y, INT cx, INT cy, BOOL fPassword = FALSE, BOOL fDisabled = FALSE, BOOL fReadOnly = FALSE, DWORD dwStyle = WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | WS_BORDER, DWORD dwExStyle = 0, WORD wAnchor = 0)
		: CControl( hInstance, uId, WC_EDIT, pszText, x, y, cx, cy, dwStyle | ( fDisabled ? WS_DISABLED : 0 ) | ( fPassword ? ES_PASSWORD : 0 ) | ( fReadOnly ? ES_READONLY : 0 ), dwExStyle, wAnchor ) {}
	
	CEditControl() {}
};

class CMultilineEditControl: public CControl
{

public:
	CMultilineEditControl(HINSTANCE hInstance, USHORT uId, LPCTSTR pszText, INT x, INT y, INT cx, INT cy, BOOL fDisabled = FALSE, BOOL fReadOnly = FALSE,
			DWORD dwStyle = WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | WS_TABSTOP | ES_WANTRETURN | WS_BORDER, DWORD dwExStyle = 0, WORD wAnchor = 0)
		: CControl( hInstance, uId, WC_EDIT, pszText, x, y, cx, cy, dwStyle | ( fDisabled ? WS_DISABLED : 0 ) | ( fReadOnly ? ES_READONLY : 0 ), dwExStyle, wAnchor ) { }

	CMultilineEditControl() {}
};

class CButtonControl: public CControl
{

public:
	CButtonControl(HINSTANCE hInstance, USHORT uId, LPCTSTR pszText, INT x, INT y, INT cx, INT cy, BOOL fDefault = FALSE, BOOL fDisabled = FALSE, WORD wAnchor = 0)
		: CControl( hInstance, uId, WC_BUTTON, pszText, x, y, cx, cy, WS_VISIBLE | WS_TABSTOP | ( fDefault ? BS_DEFPUSHBUTTON : 0 ) | ( fDisabled ? WS_DISABLED : 0 ), 0, wAnchor ) { }

	CButtonControl() {}
};

class CSplitButtonControl : public CControl
{

public:
	CSplitButtonControl( HINSTANCE hInstance, USHORT uId, LPCTSTR pszText, INT x, INT y, INT cx, INT cy, BOOL fDefault = FALSE, BOOL fDisabled = FALSE, WORD wAnchor = 0 )
		: CControl( hInstance, uId, WC_BUTTON, pszText, x, y, cx, cy, 
					WS_VISIBLE | WS_TABSTOP | ( fDefault ? BS_DEFSPLITBUTTON : BS_SPLITBUTTON ) | ( fDisabled ? WS_DISABLED : 0 ), 
					0, wAnchor ) { }

	CSplitButtonControl() {}
};


class CFrameControl: public CControl
{

public:
	CFrameControl(HINSTANCE hInstance, LPCTSTR pszText, INT x, INT y, INT cx, INT cy, BOOL fDisabled = FALSE, USHORT uId = 0xFFFF, WORD wAnchor = 0)
		: CControl(hInstance, uId, WC_BUTTON, pszText, x, y, cx, cy, WS_VISIBLE | BS_GROUPBOX | ( fDisabled ? WS_DISABLED : 0 ), 0, wAnchor) { }

	CFrameControl() {}
};

class CCheckBoxControl: public CControl
{

public:
	CCheckBoxControl(HINSTANCE hInstance, USHORT uId, LPCTSTR pszText, INT x, INT y, INT cx, INT cy, BOOL fAuto = TRUE, BOOL fTriState = FALSE, BOOL fDisabled = FALSE, WORD wAnchor = 0)
		: CControl(hInstance, uId, WC_BUTTON, pszText, x, y, cx, cy, WS_VISIBLE | WS_TABSTOP  | ( fDisabled ? WS_DISABLED : 0 ) | 
			( fAuto ? ( fTriState ? BS_AUTO3STATE : BS_AUTOCHECKBOX ) : ( fTriState ? BS_3STATE : BS_CHECKBOX ) ), 0, wAnchor) { } 

	CCheckBoxControl() {}
	
	BOOL		Check() { return (BOOL) ( 0 != cSendMessage( BM_GETCHECK ) ); }
	BOOL		IsUndetermined() { return ( BST_INDETERMINATE == cSendMessage( BM_GETCHECK ) ); }
	VOID		Check(BOOL fCheck) { cSendMessage(BM_SETCHECK, ( fCheck ? BST_CHECKED : 0 ) ); }
	VOID		Check3() { cSendMessage(BM_SETCHECK, BST_INDETERMINATE); }
};

class CRadioControl: public CControl
{

public:
	CRadioControl(HINSTANCE hInstance, USHORT uId, LPCTSTR pszText, INT x, INT y, INT cx, INT cy, BOOL fAuto = TRUE, BOOL fDisabled = FALSE, WORD wAnchor = 0)
		: CControl(hInstance, uId, WC_BUTTON, pszText, x, y, cx, cy, WS_VISIBLE | WS_TABSTOP | ( fDisabled ? WS_DISABLED : 0 ) | 
			( fAuto ? BS_AUTORADIOBUTTON : BS_RADIOBUTTON ), 0, wAnchor) { } 
	CRadioControl() {}

	BOOL		Check() { return (BOOL) ( 0 != cSendMessage( BM_GETCHECK ) ); }
	VOID		Check(BOOL fCheck) { cSendMessage(BM_SETCHECK, ( fCheck ? BST_CHECKED : 0 ) ); }
};

class CStaticControl: public CControl
{

public:
	CStaticControl(HINSTANCE hInstance, LPCTSTR pszText, INT x, INT y, INT cx, INT cy, BOOL fDisabled = FALSE, DWORD dwStyle = WS_VISIBLE | SS_RIGHT, DWORD dwExStyle = 0, USHORT uId = 0xFFFF, WORD wAnchor = 0)
	  : CControl(hInstance, uId, WC_STATIC, pszText, x, y, cx, cy, dwStyle | ( fDisabled ? WS_DISABLED : 0 ), dwExStyle, wAnchor) { }

	CStaticControl() {}
};

class CUpDownControl: public CControl
{

public:
	CUpDownControl(HINSTANCE hInstance, USHORT uId, BOOL fDisabled = FALSE)
		: CControl(hInstance, uId, UPDOWN_CLASS, NULL, 0, 0, 0, 0, WS_VISIBLE | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_AUTOBUDDY | UDS_SETBUDDYINT | UDS_WRAP | ( fDisabled ? WS_DISABLED : 0 )) { }

	CUpDownControl() {};

	VOID	SetRange(int a, int b) { cSendMessage(UDM_SETRANGE32, b, a); }
	BOOL	IsError() { BOOL fError = FALSE; cSendMessage(UDM_GETPOS32, 0, (LPARAM) &fError); return fError; }
	INT		Value() { return (INT) cSendMessage( UDM_GETPOS32 ); }
	VOID	Value(DWORD uValue) { cSendMessage(UDM_SETPOS32, 0, uValue); }
};