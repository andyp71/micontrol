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

#include "precomp.h"
#include <shellapi.h>

std::unique_ptr<CLog>	g_log;

void CALLBACK DispatcherServiceMain(DWORD nArgs, LPTSTR * pszArgs)
{
	std::vector<tstring> vArgs;
	for (decltype(nArgs)i = 0; i < nArgs; ++i)
		vArgs.push_back(pszArgs[i]);

	CMIService mi;
	mi.Start(vArgs);
}

extern "C" __declspec(dllexport) int CALLBACK ServiceMain(DWORD nArgs, LPTSTR* pszArgs)
{
	g_log = std::make_unique<CLog>(0, SERVICE_NAME, CLog::levelInfo);

	tstring strSvcName = SERVICE_NAME;
	SERVICE_TABLE_ENTRY entry[] =
	{
		{ strSvcName.data(), DispatcherServiceMain },
		{ nullptr, nullptr }
	};

	g_log->info(TEXT("Starting StartServiceCtrlDispatcher()"));
	// Sleep(15000);
	if (!StartServiceCtrlDispatcher(entry))
	{
		Win32Error err(GetLastError());
		if (ERROR_SERVICE_ALREADY_RUNNING == err.what())
		{
			DispatcherServiceMain(nArgs, pszArgs);
		}
		else
		{
			g_log->error(TEXT("Service register failed (%d): %s"), err.what(), err.description().c_str());
			return -1;
		}
	}
	g_log->info(TEXT("StartServiceCtrlDispatcher() exited"));

	return 0;
}