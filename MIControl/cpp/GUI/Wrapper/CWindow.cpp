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

BOOL __handle_accel_table( HWND hActiveWindow, MSG & msg )
{
    if ( !hActiveWindow )
        return FALSE;

    return IsDialogMessage( hActiveWindow, &msg );
}

UINT_PTR __message_loop()
{
    MSG msg = { 0 };
    while ( GetMessage( &msg, nullptr, 0, 0 ) > 0 )
    {
        HWND hActiveWindow = GetActiveWindow();

        if ( !__handle_accel_table( hActiveWindow, msg ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    };

    return msg.wParam;
}

// EOF ACCEL TABLE AREA

CControl::CControl(HINSTANCE hInstance)
    : m_hInstance( hInstance ), m_hWnd( NULL ), m_hParent( NULL )
{
    m_pre.uId = 0; 
    ZeroMemory(&m_pre.pt, sizeof(m_pre.pt));
    ZeroMemory(&m_pre.sz, sizeof(m_pre.sz));
    m_pre.dwStyle = m_pre.dwStyleEx = 0;
}

CControl::CControl()
    : m_hInstance( NULL ), m_hWnd( NULL ), m_hParent( NULL )
{
    m_pre.uId = 0; 
    ZeroMemory(&m_pre.pt, sizeof(m_pre.pt));
    ZeroMemory(&m_pre.sz, sizeof(m_pre.sz));
    m_pre.dwStyle = m_pre.dwStyleEx = 0;
}

CControl::CControl(HINSTANCE hInstance, USHORT uId, 
        LPCTSTR pszClass, LPCTSTR pszTitle, 
        INT x, INT y, INT cx, INT cy,
        DWORD dwStyle, DWORD dwStyleEx, WORD wAnchor)
        : m_hInstance( hInstance ), m_hWnd( NULL ), m_hParent( NULL )
{
    PostInit(hInstance, uId, pszClass, pszTitle, x, y, cx, cy, dwStyle, dwStyleEx, wAnchor);
}

CControl::~CControl()
{

}

VOID CControl::PostInit(HINSTANCE hInstance, USHORT uId, 
        LPCTSTR pszClass, LPCTSTR pszTitle, 
        INT x, INT y, INT cx, INT cy,
        DWORD dwStyle, DWORD dwStyleEx, WORD wAnchor)
{
    m_pre.uId = uId;
    m_pre.sClass = ( pszClass ? pszClass : TEXT("") );
    m_pre.sTitle = ( pszTitle ? pszTitle : TEXT("") );
    m_pre.pt.x = x; m_pre.pt.y = y;
    m_pre.sz.cx = cx; m_pre.sz.cy = cy;
    m_pre.dwStyle = dwStyle; m_pre.dwStyleEx = dwStyleEx;
}


VOID CControl::InitCtl()
{
    if ( !m_hWnd || !IsWindow( m_hWnd ) )
        return;

#ifdef UNICODE
    cSendMessage(CCM_SETUNICODEFORMAT, TRUE);
#endif

    PostCreate();
}

VOID CControl::PostCreate()
{
    // need for derived classes
    return;
}

HWND CControl::Create(HWND hParent, USHORT uId, 
                            int x, int y, int cx, int cy,
                            LPCTSTR pszClass, LPCTSTR pszTitle,
                            DWORD dwStyle, DWORD dwStyleEx)
{
    if ( !IsWindow(hParent) )
        return NULL;

    m_hParent = hParent;
    m_hWnd = CreateWindowEx(dwStyleEx, pszClass, pszTitle, dwStyle | WS_CHILD, x, y, cx, cy, hParent, (HMENU) uId, m_hInstance, CreateStruct() ); 
    InitCtl();
    return m_hWnd;
}

HWND CControl::Create(CWindow * pWnd, BOOL fPreCreate)
{
    if ( !pWnd )
        return NULL;

    HWND hParent = *pWnd;
    if ( !IsWindow(hParent) )
        return NULL;

    if ( fPreCreate )
    {
        union
        {
            RECT rc;
            POINT pt[2];
        } ctl_rc = { 0 };
        m_hWnd = GetDlgItem( hParent, m_pre.uId );
        if ( !m_hWnd ) 
            return NULL;

        m_hParent = hParent;

        GetWindowRect( m_hWnd, &ctl_rc.rc );
        ScreenToClient(m_hParent, &ctl_rc.pt[0]);
        ScreenToClient(m_hParent, &ctl_rc.pt[1]);
        m_pre.rc.left = ctl_rc.rc.left;
        m_pre.rc.top = ctl_rc.rc.top;
        m_pre.rc.right = ctl_rc.rc.right;
        m_pre.rc.bottom = ctl_rc.rc.bottom;
    }

    RECT rcw = { 0 };
    RECT rcc = m_pre.rc;

    GetClientRect(hParent, &rcw);

    if ( fPreCreate )
    {
        InitCtl();
        return m_hWnd;
    }

    return Create(hParent, 
        m_pre.uId, m_pre.rc.left, m_pre.rc.top, m_pre.rc.right - m_pre.rc.left, m_pre.rc.bottom - m_pre.rc.top, 
        m_pre.sClass.c_str(), m_pre.sTitle.c_str(), m_pre.dwStyle, m_pre.dwStyleEx);
}

HWND CControl::Parent()
{
    return m_hParent;
}

const PVOID CControl::CreateStruct() const
{
    return nullptr;
}

VOID CControl::UpdateSize(INT cx, INT cy)
{
    if ( m_pre.pt.x < 0 )
        m_pre.rc.left = m_pre.pt.x + cx;
    else
        m_pre.rc.left = m_pre.pt.x;

    if ( m_pre.pt.y < 0 )
        m_pre.rc.top = m_pre.pt.y + cy;
    else
        m_pre.rc.top = m_pre.pt.y;

    if ( m_pre.sz.cx < 0 )
        m_pre.rc.right = ( cx + m_pre.sz.cx );
    else
        m_pre.rc.right = m_pre.rc.left + m_pre.sz.cx;

    if ( m_pre.sz.cy < 0 )
        m_pre.rc.bottom = ( cy + m_pre.sz.cy );
    else
        m_pre.rc.bottom = m_pre.rc.top + m_pre.sz.cy;
}

INT_PTR CControl::cSendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return SendMessage(m_hWnd, uMsg, wParam, lParam);
}

const INT_PTR CControl::cSendMessage( UINT uMsg, WPARAM wParam, LPARAM lParam ) const
{
    return SendMessage( m_hWnd, uMsg, wParam, lParam );
}

CControl::operator HWND()
{
    return m_hWnd;
}

VOID CControl::Enable(BOOL fEnable)
{
    EnableWindow(*this, fEnable);
}

BOOL CControl::Enable()
{
    return IsWindowEnabled(*this);
}

VOID CControl::Visible(BOOL fVisible)
{
    ShowWindow(*this, ( fVisible ? SW_SHOW : SW_HIDE ));
}

BOOL CControl::Visible()
{
    return IsWindowVisible(*this);
}

tstring CControl::Text()
{
    INT_PTR len = cSendMessage(WM_GETTEXTLENGTH) + 1;
    tstring str;
    str.append(len, 0);
    cSendMessage(WM_GETTEXT, len + 1, (LPARAM) &str[0]);
    return str;
}

VOID CControl::Text(tstring sText)
{
    cSendMessage(WM_SETTEXT, 0, (LPARAM) sText.c_str());
}


// window

CDialog::CDialog(HINSTANCE hInstance, LPCTSTR pszResource)
    : CWindow(hInstance, (LONG_PTR) pszResource),
    m_fTemplateInstalled( FALSE ), m_fAddedDefaultButtons( FALSE )
{

}

CDialog::CDialog(HINSTANCE hInstance, DWORD dwStyle, DWORD dwExStyle, 
        INT x, INT y, INT cx, INT cy, LPCTSTR pszTitle, USHORT uMenuId, LPCTSTR pszClassName)
    : CWindow(hInstance, (LONG_PTR) 0),
    m_fTemplateInstalled( FALSE ), m_fAddedDefaultButtons( FALSE )
{
    if ( pszClassName )
        m_dt.SetDlgClass( pszClassName );

    m_dt.SetDlgPos(x, y, cx, cy);
    m_dt.SetDlgStyle( dwStyle, dwExStyle );
    m_dt.SetDlgTitle( pszTitle );
}

CDialog::CDialog(HINSTANCE hInstance, INT cx, INT cy, LPCTSTR pszTitle, BOOL fAddDefaultButtons, BOOL fSizeable, BOOL fUseIcon, BOOL fCentered)
    : CWindow(hInstance, (LONG_PTR) 0),
    m_fTemplateInstalled( FALSE ), m_fAddedDefaultButtons( FALSE )
{
    m_dt.SetDlgPos(0, 0, cx, cy);
    m_dt.SetDlgTitle( pszTitle );

    if ( fSizeable )
        m_dt.AddStyle( WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_CLIPCHILDREN );
    else if ( !fUseIcon )
        m_dt.SetDlgStyle( DS_SHELLFONT | WS_CAPTION | WS_SYSMENU | WS_OVERLAPPED, WS_EX_DLGMODALFRAME );

    if (fCentered)
        m_dt.AddStyle(DS_CENTER);

    m_cOk.PostInit( hInstance, IDOK, WC_BUTTON, TEXT("&OK"), -50 * 2 - 7 * 2, -15 - 7,
        50, 15, BS_DEFPUSHBUTTON | WS_VISIBLE | WS_TABSTOP, 0);

    m_cCancel.PostInit( hInstance, IDCANCEL, WC_BUTTON, TEXT("&Cancel"), -50 - 7, -15 - 7,
        50, 15, WS_VISIBLE | WS_TABSTOP, 0);

    if ( fAddDefaultButtons )
    {
        AddDefaultButtons();
    }
}

VOID CDialog::AddDefaultButtons()
{
    if ( !m_fAddedDefaultButtons )
    {
        pushctl( m_cCancel );
        pushctl( m_cOk );
        m_fAddedDefaultButtons = TRUE;
    }
}

VOID CDialog::InitFromTemplate()
{
    if ( m_fTemplateInstalled )
        return;

    m_fTemplateInstalled = TRUE;

    size_t nCtls = m_vCtl.size();
    for(size_t n = 0; n < nCtls; ++n)
    {
        CControl * pCtl = m_vCtl[ n ];
        
        m_dt.AddItem( pCtl->m_pre.uId, pCtl->m_pre.sClass.c_str(), pCtl->m_pre.sTitle.c_str(),
            pCtl->m_pre.pt.x, pCtl->m_pre.pt.y, 
            pCtl->m_pre.sz.cx,
            pCtl->m_pre.sz.cy,
            pCtl->m_pre.dwStyle, pCtl->m_pre.dwStyleEx);			
    }
}

LRESULT CDialog::Show(CWindow * pParent, LPARAM lParam)
{
    HWND hParent = ( pParent ? pParent->m_hWnd : NULL );
    m_pParent = pParent;
    m_fModal = TRUE;

    if ( m_idRes )
    {
        return DialogBoxParam(m_hInstance, (LPCTSTR) m_idRes, hParent, CWindow::SysDialogProc, (LPARAM) this);
    }
    else
    {
        InitFromTemplate();

        return DialogBoxIndirectParam( 
            m_hInstance, m_dt.Template(), hParent, CWindow::SysDialogProc, (LPARAM) this);
    }
}

HWND CDialog::CreateAccel( HACCEL hAccel, CWindow * pParent, LPARAM lParam )
{
    HWND hParent = ( pParent ? pParent->m_hWnd : NULL );
    HWND hDialog = NULL;
    m_pParent = pParent;
    m_fModal = FALSE;

    if ( m_idRes )
    {
        hDialog = CreateDialogParam( m_hInstance, MAKEINTRESOURCE( m_idRes ), hParent, CWindow::SysDialogProc, (LPARAM)this );
    }
    else
    {
        InitFromTemplate();

        hDialog = CreateDialogIndirectParam(
            m_hInstance, m_dt.Template(), hParent, CWindow::SysDialogProc, (LPARAM)this );
    }

    return hDialog;
}

HWND CDialog::Create(CWindow * pParent, LPARAM lParam)
{
    return CreateAccel( (HACCEL)NULL, pParent, lParam );
}

HWND CDialog::CreateAccel( LPCTSTR pszAccelName, CWindow * pParent, LPARAM lParam )
{
    return CreateAccel( LoadAccelerators( GetInstance(), pszAccelName ), pParent, lParam );
}

HWND CDialog::CreateAccel( UINT uAccelResId, CWindow * pParent, LPARAM lParam )
{
    return CreateAccel( MAKEINTRESOURCE( uAccelResId ), pParent, lParam );
}

CWindow::CWindow(HINSTANCE hInstance, tstring sClass, BOOL fAddDefaultButtons)
    : m_hInstance(hInstance), m_sClass(sClass), m_hWnd(NULL), m_fDialogMode( FALSE ), m_fModal( FALSE ), m_idRes( 0 ), m_fExternal( FALSE ), m_pParent(NULL),
    m_fInFocus( FALSE ), m_fWindowModal( FALSE ), m_uWindowModalExitCode( 0 ), m_fSelfHandle( FALSE ), m_ftWindowType( ftWindow ),
    m_cDefaultOk(hInstance, IDOK, WC_BUTTON, TEXT("&OK"),
            -(10 + 75 + 10 + 75), -(10 + 25), 75, 25, WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 0),
    m_cDefaultCancel(hInstance, IDCANCEL, WC_BUTTON, TEXT("&Cancel"),
            -(10 + 75), -(10 + 25), 75, 25, WS_TABSTOP | WS_VISIBLE | BS_PUSHBUTTON, 0)
{
    if ( fAddDefaultButtons )
    {
        pushctl(m_cDefaultCancel);
        pushctl(m_cDefaultOk);
    }
}

CWindow::CWindow( HINSTANCE hInstance, LONG_PTR idRes )
    : m_hInstance( hInstance ), m_hWnd( NULL ), m_fDialogMode( TRUE ), m_idRes( idRes ), m_fModal( FALSE ), m_fExternal( FALSE ), m_pParent( NULL ),
    m_fInFocus( FALSE ), m_fWindowModal( FALSE ), m_uWindowModalExitCode( 0 ), m_fSelfHandle( FALSE ), m_ftWindowType( ftWindow )
{

}

CWindow::CWindow(CREATESTRUCT cs, HWND hWnd)
    : m_hInstance(cs.hInstance), m_hWnd(hWnd), m_fDialogMode( FALSE ), m_idRes( 0 ), m_fModal( FALSE ), m_fExternal( TRUE ),
    m_fInFocus( FALSE ), m_fWindowModal( FALSE ), m_uWindowModalExitCode( 0 ), m_fSelfHandle( FALSE ), m_ftWindowType( ftWindow )
{
    if ( cs.hwndParent )
    {
        m_pParent = (CWindow*) GetWindowLongPtr(cs.hwndParent, 4);
        m_sClass = ( cs.lpszClass ? cs.lpszClass : TEXT("") );
    }
}

CWindow::~CWindow()
{
    if ( m_hWnd )		// warning: window still not destroyed, but we cant call OnDestroy and OnNCDestroy at this point
    {
        DestroyWindow(m_hWnd);
    }
}

LRESULT CWindow::CreateExternal(HWND hWnd, LPCREATESTRUCT lpcs, CWindow * This)
{
    if ( This )
    {
        CREATESTRUCT cs = *lpcs;
        cs.lpCreateParams = This;
        if ( CWindow::SysWindowProc(hWnd, WM_NCCREATE, 0, (LPARAM) &cs) )
            return TRUE;

        delete This;
        This = NULL;
    }
    return FALSE;
}

LRESULT CALLBACK CWindow::SysWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if ( WM_NCCREATE == uMsg )
    {
        auto pcs = ( (LPCREATESTRUCT)lParam );  //(CWindow*) ((LPCREATESTRUCT) lParam)->lpCreateParams;
        CWindow * This = (CWindow*) pcs->lpCreateParams;
        if ( pcs->dwExStyle & WS_EX_MDICHILD )
        {
            This = (CWindow*) ((LPMDICREATESTRUCT)( pcs->lpCreateParams ))->lParam;
        }
        
        if ( This )
        {
            This->m_hWnd = hWnd;
            if ( This->WindowProc(uMsg, wParam, lParam) )
            {
                SetWindowLongPtr(hWnd, 8, (LONG_PTR) This);
                return TRUE;
            }
            This->m_hWnd = NULL;
        }
        return FALSE;
    }
    else
    {
        CWindow * This = (CWindow*) GetWindowLongPtr(hWnd, 8);
        if ( This )
            return This->WindowProc(uMsg, wParam, lParam);
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK CWindow::SysDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if ( WM_INITDIALOG == uMsg )
    {
        CWindow * This = (CWindow*) lParam;
        if ( This )
        {
            This->m_hWnd = hWnd;

            LRESULT lResult = This->WindowProc(uMsg, wParam, lParam);
            SetWindowLongPtr(hWnd, DWL_USER, (LONG_PTR) This);
            return lResult;
        }
        return FALSE;
    }
    else
    {
        CWindow * This = (CWindow*) GetWindowLongPtr(hWnd, DWL_USER);
        if ( This )
            return This->WindowProc(uMsg, wParam, lParam);
    }

    return 0;
}

LRESULT CWindow::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fHandle = FALSE;
    LRESULT lRet = OnMessage(uMsg, wParam, lParam, fHandle);
    if ( fHandle )
        return lRet;

    switch(uMsg)
    {
    case WM_CREATE: return InternalOnCreate((LPCREATESTRUCT) lParam);
    case WM_INITDIALOG: return InternalOnInit();
    case WM_DESTROY: OnDestroy(); break;
    case WM_PAINT: InternalOnPaint(wParam, lParam); break;
    case WM_CLOSE: InternalOnClose(); break;
    case WM_ERASEBKGND: OnEraseBg((HDC) wParam); break;
    case WM_SIZE: InternalOnSize(wParam, lParam); break;
    case WM_SIZING: InternalOnSizing(wParam, lParam); break;
    case WM_NOTIFY: return InternalOnNotify((LPNMHDR) lParam);
    case WM_COMMAND: InternalOnCommand(HIWORD(wParam), LOWORD(wParam), (HWND) lParam); break;
    case WM_GETCUSTOMINSTANCE: return (LRESULT) this;
    case WM_TIMER: OnTimer(wParam, (FARPROC) lParam); break;
    case WM_VSCROLL:
    case WM_NCDESTROY:
        {
            m_hWnd = NULL;
            m_pParent = NULL;

            if ( IsWindowModal() )
            {
                PostQuitMessage( LODWORD( m_uWindowModalExitCode ) );
            }
            
            if ( m_fExternal )
                delete this;

            return 0;
        }
    default:
        {
            return WindowProcDefault( uMsg, wParam, lParam );
        }
    }

    return 0;
}

