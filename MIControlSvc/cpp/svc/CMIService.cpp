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

#define CLIENT_EVENT_REF(method)	std::bind(&CMIService::method, this, std::placeholders::_1)

bool CMIServiceStateFlag::IsFlagSet(uint8_t flag) const
{
	if (flag >= 32)
		return false;

	uint32_t f = (1 << flag);
	return (f == (f & m_uFlags));
}

bool CMIServiceStateFlag::SetFlag(uint8_t flag, bool fState)
{
	if (flag >= 32)
		return false;

	uint32_t f = (1 << flag);

	if (fState)
		m_uFlags |= f;
	else
		m_uFlags &= ~f;
	
	return fState;
}

bool CMIServiceStateFlag::IsFlagSetLock(uint8_t flag)
{
	if (flag >= 32)
		return false;

	uint32_t f = (1 << flag);
	if (!(f == (f & m_uFlags)))
	{
		m_uFlags |= f;
		return false;
	}

	return true;
}

CMIService::CMIService()
	: CService(SERVICE_NAME, SERVICE_WIN32_OWN_PROCESS)
{
	m_mClientCallback[0x03] = CLIENT_EVENT_REF(OnDataReceived_03);
	m_mClientCallback[0xC0] = CLIENT_EVENT_REF(OnDataReceived_C0);
	m_mClientCallback[0xC1] = CLIENT_EVENT_REF(OnDataReceived_C1);
}

DWORD CMIService::HandlerEx(
	__in DWORD dwControl, 
	__in DWORD dwEventType,
	__in LPVOID lpEventData)
{
	g_log->info(TEXT("Service got control %d"), dwControl);	

	switch (dwControl)
	{
	case SERVICE_CONTROL_STOP:
		{
			ReportStatus(SERVICE_STOP_PENDING, 0);
			SignalEvent();
			break;
		}
	}

	return NO_ERROR;
}

LARGE_INTEGER CMIService::ServiceMain(
	__in const std::vector<tstring> m_vArgs)
{
	std::unique_ptr<WMIService> wmi;
	try
	{
		wmi = std::make_unique<WMIService>();
	}
	catch (COMException& comerr)
	{
		if (RPC_E_TOO_LATE != comerr.what())
			return { ERROR_REQUEST_ABORTED, comerr.what() };
	}

	std::unique_ptr<WMIListener> wmi_listener;
	
	
	try
	{
		CSvcParameters params;
		CString strHidEvent;

		auto hid = params.GetParam(TEXT("EventId"));
		hid = (hid ? hid : 20);
		strHidEvent.Format(TEXT("SELECT * FROM HID_EVENT%02d"), hid);

		wmi_listener = std::make_unique<WMIListener>(
			TEXT("ROOT\\WMI"),
			strHidEvent.GetString(),
			std::bind(&CMIService::OnWMIEvent, this, std::placeholders::_1)
		);

	}
	catch (COMException& ex)
	{
		g_log->error(TEXT("Unable to listen for WMI: 0x%08X, %s"), ex.what(), ex.Error().ErrorMessage());
		return { ERROR_REQUEST_ABORTED, ex.what() };
	}

	try
	{
		m_pipe = std::make_unique<CNamedPipeFactory>(EVENT_PIPE_NAME, ControlEvent());
		m_pipe->SetListener(std::bind(&CMIService::OnDataReceived, this, std::placeholders::_1));
	}
	catch (Win32Error& w32err)
	{
		g_log->error(TEXT("Unable to make pipe: %d"), w32err.what());
		return { w32err.what(), 0 };
	}

	m_cmd = std::make_shared<CMiLowLevelCommand>();
	if (FAILED(m_cmd->Error()))
		return { ERROR_REQUEST_ABORTED, m_cmd->Error() };

	try
	{
		m_pcGuard = std::make_unique<CPowerPowerChargeProtectGuard>(m_cmd);
	}
	catch (COMException& comerr)
	{
		return { ERROR_REQUEST_ABORTED, comerr.what() };
	}

	try
	{
		m_settings = std::make_unique<CSvcParameters>();
	}
	catch (Win32Error& err)
	{
		return { err.what(), 0 };
	}

	m_pcGuard->PreserveStatus(m_settings->GetParam(TEXT("PreservedPowerStatus")));

	auto fQueue = std::async(
		std::launch::async,
		std::bind(&CMIService::QueueHandler, this)
	);

	// update mute state from the button state
	CAudioInputControl mic;
	if (mic)
	{
		std::vector<uint8_t> state;
		if (m_cmd->Get(llcMicrophone, 0x05, state) && state.size() > 6)
			mic.SetMute(!state[6]);
	}

	ReportStatus(SERVICE_RUNNING, SERVICE_ACCEPT_STOP);
	g_log->info(TEXT("Service loop"));

	while(!ControlEvent().Wait({}, 1000))	// check for pipes states every second
	{
		DWORD err = 0;
		if (ERROR_SUCCESS != (err = m_pipe->FailureCode()))
		{
			SignalEvent();
			g_log->error(TEXT("Pipe worker failed: %d"), err);
		}
	}

	g_log->info(TEXT("Service loop finished"));

	return { ERROR_SUCCESS, 0 };
}
