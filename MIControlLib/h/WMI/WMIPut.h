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

class WMISysString
{
	BSTR			m_pSysString = nullptr;

	void			FreeStr();

public:
	WMISysString(const tstring& str);
	WMISysString(const WMISysString& other);
	WMISysString(WMISysString&& other) noexcept;
	~WMISysString();

	WMISysString& operator=(const WMISysString& other);
	WMISysString& operator=(WMISysString&& other);
	WMISysString& operator=(const tstring & str);
	operator BSTR();
};

class WMIPut
{
	WbemLocator				m_locator;
	WbemServices			m_svc;

	const tstring			m_strClassName, m_strMethod;

	IWbemClassObject*		m_pInstance = nullptr;

public:
	WMIPut(const tstring & strRoot, const tstring& strClassName, const tstring & strMethod);
	~WMIPut();

	void	Put(const tstring & strProp, VARIANT& vt);
	WbemClassItem			Exec();
	WbemClassItem			Exec(const tstring& target);

};