LRESULT CWindow::WindowProcDefault( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch ( m_ftWindowType )
    {
        case ftWindow: return ( m_fDialogMode ? 0 : DefWindowProc( *this, uMsg, wParam, lParam ) );
    }

    return 0;
}

VOID CWindow::pushctl( CControl & ctl )
{
    m_vCtl.push_back( &ctl );	// store a pointer to control instead of control, because vector creates a new instance of control
}

BOOL CWindow::InternalOnCreate(LPCREATESTRUCT lpcs)
{
    size_t count = m_vCtl.size();
    RECT wrc = { 0 };
    GetClientRect(*this, &wrc);

    for(size_t n = 0; n < count; ++n)
    {
        m_vCtl[n]->UpdateSize(wrc.right - wrc.left, wrc.bottom - wrc.top);
        m_vCtl[n]->Create(this, m_fDialogMode);
    }

    return OnCreate(lpcs);
}

BOOL CWindow::InternalOnInit()
{
    size_t count = m_vCtl.size();
    RECT wrc = { 0 };
    GetClientRect(*this, &wrc);

    for(size_t n = 0; n < count; ++n)
    {
        m_vCtl[n]->UpdateSize(wrc.right - wrc.left, wrc.bottom - wrc.top);
        m_vCtl[n]->Create(this, m_fDialogMode);
    }

    if ( m_pParent )
    {
        SendMessage( *this, WM_SETICON, ICON_BIG, 
            (LPARAM) SendMessage( *m_pParent, WM_GETICON, ICON_BIG, 0 ) );

        SendMessage( *this, WM_SETICON, ICON_SMALL, 
            (LPARAM) SendMessage( *m_pParent, WM_GETICON, ICON_SMALL, 0 ) );
    }

    return OnInit(m_lDlgParam);
}

