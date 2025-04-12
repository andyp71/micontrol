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

CSvcParameters::CSvcParameters()
{
	auto err = m_keyParams.Open(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Services\\")  SERVICE_NAME TEXT("\\Parameters"), KEY_QUERY_VALUE | KEY_SET_VALUE);
	if (err)
		throw Win32Error(err);
}

DWORD CSvcParameters::GetParam(const tstring& strParam)
{
	DWORD value = 0;
	auto err = m_keyParams.QueryDWORDValue(strParam.c_str(), value);
	if (!err)
		return value;

	return 0;
}

bool CSvcParameters::SetParam(const tstring& strParam, DWORD dwValue)
{
	auto err = m_keyParams.SetDWORDValue(strParam.c_str(), dwValue);
	return (ERROR_SUCCESS == err);
}