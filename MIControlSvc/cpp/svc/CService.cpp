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

CService::CService(LPCTSTR pszService, DWORD dwServiceType)
	: m_strSvcName(pszService), m_dwServiceType(dwServiceType),
	m_ctlHandle(CreateEvent(nullptr, TRUE, FALSE, nullptr))
{

}

DWORD CALLBACK CService::LphandlerFunctionEx(
	__in DWORD dwControl, 
	__in DWORD dwEventType,
	__in LPVOID lpEventData,
	__in LPVOID lpContext)
{
	return ((CService*)lpContext)->HandlerEx(dwControl, dwEventType, lpEventData);
}

CService::operator SERVICE_STATUS_HANDLE()
{
	return m_hStatus;
}

void CService::Start(__in const std::vector<tstring> vArgs)
{
	g_log->info(TEXT("Initializing service %s"), m_strSvcName.c_str());
	if (!(m_hStatus = RegisterServiceCtrlHandlerEx(m_strSvcName.c_str(), CService::LphandlerFunctionEx, this)))
	{
		Win32Error err;
		g_log->info(TEXT("RegisterServiceCtrlHandlerEx(%s) failed (%d): %s"), m_strSvcName.c_str(), err.what(), err.description().c_str());
	}

	SetStatus(SERVICE_START_PENDING, 0, 0, 0, 0, 0);

	LARGE_INTEGER status = ServiceMain(vArgs);

	SetStatus(SERVICE_STOPPED, 0, status.LowPart, status.HighPart, 0, 0);

	g_log->info(TEXT("Service %s is shutting down"), m_strSvcName.c_str());
}

BOOL CService::SetStatus(
	__in DWORD dwCurrentState, 
	__in DWORD dwControlsAccepted, 
	__in DWORD dwWin32ExitCode, 
	__in DWORD dwServiceSpecificExitCode, 
	__in DWORD dwCheckPoint, 
	__in DWORD dwWaitHint)
{
	SERVICE_STATUS ss({ 0 });
	ss.dwServiceType = m_dwServiceType;
	ss.dwCurrentState = dwCurrentState;
	ss.dwControlsAccepted = dwControlsAccepted;
	ss.dwWin32ExitCode = dwWin32ExitCode;
	ss.dwServiceSpecificExitCode = dwServiceSpecificExitCode;
	ss.dwCheckPoint = dwCheckPoint;
	ss.dwWaitHint = dwWaitHint;
	return SetServiceStatus(*this, &ss);
}

BOOL CService::ReportStatus(
	__in DWORD dwCurrentState, 
	__in DWORD dwControlsAccepted)
{
	if (SetStatus(dwCurrentState, dwControlsAccepted, 0, 0, 0, 0))
	{
		m_state.dwCurrentState = dwCurrentState;
		m_state.dwControlsAccepted = dwControlsAccepted;
		return TRUE;
	}

	return FALSE;
}

BOOL CService::UpdateProgress(
	__in DWORD dwCheckPoint, 
	__in DWORD dwWaitHint)
{
	return SetStatus(m_state.dwCurrentState, m_state.dwControlsAccepted, 0, 0, dwCheckPoint, dwWaitHint);
}

void CService::SignalEvent()
{
	m_ctlHandle.RaiseEvent();
}

CControlHandle& CService::ControlEvent()
{
	return m_ctlHandle;
}