VOID CWindow::InternalOnPaint(WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps = { 0 };
    HDC hDC = BeginPaint(*this, &ps);
    if ( hDC )
    {
        OnPaint(hDC, ps.fErase, &ps.rcPaint);
        EndPaint(*this, &ps);
    }
}

VOID CWindow::InternalOnClose()
{
    if ( OnClose() )
        Close();
}

VOID CWindow::InternalOnSize(WPARAM wParam, LPARAM lParam)
{
    USHORT cx = GET_X_LPARAM(lParam), cy = GET_Y_LPARAM(lParam);
    OnSize((INT) wParam, cx, cy);
}

VOID CWindow::InternalOnSizing(WPARAM wParam, LPARAM lParam)
{
    RECT & rc = *((PRECT) lParam);
    USHORT cx = LOWORD(rc.right - rc.left), cy = LOWORD(rc.bottom - rc.top);
    OnSizing((INT) wParam, cx, cy);
}

INT_PTR CWindow::InternalOnNotify(LPNMHDR lpnm)
{
    INT_PTR ret = OnNotify(lpnm);

    SetWindowLongPtr(*this, DWL_MSGRESULT, ret);
    return ret;
}

VOID CWindow::InternalOnCommand(USHORT uCmd, USHORT uId, HWND hCtl)
{
    switch(uCmd)
    {
    case BN_CLICKED:
        {
            switch(uId)
            {
            case IDOK:		OnOK(); break;
            case IDCANCEL:	OnCancel(); break;
            default:		OnButton(uId); break;
            }
            break;
        }
    default: OnCommand(uCmd, uId, hCtl);
    }
}


