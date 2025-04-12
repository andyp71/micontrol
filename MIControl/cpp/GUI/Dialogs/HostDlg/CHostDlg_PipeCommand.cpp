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
#include <Common.h>
#include <shellapi.h>

void CHostDlg::OnPipeStartStop(bool bStarted)
{
    m_pipe.fPipeAvailable = bStarted;

    if (bStarted)
    {
        Sleep(100);
        SendCommandGet(0x10, 0);
        SendCommandGet(0x0A, 0);
        SendCommandGet(0x08, 0);
    }

    SendMessage(*this, WM_CUSTOM_SHOW_SPLASH, 0, 0);
}

VOID CHostDlg::OnEventDetail(const CEventHandler<EventDetail>& ev)
{
    auto details = ev->details();
    if (details.size() < 3)
        return;

    auto itEv = m_dtlEv.find(details[1]);
    if (itEv != m_dtlEv.end())
    {
        auto& fn = itEv->second;
        fn(details[2]);
    }

    Broadcast_EventDetail(details[1], details[2]);
}

VOID CHostDlg::OnCommonRequestByte(const CEventHandler<CommonRequestByte>& ev)
{
    auto cmd = ev->command();
    if (cmd.empty())
        return;

    Broadcast_Common(cmd[0], std::vector<uint8_t>(cmd.begin(), cmd.end()));

    auto itCmd = m_getEv.find(cmd[0]);
    if (itCmd == m_getEv.end())
        return;

    auto& fn = itCmd->second;
    fn(std::vector<uint8_t>(cmd.begin(), cmd.end()));
}

VOID CHostDlg::OnEventWorkload(uint8_t arg)
{
    SendMessage(*this, WM_CUSTOM_SHOW_SPLASH, 0, 0);
}

VOID CHostDlg::OnEventMiclock(uint8_t arg)
{
    return VOID();
}

VOID CHostDlg::OnWorkloadModeGet(const std::vector<uint8_t>& data)
{
    if (data.size() <= 5)
        return;

    OnEventWorkload(data[5]);
}

VOID CHostDlg::OnPowerModeGet(const std::vector<uint8_t>& data)
{
    if (data.size() <= 7)
        return;

    SendMessage(*this, WM_CUSTOM_SHOW_SPLASH, 0, 0);
}

VOID CHostDlg::OnCameraModeGet(const std::vector<uint8_t>& data)
{
    if (data.size() <= 2)
        return;

    OnSpecialButtonPressed(0xA0, data[2]);
}

VOID CHostDlg::OnMicLockGet(const std::vector<uint8_t>& data)
{
    if (data.size() <= 7)
        return;

    OnEventMiclock(data[7]);
}

VOID CHostDlg::Broadcast_EventDetail(uint8_t cmd, uint8_t arg)
{
    for (auto& page : m_mPage)
        page.second->OnEventDetail(cmd, arg);

    OnSpecialButtonPressed(cmd, arg);
}

VOID CHostDlg::Broadcast_Common(uint8_t cmd, const std::vector<uint8_t>& data)
{
    for (auto& page : m_mPage)
        page.second->OnEventCommon(cmd, data);
}

bool CHostDlg::SendCommand(uint8_t type, uint8_t cmd, uint8_t arg)
{
    if (!m_pipe.fPipeAvailable)
        return false;

    CommonRequestByte cr;
    std::vector<uint8_t> command = { cmd, arg };
    cr.set_command(command.data(), command.size());

    std::vector<uint8_t> data = { 0x00, 0x00, 0x00, 0x00, type };
    std::string serialized;
    if (cr.SerializeToString(&serialized))
    {
        data.insert(data.end(), serialized.begin(), serialized.end());
        m_pipe.listener->SendData(data);
        return true;
    }

    return false;
}

bool CHostDlg::SendCommandPut(uint8_t command, uint8_t arg)
{
    return SendCommand(0xC0, command, arg);
}

bool CHostDlg::SendCommandGet(uint8_t command, uint8_t arg)
{
    return SendCommand(0xC1, command, arg);
}

void CHostDlg::OnSpecialButtonPressed(uint8_t cmd, uint8_t arg)
{
    USHORT m = MAKEWORD(arg, cmd);
    UINT res = 0;

    switch (m)
    {
    case 0x0700: res = IDB_FNLOCK_OFF; break;
    case 0x0701: res = IDB_FNLOCK_ON; break;
    case 0x2100: res = IDB_MIC_OFF; break;
    case 0x2101: res = IDB_MIC_ON; break;
    case 0x0100: res = OnScreenSettings(); break;
    case 0x0200: res = OnScreenCut(); break;
    case 0x1B00: res = OnSystemSettings(); break;
    case 0x0500: res = IDB_KBHL_0; break;
    case 0x0501: res = IDB_KBHL_1; break;
    case 0x0502: res = IDB_KBHL_2; break;
    case 0x0503: res = IDB_KBHL_3; break;
    case 0x0504: res = IDB_KBHL_4; break;
    case 0x0505: res = IDB_KBHL_5; break;
    case 0x0506: res = IDB_KBHL_6; break;
    case 0x0507: res = IDB_KBHL_7; break;
    case 0x0508: res = IDB_KBHL_8; break;
    case 0x0509: res = IDB_KBHL_9; break;
    case 0x050A: res = IDB_KBHL_10; break;
    case 0x0580: res = IDB_KBHL_AUTO; break;
    case 0x1801: __fallthrough;
    case 0x1901: res = OnMiButton(cmd - 0x18); break;
    case 0x0900: res = IDB_CAPSLOCK_OFF; break;
    case 0x0901: res = IDB_CAPSLOCK_ON; break;
    case 0x1300: __fallthrough;
    case 0x1A00: res = OnDisplateRateChange(); break;
    case 0x1601: res = IDB_PERFOMANCE_BALANCE; break;
    case 0x1602: res = IDB_PERFOMANCE_SILENCE; break;
    case 0x1603: res = IDB_PERFOMANCE_TURBO; break;
    case 0x1604: res = IDB_PERFOMANCE_FULLSPEED; break;
    case 0x1609: res = IDB_PERFOMANCE_AUTO; break;
    case 0x1000: res = OnTouchpadChange(); break;
    case 0x0102: res = IDB_WRONG_CHARGER; break;
    case 0xA000: res = IDB_MIC_OFF; break;
    case 0xA001: res = IDB_MIC_ON; break;
    default: res = OnUnknownButton(cmd, arg); break;
    }

    if (res > 0)
        SendMessage(*this, WM_CUSTOM_SHOW_SPLASH, res, 0);
}

