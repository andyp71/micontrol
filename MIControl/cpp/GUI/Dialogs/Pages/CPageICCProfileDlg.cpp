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
#include <Icm.h>
#pragma comment(lib,"Mscms.lib")

CPageICCProfileDlg::CPageICCProfileDlg(HINSTANCE hInstance, SendCommand_t fn, HOST_DATA& status)
    : CPageDlg(hInstance, fn, status)
    , m_lvList(hInstance, 0x10, 7, 7, 10, 150, WS_VISIBLE | WS_TABSTOP | WS_BORDER | LVS_REPORT | LVS_NOCOLUMNHEADER | LVS_SHOWSELALWAYS | LVS_SINGLESEL, WS_EX_CLIENTEDGE)
{
    pushctl(m_lvList);
}

BOOL CPageICCProfileDlg::OnPageInit(LPARAM lParam)
{
    RECT rc({ 0 });
    GetClientRect(m_lvList, &rc);

    m_lvList.AddColumn(TEXT(""), 600);
    return TRUE;
}

void CPageICCProfileDlg::OnShow()
{
    std::vector<tstring> vstrDevice;
    DISPLAY_DEVICE DisplayDevice;
    DWORD v = 0;
    DisplayDevice.cb = sizeof(DisplayDevice);
    DWORD dwBufSize = 32767, nProfiles = 0;
    std::vector<uint8_t> buffer(dwBufSize, 0);
    ENUMTYPE et({ 0 });
    tstring strDisplayName;

    while (EnumDisplayDevices(nullptr, v++, &DisplayDevice, EDD_GET_DEVICE_INTERFACE_NAME))
    {
        if ((DisplayDevice.StateFlags & 1) != 0 && (DisplayDevice.StateFlags & 4) != 0)
        {
            strDisplayName = DisplayDevice.DeviceName;
            break;
        }
    }

    if (strDisplayName.empty())
        return;

    if (EnumDisplayDevices(strDisplayName.c_str(), 0, &DisplayDevice, EDD_GET_DEVICE_INTERFACE_NAME))
        vstrDevice.emplace_back(DisplayDevice.DeviceKey);

    if (vstrDevice.empty())
        return;

    m_strDevice = vstrDevice[0];

    et.dwSize = sizeof(et);
    et.dwVersion = ENUM_TYPE_VERSION;
    et.dwFields = ET_CLASS;
    et.dwClass = CLASS_MONITOR;

    if (!EnumColorProfiles(nullptr, &et, buffer.data(), &dwBufSize, &nProfiles))
        return;

    m_lvList.ResetContent();

    std::vector<tstring> vstrProfiles;
    LPARAM i = 0;
    LPCTSTR pStr = reinterpret_cast<LPCTSTR>(buffer.data());
    while (*pStr)
    {
        tstring strProfile = pStr;
        std::for_each(strProfile.begin(), strProfile.end(), [](wchar_t& ch) {ch = std::toupper(ch); });

        if (
            strProfile.ends_with(TEXT(".ICC"))
            || strProfile.ends_with(TEXT(".ICM"))
            || strProfile.ends_with(TEXT(".CDMP"))
            )
        {
            vstrProfiles.emplace_back(pStr);
            m_lvList.AddItem(pStr, i++);
        }

        pStr += lstrlen(pStr) + 1;
    }

    m_vstrProfiles = vstrProfiles;

    tstring strDefaultProfile(1000, 0);

    for (auto& profile : { CPST_ABSOLUTE_COLORIMETRIC,
            CPST_PERCEPTUAL,
            CPST_RELATIVE_COLORIMETRIC,
            CPST_SATURATION,
            CPST_RGB_WORKING_SPACE,
            CPST_CUSTOM_WORKING_SPACE,
            CPST_CUSTOM_WORKING_SPACE,
            CPST_EXTENDED_DISPLAY_COLOR_MODE })
    {
        if (WcsGetDefaultColorProfile(
            WCS_PROFILE_MANAGEMENT_SCOPE_CURRENT_USER,
            vstrDevice[0].c_str(),
            CPT_ICC,
            profile, 1,
            static_cast<DWORD>(strDefaultProfile.size()),
            strDefaultProfile.data())
            )
            break;
    }

    strDefaultProfile.resize(lstrlen(strDefaultProfile.c_str()));
    if (strDefaultProfile.empty())
        return;

    auto it = std::find(vstrProfiles.begin(), vstrProfiles.end(), strDefaultProfile);
    if (it == vstrProfiles.end())
        return;

    m_lvList.CurSel(static_cast<INT>(it - vstrProfiles.begin()));
}

void CPageICCProfileDlg::OnSizeChanged(INT dw, USHORT x, USHORT y)
{
    RECT rc({ 0 }), rcLv({ 0 });
    GetClientRect(*this, &rc);
    GetWindowRect(m_lvList, &rcLv);
    ScreenToClient(*this, LPPOINT(&rcLv));
    ScreenToClient(*this, LPPOINT(&rcLv) + 1);

    SetWindowPos(m_lvList, nullptr, 0, 0, rc.right - (rcLv.left << 1), rc.bottom - (rcLv.top << 1), SWP_NOMOVE);
}

void CPageICCProfileDlg::OnItemSelected(LPARAM id)
{
    if (id >= static_cast<LPARAM>(m_vstrProfiles.size()))
        return;

    const auto& strProfile = m_vstrProfiles[id];

    std::vector<COLORPROFILESUBTYPE> profiles = { CPST_RGB_WORKING_SPACE };
    
    if (!strProfile.ends_with(TEXT(".icc")))
    {
        profiles = { 
            CPST_ABSOLUTE_COLORIMETRIC, 
            CPST_PERCEPTUAL, 
            CPST_RELATIVE_COLORIMETRIC, 
            CPST_SATURATION, 
            CPST_RGB_WORKING_SPACE, 
            CPST_CUSTOM_WORKING_SPACE, 
            CPST_CUSTOM_WORKING_SPACE, 
            CPST_EXTENDED_DISPLAY_COLOR_MODE 
        };
    }

    bool fApplied = false;
    for (const auto& profile : profiles)
    {
        if (WcsSetDefaultColorProfile(
            WCS_PROFILE_MANAGEMENT_SCOPE_CURRENT_USER,
            m_strDevice.c_str(),
            CPT_ICC, profile, 1,
            strProfile.c_str()))
        {
            fApplied = true;
            break;
        }
    }

    if (!fApplied)
    {
        Win32Error err;
        ShowError(err.description().c_str());
    }
}

INT_PTR CPageICCProfileDlg::OnNotify(LPNMHDR lpnm)
{
    switch (lpnm->idFrom)
    {
    case 0x10:
    {
        switch (lpnm->code)
        {
        case LVN_ITEMCHANGED:
        {
            auto sel = m_lvList.CurSel();
            if (sel >= 0)
                OnItemSelected(m_lvList.Param(static_cast<INT>(sel)));
            break;
        }
        }
        break;
    }
    }
    return INT_PTR();
}