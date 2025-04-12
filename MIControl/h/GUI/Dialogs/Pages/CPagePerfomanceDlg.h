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

class CPagePerfomanceDlg : public CPageDlg
{
	CToolbar		m_tb;
	CStaticControl	m_st;
	uint8_t			m_currentMode = 0;

	BOOL			OnPageInit(LPARAM lParam) override;
	void			OnEventDetail(uint8_t cmd, uint8_t arg) override;
	void			OnEventCommon(uint8_t cmd, const std::vector<uint8_t>& data) override;
	void			OnButton(USHORT uId) override;
	void			OnServiceChange(bool bState);
	bool			GetLineColor(COLORREF& clr, UINT& iconId) const override;

public:
	CPagePerfomanceDlg(HINSTANCE hInstance, SendCommand_t, HOST_DATA& status);
};