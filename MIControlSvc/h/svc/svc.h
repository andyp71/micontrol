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

#include <atomic>
#include "CLog.h"

extern std::unique_ptr<CLog>	g_log;
using HEVENT = CAutoHandle<HANDLE, nullptr, CloseHandle>;

#include <proto/EventDetail.pb.h>
#include <proto/ClientRequest.pb.h>
#include <proto/ClientRequestBin.pb.h>
#include <proto/ClientPipeRequest.pb.h>
#include <proto/CommonRequestByte.pb.h>

#include <WMI/WMI.h>
#include "CMiLowLevelCommand.h"
#include "CControlHandle.h"
#include "CNamedPipe.h"
#include "CSvcParameters.h"
#include "CService.h"
#include "CPowerPowerChargeProtectGuard.h"
#include "CMIService.h"