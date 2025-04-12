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

#include <Service/CAudioInputControl.h>

void CMIService::OnWMIEvent(WbemClassItem& event)
{
    CGuard lock(m_cs);
    auto eventDetail = event.find(TEXT("EventDetail"));
    if (eventDetail == event.end())
        return;

    auto& vt = eventDetail->second;

    if ((VT_ARRAY | VT_UI1) != vt->vt)
    {
        g_log->info(TEXT("Unknown EventDetail data type received: %d"), vt->vt);
        return;
    }

    SafeArray1 arr(vt->parray);
    EventDetail ev;
    auto i1 = arr.begin(), i2 = arr.end() + 1;
    if (i2 - i1 < 7)
    {
        g_log->info(TEXT("Too small EventDetail data received"));
        return;
    }

    std::vector<uint8_t> data_event_detail;

    for (decltype(i1)i = i1; i < i2; ++i)
        data_event_detail.push_back(arr.Get<uint8_t>(i));

    HandleFnKey(data_event_detail[1], data_event_detail[2]);

    ev.set_name("EventDetail");
    ev.set_event_type("HID_EVENT20");
    ev.set_details(data_event_detail.data(), data_event_detail.size());
    SendCommand(0x02, ev);
}

void CMIService::SendDisplayDescription()
{
    CGuard lock(m_cs);

    try
    {
        WMIQuery q(TEXT("root\\WMI"));
        std::vector<WbemClassItem> rows;
        if (!q.Query(TEXT("SELECT * FROM WmiMonitorConnectionParams"), rows))
            return;

        for (auto& row : rows)
        {
            auto it = row.find(TEXT("InstanceName"));
            if (it == row.end())
                continue;

            auto& dev = it->second;
            if (VT_BSTR != dev->vt)
                continue;

            auto length = CComBSTR(dev->bstrVal).Length();

            std::string device_id(length, 0);
            WideCharToMultiByte(CP_ACP, 0, dev->bstrVal, length, device_id.data(), static_cast<int>(device_id.size()), nullptr, nullptr);

            ClientRequest cr;
            cr.set_request_type("CLIENTREQUEST");
            cr.set_action("GetnternalScreenId");
            cr.set_display_info(device_id);

            SendCommand(0x03, cr);
        }
    }
    catch (COMException& comerr)
    {
        g_log->warning(TEXT("Unable to fetch display information: 0x%08X"), comerr.what());
    }
}

void CMIService::HandleFnKey(uint8_t cmd, uint8_t arg)
{
    switch (cmd)
    {
    case 0x21:
        {
            g_log->info(TEXT("Request mic to %s"), !arg ? TEXT("mute") : TEXT("unmute"));
            CAudioInputControl mic;
            if (mic)
            {
                g_log->info(TEXT("Update mic status from %d to %d"), mic.GetMute(), !arg);
                mic.SetMute(!arg);
            }

            break;
        }
    case 0x23: // Fn+F7 for NON-PRO laptop
        {
            CAudioInputControl mic;
            if (!mic)
            {
                g_log->info(TEXT("Got Fn+F7 (2), but unable to query microphone state"));
                break;
            }

            auto arg2 = mic.GetMute();
            g_log->info(TEXT("Request(2) mic to %s"), !arg2 ? TEXT("mute") : TEXT("unmute"));
            g_log->info(TEXT("Update(2) mic status from %d to %d"), !arg2, arg2);
            mic.SetMute(!arg2);

            // send fake mic command
            std::vector<uint8_t> data_event_detail(0x20, 0);
            data_event_detail[1] = 0x21;
            data_event_detail[2] = arg2 ? 1 : 0;
            EventDetail ev;
            ev.set_name("EventDetail");
            ev.set_event_type("HID_EVENT20");
            ev.set_details(data_event_detail.data(), data_event_detail.size());
            SendCommand(0x02, ev);

            std::vector<uint8_t> __unused;
            m_cmd->Put(llcMicrophone, 0x05, arg2, __unused);

            break;
        }
    }
}
