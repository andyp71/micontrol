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

WMIQuery::WMIQuery(const tstring& strRoot)
	: m_locator()
	, m_svc(m_locator, strRoot)
{
	m_svc.SetProxyBlanket();
}

bool WMIQuery::Query(const tstring& strQuery, std::vector<WbemClassItem>& result)
{
	IEnumWbemClassObject* pEnumerator = nullptr;
	WMISysString q(strQuery);

	auto hres = m_svc->ExecQuery(BSTR(L"WQL"), q,
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
	
	if (FAILED(hres))
		throw COMException(hres);

	IWbemClassObject* pclsObj = NULL;
	ULONG uReturn = 0;
	result.clear();
	while (true)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
		if (0 == uReturn) 
		{
			break;
		}
		else if (FAILED(hr))
		{
			throw COMException(hr);
		}

		result.emplace_back(pclsObj);
	}

	return !result.empty();
}