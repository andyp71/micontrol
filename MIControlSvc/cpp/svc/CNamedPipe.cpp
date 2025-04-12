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
#include <queue>
#include <sddl.h>

static tstring vec2str(std::vector<uint8_t> v)
{
    tstring str;
    TCHAR p[5];
    while (!v.back())
        v.pop_back();

    for (const auto& ch : v)
    {
        wsprintf(p, TEXT("%02X "), ch);
        str += p;
    }

    return str;
}

CThreadNamedPipe::CThreadNamedPipe(
    __in CNamedPipeBuffer<uint8_t>& buffer,
    __in const CControlHandle& ctlHandle,
    __in HANDLE hWaitConnectEvent,
    __in FN_PIPE_LISTENER listener,
    __in const tstring& lpName,
    __in_opt DWORD dwOpenMode,
    __in_opt DWORD dwPipeMode,
    __in_opt DWORD nMaxInstances,
    __in_opt DWORD nOutBufferSize,
    __in_opt DWORD nInBufferSize,
    __in_opt DWORD nDefaultTimeOut,
    __in_opt LPSECURITY_ATTRIBUTES lpSecurityAttributes
)
    : m_ctlHandle(ctlHandle)
    , m_listener(listener)
    , m_nInBufferSize(nInBufferSize)
    , m_nOutBufferSize(nOutBufferSize)
    , m_hPipe(
        CreateNamedPipe(
            lpName.c_str(),
            dwOpenMode,
            dwPipeMode,
            nMaxInstances,
            nOutBufferSize,
            nInBufferSize,
            nDefaultTimeOut,
            lpSecurityAttributes
        )
    )
    , m_buffer(buffer)
    , m_hWaitConnectEvent(hWaitConnectEvent)
{
    if (!m_hPipe)
        throw Win32Error();
}

void CThreadNamedPipe::PipeClosed()
{
    DisconnectNamedPipe(m_hPipe);
}

void CThreadNamedPipe::Start()
{
    OVERLAPPED ov({ 0 });
    ov.hEvent = m_hWaitConnectEvent;

    auto result = ConnectNamedPipe(m_hPipe, &ov);

    if (result)
        return StartPipeProcessing();

    switch (GetLastError())
    {
    case ERROR_PIPE_CONNECTED:
        {
            return StartPipeProcessing();
        }
    case ERROR_IO_PENDING:
        {
            if ( 2 == m_ctlHandle.Wait(m_hWaitConnectEvent))
                return StartPipeProcessing();

            break;
        }
    }
}

bool CThreadNamedPipe::IsClientConnected() const
{
    // check if pipe is connected. Otherwise, break the connection
    DWORD bytesAvailable = 0;
    if (!PeekNamedPipe(m_hPipe, NULL, 0, NULL, &bytesAvailable, NULL))
    {
        return (ERROR_BROKEN_PIPE != GetLastError());
    }

    return true;
}

void CThreadNamedPipe::StartPipeProcessing()
{
    m_buffer.AddThread(std::bind(&CThreadNamedPipe::SendData, this, std::placeholders::_1));
    ReadData();
    m_buffer.DelThread();
    PipeClosed();
}

void CThreadNamedPipe::LpoverlappedCompletionRoutine(
    __in DWORD dwErrorCode,
    __in DWORD dwNumberOfBytesTransfered,
    __inout LPOVERLAPPED lpOverlapped)
{
    EVENT_OVERLAPPED* pOverlapped = reinterpret_cast<EVENT_OVERLAPPED*>(lpOverlapped);
    if (!dwErrorCode && dwNumberOfBytesTransfered && pOverlapped->callback)
        return pOverlapped->callback(dwNumberOfBytesTransfered);
}

void CThreadNamedPipe::ReadData()
{
    bool fContinue = true;
    std::vector<uint8_t> data(m_nInBufferSize, 0);

    EVENT_OVERLAPPED eoav{
        .ov = {},
        .callback = [&](DWORD num) -> void
        {
            g_log->info(TEXT("Got from client %s\n"), vec2str(data).c_str());
            if (m_listener)
                m_listener(std::vector(data.begin(), data.begin() + num));
        }
    };

    while (fContinue)
    {
        if (!ReadFileEx(
            m_hPipe,
            data.data(), static_cast<DWORD>(data.size()),
            reinterpret_cast<LPOVERLAPPED>(&eoav),
            CThreadNamedPipe::LpoverlappedCompletionRoutine
        ))
            break;

        fContinue = (WAIT_OBJECT_0 != m_ctlHandle.WaitAlertable(true));
    }
}

