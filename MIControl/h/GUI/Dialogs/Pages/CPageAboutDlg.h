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

class CPageAboutDlg : public CPageDlg
{
	CMultilineEditControl	m_edInfo;
	CControl				m_lnkMailMe;
	CCheckBoxControl		m_chkAutorun;

	BOOL			OnPageInit(LPARAM lParam) override;
	void			OnSizeChanged(INT, USHORT, USHORT) override;
	INT_PTR			OnNotify(LPNMHDR lpnm) override;
	void			OnButton(USHORT uId) override;
	
	bool			IsAutorun() const;
	void			SetAutorun(bool fState) const;

public:
	CPageAboutDlg(HINSTANCE hInstance, SendCommand_t fn, HOST_DATA& status);
};