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

#include <atltime.h>
#include <atlstr.h>

using HSTDFILE = CAutoHandle<FILE*, nullptr, fclose>;

class CLog
{
	HSTDFILE			m_hFile;
	const tstring		m_strName;
	CCriticalSection	m_cs;

public:
	enum CLogLevel
	{
		levelNone = 0,
		levelError = 1,
		levelWarning = 2,
		levelInfo = 3,
		levelDebug = 4
	};

private:
	CLogLevel		m_logLevel = levelWarning;
	tstring m_strDateFormat = TEXT("%Y-%m-%d %X");

	tstring Level2String(CLogLevel level)
	{
		switch (level)
		{
		case levelError: return tstring(TEXT("ERROR"));
		case levelWarning: return tstring(TEXT("WARNING"));
		case levelInfo: return tstring(TEXT("INFO"));
		case levelDebug: return tstring(TEXT("DEBUG"));
		}

		return tstring();
	}

public:
	CLog(int, const tstring& strName, CLogLevel level = CLogLevel::levelWarning);
	CLog(const tstring& strName, CLogLevel level = CLogLevel::levelWarning);
	CLog(const tstring& strFilename, const tstring& strName, CLogLevel level = CLogLevel::levelWarning);

	void SetDateFormat(const tstring& strFormat = TEXT("%Y.%m.%d %X"));
	template<typename... Args>
	void log(CLogLevel level, const tstring & format, Args... args)
	{
		if (level > m_logLevel)
			return;

		CTime ttime(CTime::GetCurrentTime());

		CGuard lock(m_cs);
		if (m_hFile)
		{
#ifdef UNICODE
			fwprintf(m_hFile, TEXT("[%s] [%s] %s: "), ttime.Format(m_strDateFormat.c_str()).GetString(), m_strName.c_str(), Level2String(level).c_str());
			fwprintf(m_hFile, (format + TEXT("\n")).c_str(), args...);
#else
			fprintf(m_hFile, TEXT("[%s] [%s] %s: "), ttime.Format(m_strDateFormat.c_str()).GetString(), m_strName.c_str(), Level2String(level).c_str());
			fprintf(m_hFile, (format + TEXT("\n")).c_str(), args...);
#endif

			fflush(m_hFile);
		}
		else
		{
			CString strFormat;
			CString strOutput;
			strFormat.Format(TEXT("[%s] [%s] %s: "), ttime.Format(m_strDateFormat.c_str()).GetString(), m_strName.c_str(), Level2String(level).c_str());
			strOutput.Format((format + TEXT("\n")).c_str(), args...);
			OutputDebugString(strFormat + strOutput);
		}
	}

	template<typename... Args>
	void debug(const tstring& format, Args... args)
	{
		log(levelDebug, format, args...);
	}

	template<typename... Args>
	void info(const tstring& format, Args... args)
	{
		log(levelInfo, format, args...);
	}

	template<typename... Args>
	void warning(const tstring& format, Args... args)
	{
		log(levelWarning, format, args...);
	}

	template<typename... Args>
	void error(const tstring& format, Args... args)
	{
		log(levelError, format, args...);
	}
};