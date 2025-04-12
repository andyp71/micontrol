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

class Win32Error
{
	const DWORD			m_dwError = GetLastError();
	const tstring		m_strDesc = tstring();

public:
	Win32Error() = default;
	Win32Error(const tstring& strDesc)
		: m_strDesc(strDesc) {}
	Win32Error(const tstring& strDesc, DWORD dwError)
		: m_strDesc(strDesc), m_dwError(dwError) {}
	Win32Error(DWORD dwError)
		: m_dwError(dwError) {}

	DWORD				what() const
	{
		return m_dwError;
	}

	tstring				description() const
	{
        LPTSTR messageBuffer = nullptr;

        //Ask Win32 to give us the string version of that message ID.
        //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
		size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, m_dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&messageBuffer), 0, nullptr);

		if (messageBuffer)
		{
			tstring desc(messageBuffer, size);
			LocalFree(messageBuffer);
			return desc;
		}

		return tstring();
	}

	tstring				tip() const
	{
		return m_strDesc;
	}
};