BOOL CWindow::OnClose()
{
    return TRUE;
}

INT_PTR CWindow::dSendMessage(USHORT uId, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return SendDlgItemMessage(*this, uId, uMsg, wParam, lParam);
}

INT_PTR CWindow::wSendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return SendMessage(*this, uMsg, wParam, lParam);
}

BOOL CWindow::OnCreate(LPCREATESTRUCT lpcs)
{
    return FALSE;
}

BOOL CWindow::OnInit(LPARAM lParam)
{
    return TRUE;
}

VOID CWindow::OnPaint(HDC hDC, BOOL fErase, PRECT prc)
{
    // do nothing
    return;
}

VOID CWindow::OnSize(INT dwState, USHORT cx, USHORT cy)
{
    return;
}

VOID CWindow::OnSizing(INT dwState, USHORT cx, USHORT cy)
{
    return;
}

VOID CWindow::OnEraseBg(HDC hDC)
{
    DefWindowProc(*this, WM_ERASEBKGND, (WPARAM) hDC, 0);
    return;
}

INT_PTR CWindow::OnNotify(LPNMHDR lpnm)
{
    return 0;
}

VOID CWindow::OnButton(USHORT uId)
{
    return;
}

VOID CWindow::OnOK()
{
    return;
}

VOID CWindow::OnCancel()
{
    Close(0);
}

