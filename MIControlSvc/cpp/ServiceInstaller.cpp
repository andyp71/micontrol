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
#include <Msi.h>
#include <Common.h>
#include <Service/ServiceDef.h>
#include <Service/CSvcInstall.h>

HINSTANCE g_hInstance;

BOOL WINAPI DllMain(
	__in HINSTANCE hinstDLL,  // handle to DLL module
	__in DWORD fdwReason,     // reason for calling function
	__in LPVOID lpvReserved)  // reserved
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH: __fallthrough;
	case DLL_THREAD_ATTACH: g_hInstance = hinstDLL; break;
	}

	return TRUE;
}

extern "C" __declspec(dllexport) bool InitializeMI(HINSTANCE hInstance)
{
	try
	{
		CSvcInstall svc;
		tstring strDll(MAX_PATH, 0);
		strDll.resize(GetModuleFileName(hInstance, strDll.data(), static_cast<DWORD>(strDll.size())));

		auto strImagePath = CSvcInstall::GetSharedBinaryPath(TEXT("LocalService"));
		if (strImagePath.empty())
			return false;

		OutputDebugString(TEXT("Installing service... "));

		svc.Install(SERVICE_NAME, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, strImagePath, TEXT("MI Hardware Controller and Monitor"), {}, tstring());
		svc.AppendSvcGroup(SERVICE_NAME, TEXT("LocalService"), strDll);

		OutputDebugString(TEXT("OK\n"));
		OutputDebugString(TEXT("Starting service... "));
		svc.Start();
		OutputDebugString(TEXT("OK\n"));
	}
	catch (Win32Error&)
	{
		return false;
	}

	return true;
}

extern "C" __declspec(dllexport) bool UninitializeMI(HINSTANCE hInstance)
{
	try
	{
		CSvcInstall svc;
		svc.Stop(SERVICE_NAME);
		svc.Uninstall();
		return true;
	}
	catch (Win32Error&)
	{}

	return false;
}

extern "C" __declspec(dllexport) UINT ValidateMI(MSIHANDLE hInstall)
{
	return !InitializeMI(g_hInstance);
}

extern "C" __declspec(dllexport) UINT ValidateMI2(MSIHANDLE hInstall)
{
	UninitializeMI(g_hInstance);
	return 0;
}
