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

CPagePerfomanceDlg::CPagePerfomanceDlg(HINSTANCE hInstance, SendCommand_t fn, HOST_DATA& status)
    : CPageDlg(hInstance, fn, status)
    , m_tb(hInstance, 0x10, 0, 0, 0, 0, TBSTYLE_FLAT | CCS_NODIVIDER | TBSTYLE_TOOLTIPS | WS_TABSTOP | WS_VISIBLE, WS_EX_STATICEDGE)
    , m_st(hInstance, TEXT("Press Fn+K to change mode"), 7, 75, 100, 10)
{
    pushctl(m_tb);
    pushctl(m_st);
    AutosizeCtl(m_tb);
}

BOOL CPagePerfomanceDlg::OnPageInit(LPARAM lParam)
{
    CBitmap bmTb;
    DeleteBitmap(m_tb.SetImageList(MergeBitmap(bmTb, { IDB_PERFOMANCE_BALANCE , IDB_PERFOMANCE_FULLSPEED , IDB_PERFOMANCE_SILENCE , IDB_PERFOMANCE_TURBO , IDB_PERFOMANCE_AUTO }).Bitmap()));
    
    std::map<tstring, tstring> mComputer;
    try
    {
        CPageSysInfoDlg::QueryInfo(TEXT("SELECT * FROM Win32_ComputerSystem "), { TEXT("SystemSKUNumber") }, mComputer);
    }
    catch (COMException&)
    {}

    const bool bIsRedmibool2025 = mComputer[TEXT("SystemSKUNumber")].starts_with(TEXT("TM24"));
    m_tb.AddRadio(2, IDB_PERFOMANCE_SILENCE);
    
    if (!bIsRedmibool2025)
        m_tb.AddRadio(0, IDB_PERFOMANCE_BALANCE);
    m_tb.AddRadio(3, IDB_PERFOMANCE_TURBO);
    m_tb.AddRadio(1, IDB_PERFOMANCE_FULLSPEED);

    // check if it is redmibook 2025
    if (bIsRedmibool2025)
        m_tb.AddRadio(4, IDB_PERFOMANCE_AUTO);
    
    m_tb.cSendMessage(TB_CHECKBUTTON, IDB_PERFOMANCE_SILENCE, true);

    m_tb.Enable(m_status);

    return false;
}

void CPagePerfomanceDlg::OnEventCommon(uint8_t cmd, const std::vector<uint8_t>& data)
{
    if (data.size() <= 5 || 0x08 != cmd)
        return;

    OnEventDetail(0x16, data[5]);
}

void CPagePerfomanceDlg::OnEventDetail(uint8_t cmd, uint8_t arg)
{
    if (0x16 != cmd)
        return;

    switch (arg)
    {
    case 0x02: m_tb.cSendMessage(TB_CHECKBUTTON, IDB_PERFOMANCE_SILENCE, true); break;
    case 0x03: m_tb.cSendMessage(TB_CHECKBUTTON, IDB_PERFOMANCE_TURBO, true); break;
    case 0x01: m_tb.cSendMessage(TB_CHECKBUTTON, IDB_PERFOMANCE_BALANCE, true); break;
    case 0x04: m_tb.cSendMessage(TB_CHECKBUTTON, IDB_PERFOMANCE_FULLSPEED, true); break;
    case 0x09: m_tb.cSendMessage(TB_CHECKBUTTON, IDB_PERFOMANCE_AUTO, true); break;
    }

    m_tb.cSendMessage(TB_MARKBUTTON, IDB_PERFOMANCE_FULLSPEED, false);
    m_tb.cSendMessage(TB_MARKBUTTON, IDB_PERFOMANCE_TURBO, false);
    m_tb.cSendMessage(TB_MARKBUTTON, IDB_PERFOMANCE_BALANCE, false);
    m_tb.cSendMessage(TB_MARKBUTTON, IDB_PERFOMANCE_SILENCE, false);
    m_tb.cSendMessage(TB_MARKBUTTON, IDB_PERFOMANCE_AUTO, false);
    m_currentMode = arg;
}

void CPagePerfomanceDlg::OnButton(USHORT uId)
{
    switch (uId)
    {
    case IDB_PERFOMANCE_SILENCE: SendCommandPut(0x08, 0x02); break;
    case IDB_PERFOMANCE_BALANCE: SendCommandPut(0x08, 0x01); break;
    case IDB_PERFOMANCE_TURBO: SendCommandPut(0x08, 0x03); break;
    case IDB_PERFOMANCE_FULLSPEED: SendCommandPut(0x08, 0x04); break;
    case IDB_PERFOMANCE_AUTO: SendCommandPut(0x08, 0x09); break;
    }
}

void CPagePerfomanceDlg::OnServiceChange(bool bState)
{
    m_tb.Enable(bState);
}

bool CPagePerfomanceDlg::GetLineColor(COLORREF& clr, UINT& iconId) const
{
    iconId = 0;
    if (m_status && 4 == m_currentMode)
    {
        iconId = 2;
    }
    clr = CLR_INVALID;

    return true;
}