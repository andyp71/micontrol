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
#include <shellapi.h>
#include <Common.h>

CPageAboutDlg::CPageAboutDlg(HINSTANCE hInstance, SendCommand_t fn, HOST_DATA& status)
	: CPageDlg(hInstance, fn, status)
	, m_edInfo(hInstance, 0x10, TEXT(""), 7, 7, 100, 200, FALSE, TRUE)
	, m_lnkMailMe(hInstance, 0x11, WC_LINK, TEXT("<a href=\"mailto:micontrol@filezz.net\">micontrol@filezz.net</a>"),
		7, 220, 100, 15, WS_TABSTOP | WS_VISIBLE)
	, m_chkAutorun(hInstance, 0x12, TEXT("Run with Logon"), 150, 220, 120, 10, false)
{
	pushctl(m_edInfo);
	pushctl(m_lnkMailMe);
	pushctl(m_chkAutorun);
}

BOOL CPageAboutDlg::OnPageInit(LPARAM lParam)
{
	CString str;
	std::ignore = str.LoadStringW(IDS_ABOUT);
	m_edInfo.Text(str.GetString());
	m_chkAutorun.Check(IsAutorun());
	return TRUE;
}

void CPageAboutDlg::OnSizeChanged(INT, USHORT, USHORT)
{
	RECT rc({ 0 }), rcEd({ 0 });
	GetWindowRect(*this, &rc);
	GetWindowRect(m_edInfo, &rcEd);
	ScreenToClient(*this, LPPOINT(&rcEd));
	ScreenToClient(*this, LPPOINT(&rcEd) + 1);
	GetClientRect(*this, &rc);

	SetWindowPos(m_edInfo, nullptr, 0, 0, rc.right - (rcEd.left << 1), rcEd.bottom - rcEd.top, SWP_NOMOVE);
}

void CPageAboutDlg::OnButton(USHORT uId)
{
	if (0x12 != uId)
		return;

	if (!m_chkAutorun.Check())
		SetAutorun(true);
	else
		SetAutorun(false);

	m_chkAutorun.Check(IsAutorun());
}

INT_PTR CPageAboutDlg::OnNotify(LPNMHDR lpnm)
{
	switch (lpnm->idFrom)
	{
		case 0x11:
		{
			if (lpnm->code == NM_CLICK || lpnm->code == NM_RETURN)
			{
				NMLINK* nml = (NMLINK*)lpnm;
				ShellExecute(*this, TEXT("open"), TEXT("mailto:micontrol@filezz.net?subject=MiControl%20question"), NULL, NULL, SW_SHOW);
			}
			break;
		}
	}

	return 0;
}

bool CPageAboutDlg::IsAutorun() const
{
	CRegKey keyRun;
	tstring str = TEXT("Soft");
	str += TEXT("ware");
	str += TEXT("\\");
	str += TEXT("Mi");
	str += TEXT("crosoft\\");
	str += TEXT("W");
	str += TEXT("indows\\Cur");
	str += TEXT("rentVer");
	str += TEXT("sion\\R");
	str += TEXT("un");

	if (ERROR_SUCCESS != keyRun.Open(HKEY_CURRENT_USER, str.c_str(), KEY_QUERY_VALUE))
		return false;

	ULONG unChars = 1000;
	tstring strValue(unChars, 0);
	
	if (ERROR_SUCCESS != keyRun.QueryStringValue(TEXT("MIControl"), strValue.data(), &unChars))
		return false;

	return true;
}

void CPageAboutDlg::SetAutorun(bool fState) const
{
	CRegKey keyRun;
	tstring str = TEXT("Soft");
	str += TEXT("ware");
	str += TEXT("\\");
	str += TEXT("Mi");
	str += TEXT("crosoft\\");
	str += TEXT("W");
	str += TEXT("indows\\Cur");
	str += TEXT("rentVer");
	str += TEXT("sion\\R");
	str += TEXT("un");

	if (ERROR_SUCCESS != keyRun.Open(HKEY_CURRENT_USER, str.c_str(), KEY_QUERY_VALUE | KEY_SET_VALUE))
		return;

	if (fState)
		keyRun.SetStringValue(TEXT("MIControl"), (TEXT("\"") + ImagePath() + TEXT("\" /tray")).c_str(), REG_SZ);
	else
		keyRun.DeleteValue(TEXT("MIControl"));
}

