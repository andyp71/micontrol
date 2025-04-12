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

#pragma once

#define WMI_PLL_ROOT		TEXT("ROOT\\CIMv2")
#define WMI_PLL_WQL			TEXT("SELECT * FROM Win32_PowerManagementEvent")

using HSUSPENDRESUME = CAutoHandle<HPOWERNOTIFY, nullptr, UnregisterSuspendResumeNotification>;
using SUSPENDRESUME_CALLBACK = std::function<ULONG(ULONG, PVOID)>;

class CSuspendResumeKeeper
{
	SUSPENDRESUME_CALLBACK		m_callback;
	HSUSPENDRESUME				m_hNotify;

	static ULONG CALLBACK _DeviceNotifyCallbackRoutine(
		PVOID Context,
		ULONG Type,
		PVOID Setting
	);

	ULONG DeviceNotifyCallbackRoutine(ULONG Type, PVOID Setting);

public:
	CSuspendResumeKeeper(SUSPENDRESUME_CALLBACK fnCallback);
};

class CPowerPowerChargeProtectGuard
{
	CCriticalSection						m_cs;
	std::atomic_bool						m_fChargeProtection = false;
	WMIListener								m_powerListener;
	std::shared_ptr<CMiLowLevelCommand>		m_pCmd;
	CSuspendResumeKeeper					m_SRKeeper;
	
	ULONG									SuspendResumeCallback(ULONG Type, PVOID Setting);
	void									OnPowerLineChange(WbemClassItem&);
	void									UpdateStatus();

public:
	CPowerPowerChargeProtectGuard(std::shared_ptr<CMiLowLevelCommand> cmd);
	void		PreserveStatus(bool fChargeProtection);
	bool		Status() const;
};
