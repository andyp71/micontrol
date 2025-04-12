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

class CPageRefreshRateDlg: public CPageDlg
{
	CToolbar		m_tb;
	CListView		m_lvListRates;
	CStaticControl	m_st;
	tstring			m_strDisplay;
	std::atomic_bool m_fListUpdatePending = false;

	void			ResolveDisplayDevName();

	BOOL			OnPageInit(LPARAM lParam) override;
	void			OnButton(USHORT uId) override;
	void			OnShow() override;
	INT_PTR			OnNotify(LPNMHDR lpnm) override;
	INT_PTR			LvControl(NMLISTVIEW* pnmlv);
	void			UpdateToolbar(bool fUpdateList = true);

public:
	enum class ModifyDeniedModeOp
	{
		Add,
		Remove
	};

	CPageRefreshRateDlg(HINSTANCE hInstance, SendCommand_t, HOST_DATA& status);
	static void		ListDeniedModes(std::vector<uint8_t>& deniedModes);
	static void		ModifyDeniedModes(uint8_t mode, ModifyDeniedModeOp op);
};
