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

template <class H, H DEFAULT, BOOL(WINAPI* fnRelease)(H)>
class CAutoHandle
{
	DWORD			m_dwError = ERROR_SUCCESS;
	H				m_hHandle = DEFAULT;

public:
	CAutoHandle() {};
	CAutoHandle(const CAutoHandle&) = delete;
	CAutoHandle(const H& h)
		: m_hHandle(h) {
		if (!operator bool())
			m_dwError = GetLastError();
	}

	CAutoHandle& operator=(const H& h)
	{
		m_hHandle = h;

		if (!operator bool())
			m_dwError = GetLastError();

		return *this;
	}

	operator bool() const
	{
		return m_hHandle != DEFAULT;
	}

	H* operator&() const
	{
		return &m_hHandle;
	}

	H* operator->() const
	{
		return &m_hHandle;
	}

	H* operator&()
	{
		return &m_hHandle;
	}

	H* operator->()
	{
		return &m_hHandle;
	}

	operator H() const
	{
		return m_hHandle;
	}

	H get()
	{
		return m_hHandle;
	}

	H get() const
	{
		return m_hHandle;
	}

	bool operator==(const H& h) const
	{
		return h == m_hHandle && DEFAULT != m_hHandle;
	}

	void reset()
	{
		if (this->operator bool())
		{
			fnRelease(m_hHandle);
			m_hHandle = DEFAULT;
		}
	}

	DWORD Error() const
	{
		return m_dwError;
	}

	~CAutoHandle()
	{
		reset();
	}
};

using HEVENT = CAutoHandle<HANDLE, nullptr, CloseHandle>;
using HPIPE = CAutoHandle<HANDLE, INVALID_HANDLE_VALUE, CloseHandle>;