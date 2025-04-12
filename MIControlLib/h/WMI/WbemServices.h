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

#include <wbemcli.h>

class WbemServices
{
	IWbemServices*			m_pSvc;

public:
	WbemServices(WbemLocator& locator, const tstring& strService);
	~WbemServices();

	IWbemServices* operator->();
	operator IWbemServices* ();

    void    SetProxyBlanket(
        _In_ DWORD dwAuthnSvc,
        _In_ DWORD dwAuthzSvc,
        _In_opt_ OLECHAR* pServerPrincName,
        _In_ DWORD dwAuthnLevel,
        _In_ DWORD dwImpLevel,
        _In_opt_ RPC_AUTH_IDENTITY_HANDLE pAuthInfo,
        _In_ DWORD dwCapabilities
    );

    void    SetProxyBlanket();
};
