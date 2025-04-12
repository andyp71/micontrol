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

CEventListener::CEventListener(const tstring& strEventName, CEventStatus& status)
    :
    m_hPipe(CreateFile(strEventName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr))
    , m_hPipeEvent(CreateEvent(nullptr, TRUE, FALSE, nullptr))
    , m_hSyncEvent(CreateEvent(nullptr, TRUE, FALSE, nullptr))
    , m_hQueueEvent(CreateEvent(nullptr, FALSE, FALSE, nullptr))
    , m_status(status)
{
    m_status.SetStatus(false);

    if (!m_hPipe)
        throw Win32Error(TEXT("Open Pipe \"") + strEventName + TEXT("\" failed"), m_hPipe.Error());

    if (!m_hPipeEvent || !m_hSyncEvent)
        throw Win32Error(TEXT("Sync failed"), !m_hPipeEvent ? m_hPipeEvent.Error() : m_hSyncEvent.Error());

    m_listener = std::async(std::launch::async, std::bind(&CEventListener::Listener, this));
    m_queueHandler= std::async(std::launch::async, std::bind(&CEventListener::QueueHandler, this));
}

CEventListener::~CEventListener()
{
    if (m_listener.valid())
    {
        SetEvent(m_hSyncEvent);
        m_listener.get();
    }
}

void CEventListener::LpoverlappedCompletionRoutine(
    __in DWORD dwErrorCode, 
    __in DWORD dwNumberOfBytesTransfered,
    __inout LPOVERLAPPED lpOverlapped)
{
    EVENT_OVERLAPPED* pOverlapped = reinterpret_cast<EVENT_OVERLAPPED*>(lpOverlapped);
    if (!dwErrorCode && dwNumberOfBytesTransfered && pOverlapped->callback)
        return pOverlapped->callback(dwNumberOfBytesTransfered);
}

void CEventListener::Listener()
{
    OVERLAPPED ov({ 0 });

    BOOL fWaitLoop = TRUE;
    DWORD num = 0;
    HANDLE hEv[] = { m_hSyncEvent, m_hPipeEvent };
    std::vector<uint8_t> buffer(EVENT_BUFFER_SIZE);
    SetLastError(ERROR_SUCCESS);
    ov.hEvent = m_hPipeEvent;
    m_status.SetStatus(true);
    if (m_fnClose)
        m_fnClose(true);

    EVENT_OVERLAPPED eoav{
        .ov = {},
        .callback = [&](DWORD num) -> void
        {
            _tprintf(TEXT("Got message 0x%02X\n"), buffer[4]);
            auto handler = m_mHandler.find(buffer[4]);
            if (m_mHandler.end() != handler)
            {
                eventid_t idx = buffer[4];
                handler->second->EventReceived({ buffer.begin() + 5, buffer.begin() + num });
            }
        }
    };

    while (fWaitLoop)
    {
        if (!ReadFileEx(
            m_hPipe, 
            buffer.data(), static_cast<DWORD>(buffer.size()), 
            reinterpret_cast<LPOVERLAPPED>(&eoav), 
            CEventListener::LpoverlappedCompletionRoutine
        ))
            break;

        fWaitLoop = (WAIT_OBJECT_0 != WaitForSingleObjectEx(hEv[0], INFINITE, true));
    }

    m_status.SetStatus(false);

    if (m_fnClose)
        m_fnClose(false);
}

void CEventListener::push_close_handler(CEventHandlerBase::CLOSE_CALLBACK fnClose)
{
    m_fnClose = fnClose;
}

void CEventListener::SendData(const std::vector<uint8_t>& data)
{
    CGuard lock(m_cs);
    m_queue.push_back(data);
    SetEvent(m_hQueueEvent);
}

void CEventListener::QueueHandler()
{
    HEVENT hPipeEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    HANDLE hEv[] = { m_hSyncEvent, m_hQueueEvent };
    BOOL fWaitLoop = TRUE;
    DWORD num = 0;
    OVERLAPPED ov({ 0 });
    ov.hEvent = m_hPipeEvent;

    while (fWaitLoop)
    {
        switch (DWORD dwResult = WaitForMultipleObjects(std::size(hEv), hEv, FALSE, INFINITE))
        {
            case WAIT_OBJECT_0 + 1:
            {
                // got new event
                break;
            }
            default:
            {
                fWaitLoop = false;
                break;
            }
        }

        if (!fWaitLoop)
            continue;

        std::deque<std::vector<uint8_t>> queue;
        {
            CGuard lock(m_cs);
            queue = m_queue;
            m_queue.clear();
        }

        while (!queue.empty() && fWaitLoop)
        {
            auto& data = queue.front();

            EVENT_OVERLAPPED eoav =
            {
                .ov = {},
                .callback = nullptr // no callback required
            };

            _tprintf(TEXT("Sending message 0x%02X\n"), data[4]);
            if (
                !WriteFileEx(
                    m_hPipe, 
                    data.data(), static_cast<DWORD>(data.size()), 
                    reinterpret_cast<LPOVERLAPPED>(&eoav), 
                    CEventListener::LpoverlappedCompletionRoutine
                )
                )
            {
                fWaitLoop = (WAIT_OBJECT_0 != WaitForSingleObjectEx(hEv[0], INFINITE, true));
            }

            queue.pop_front();
        }
    }
}

const CEventStatus& CEventListener::Status() const
{
    return m_status;
}