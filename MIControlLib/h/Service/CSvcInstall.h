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

using HSVCMGR = CAutoHandle<SC_HANDLE, nullptr, CloseServiceHandle>;
using HSERVICE = CAutoHandle<SC_HANDLE, nullptr, CloseServiceHandle>;

class CSvcInstall
{
	HSVCMGR			m_hSvcMgr = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
	HSERVICE		m_hService = nullptr;
	tstring			m_strServiceName;

public:
	CSvcInstall();
	void		Install(
		__in const tstring & strServiceName,
		__in DWORD dwServiceType,
		__in DWORD dwStartType,
		__in const tstring & lpBinaryPathName = tstring(),
		__in const tstring& strServiceDescription = tstring(),
		__in const std::vector<tstring>& lpDependencies = {},
		__in const tstring lpServiceStartName = tstring(TEXT("NT AUTHORITY\\LocalService")),
		__in const tstring lpPassword = tstring()
	);

	void		Start(const std::vector<tstring>& strArgs = {});
	void		Stop(const tstring& svcName = tstring());
	void		Uninstall();
	static tstring	GetSharedBinaryPath(const tstring& strGroupName);
	static void	AppendSvcGroup(const tstring& strServiceName, const tstring& strGroupName, const tstring& strDllName);

};