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

void CMIService::OnDataReceived(const std::vector<uint8_t>& data)
{
    if (data.size() > 4)
    {
        auto itfn = m_mClientCallback.find(data[4]);
        if (itfn != m_mClientCallback.end())
            return itfn->second(std::vector<uint8_t>(data.begin() + 5, data.end()));
    }

    tstring info;
    for (const auto& ch : data)
    {
        TCHAR x[8];
        wsprintf(x, TEXT("%02X "), ch);
        info += x;
    }

    g_log->info(TEXT("Unknown request from client: %s"), info.c_str());
}

void CMIService::OnDataReceived_03(const std::vector<uint8_t>& data)
{
    ClientPipeRequest prq;
    if (prq.ParseFromArray(data.data(), static_cast<int>(data.size())))
    {
        CString strReq = prq.request_type().c_str(),
            strType = prq.action().c_str();

        OnClientRequest(strReq.GetString(), strType.GetString());
    }
}


void CMIService::OnClientRequest(const tstring& strReq, const tstring& strType)
{
    if (TEXT("CLIENTREQUEST") == strReq)
    {
        if (TEXT("GetnternalScreenId") == strType)
        {
            SendDisplayDescription();
        }
    }
    else
    {
        if (TEXT("GetKeyBoardMuteState") == strType)
        {
            ClientRequestBin crb;
            std::vector<uint8_t> data = { 1 };
            std::vector<uint8_t> mi_data = { 1 };
            crb.set_request_type("");
            crb.set_action("GetKeyBoardMuteState");

            if (m_cmd->Get(llcMicrophone, 0x05, mi_data))
            {
                data[0] = mi_data[7];
            }

            crb.set_additional_info(data.data(), data.size());

            SendCommand(0x03, crb);
        }
    }
}


void CMIService::OnDataReceived_C0(const std::vector<uint8_t>& data)
{
    CommonRequestByte cmrq;
    if (!cmrq.ParseFromArray(data.data(), static_cast<int>(data.size())))
        return;

    auto cmd = cmrq.command();
    if (cmd.size() < 2)
        return;

    auto fnLowLevelCommand = [&](MiLowLevelCommand cmd, uint8_t arg1, uint8_t arg2 = 0)
        {
            std::vector<uint8_t> __unused;
            g_log->info(TEXT("Sending low level command (%d [%d:%d])"), cmd, arg1, arg2);
            m_cmd->Put(cmd, arg1, arg2, __unused);
        };

    switch (cmd[0])
    {
    case 0x08: fnLowLevelCommand(llcPowerBalance, cmd[1]); break;
    case 0x0A: fnLowLevelCommand(llcMicrophone, 5, cmd[1]); break;
    case 0x10:
        {
            m_settings->SetParam(TEXT("PreservedPowerStatus"), cmd[1] ? 1 : 0);
            m_pcGuard->PreserveStatus(cmd[1]);
            break;
        }
    }
}

void CMIService::OnDataReceived_C1(const std::vector<uint8_t>& data)
{
    CommonRequestByte cmrq;
    if (!cmrq.ParseFromArray(data.data(), static_cast<int>(data.size())))
        return;

    auto req = cmrq.command();
    if (req.empty())
        return;

    std::vector<uint8_t> result;
    bool fresult = false;
    switch (req[0])
    {
    case 0x08: fresult = m_cmd->Get(llcPowerBalance, 0x00, result); break;
    case 0x0A: fresult = m_cmd->Get(llcMicrophone, 0x05, result); break;
    case 0x10: fresult = m_cmd->Get(llcChargeProtect, 0x02, result); break;
    }

    if (!fresult)
        return;

    CommonRequestByte cmrqReply;
    result.push_back(req[0]);
    std::rotate(result.rbegin(), result.rbegin() + 1, result.rend()); // move cmd id to front ov data
    cmrqReply.set_command(result.data(), result.size());
    req.clear();

    SendCommand(0xC1, cmrqReply);
}