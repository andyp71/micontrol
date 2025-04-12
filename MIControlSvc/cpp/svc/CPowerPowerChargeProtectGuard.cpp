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
#include <powrprof.h>

CSuspendResumeKeeper::CSuspendResumeKeeper(SUSPENDRESUME_CALLBACK fnCallback)
	:
	m_callback(fnCallback)
{
	DEVICE_NOTIFY_SUBSCRIBE_PARAMETERS ntf({ 0 });
	ntf.Callback = CSuspendResumeKeeper::_DeviceNotifyCallbackRoutine;
	ntf.Context = this;
	m_hNotify = RegisterSuspendResumeNotification(&ntf, DEVICE_NOTIFY_CALLBACK);
}

ULONG CSuspendResumeKeeper::_DeviceNotifyCallbackRoutine(PVOID Context, ULONG Type, PVOID Setting)
{
	auto This = (CSuspendResumeKeeper*)Context;

	return This->DeviceNotifyCallbackRoutine(Type, Setting);
}

ULONG CSuspendResumeKeeper::DeviceNotifyCallbackRoutine(ULONG Type, PVOID Setting)
{
	if (m_callback)
		return m_callback(Type, Setting);

	return 0;
}

CPowerPowerChargeProtectGuard::CPowerPowerChargeProtectGuard(std::shared_ptr<CMiLowLevelCommand> cmd)
	:
	m_powerListener(WMI_PLL_ROOT, WMI_PLL_WQL, std::bind(&CPowerPowerChargeProtectGuard::OnPowerLineChange, this, std::placeholders::_1))
	, m_pCmd(cmd)
	, m_SRKeeper(std::bind(&CPowerPowerChargeProtectGuard::SuspendResumeCallback, this, std::placeholders::_1, std::placeholders::_2))
{
	std::vector<uint8_t> result;
	if (m_pCmd->Get(llcChargeProtect, 0x02, result))
	{
		if (result.size() > 6)
			PreserveStatus(result[6]);
	}
}

void CPowerPowerChargeProtectGuard::PreserveStatus(bool fChargeProtection)
{
	m_fChargeProtection = fChargeProtection;
	UpdateStatus();
}

ULONG CPowerPowerChargeProtectGuard::SuspendResumeCallback(ULONG Type, PVOID Setting)
{
	UpdateStatus();
	return 0;
}

void CPowerPowerChargeProtectGuard::UpdateStatus()
{
	CGuard lock(m_cs);
	std::vector<uint8_t> __unused;

	auto result = m_pCmd->Put(llcChargeProtect, 0x02, 0, __unused);	// we need to switch off this mode, before setup again

	if (m_fChargeProtection)
	{
		Sleep(50);
		result &= m_pCmd->Put(llcChargeProtect, 0x02, 1, __unused);
	}

	g_log->info(TEXT("Updating status (%s) due to power line status changed: %s"), m_fChargeProtection ? TEXT("protected") : TEXT("unprotected"), result ? TEXT("SUCCESS") : TEXT("FAILED"));
}

void CPowerPowerChargeProtectGuard::OnPowerLineChange(WbemClassItem&)
{
	UpdateStatus();
}

bool CPowerPowerChargeProtectGuard::Status() const
{
	return m_fChargeProtection;
}