VOID CWindow::OnCommand(USHORT uCmd, USHORT uId, HWND hCtl)
{
    return;
}

LRESULT CWindow::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & fHandle)
{
    fHandle = FALSE;
    return 0;
}

VOID CWindow::OnDestroy()
{
    return;
}

VOID CWindow::OnTimer(DWORD_PTR idTimer, FARPROC pTimerProc)
{
    return;
}


ATOM CWindow::Register(DWORD style, HICON hIcon, HCURSOR hCursor, HBRUSH hBg)
{
    return Register(m_hInstance, style, CWindow::SysWindowProc, m_sClass, hIcon, hCursor, hBg);
}

ATOM CWindow::Register(HINSTANCE hInstance, DWORD style, WNDPROC lpfnCustomProcm, tstring sClass, HICON hIcon, HCURSOR hCursor, HBRUSH hBg)
{
    WNDCLASS wc = { 0 };
    wc.style = style;
    wc.cbClsExtra = 16;
    wc.cbWndExtra = 16;
    wc.hbrBackground = hBg;
    wc.hCursor = hCursor;
    wc.hIcon = hIcon;
    wc.hInstance = hInstance;
    wc.lpfnWndProc = lpfnCustomProcm;
    wc.lpszClassName = sClass.c_str();
    wc.lpszMenuName = NULL;
    return RegisterClass(&wc);
}

HWND CWindow::InternalCreate( HACCEL hAccel, DWORD dwStyle, DWORD dwStyleEx, LPCTSTR pszCaption, HWND hParent, USHORT uId)
{
    return InternalCreate( hAccel, dwStyle, dwStyleEx, pszCaption, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hParent, uId );
}

HWND CWindow::InternalCreate( HACCEL hAccel, DWORD dwStyle, DWORD dwStyleEx, LPCTSTR pszCaption, int x, int y, int cx, int cy, HWND hParent, USHORT uId)
{
    m_fModal = FALSE;
    HWND hwnd = CreateWindowEx( dwStyleEx, m_sClass.c_str(), pszCaption, dwStyle, x, y, cx, cy, hParent, (HMENU)uId, m_hInstance, this );
    
    if ( hwnd )
    {
        SetWindowText(hwnd, pszCaption);
        return hwnd;
    }
    return NULL;
}

VOID CWindow::DialogStyleWindow( BOOL fDialogStyle )
{
    m_fSelfHandle = ( !fDialogStyle );
}

BOOL CWindow::IsDialogStyleWindow()
{
    return ( !m_fSelfHandle );
}

VOID CWindow::TitleText( const tstring & sText )
{
    m_sCaption = sText;
    if ( IsWindow( *this ) )
    {
        SetWindowText( *this, sText.c_str() );
    }
}

tstring CWindow::TitleText() const
{
    if ( IsWindow( *this ) )
    {
        int len = GetWindowTextLength( *this );
        if ( len > 0 )
        {
            std::vector<TCHAR> _pszText( len + 1, 0 );
            GetWindowText( *this, _pszText.data(), len + 1 );
            return _pszText.data();
        }
    }
    
    return m_sCaption;
}

UINT_PTR CWindow::MessageLoop()
{
    return __message_loop();
}

BOOL CWindow::IsWindowModal()
{
    return m_fWindowModal;
}

