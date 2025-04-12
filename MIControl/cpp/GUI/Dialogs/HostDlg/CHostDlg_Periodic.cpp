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
#include <Common.h>

VOID CHostDlg::OnTimer(DWORD_PTR idTimer, FARPROC pTimerProc)
{
    switch (idTimer)
    {
    case 100: OnPeriodic(); break;
    }
}

void CHostDlg::OnPeriodic()
{
    m_pipe.counter++;
    if (m_pipe.counter > 10000)
        m_pipe.counter = 0;

    if (m_pipe.fPipeAvailable)
    {
        if (!(m_pipe.counter % 20))
            SendCommandGet(0x10, 0);

        return;
    }

    try
    {
        if (m_pipe.listener)
            m_pipe.listener.reset();

        m_pipe.listener = std::make_unique<CEventListener>(EVENT_PIPE_NAME, m_status);
        m_pipe.listener->push_close_handler(std::bind(&CHostDlg::OnPipeStartStop, this, std::placeholders::_1));
        m_pipe.listener->push_handler<EventDetail>(0x02, EVENT_REF(CHostDlg, OnEventDetail));
        m_pipe.listener->push_handler<CommonRequestByte>(0xC1, EVENT_REF(CHostDlg, OnCommonRequestByte));
    }
    catch (Win32Error&)
    {}
}