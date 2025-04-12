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

CPageServiceStatusDlg::CPageServiceStatusDlg(HINSTANCE hInstance, SendCommand_t fn, HOST_DATA& status)
	: CPageDlg(hInstance, fn, status)
	, m_edInfo(hInstance, 0x10, TEXT(""), 7, 7, 100, 200, FALSE, TRUE)
	, m_btnService(hInstance, 0x11, TEXT("Services..."), 7, 215, 50, 15)
{
	pushctl(m_edInfo);
	pushctl(m_btnService);
}

BOOL CPageServiceStatusDlg::OnPageInit(LPARAM lParam)
{
	return TRUE;
}

void CPageServiceStatusDlg::OnShow()
{
	SetServiceStateText(m_status);
}

void CPageServiceStatusDlg::OnButton(USHORT uId)
{
	if (0x11 == uId)
		ShellExecute(*this, TEXT("runas"), TEXT("services.msc"), nullptr, nullptr, SW_SHOW);
}

bool CPageServiceStatusDlg::GetLineColor(COLORREF& clr, UINT& iconId) const
{
	clr = (m_status ? RGB(18, 150, 48) : RGB(150, 18, 48));
	iconId = (m_status ? 1 : 2);
	return true;
}

void CPageServiceStatusDlg::OnSizeChanged(INT, USHORT, USHORT)
{
	RECT rc({ 0 }), rcEd({ 0 });
	
	GetWindowRect(*this, &rc);
	GetWindowRect(m_edInfo, &rcEd);
	ScreenToClient(*this, LPPOINT(&rcEd));
	ScreenToClient(*this, LPPOINT(&rcEd) + 1);
	GetClientRect(*this, &rc);

	SetWindowPos(m_edInfo, nullptr, 0, 0, rc.right - (rcEd.left << 1), rcEd.bottom - rcEd.top, SWP_NOMOVE);
}

void CPageServiceStatusDlg::OnServiceChange(bool bState)
{
	SetServiceStateText(bState);
}

void CPageServiceStatusDlg::SetServiceStateText(bool bState)
{
	// void CPageServiceStatusDlg::OnServiceChange(bool bState)
	const tstring strLastLine = TEXT("Press button below to find service 'MI Hardware Controller and Monitor'");

	tstring strInfo =
		(
			bState ?
			TEXT("Everything is OK, service seems to be works fine.\r\nNo actions is required.")
			:
			TEXT("Can't connect to the 'MI Hardware Controller and Monitor'!\r\n")
			TEXT("Try to restart the service\r\n")
			TEXT("If this action doesn't help, try to reinstall service as follow:\r\n")
			TEXT("\t- Run the command line (cmd.exe) from the Administrator\r\n")
			TEXT("\t- Go to the folder of application and execute commands:\r\n")
			TEXT("\t> MIControl.exe /uninstall\r\n")
			TEXT("\t> MIControl.exe /install\r\n")
			TEXT("\t- If it is doesn't help, ask me here: https://t.me/micontrol_utility")
			);

	m_edInfo.Text(strInfo + TEXT("\r\n\r\n") + strLastLine);
}