LRESULT CWindow::ShowCustomAccel( HACCEL hAccel, CWindow * pParent, LPARAM lParam, LPCTSTR pszCaption, DWORD dwStyle, DWORD dwStyleEx, int x, int y, int cx, int cy )
{
    LRESULT lres = 0;
    m_lDlgParam = lParam;

    if ( pParent )
    {
        BOOL fSavedEnableState = IsWindowEnabled( *pParent );
        EnableWindow( *pParent, FALSE );
        HWND hSavedWnd = CreateAccel( hAccel, dwStyle, dwStyleEx, pszCaption, x, y, cx, cy, pParent, 0 );

        if ( hSavedWnd )
        {
            m_fWindowModal = TRUE;
            ShowWindow( *this, SW_SHOWNORMAL );
            UpdateWindow( *this );
            lres = MessageLoop();
            m_fWindowModal = FALSE;

        }
        EnableWindow( *pParent, fSavedEnableState );
        SetActiveWindow( *pParent );
    }
    else
    {
        CreateAccel( hAccel, dwStyle, dwStyleEx, pszCaption, x, y, cx, cy, pParent, 0 );
        if ( m_hWnd )
        {
            m_fWindowModal = TRUE;
            ShowWindow( *this, SW_SHOWNORMAL );
            UpdateWindow( *this );
            lres = MessageLoop();
            m_fWindowModal = FALSE;
        }
    }

    return lres;
}

LRESULT CWindow::ShowCustomAccel( LPCTSTR pszAccelName, CWindow * pParent, LPARAM lParam, LPCTSTR pszCaption, DWORD dwStyle, DWORD dwStyleEx, int x, int y, int cx, int cy )
{
    return ShowCustomAccel( LoadAccelerators( GetInstance(), pszAccelName ), pParent, lParam, pszCaption, dwStyle, dwStyleEx, x, y, cx, cy );
}

LRESULT CWindow::ShowCustomAccel( UINT uAccelResId, CWindow * pParent, LPARAM lParam, LPCTSTR pszCaption, DWORD dwStyle, DWORD dwStyleEx, int x, int y, int cx, int cy )
{
    return ShowCustomAccel( MAKEINTRESOURCE( uAccelResId ), pParent, lParam, pszCaption, dwStyle, dwStyleEx, x, y, cx, cy );
}

LRESULT CWindow::ShowCustom( CWindow * pParent, LPARAM lParam, LPCTSTR pszCaption, DWORD dwStyle, DWORD dwStyleEx, int x, int y, int cx, int cy )
{
    return ShowCustomAccel( (HACCEL)NULL, pParent, lParam, pszCaption, dwStyle, dwStyleEx, x, y, cx, cy );
}

LRESULT CWindow::ShowAccel( HACCEL hAccel, CWindow * pParent, LPARAM lParam )
{
    return ShowCustomAccel( hAccel, pParent, lParam );
}

LRESULT CWindow::ShowAccel( LPCTSTR pszAccelName, CWindow * pParent, LPARAM lParam )
{
    return ShowAccel( LoadAccelerators( GetInstance(), pszAccelName ), pParent, lParam );
}

LRESULT CWindow::ShowAccel( UINT uAccelResId, CWindow * pParent, LPARAM lParam )
{
    return ShowAccel( MAKEINTRESOURCE( uAccelResId ), pParent, lParam );
}

LRESULT CWindow::Show(CWindow * pParent, LPARAM lParam)
{
    return ShowAccel( (HACCEL) NULL, pParent, lParam );
}

HWND CWindow::CreateAccel( HACCEL hAccel, CWindow * pParent, LPARAM lParam )
{
    HWND hParent = ( pParent ? pParent->m_hWnd : NULL );
    m_pParent = pParent;
    m_fModal = FALSE;

    if ( InternalCreate( hAccel, WS_OVERLAPPEDWINDOW | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION | WS_DLGFRAME | WS_SYSMENU,
                         0, m_sCaption.c_str(), CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hParent, 0 ) && hParent )
    {
        SetWindowLongPtr( *this, GWL_HWNDPARENT, (LONG_PTR)hParent );
        return *this;
    }

    return NULL;
}

HWND CWindow::CreateAccel( LPCTSTR pszAccelName, CWindow * pParent, LPARAM lParam )
{
    return CreateAccel( LoadAccelerators( GetInstance(), pszAccelName ), pParent, lParam );
}

HWND CWindow::CreateAccel( UINT uAccelResId, CWindow * pParent, LPARAM lParam )
{
    return CreateAccel( MAKEINTRESOURCE( uAccelResId ), pParent, lParam );
}

HWND CWindow::Create(CWindow * pParent, LPARAM lParam)
{
    return CreateAccel( (HACCEL)NULL, pParent, lParam );
}

HWND CWindow::Create(DWORD dwStyle, DWORD dwStyleEx, LPCTSTR pszCaption, CWindow * pParent, USHORT uId)
{
    return CreateAccel( (HACCEL)NULL, dwStyle, dwStyleEx, pszCaption, pParent, uId );
}

HWND CWindow::CreateAccel( LPCTSTR pszAccelName, DWORD dwStyle, DWORD dwStyleEx, LPCTSTR pszCaption, CWindow * pParent, USHORT uId )
{
    return CreateAccel( LoadAccelerators( GetInstance(), pszAccelName ), dwStyle, dwStyleEx, pszCaption, pParent, uId );
}