UINT CHostDlg::OnScreenSettings()
{
    // press Win+P
    INPUT inp({ 0 });
    ZeroMemory(&inp, sizeof(inp)); inp.type = 1; inp.ki.wVk = VK_LWIN;	SendInput(1, &inp, sizeof(inp));
    ZeroMemory(&inp, sizeof(inp)); inp.type = 1; inp.ki.wVk = 'P';		SendInput(1, &inp, sizeof(inp));
    ZeroMemory(&inp, sizeof(inp)); inp.type = 1; inp.ki.wVk = 'P';		inp.ki.dwFlags = KEYEVENTF_KEYUP;	SendInput(1, &inp, sizeof(inp));
    ZeroMemory(&inp, sizeof(inp)); inp.type = 1; inp.ki.wVk = VK_LWIN;	inp.ki.dwFlags = KEYEVENTF_KEYUP;	SendInput(1, &inp, sizeof(inp));

    return 0;
}

UINT CHostDlg::OnScreenCut()
{
    ShellExecute(*this, TEXT("open"), TEXT("snippingtool.exe"), nullptr, nullptr, SW_SHOW);
    return 0;
}

UINT CHostDlg::OnSystemSettings()
{
    ShellExecute(*this, TEXT("open"), TEXT("ms-settings:"), nullptr, nullptr, SW_SHOW);
    return 0;
}

UINT CHostDlg::OnMiButton(BOOL fReleased)
{
    SendMessage(*this, WM_CUSTOM_SHOW_SPLASH, fReleased, 1);

    return 0;
}

UINT CHostDlg::OnDisplateRateChange()
{
    int modeNum = 0;
    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(DEVMODE));
    devMode.dmSize = sizeof(DEVMODE);
    std::vector<DWORD> rates;
    std::vector<uint8_t> vDeniedModes;
    CPageRefreshRateDlg::ListDeniedModes(vDeniedModes);


    while (EnumDisplaySettings(nullptr, modeNum++, &devMode))
    {
        if (std::find(rates.begin(), rates.end(), devMode.dmDisplayFrequency) != rates.end())
            continue;

        rates.emplace_back(devMode.dmDisplayFrequency);
    }

    std::sort(rates.begin(), rates.end());

    ZeroMemory(&devMode, sizeof(DEVMODE));
    devMode.dmSize = sizeof(DEVMODE);
    EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);

    auto rate = std::find(rates.begin(), rates.end(), devMode.dmDisplayFrequency);
    if (rate == rates.end())
        return 0;

    do
    {
        rate++;
        if (rate >= rates.end())
            rate = rates.begin();

        auto it = std::find(vDeniedModes.begin(), vDeniedModes.end(), *rate);
        if (it == vDeniedModes.end())
            break;

    } while (*rate != devMode.dmDisplayFrequency);

    if (*rate == devMode.dmDisplayFrequency)
        return 0;

    devMode.dmDisplayFrequency = *rate;
    ChangeDisplaySettings(&devMode, CDS_UPDATEREGISTRY);

    m_mPage[pageDisplayRate]->OnShow();

    switch (*rate)
    {
    case 48: return IDB_RATE_48; break;
    case 60: return IDB_RATE_60; break;
    case 72: return IDB_RATE_72; break;
    case 75: return IDB_RATE_75; break;
    case 90: return IDB_RATE_90; break;
    case 100: return IDB_RATE_100; break;
    case 120: return IDB_RATE_120; break;
    case 144: return IDB_RATE_144; break;
    case 165: return IDB_RATE_165; break;
    case 240: return IDB_RATE_240; break;
    }

    return 0;
}

UINT CHostDlg::OnUnknownButton(uint8_t cmd, uint8_t arg)
{
    _tprintf(TEXT("Button %d, arg %d\n"), cmd, arg);
    return 0;
}

UINT CHostDlg::OnTouchpadChange()
{
    CRegKey keyTouchpad;
    if (ERROR_SUCCESS != keyTouchpad.Open(HKEY_CURRENT_USER, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\PrecisionTouchPad\\Status"), KEY_QUERY_VALUE))
        return 0;

    DWORD val = 0;
    if (ERROR_SUCCESS != keyTouchpad.QueryDWORDValue(TEXT("Enabled"), val))
        return 0;

    bool state = !val;
    EnableTouchPad(state);
    m_mPage[pageTouchpad]->OnShow();

    if (state)
        return IDB_TOUCHPAD_ON;
    else
        return IDB_TOUCHPAD_OFF;
}