void CThreadNamedPipe::SendData(const std::vector<uint8_t>& data)
{
    HEVENT hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    OVERLAPPED eoav{ .hEvent = hEvent };
    DWORD dw = static_cast<DWORD>(data.size());

    if (!WriteFile(
        m_hPipe,
        data.data(), dw,
        &dw,
        &eoav
    )
        )
    {
        if (!m_ctlHandle.Wait(eoav.hEvent))
            CancelIoEx(m_hPipe, reinterpret_cast<LPOVERLAPPED>(&eoav));
    }
    Sleep(1);
}

CNamedPipeFactory::CNamedPipeFactory(const tstring& strPipeName, const CControlHandle& ctlHandle)
    : m_strPipeName(strPipeName), m_ctlHandle(ctlHandle)
{
    m_ftConnector = std::async(
        std::launch::async,
        std::bind(&CNamedPipeFactory::Connector, this)
    );
}

void CNamedPipeFactory::Send(const std::vector<uint8_t>& buffer)
{
    m_buffer.Write(buffer);
}

void CNamedPipeFactory::Connector()
{
    DWORD tid = 0;
    BOOL fContinue = TRUE;

    auto fnStartNewListenThread = [&]()
        {
            ResetEvent(m_hWaitConnectEvent);
            auto h = CreateThread(nullptr, 0, CNamedPipeFactory::_Connector, this, 0, &tid);
            if (h)
                CloseHandle(h);
        };

    fnStartNewListenThread();

    do
    {

        switch (m_ctlHandle.Wait({ m_hWaitConnectEvent,m_hGotDataEvent }, INFINITE))
        {
        case 2: // new listen await
            {
                fnStartNewListenThread();
                break;
            }
        case 3:	// got new data from pipe
            {
                OnDataReceived();
                break;
            }
        default:
            {
                fContinue = FALSE;
                break;
            }
        }
    } while (ERROR_SUCCESS == m_dwFailure && fContinue);
}

void CNamedPipeFactory::OnDataCallback(const std::vector<uint8_t>& data)
{
    {
        CGuard lock(m_csQueue);
        m_queue.push_back(data);
    }

    SetEvent(m_hGotDataEvent);
}

void CNamedPipeFactory::OnDataReceived()
{
    decltype(m_queue) queue;
    {
        CGuard lock(m_csQueue);
        queue = m_queue;
        m_queue.clear();
    }

    // g_log->info(TEXT("Queue size: %zu"), queue.size());

    while (!queue.empty())
    {
        auto& data = queue.front();
        
        if (m_fnListener)
            m_fnListener(data);

        queue.pop_front();
    }
}

void CNamedPipeFactory::SetListener(FN_PIPE_LISTENER fnListener)
{
    m_fnListener = fnListener;
}

DWORD CALLBACK CNamedPipeFactory::_Connector(LPVOID pParam)
{
    CNamedPipeFactory* This = reinterpret_cast<CNamedPipeFactory*>(pParam);
    
    try
    {
        auto sa = This->m_security.SecurityAttributes();

        CThreadNamedPipe pipe(
            This->m_buffer,
            This->m_ctlHandle,
            This->m_hWaitConnectEvent,
            std::bind(&CNamedPipeFactory::OnDataCallback, This, std::placeholders::_1),
            This->m_strPipeName,
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_REJECT_REMOTE_CLIENTS,
            PIPE_UNLIMITED_INSTANCES,
            NPIPE_MAX_BUFFER,
            NPIPE_MAX_BUFFER,
            0,
            &sa
        );

        pipe.Start();
    }
    catch (Win32Error& err)
    {
        g_log->error(TEXT("Failed to make pipe %s. Error is %d"), This->m_strPipeName.c_str(), err.what());
        This->m_dwFailure = err.what();
        SetEvent(This->m_hWaitConnectEvent);
    }

    return 0;
}

DWORD CNamedPipeFactory::FailureCode() const
{
    return m_dwFailure;
}