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

CPageMicDlg::CPageMicDlg(HINSTANCE hInstance, SendCommand_t fn, HOST_DATA& status)
    : CPageDlg(hInstance, fn, status)
    , m_tb(hInstance, 0x10, 0, 0, 0, 0, TBSTYLE_FLAT | CCS_NODIVIDER | TBSTYLE_TOOLTIPS | WS_TABSTOP | WS_VISIBLE, WS_EX_STATICEDGE)
    , m_st(hInstance, TEXT("Press Fn+F4 (Fn+F7 for NON-PRO laptop) to change mode"), 7, 75, 200, 10)
{
    pushctl(m_tb);
    pushctl(m_st);
    AutosizeCtl(m_tb);
}

BOOL CPageMicDlg::OnPageInit(LPARAM lParam)
{
    CBitmap bmTb;
    DeleteBitmap(m_tb.SetImageList(MergeBitmap(bmTb, { IDB_MIC_OFF, IDB_MIC_ON }).Bitmap()));

    m_tb.AddRadio(0, IDB_MIC_OFF);
    m_tb.AddRadio(1, IDB_MIC_ON);
    m_tb.CheckAll(true, IDB_MIC_OFF, 0);

    m_tb.Enable(m_status);

    return true;
}

void CPageMicDlg::OnEventDetail(uint8_t cmd, uint8_t arg)
{
    if (0x21 != cmd)
        return;

    if (!arg)
        m_tb.cSendMessage(TB_CHECKBUTTON, IDB_MIC_OFF, true);
    else
        m_tb.cSendMessage(TB_CHECKBUTTON, IDB_MIC_ON, true);

    m_uCurrentMode = arg;
}

void CPageMicDlg::OnEventCommon(uint8_t cmd, const std::vector<uint8_t>& data)
{
    if (0x0A != cmd || data.size() <= 7)
        return;

    OnEventDetail(0x21, data[7]);
}

void CPageMicDlg::OnButton(USHORT uId)
{
    switch (uId)
    {
    case IDB_MIC_ON: SendCommandPut(0x0A, true); break;
    case IDB_MIC_OFF: SendCommandPut(0x0A, false); break;
    }
}

void CPageMicDlg::OnServiceChange(bool bState)
{
    m_tb.Enable(bState);
}

bool CPageMicDlg::GetLineColor(COLORREF& clr, UINT& iconId) const
{
    iconId = 0;
    if (m_status && m_uCurrentMode)
    {
        iconId = 1;
    }

    return true;
}