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

#include <Windows.h>
#include <map>
#include <vector>
#include <functional>
#include <string>
#include <future>

#ifdef UNICODE
using tstring = std::wstring;
#else
using tstring = std::string;
#endif

#include "Common.h"
#include <cfgmgr32.h>
#include "md5.h"
#include "iostream.h"
#include "CAutoHandle.h"
#include "Win32Error.h"
#include "CSecurityDescriptior.h"
#include "Service/ServiceDef.h"
#include "Service/CSvcInstall.h"
#include "Service/CAudioInputControl.h"
#include "WMI/WMI.h"
#include "CCriticalSection.h"
#include <Splash/splash.h>