HWND CWindow::CreateAccel( UINT uAccelRes, DWORD dwStyle, DWORD dwStyleEx, LPCTSTR pszCaption, CWindow * pParent, USHORT uId )
{
    return CreateAccel( MAKEINTRESOURCE( uAccelRes ), dwStyle, dwStyleEx, pszCaption, pParent, uId );
}

HWND CWindow::CreateAccel( HACCEL hAccel, DWORD dwStyle, DWORD dwStyleEx, LPCTSTR pszCaption, CWindow * pParent, USHORT uId )
{
    HWND hParent = ( pParent ? pParent->m_hWnd : NULL );
    m_pParent = pParent;
    return InternalCreate( hAccel, dwStyle, dwStyleEx, pszCaption, hParent, 0 );
}

HWND CWindow::Create(DWORD dwStyle, DWORD dwStyleEx, LPCTSTR pszCaption, int x, int y, int cx, int cy, CWindow * pParent, USHORT uId)
{
    return CreateAccel( (HACCEL)NULL, dwStyle, dwStyleEx, pszCaption, x, y, cx, cy, pParent, uId );
}

HWND CWindow::CreateAccel( HACCEL hAccel, DWORD dwStyle, DWORD dwStyleEx, LPCTSTR pszCaption, int x, int y, int cx, int cy, CWindow * pParent, USHORT uId )
{
    HWND hParent = ( pParent ? pParent->m_hWnd : NULL );
    m_pParent = pParent;
    return InternalCreate( hAccel, dwStyle, dwStyleEx, pszCaption, x, y, cx, cy, hParent, 0 );
}

HWND CWindow::CreateAccel( LPCTSTR pszAccelName, DWORD dwStyle, DWORD dwStyleEx, LPCTSTR pszCaption, int x, int y, int cx, int cy, CWindow * pParent, USHORT uId )
{
    return CreateAccel( LoadAccelerators( GetInstance(), pszAccelName ), dwStyle, dwStyleEx, pszCaption, x, y, cx, cy, pParent, uId );
}

HWND CWindow::CreateAccel( UINT uAccelRes, DWORD dwStyle, DWORD dwStyleEx, LPCTSTR pszCaption, int x, int y, int cx, int cy, CWindow * pParent, USHORT uId )
{
    return CreateAccel( MAKEINTRESOURCE( uAccelRes ), dwStyle, dwStyleEx, pszCaption, x, y, cx, cy, pParent, uId );
}

VOID CWindow::Close(INT_PTR uExitCode)
{
    if ( m_fModal )
    {
        EndDialog(m_hWnd, uExitCode);
    }
    else
    {
        m_uWindowModalExitCode = uExitCode;
        DestroyWindow(m_hWnd);
    }
}

CWindow * CWindow::Parent()
{
    return m_pParent;
}

CWindow::operator HWND()
{
    return m_hWnd;
}

CWindow::operator HWND() const
{
    return m_hWnd;
}

HFONT CWindow::SetWndFont(HFONT hFont)
{
    HFONT hPrev = (HFONT) wSendMessage(WM_GETFONT);
    SetWindowFont(*this, hFont, TRUE);
    return hPrev;
}

HINSTANCE CWindow::GetInstance()
{
    return m_hInstance;
}

INT CWindow::MsgBox(LPCTSTR pszText, LPCTSTR pszCaption, UINT uType)
{
    static HRESULT (WINAPI * pTaskDialog)(HWND,HINSTANCE,PCWSTR,PCWSTR,PCWSTR,TASKDIALOG_COMMON_BUTTON_FLAGS,PCWSTR,int *)
        =
        (HRESULT (WINAPI * )(HWND,HINSTANCE,PCWSTR,PCWSTR,PCWSTR,TASKDIALOG_COMMON_BUTTON_FLAGS,PCWSTR,int *)) 
            GetProcAddress(GetModuleHandle(TEXT("COMCTL32")), "TaskDialog");

    HWND hTitle = m_hWnd;
    TCHAR pszTitle[MAX_PATH+1];
    if ( !lstrlen( pszCaption ) )
    {
        do
        {
            GetWindowText(hTitle, pszTitle, MAX_PATH);
            hTitle = GetParent(hTitle);
        } while( !lstrlen(pszTitle) && NULL != hTitle );
    }
    else
        std::ignore = lstrcpyn(pszTitle, pszCaption, _ARRAYSIZE( pszTitle ));

    int ret = IDOK, fDoDefault = ( NULL == pTaskDialog );

    if ( !fDoDefault )
    {
        DWORD dwButtons;
        LPTSTR dwIcon;
        switch(uType & MB_TYPEMASK)
        {
        case MB_OK: dwButtons = TDCBF_OK_BUTTON; break;
        case MB_OKCANCEL: dwButtons = TDCBF_OK_BUTTON | TDCBF_CANCEL_BUTTON; break;
        case MB_YESNO:  dwButtons = TDCBF_YES_BUTTON | TDCBF_NO_BUTTON; break;
        case MB_YESNOCANCEL:  dwButtons = TDCBF_YES_BUTTON | TDCBF_NO_BUTTON | TDCBF_CANCEL_BUTTON; break;
        default: fDoDefault = TRUE; break;
        }

        switch(uType & MB_ICONMASK)
        {
        case MB_ICONERROR: dwIcon = TD_ERROR_ICON; break;
        case MB_ICONWARNING: dwIcon = TD_WARNING_ICON; break;
        case MB_ICONQUESTION:
        case MB_ICONINFORMATION: dwIcon = TD_INFORMATION_ICON; break;
        default: dwIcon = NULL; break;
        }

        if ( !fDoDefault && 0 == (pTaskDialog(m_hWnd, m_hInstance, pszTitle, NULL, pszText, dwButtons, dwIcon, &ret ) ) ) 
            return ret;
    }
        
    return MessageBox(m_hWnd, pszText, pszTitle, uType);
}

