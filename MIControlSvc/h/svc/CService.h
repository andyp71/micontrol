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

class CService
{
	SERVICE_STATUS_HANDLE		m_hStatus = nullptr;
	const tstring				m_strSvcName;
	const DWORD					m_dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	CControlHandle				m_ctlHandle;

	struct
	{
		DWORD	dwCurrentState		= SERVICE_STOPPED;
		DWORD	dwControlsAccepted	= 0;
	} m_state;
	

	static DWORD CALLBACK LphandlerFunctionEx(
		__in DWORD dwControl,
		__in DWORD dwEventType,
		__in LPVOID lpEventData,
		__in LPVOID lpContext
	);

	virtual DWORD HandlerEx(
		__in DWORD dwControl,
		__in DWORD dwEventType,
		__in LPVOID lpEventData
	) = 0;

	virtual LARGE_INTEGER ServiceMain(
		__in const std::vector<tstring> m_vArgs
	) = 0;

protected:
	operator SERVICE_STATUS_HANDLE();

	BOOL	SetStatus(
		__in DWORD dwCurrentState,
		__in DWORD dwControlsAccepted,
		__in DWORD dwWin32ExitCode,
		__in DWORD dwServiceSpecificExitCode,
		__in DWORD dwCheckPoint,
		__in DWORD dwWaitHint
	);

	BOOL ReportStatus(
		__in DWORD dwCurrentState,
		__in DWORD dwControlsAccepted
	);

	BOOL UpdateProgress(
		__in DWORD dwCheckPoint,
		__in DWORD dwWaitHint
	);

	void SignalEvent();

	CControlHandle& ControlEvent();

public:
	CService(LPCTSTR pszService, DWORD dwServiceType = SERVICE_WIN32_OWN_PROCESS);
	void		Start(
		__in const std::vector<tstring> m_vArgs
	);
};