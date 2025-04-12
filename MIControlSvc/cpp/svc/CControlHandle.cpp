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

int CControlHandle::Wait(std::initializer_list<HANDLE> handlesList, DWORD dwTimeout) const
{
	std::vector<HANDLE> v = { m_hEvent };
	v.insert(v.end(), handlesList.begin(), handlesList.end());
	DWORD tid = GetCurrentThreadId();

	// g_log->info(TEXT("Queued wait for thread %d"), tid);

	auto result = WaitForMultipleObjects(static_cast<DWORD>(v.size()), v.data(), FALSE, dwTimeout);
	switch (result)
	{
	case WAIT_FAILED: /* g_log->info(TEXT("Wait for thread %d failed"), tid); */ return -1;
	case WAIT_TIMEOUT: /* g_log->info(TEXT("Wait for thread %d timed out"), tid); */ return 0;
	case WAIT_OBJECT_0 + 0: /*g_log->info(TEXT("Control handler for thread %d fired"), tid);*/ return 1;
	}

	/*g_log->info(TEXT("Local object fired for thread %d"), tid);*/
	return 1 + (result - WAIT_OBJECT_0);
}

int CControlHandle::Wait(HANDLE h, DWORD dwTimeout) const
{
	return Wait({ h }, dwTimeout);
}

int CControlHandle::Wait(HANDLE h) const
{
	return Wait({ h }, INFINITE);
}

void CControlHandle::RaiseEvent()
{
	SetEvent(m_hEvent);
}

int CControlHandle::WaitAlertable(bool bAlertable, DWORD dwTimeout) const
{
	return WaitForSingleObjectEx(m_hEvent, dwTimeout, bAlertable);
}