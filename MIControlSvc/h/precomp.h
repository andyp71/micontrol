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

#pragma comment(linker, \
    "\"/manifestdependency:type='Win32' "\
    "name='Microsoft.Windows.Common-Controls' "\
    "version='6.0.0.0' "\
    "processorArchitecture='*' "\
    "publicKeyToken='6595b64144ccf1df' "\
    "language='*'\"")

#pragma warning(disable:4838)

#define _CRT_SECURE_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <map>
#include <vector>
#include <deque>
#include <functional>
#include <string>
#include <future>
#include <objidl.h>
#include <atlstr.h>

#ifdef UNICODE
using tstring = std::wstring;
#else
using tstring = std::string;
#endif

#include <Common.h>
#include <Service/ServiceDef.h>
#include <CCriticalSection.h>
#include <Win32Error.h>
#include <CAutoHandle.h>
#include "svc/svc.h"