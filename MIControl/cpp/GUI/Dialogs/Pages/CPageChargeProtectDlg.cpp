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

CPageChargeProtectDlg::CPageChargeProtectDlg(HINSTANCE hInstance, SendCommand_t fn, HOST_DATA& status)
	: CPageDlg(hInstance, fn, status)
	, m_tb(hInstance, 0x10, 0, 0, 0, 0, TBSTYLE_FLAT | CCS_NODIVIDER | TBSTYLE_TOOLTIPS | WS_TABSTOP | WS_VISIBLE, WS_EX_STATICEDGE)
{
	pushctl(m_tb);
	AutosizeCtl(m_tb);
}

CBitmap& CPageChargeProtectDlg::AddHeart(CBitmap& bmTb)
{
	CBitmap bmHeart;
	auto hFont = CreateFont(60, 30, 0, 0, 0, false, false, false, 0, 0, 0, 0, 0, TEXT("Arial"));
	bmHeart.Create(bmTb.Height(), bmTb.Height());
	bmHeart.RenderText(0, 0, bmHeart.Width(), bmHeart.Height(), TEXT("\x2665"), hFont, TRUE, RGB(68, 210, 118), 0, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	DeleteFont(hFont);

	auto pBits = bmHeart.Bits();
	auto uSize = bmHeart.Width() * bmHeart.Height();
	for (decltype(uSize)i = 0; i < uSize; ++i)
	{
		if (pBits->rgbRed || pBits->rgbBlue || pBits->rgbGreen)
			pBits->rgbReserved = pBits->rgbGreen;
		else
			pBits->rgbReserved = 0;

		pBits++;
	}

	bmTb.RenderBitmap(bmHeart, 0, 0, bmHeart.Width(), bmHeart.Height(), 5 * bmTb.Height() / 4, bmTb.Height() / 4, 255, TRUE);
	return bmTb;
}

BOOL CPageChargeProtectDlg::OnPageInit(LPARAM lParam)
{
	CBitmap bmTb, bmHeart;

	DeleteBitmap(m_tb.SetImageList(AddHeart(MergeBitmap(bmTb, { IDB_BAT_100, IDB_BAT_70 }, false)).Bitmap()));

	m_tb.AddRadio(0, IDB_BAT_100);
	m_tb.AddRadio(1, IDB_BAT_70);
	m_tb.Enable(m_status);

	return true;
}

void CPageChargeProtectDlg::OnEventCommon(uint8_t cmd, const std::vector<uint8_t>& data)
{
	if (0x10 != cmd)
		return;

	if (data[7])
		m_tb.cSendMessage(TB_CHECKBUTTON, IDB_BAT_70, true);
	else
		m_tb.cSendMessage(TB_CHECKBUTTON, IDB_BAT_100, true);

	m_uCurrentMode = data[7];
}

void CPageChargeProtectDlg::OnShow()
{
	SendCommandGet(0x10, 0);
}

void CPageChargeProtectDlg::OnButton(USHORT uId)
{
	switch (uId)
	{
	case IDB_BAT_100: SendCommandPut(0x10, false); m_uCurrentMode = false; break;
	case IDB_BAT_70: SendCommandPut(0x10, true); m_uCurrentMode = true; break;
	}

	SendCommandGet(0x10, 0);
}

void CPageChargeProtectDlg::OnServiceChange(bool bState)
{
	m_tb.Enable(bState);
	SendCommandGet(0x10, 0);
}

bool CPageChargeProtectDlg::GetLineColor(COLORREF& clr, UINT& iconId) const
{
	if (m_status)
	{
		iconId = m_uCurrentMode ? 1 : 2;
	}
	else
		iconId = 0;

	return true;
}