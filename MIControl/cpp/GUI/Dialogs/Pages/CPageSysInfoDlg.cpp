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

CPageSysInfoDlg::CPageSysInfoDlg(HINSTANCE hInstance, SendCommand_t fn, HOST_DATA& status)
    : CPageDlg(hInstance, fn, status)
    , m_edInfo(hInstance, 0x10, TEXT(""), 7, 7, 100, 100, FALSE, TRUE)
{
    pushctl(m_edInfo);
}

bool CPageSysInfoDlg::QueryInfo(const tstring& qStr, std::initializer_list<tstring> fields, std::map<tstring, tstring>& values)
{
    WMIQuery q(TEXT("root\\CIMV2"));
    std::vector<WbemClassItem> rows;
    if (!q.Query(qStr, rows))
        return false;

    for (auto& row : rows)
    {
        for (const auto& field : fields)
        {
            auto it = row.find(field);
            if (it == row.end())
                continue;

            auto& vt = it->second;
            if (VT_BSTR != vt->vt)
                continue;

            values[field] = vt->bstrVal;
        }
    }

    return true;
}

BOOL CPageSysInfoDlg::OnPageInit(LPARAM lParam)
{
    std::map<tstring, tstring> mBios, mComputer;
    try
    {
        QueryInfo(TEXT("SELECT * FROM Win32_BIOS"), { TEXT("SerialNumber"), TEXT("SMBIOSBIOSVersion") }, mBios);
        QueryInfo(TEXT("SELECT * FROM Win32_ComputerSystem "), { TEXT("Caption"), TEXT("Manufacturer"), TEXT("Model"), TEXT("SystemSKUNumber"), TEXT("UserName") }, mComputer);
    }
    catch (COMException&)
    {
        return TRUE;
    }

    CString strFormat;

    strFormat.Format(TEXT("Device serial number: %s\r\nBios version: %s\r\nComputer name: %s\r\nManufacturer: %s\r\nModel: %s\r\nModel ID: %s\r\nUsername: %s\r\n")
        , mBios[TEXT("SerialNumber")].c_str()
        , mBios[TEXT("SMBIOSBIOSVersion")].c_str()
        , mComputer[TEXT("Caption")].c_str()
        , mComputer[TEXT("Manufacturer")].c_str()
        , mComputer[TEXT("Model")].c_str()
        , mComputer[TEXT("SystemSKUNumber")].c_str()
        , mComputer[TEXT("UserName")].c_str()
    );

    m_edInfo.Text(strFormat.GetString());

    return TRUE;
}

void CPageSysInfoDlg::OnSizeChanged(INT dw, USHORT x, USHORT y)
{
    RECT rc({ 0 }), rcEd({ 0 });
    GetWindowRect(*this, &rc);
    GetWindowRect(m_edInfo, &rcEd);
    ScreenToClient(*this, LPPOINT(&rcEd));
    ScreenToClient(*this, LPPOINT(&rcEd) + 1);
    GetClientRect(*this, &rc);

    SetWindowPos(m_edInfo, nullptr, 0, 0, rc.right - ( rcEd.left << 1 ), rc.bottom - ( rcEd.top << 1 ), SWP_NOMOVE);
}