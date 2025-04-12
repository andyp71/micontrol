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

CPageMiButtonDlg::CPageMiButtonDlg(HINSTANCE hInstance, SendCommand_t fn, HOST_DATA& status)
	: CPageDlg(hInstance, fn, status)
	, m_rdUtility(hInstance, 0x10, TEXT("Show this utility"), 7, 7, 100, 15)
	, m_rdCommand(hInstance, 0x11, TEXT("Run command below"), 7, 27, 100, 15)
	, m_edCtl(hInstance, 0x12, TEXT(""), 7, 47, 250, 15)
	, m_chkElevated(hInstance, 0x20, TEXT("Run as Administrator"), 7, 67, 100, 15)
	, m_btnApply(hInstance, 0x13, TEXT("&Apply"), 7, 87, 50, 15)
{
	pushctl(m_rdUtility);
	pushctl(m_rdCommand);
	pushctl(m_chkElevated);
	pushctl(m_edCtl);
	pushctl(m_btnApply);
}

BOOL CPageMiButtonDlg::OnPageInit(LPARAM lParam)
{
	m_edCtl.Enable(FALSE);
	m_btnApply.Enable(FALSE);
	m_chkElevated.Enable(FALSE);

	m_rdUtility.Check(TRUE);

	CRegKey keyMI;
	if (ERROR_SUCCESS != keyMI.Create(HKEY_CURRENT_USER, TEXT("Software\\MIControl"), nullptr, 0, KEY_QUERY_VALUE))
		return TRUE;

	ULONG l = 1000;
	tstring strCommand(l, 0);
	if (keyMI.QueryStringValue(nullptr, strCommand.data(), &l))
		return TRUE;

	strCommand.resize(l);
	while (!strCommand.empty() && '\x00' == strCommand.back())
		strCommand.pop_back();

	if (strCommand.empty())
		return TRUE;

	l = 0; keyMI.QueryDWORDValue(TEXT("Elevated"), l);
	m_rdUtility.Check(FALSE);
	m_rdCommand.Check(TRUE);
	m_chkElevated.Check(l);
	m_edCtl.Text(strCommand);
	EnableAll(TRUE, 0x12, 0x20, 0);

	return TRUE;
}

void CPageMiButtonDlg::OnButton(USHORT uId)
{
	switch (uId)
	{
	case 0x10:
	{
		CRegKey keyMI;
		if (ERROR_SUCCESS != keyMI.Create(HKEY_CURRENT_USER, TEXT("Software\\MIControl")))
			if (ERROR_SUCCESS != keyMI.Open(HKEY_CURRENT_USER, TEXT("Software\\MIControl"), KEY_SET_VALUE))
				break;
				
		keyMI.SetStringValue(nullptr, TEXT(""), REG_EXPAND_SZ);
		EnableAll(FALSE, 0x12, 0x13, 0x20, 0);
		break;
	}
	case 0x11: EnableAll(TRUE, 0x12, 0x20, 0); __fallthrough;
	case 0x13:
		{
			CRegKey keyMI;
			if (ERROR_SUCCESS != keyMI.Create(HKEY_CURRENT_USER, TEXT("Software\\MIControl")))
				if (ERROR_SUCCESS != keyMI.Open(HKEY_CURRENT_USER, TEXT("Software\\MIControl"), KEY_SET_VALUE))
					break;

			auto err = keyMI.SetStringValue(nullptr, m_edCtl.Text().c_str(), REG_EXPAND_SZ);
			if (err)
				ShowError(Win32Error(err).description().c_str());
			else
				EnableAll(FALSE, 0x13, 0);

			OnButton(0x20);	// also update Elevated
			break;
		}
	case 0x20:
		{
			CRegKey keyMI;
			if (ERROR_SUCCESS != keyMI.Open(HKEY_CURRENT_USER, TEXT("Software\\MIControl"), KEY_SET_VALUE))
				break;

			keyMI.SetDWORDValue(TEXT("Elevated"), m_chkElevated.Check() ? 1 : 0);
			break;
		}
	}
}

void CPageMiButtonDlg::OnCommand(USHORT uCmd, USHORT uId, HWND hCtl)
{
	if (EN_CHANGE == uCmd && 0x12 == uId)
		EnableAll(TRUE, 0x13, 0);
}