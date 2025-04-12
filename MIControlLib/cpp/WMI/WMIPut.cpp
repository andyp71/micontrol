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

WMISysString::WMISysString(const tstring& str)
	: m_pSysString(SysAllocString(str.c_str()))
{
}

WMISysString::WMISysString(const WMISysString& other)
{
	if (this == &other) return;

	m_pSysString = SysAllocString(other.m_pSysString);
}

WMISysString::WMISysString(WMISysString&& other) noexcept
{
	m_pSysString = other.m_pSysString;
	other.m_pSysString = nullptr;
}

WMISysString& WMISysString::operator=(const WMISysString& other)
{
	if (this == &other) return *this;

	FreeStr();
	m_pSysString = SysAllocString(other.m_pSysString);
	return *this;
}

WMISysString& WMISysString::operator=(WMISysString&& other)
{
	if (this == &other)
		return *this;

	m_pSysString = other.m_pSysString;
	other.m_pSysString = nullptr;
	return *this;
}

WMISysString& WMISysString::operator=(const tstring& str)
{
	FreeStr();
	m_pSysString = SysAllocString(str.c_str());
	return *this;
}

WMISysString::operator BSTR()
{
	return m_pSysString;
}

void WMISysString::FreeStr()
{
	if (m_pSysString)
		SysFreeString(m_pSysString);
}

WMISysString::~WMISysString()
{
	FreeStr();
}

WMIPut::WMIPut(const tstring& strRoot, const tstring& strClassName, const tstring& strMethod)
	: m_locator()
	, m_svc(m_locator, strRoot)
	, m_strClassName(strClassName)
	, m_strMethod(strMethod)
{
	WMISysString str(strClassName);
	IWbemClassObject* pInstanceClass = nullptr;
	IWbemClassObject* pMethod = nullptr;
	m_svc.SetProxyBlanket();
	auto hRes = m_svc->GetObject(str, 0, nullptr,
		&pInstanceClass, nullptr);

	if (FAILED(hRes))
		throw COMException(hRes);

	WMISysString sMethod(strMethod);
	hRes = pInstanceClass->GetMethod(sMethod, 0, &pMethod, nullptr);
	if (FAILED(hRes))
	{
		pInstanceClass->Release();
		throw COMException(hRes);
	}

	hRes = pMethod->SpawnInstance(0, &m_pInstance);
	pMethod->Release();
	pInstanceClass->Release();

	if (FAILED(hRes))
	{
		throw COMException(hRes);
	}
}

void WMIPut::Put(const tstring& strProp, VARIANT& vt)
{
	WMISysString str(strProp);

	auto hRes = m_pInstance->Put(str, 0, &vt, 0);
	if (FAILED(hRes))
		throw COMException(hRes);
}

WbemClassItem WMIPut::Exec()
{
	return Exec(m_strClassName);
}

WbemClassItem WMIPut::Exec(const tstring& target)
{
	WMISysString className(target), method(m_strMethod);
	IWbemClassObject* pOutput = nullptr;
	auto hRes = m_svc->ExecMethod(
		className,
		method,
		0, nullptr, m_pInstance, &pOutput, nullptr
	);

	if (FAILED(hRes))
		throw COMException(hRes);

	return WbemClassItem(pOutput);
}

WMIPut::~WMIPut()
{
	if (m_pInstance)
		m_pInstance->Release();
}