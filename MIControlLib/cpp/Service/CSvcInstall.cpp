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

#include <PathCch.h>
#include <atlbase.h>
#include <algorithm>

#pragma comment(lib, "pathcch.lib")

CSvcInstall::CSvcInstall()
{
	if (!m_hSvcMgr)
		throw Win32Error();
}

void CSvcInstall::Install(
	__in const tstring& strServiceName,
	__in DWORD dwServiceType,
	__in DWORD dwStartType,
	__in const tstring& lpBinaryPathName,
	__in const tstring& strServiceDescription,
	__in const std::vector<tstring>& lpDependencies,
	__in const tstring lpServiceStartName,
	__in const tstring lpPassword)
{
	if (m_hService)
		throw Win32Error(ERROR_ALREADY_REGISTERED);

	auto lpBinary = (lpBinaryPathName.empty() ? ImagePath() : lpBinaryPathName);
	auto strSvcDesc = (strServiceDescription.empty() ? strServiceName : strServiceDescription);
	std::vector<TCHAR> vDeps;

	if (!lpDependencies.empty())
	{
		vDeps.reserve(1024);
		std::for_each(lpDependencies.begin(), lpDependencies.end(),
			[&vDeps](const auto& dep)
			{
				vDeps.insert(vDeps.end(), dep.begin(), dep.end());
				vDeps.push_back(0);
			});
		vDeps.push_back(0);
	}

	m_hService = CreateService(
		m_hSvcMgr,
		strServiceName.c_str(),
		strSvcDesc.c_str(),
		SERVICE_ALL_ACCESS,
		dwServiceType,
		dwStartType,
		SERVICE_ERROR_NORMAL,
		lpBinary.c_str(),
		nullptr,
		nullptr,
		vDeps.empty() ? nullptr : vDeps.data(),
		lpServiceStartName.empty() ? nullptr : lpServiceStartName.c_str(),
		lpPassword.c_str()
	);

	if (!m_hService)
		throw Win32Error();

	m_strServiceName = strServiceName;
}

void CSvcInstall::Start(const std::vector<tstring>& strArgs)
{
	if (!m_hService)
		throw Win32Error(ERROR_SERVICE_NOT_FOUND);

	std::vector<LPCTSTR> vArgs;
	if (!strArgs.empty())
	{
		vArgs.push_back(m_strServiceName.c_str());
		std::for_each(strArgs.begin(), strArgs.end(), [&vArgs](const auto& arg)
			{
				vArgs.push_back(arg.c_str());
			});
	}

	if (!StartService(m_hService, static_cast<DWORD>(vArgs.size()), (vArgs.empty() ? nullptr : vArgs.data())))
		throw Win32Error();
}

void CSvcInstall::Stop(const tstring & svcName)
{
	if (!m_hService)
		if (!svcName.empty())
		{
			m_hService = OpenService(m_hSvcMgr, svcName.c_str(), SERVICE_ALL_ACCESS);
			if (!m_hService)
				throw Win32Error();
		}
		else
			throw Win32Error(ERROR_SERVICE_NOT_FOUND);

	SERVICE_STATUS ss({ 0 });
	ControlService(m_hService, SERVICE_CONTROL_STOP, &ss);
}

void CSvcInstall::Uninstall()
{
	if (!m_hService)
		throw Win32Error(ERROR_SERVICE_NOT_FOUND);

	DeleteService(m_hService);
	m_hService.reset();
}

tstring CSvcInstall::GetSharedBinaryPath(const tstring& strGroupName)
{
	const tstring strSvcHost = TEXT("%SYSTEMROOT%\\System32\\svchost.exe");

	tstring strRealSvchostPath(MAX_PATH, 0);
	strRealSvchostPath.resize(ExpandEnvironmentStrings(strSvcHost.c_str(), strRealSvchostPath.data(), static_cast<DWORD>(strRealSvchostPath.size())));
	if (strRealSvchostPath.empty())
		return tstring();

	strRealSvchostPath.pop_back();

	return strRealSvchostPath + TEXT(" -k ") + strGroupName + TEXT(" -p");
}

void CSvcInstall::AppendSvcGroup(const tstring& strServiceName, const tstring& strGroupName, const tstring& strDllName)
{
	if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(strDllName.c_str()))
		return;

	CRegKey svcKey;
	if (ERROR_SUCCESS != svcKey.Open(HKEY_LOCAL_MACHINE, (TEXT("SYSTEM\\CurrentControlSet\\Services\\") + strServiceName).c_str(), KEY_ALL_ACCESS))
		return;

	tstring strRealDllPath(MAX_PATH, 0);
	if (FAILED(PathCchCanonicalize(strRealDllPath.data(), strRealDllPath.size(), strDllName.c_str())))
		return;

	strRealDllPath.resize(lstrlen(strRealDllPath.c_str()));

	CRegKey svcParam;
	if (ERROR_SUCCESS != svcParam.Create(svcKey, TEXT("Parameters")))
		return;

	if (ERROR_SUCCESS != svcParam.SetStringValue(TEXT("ServiceDll"), strRealDllPath.c_str(), REG_EXPAND_SZ))
		return;

	if (ERROR_SUCCESS != svcParam.SetDWORDValue(TEXT("ServiceDllUnloadOnStop"), 1))
		return;
	
	// svcKey.SetDWORDValue(TEXT("SvcHostSplitDisable"), 1);
	svcParam.SetDWORDValue(TEXT("LegacyCOMBehavior"), 1);

	CRegKey key;
	if (ERROR_SUCCESS != key.Open(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Svchost"), KEY_QUERY_VALUE | KEY_SET_VALUE | KEY_ENUMERATE_SUB_KEYS))
		return;

	tstring strValue(4096, 0);
	ULONG dw = static_cast<ULONG>(strValue.size());
	if (ERROR_SUCCESS != key.QueryMultiStringValue(strGroupName.c_str(), strValue.data(), &dw))
		return;

	strValue.resize(dw);

	std::vector<tstring> arrServices;
	auto ptr = strValue.c_str();

	size_t n1 = 0;
	while(*ptr)
	{
		arrServices.push_back(ptr);
		ptr += lstrlen(ptr) + 1;
	} 

	std::erase_if(arrServices, [](auto& str) { return str.empty(); });

	if (arrServices.end() != std::find_if(arrServices.begin(), arrServices.end(), [&strServiceName](const auto & name)
		{
			if (strServiceName.length() != name.length())
				return false;
			return std::equal(
				strServiceName.begin(), strServiceName.end(), name.begin(), name.end(), 
				[](auto& ch1, auto& ch2) { return std::tolower(ch1) == std::tolower(ch2); }
			);
		}))
		return;

	tstring strGroupServiceList;
	arrServices.push_back(strServiceName);
	
	for (const auto& name : arrServices)
	{
		if (!strGroupServiceList.empty())
			strGroupServiceList.push_back(0);

		strGroupServiceList.insert(strGroupServiceList.end(), name.begin(), name.end());
	}

	strGroupServiceList.insert(strGroupServiceList.end(), { 0, 0 });

	key.SetMultiStringValue(strGroupName.c_str(), strGroupServiceList.data());
}