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

#pragma once

#pragma comment(linker, \
    "\"/manifestdependency:type='Win32' "\
    "name='Microsoft.Windows.Common-Controls' "\
    "version='6.0.0.0' "\
    "processorArchitecture='*' "\
    "publicKeyToken='6595b64144ccf1df' "\
    "language='*'\"")

#pragma warning(disable:4838)

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <WindowsX.h>
#include <CommCtrl.h>
#include <CommDlg.h>

extern void* __cdecl operator new(size_t size);
extern void __cdecl operator delete(void* ptr);
extern void __cdecl operator delete[](void* ptr);


// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <vector>
#include <map>
#include <string>
#include <objidl.h>
#include <gdiplus.h>
#include <Resource.h>
#include <uxtheme.h>
#include <atlstr.h>

#ifdef UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif

#ifndef LODWORD
typedef unsigned __int64 ULONGLONG;
#define LODWORD(l) ((DWORD)(l))
#define HIDWORD(l) ((DWORD)(((ULONGLONG(l)>>32)&0xFFFFFFFF))
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

#include "h\GUI\Wrapper\Controls.h"
#include <future>
#include <WMI/WMI.h>
#include "MIEvent/MIEvent.h"

#define ON_EVENT(cls, method) ((void(cls::*)(const CEventHandlerBase&)) &cls::method)
#define EVENT_REF(cls, method) std::bind(ON_EVENT(cls, method), this, std::placeholders::_1)

#include <CommonRequestByte.pb.h>
#include <EventDetail.pb.h>

// #include "GUI/MainWindow/CMainWindow.h"
// #include "GUI/Dialogs/CMiControlDlg.h"

#include "GUI/Dialogs/CSplashScreen.h"
#include "GUI/Dialogs/Pages/Pages.h"
#include "GUI/Dialogs/CHostDlg.h"