VOID CWindow::ShowMessage(LPCTSTR pszText)
{
    MsgBox(pszText, NULL, MB_ICONINFORMATION);
}

VOID CWindow::ShowMessageF(LPCTSTR pszText, ...)
{
    va_list argptr;
    CString msg;

    va_start(argptr, pszText);
    msg.FormatV(pszText, argptr);
    ShowMessage(msg);
    va_end(argptr);
}

VOID CWindow::ShowAlert(LPCTSTR pszText)
{
    MsgBox(pszText, NULL, MB_ICONWARNING);
}

VOID CWindow::ShowAlertF(LPCTSTR pszText, ...)
{
    va_list argptr;
    CString msg;

    va_start(argptr, pszText);
    msg.FormatV(pszText, argptr);
    ShowAlert(msg);
    va_end(argptr);
}

VOID CWindow::ShowError(LPCTSTR pszText)
{
    MsgBox(pszText, NULL, MB_ICONERROR);	
}

VOID CWindow::ShowErrorF(LPCTSTR pszText, ...)
{
    va_list argptr;
    CString msg;

    va_start(argptr, pszText);
    msg.FormatV(pszText, argptr);
    ShowError(msg);
    va_end(argptr);
}

BOOL CWindow::Confirm(LPCTSTR pszText, BOOL fDefaultNo)
{
    return ( IDYES == MsgBox(pszText, NULL, MB_ICONQUESTION | MB_YESNO | ( fDefaultNo ? MB_DEFBUTTON2 : 0 )) );
}

BOOL CWindow::ConfirmF(LPCTSTR pszText, BOOL fDefaultNo, ...)
{
    va_list argptr;
    CString msg;

    va_start(argptr, fDefaultNo);
    msg.FormatV(pszText, argptr);
    BOOL fResult = Confirm(msg, fDefaultNo);
    va_end(argptr);
    return fResult;
}

VOID CWindow::Enable(BOOL fEnable)
{
    EnableWindow(*this, fEnable);
}

BOOL CWindow::Enable()
{
    return IsWindowEnabled(*this);
}

VOID CWindow::Visible(BOOL fVisible)
{
    ShowWindow(*this, ( fVisible ? SW_SHOW : SW_HIDE ));
}

BOOL CWindow::Visible()
{
    return IsWindowVisible(*this);
}

VOID CWindow::EnableAll(BOOL fEnable, ...)
{
    va_list argptr;
    va_start(argptr, fEnable);

    do
    {
        int id = va_arg( argptr, int );
        if ( !id )
            break;
        EnableWindow(GetDlgItem(*this, id), fEnable);
    } while( true );

    va_end(argptr);
}

VOID CWindow::ShowAll(BOOL fVisible, ...)
{
    va_list argptr;
    va_start(argptr, fVisible);

    int nCmd = ( fVisible ? SW_SHOW : SW_HIDE );

    do
    {
        int id = va_arg( argptr, int );
        if ( !id )
            break;
        ShowWindow(Ctl(id), nCmd);
    } while( true );

    va_end(argptr);
}

HWND CWindow::Ctl(INT id)
{
    return GetDlgItem(*this, id);
}

tstring CWindow::ShellFileDialog(BOOL fOpenFile, LPCTSTR pszDefaultFilename, std::vector<tstring> & vExtensions)
{
    tstring pszFile = pszDefaultFilename;
    pszFile.append(MAX_PATH, 0);

    tstring szList;
    size_t count = vExtensions.size();
    for(size_t n = 0; n < count; ++n)
    {
        szList.append(vExtensions[n].begin(), vExtensions[n].end());
        szList.push_back(0);
    }
    szList.push_back(0);

    OPENFILENAME ofn = { 0 };

    ofn.lStructSize = sizeof(ofn);
    ofn.Flags = 
        ( fOpenFile ?
            OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST :
            OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST
        );

    ofn.hInstance = GetInstance();
    ofn.hwndOwner = *this;
    ofn.lpstrFile = &pszFile[0];
    ofn.lpstrFilter = &szList[0];
    ofn.nMaxFile = (DWORD) pszFile.length();
    
    BOOL fResult = ( fOpenFile ? GetOpenFileName(&ofn) : GetSaveFileName(&ofn) );

    if ( fResult )
        return pszFile;
    
    return TEXT("");
}

tstring CWindow::OpenFile(LPCTSTR pszDefaultFilename, ...)
{
    va_list argptr;
    va_start(argptr, pszDefaultFilename);

    std::vector<tstring> vExt;

    do
    {
        LPCTSTR pszArg = va_arg( argptr, LPCTSTR );
        if ( !pszArg )
            break;
        vExt.push_back( pszArg );
    } while( true );

    return ShellFileDialog(TRUE, pszDefaultFilename, vExt);
}

tstring CWindow::SaveFile(LPCTSTR pszDefaultFilename, ...)
{
    va_list argptr;
    va_start(argptr, pszDefaultFilename);

    std::vector<tstring> vExt;
    do
    {
        LPCTSTR pszArg = va_arg( argptr, LPCTSTR );
        if ( !pszArg )
            break;
        vExt.push_back( pszArg );
    } while( true );

    return ShellFileDialog(FALSE, pszDefaultFilename, vExt);
}
