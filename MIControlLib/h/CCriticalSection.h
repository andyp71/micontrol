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

class CCriticalSection
{
	CRITICAL_SECTION		m_cs;

public:
	CCriticalSection()
	{
		InitializeCriticalSection(&m_cs);
	}

	~CCriticalSection()
	{
		DeleteCriticalSection(&m_cs);
	}

	LPCRITICAL_SECTION operator&()
	{
		return &m_cs;
	}
};

class CGuard
{
	CCriticalSection&		m_cs;

public:
	CGuard(CCriticalSection& cs)
		: m_cs(cs)
	{
		EnterCriticalSection(&m_cs);
	}

	~CGuard()
	{
		LeaveCriticalSection(&m_cs);
	}
};