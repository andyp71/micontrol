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

CPageDlg::CPageDlg(HINSTANCE hInstance, SendCommand_t fnSendCommand, HOST_DATA& data)
	: CDialog(hInstance, WS_CHILD | DS_CONTROL | DS_SHELLFONT, WS_EX_STATICEDGE,
		0, 0, 0, 0, TEXT(""), 0, nullptr)
	, m_fnSendCommand(fnSendCommand)
	, m_hostdata(data)
	, m_status(data.status)
{

}

BOOL CPageDlg::OnInit(LPARAM lParam)
{
	SetTimer(*this, 10, 100, nullptr);
	return OnPageInit(lParam);
}

BOOL CPageDlg::OnPageInit(LPARAM lParam)
{
	return TRUE;
}

VOID CPageDlg::OnCancel()
{
	return VOID();
}

BOOL CPageDlg::OnClose()
{
	return FALSE;
}

void CPageDlg::OnSize(INT dw, USHORT x, USHORT y)
{
	OnSizing(dw, x, y);
}

void CPageDlg::OnSizing(INT dw, USHORT x, USHORT y)
{
	for (auto& pctl : m_vAutoSizeCtl)
		pctl->cSendMessage(WM_SIZE);

	OnSizeChanged(dw, x, y);
}

void CPageDlg::OnSizeChanged(INT, USHORT, USHORT)
{
	return void();
}

CBitmap& CPageDlg::MergeBitmap(CBitmap& bmTarget, const std::vector<UINT>& rcList, bool fConvert)
{
	std::vector<CBitmap> bmList;

	for (const auto& rc : rcList)
	{
		CPngLoader png(GetInstance(), MAKEINTRESOURCE(rc), TEXT("PNG"));
		png.FixedBitmap(bmList.emplace_back());
	}

	int y = 0, x = 0;
	for (auto& bm : bmList)
	{
		y = std::max(y, bm.Height());
		x = std::max(x, bm.Width());
	}

	auto width = std::max(x, y);

	bmTarget.Create(width * static_cast<int>(bmList.size()), width);

	x = 0;
	for (auto& bm : bmList)
	{
		bmTarget.RenderBitmap(bm, 0, 0, bm.Width(), bm.Height(), x + std::abs(bm.Width() - width) / 2, std::abs(bm.Height() - width) / 2);
		x += width;
	}

	if (!fConvert)
		return bmTarget;

	DWORD size = bmTarget.Width() * bmTarget.Height();
	auto rgb = bmTarget.Bits();
	for (decltype(size)i = 0; i < size; ++i)
	{
		rgb->rgbReserved = rgb->rgbRed;

		if (rgb->rgbRed == rgb->rgbGreen && rgb->rgbGreen == rgb->rgbBlue)
		{
			rgb->rgbGreen = 255 - rgb->rgbGreen;
			rgb->rgbBlue = 255 - rgb->rgbBlue;
			rgb->rgbRed = 255 - rgb->rgbRed;
		}
		rgb++;
	}

	return bmTarget;
}

void CPageDlg::AutosizeCtl(CControl& ctl)
{
	m_vAutoSizeCtl.push_back(&ctl);
}

void CPageDlg::OnEventDetail(uint8_t cmd, uint8_t arg)
{

}

void CPageDlg::OnEventCommon(uint8_t cmd, const std::vector<uint8_t>& data)
{

}

bool CPageDlg::SendCommand(uint8_t type, uint8_t cmd, uint8_t arg)
{
	return m_fnSendCommand(type, cmd, arg);
}


bool CPageDlg::SendCommandPut(uint8_t command, uint8_t arg)
{
	return SendCommand(0xC0, command, arg);
}

bool CPageDlg::SendCommandGet(uint8_t command, uint8_t arg)
{
	return SendCommand(0xC1, command, arg);
}

void CPageDlg::OnShow()
{

}

VOID CPageDlg::OnTimer(DWORD_PTR idTimer, FARPROC pTimerProc)
{
	if (10 == idTimer)
	{
		bool bState = m_status;
		if (bState ^ m_bStatus)
		{
			m_bStatus = bState;
			OnServiceChange(m_bStatus);
		}
	}
}

void CPageDlg::OnServiceChange(bool bState)
{
	return void();
}

bool CPageDlg::GetLineColor(COLORREF& clr, UINT& iconId) const
{
	clr;
	iconId;
	return false;
}

bool CPageDlg::IsCanQuit() const
{
	return true;
}