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

CLog::CLog(const tstring& strName, CLogLevel level)
#ifdef UNICODE
	: m_hFile(_wfopen((ImagePath()+TEXT(".log")).c_str(), TEXT("a+"))), m_strName(strName), m_logLevel(level)
#else
	: m_hFile(fopen((ImagePath() + TEXT(".log")).c_str(), TEXT("a+"))), m_strName(strName), m_logLevel(level)
#endif
{
}

CLog::CLog(const tstring& strFilename, const tstring& strName, CLogLevel level)
#ifdef UNICODE
	: m_hFile(_wfopen(strFilename.c_str(), TEXT("a+"))), m_strName(strName), m_logLevel(level)
#else
	: m_hFile(fopen(strFilename.c_str(), TEXT("a+"))), m_strName(strName), m_logLevel(level)
#endif
{
	if (!m_hFile)
		m_hFile = stdout;
}

CLog::CLog(int, const tstring& strName, CLogLevel level)
	: m_hFile(nullptr), m_strName(strName), m_logLevel(level)
{
}

void CLog::SetDateFormat(const tstring& strFormat)
{
	m_strDateFormat = strFormat;
}