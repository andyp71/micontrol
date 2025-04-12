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

WbemServices::WbemServices(WbemLocator& locator, const tstring& strService)
{
	auto hres = locator->ConnectServer(BSTR(strService.c_str()), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &m_pSvc);
	if (FAILED(hres))
		throw COMException(hres);
}

WbemServices::~WbemServices()
{
	m_pSvc->Release();
}

IWbemServices* WbemServices::operator->()
{
	return m_pSvc;
}

WbemServices::operator IWbemServices* ()
{
	return operator->();
}

void WbemServices::SetProxyBlanket(
	DWORD dwAuthnSvc, 
	DWORD dwAuthzSvc, 
	OLECHAR* pServerPrincName, 
	DWORD dwAuthnLevel, 
	DWORD dwImpLevel, 
	RPC_AUTH_IDENTITY_HANDLE pAuthInfo, 
	DWORD dwCapabilities)
{
	auto hres = CoSetProxyBlanket(
		m_pSvc,
		dwAuthnSvc,
		dwAuthzSvc,
		pServerPrincName,
		dwAuthnLevel,
		dwImpLevel,
		pAuthInfo,
		dwCapabilities
	);

	if (FAILED(hres))
		throw COMException(hres);
}

void WbemServices::SetProxyBlanket()
{
	SetProxyBlanket(RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
		RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL, EOAC_NONE);
}