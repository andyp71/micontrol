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

tstring ImagePath()
{
    tstring sModulePath(1024, 0);
    auto len = GetModuleFileName(nullptr, sModulePath.data(), static_cast<DWORD>(sModulePath.size()));
    if (len > 0)
    {
        sModulePath.resize(len);
        return sModulePath;
    }

    return tstring();
}

bool CheckServicePresence()
{
    using HSCMANAGER = CAutoHandle<SC_HANDLE, nullptr, CloseServiceHandle>;
    using HSERVICE = CAutoHandle<SC_HANDLE, nullptr, CloseServiceHandle>;
    HSCMANAGER hSC = OpenSCManager(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE);
    if (!hSC)
        return false;

    HSERVICE hService = OpenService(hSC, SERVICE_NAME, SERVICE_QUERY_STATUS);
    if (!hService)
        return false;

    return true;
}