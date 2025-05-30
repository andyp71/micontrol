#pragma once

#include <windows.h>
#include <WindowsX.h>
#include <CommCtrl.h>
#include <CommDlg.h>

#include <tchar.h>
#include <vector>
#include <map>
#include <string>
#include <objidl.h>
#include <gdiplus.h>
#include <uxtheme.h>
#include <atlstr.h>
#include <functional>
#include <algorithm> 

#ifdef UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif

#ifndef LODWORD
typedef unsigned __int64 ULONGLONG;
#define LODWORD(l) ((DWORD)(ULONG_PTR(l)&0xFFFFFFFF))
#define HIDWORD(l) ((DWORD)(((ULONG_PTR(l)>>32)&0xFFFFFFFF))
#endif

// TODO: reference additional headers your program requires here
#ifdef _WIN64
#undef GetWindowLong
#define GetWindowLong GetWindowLongPtr
#undef SetWindowLong
#define SetWindowLong SetWindowLongPtr
#undef GetClassLong
#define GetClassLong GetClassLongPtr

#ifndef GWL_HINSTANCE
#define GWL_HINSTANCE GWLP_HINSTANCE
#endif

#ifndef GWL_WNDPROC
#define GWL_WNDPROC GWLP_WNDPROC
#endif

#ifndef GWL_HWNDPARENT
#define GWL_HWNDPARENT GWLP_HWNDPARENT
#endif

#ifndef GWL_USERDATA
#define GWL_USERDATA GWLP_USERDATA
#endif

#ifndef GWL_ID
#define GWL_ID GWLP_ID
#endif

#ifndef DWL_MSGRESULT
#define DWL_MSGRESULT DWLP_MSGRESULT
#endif

#ifndef DWL_USER
#define DWL_USER DWLP_USER
#endif

#endif

#ifndef _DEBUG
#undef _tprintf
#define _tprintf(...)
#endif

#include "CBitmap.h"
#include "CWindow.h"
#include "Controls\cimgloader.h"
#include "Controls\CIconBits.h"
#include "Controls\CListView.h"
#include "Controls\ctoolbar.h"
