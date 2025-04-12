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

#define	WMI_EXEC_ROOT			TEXT("ROOT\\WMI")
#define WMI_EXEC_CLASS			TEXT("MiCommonInterface")
#define WMI_EXEC_METHOD			TEXT("MiInterface")
#define WMI_EXEC_CLASS_PRED		WMI_EXEC_CLASS TEXT(".InstanceName=")
#define WMI_EXEC_IN_DATA		TEXT("InData")
#define WMI_EXEC_OUT_DATA		TEXT("OutData")

enum MiLowLevelCommand : uint8_t
{
	llcPowerBalance		= 0x08,
	llcMicrophone		= 0x0A,
	llcChargeProtect	= 0x10
};

class CMiLowLevelCommand
{
	HRESULT							m_hres = S_OK;
	tstring							m_strInstanceName;

	bool		ExecQuery(const std::vector<uint8_t>& inData, std::vector<uint8_t>& outData);

public:
	CMiLowLevelCommand();

	bool		Put(MiLowLevelCommand cmd, uint8_t arg, uint8_t arg2, std::vector<uint8_t>& result);
	bool		Put(MiLowLevelCommand cmd, uint8_t arg, std::vector<uint8_t>& result);
	bool		Get(MiLowLevelCommand cmd, uint8_t arg, uint8_t arg2, std::vector<uint8_t>& result);
	bool		Get(MiLowLevelCommand cmd, uint8_t arg, std::vector<uint8_t>& result);

	HRESULT		Error() const;
};