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

#include <thread>

using HPIPE = CAutoHandle<HANDLE, INVALID_HANDLE_VALUE, CloseHandle>;

using FN_PIPE_LISTENER = std::function<void(const std::vector<uint8_t>&)>;

constexpr auto NPIPE_MAX_BUFFER = 0x2000;

#include <CSecurityDescriptior.h>

class CPipeSecurity
{
    SysPerson			m_owner, m_everyone;
    AccessObject		m_ownerAccess, m_everyoneAccess;
    AccessObjectArray	m_accessList;
    SecurityDescriptor	m_securityDescriptor;

public:
    CPipeSecurity()
        :
        m_owner(SECURITY_NT_AUTHORITY, { SECURITY_LOCAL_SYSTEM_RID }, TRUSTEE_IS_GROUP)
        , m_everyone(SECURITY_WORLD_SID_AUTHORITY, { SECURITY_WORLD_RID }, TRUSTEE_IS_WELL_KNOWN_GROUP)
        , m_ownerAccess(m_owner, SET_ACCESS, GENERIC_ALL)
        , m_everyoneAccess(m_everyone, SET_ACCESS, GENERIC_READ | GENERIC_WRITE)
        , m_accessList(m_ownerAccess, m_everyoneAccess)
        , m_securityDescriptor(m_accessList) {}

    SECURITY_ATTRIBUTES SecurityAttributes()
    {
        return m_securityDescriptor.Attributes();
    }
};

class SRWLock
{
    SRWLOCK			m_lock;

public:
    SRWLock()
    {
        InitializeSRWLock(&m_lock);
    }

    PSRWLOCK operator&() { return &m_lock; }
};

class SRWLockExclusive
{
    SRWLock&		m_lock;

public:
    SRWLockExclusive(SRWLock& lock)
        : m_lock(lock)
    {
        AcquireSRWLockExclusive(&m_lock);
    }

    ~SRWLockExclusive()
    {
        ReleaseSRWLockExclusive(&m_lock);
    }
};

class SRWLockShared
{
    SRWLock& m_lock;

public:
    SRWLockShared(SRWLock& lock)
        : m_lock(lock)
    {
        AcquireSRWLockShared(&m_lock);
    }

    ~SRWLockShared()
    {
        ReleaseSRWLockShared(&m_lock);
    }
};

using PipeListener = std::function<void(const std::vector<uint8_t>&)>;


tstring vec2str(std::vector<uint8_t> v);

template <class T>
class CNamedPipeBuffer
{
    SRWLock						    m_lock;
    CCriticalSection			    m_cs;
    std::vector<T>				    m_vData;
    std::map<DWORD, PipeListener>   m_mListener;
    HEVENT						    m_hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    HEVENT						    m_hWriteEvent = CreateEvent(nullptr, TRUE, TRUE, nullptr);

public:
    HANDLE			WriteEvent()
    {
        return m_hWriteEvent;
    }
    
    void			Write(const std::vector<T>& data)
    {
        CGuard lock(m_cs);
        std::vector<std::future<void>> v;

        g_log->info(TEXT("Sending to client %s\n"), vec2str(data).c_str());
        for (auto& listener : m_mListener)
        {
            v.emplace_back(
                std::async(
                    [](PipeListener listener, const auto& data)
                    {
                        listener(data);
                    }
                    , listener.second
                    , data
                )
            );
        }
    }

    void			AddThread(PipeListener listener)
    {
        CGuard guard_lock(m_cs);
        m_mListener[GetCurrentThreadId()] = listener;
    }

    void			DelThread()
    {
        CGuard guard_lock(m_cs);
        auto tid = GetCurrentThreadId();
        std::erase_if(m_mListener, [&tid](const auto& it) { return it.first == tid; });
    }
};

class CThreadNamedPipe
{
    const CControlHandle&		m_ctlHandle;
    HPIPE						m_hPipe;
    HANDLE						m_hWaitConnectEvent = nullptr;
    HEVENT						m_hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    CNamedPipeBuffer<uint8_t>&	m_buffer;
    const DWORD					m_nInBufferSize, m_nOutBufferSize;
    FN_PIPE_LISTENER			m_listener;

    struct EVENT_OVERLAPPED
    {
        OVERLAPPED ov;
        std::function<void(DWORD)>
            callback;
    };

    static void CALLBACK LpoverlappedCompletionRoutine(
        __in      DWORD dwErrorCode,
        __in      DWORD dwNumberOfBytesTransfered,
        __inout   LPOVERLAPPED lpOverlapped
    );

    bool IsClientConnected() const;
    void StartPipeProcessing();
    void SendData(const std::vector<uint8_t>& data);
    void ReadData();
    void PipeClosed();

public:
    CThreadNamedPipe(
        __in CNamedPipeBuffer<uint8_t> & buffer,
        __in const CControlHandle& ctlHandle,
        __in HANDLE hWaitConnectEvent,
        __in FN_PIPE_LISTENER listener,
        __in const tstring& lpName,
        __in_opt DWORD dwOpenMode = PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        __in_opt DWORD dwPipeMode = PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_REJECT_REMOTE_CLIENTS,
        __in_opt DWORD nMaxInstances = PIPE_UNLIMITED_INSTANCES,
        __in_opt DWORD nOutBufferSize = NPIPE_MAX_BUFFER,
        __in_opt DWORD nInBufferSize = NPIPE_MAX_BUFFER,
        __in_opt DWORD nDefaultTimeOut = 0,
        __in_opt LPSECURITY_ATTRIBUTES lpSecurityAttributes = nullptr
    );

    void Start();
};

class CNamedPipeFactory
{
    CNamedPipeBuffer<uint8_t>	m_buffer;
    HEVENT						m_hWaitConnectEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    HEVENT						m_hGotDataEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    const tstring				m_strPipeName;
    const CControlHandle&		m_ctlHandle;
    std::future<void>			m_ftConnector;
    std::atomic<DWORD>			m_dwFailure = ERROR_SUCCESS;
    CPipeSecurity				m_security;

    CCriticalSection					m_csQueue;
    std::deque<std::vector<uint8_t>>	m_queue;
    FN_PIPE_LISTENER					m_fnListener;

    static DWORD CALLBACK _Connector(LPVOID pParam);

    void	Connector();
    void	OnDataReceived();
    void	OnDataCallback(const std::vector<uint8_t>& data);

public:
    CNamedPipeFactory(const tstring& strPipeName, const CControlHandle& ctlHandle);

    void Send(const std::vector<uint8_t>& buffer);
    void SetListener(FN_PIPE_LISTENER fnListener);

    DWORD						FailureCode() const;
};