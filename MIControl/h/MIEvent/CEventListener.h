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
#include <deque>
#include <CCriticalSection.h>

constexpr auto EVENT_BUFFER_SIZE = 2000;

class CEventHandlerBase
{
public:
    using EVENT_CALLBACK = std::function<void(const CEventHandlerBase&)>;
    using CLOSE_CALLBACK = std::function<void(bool bStarted)>;

private:
    EVENT_CALLBACK		m_fnHandler;

protected:
    void				Callback()
    {
        if (m_fnHandler)
            m_fnHandler(*this);
    }

public:
    CEventHandlerBase(EVENT_CALLBACK fnHandler)
        : m_fnHandler(fnHandler) {}

    virtual void		EventReceived(const std::vector<uint8_t>&) = 0;
};

template <class DATA>
class CEventHandler : public CEventHandlerBase
{
    DATA		m_data;

public:
    CEventHandler(EVENT_CALLBACK fnHandler)
        : CEventHandlerBase(fnHandler) {}

    virtual void		EventReceived(const std::vector<uint8_t>& protobuf)
    {
        if (m_data.ParseFromArray(protobuf.data(), static_cast<int>(protobuf.size())))
        {
            Callback();
        }
        else
            _tprintf(TEXT("Failed to parse the data!\n"));
    }

    const DATA* operator->() const
    {
        return &m_data;
    }

    const DATA& operator*() const
    {
        return m_data;
    }
};

using eventid_t = uint8_t;
using CEventHandlerBase_ptr = std::unique_ptr<CEventHandlerBase>;

class CEventListener
{
    struct EVENT_OVERLAPPED
    {
        OVERLAPPED ov;
        std::function<void(DWORD)>
            callback;
    };

    std::future<void>								m_listener;
    std::future<void>								m_queueHandler;
    HPIPE											m_hPipe;
    HEVENT											m_hPipeEvent, m_hSyncEvent, m_hQueueEvent;
    std::map<eventid_t, CEventHandlerBase_ptr>		m_mHandler;
    CEventHandlerBase::CLOSE_CALLBACK				m_fnClose;
    std::deque<std::vector<uint8_t>>				m_queue;
    CCriticalSection								m_cs;
    CEventStatus&									m_status;

    static void CALLBACK LpoverlappedCompletionRoutine(
        __in      DWORD dwErrorCode,
        __in      DWORD dwNumberOfBytesTransfered,
        __inout   LPOVERLAPPED lpOverlapped
    );

    void			Listener();
    void			QueueHandler();

public:
    CEventListener(const tstring& strEventName, CEventStatus& status);
    ~CEventListener();

    template <class EVENT_DATA>
    void			push_handler(eventid_t idx, CEventHandlerBase::EVENT_CALLBACK callback)
    {
        m_mHandler[idx] = std::unique_ptr<CEventHandlerBase>(new CEventHandler<EVENT_DATA>(callback));
    }

    void			push_close_handler(CEventHandlerBase::CLOSE_CALLBACK fnClose);
    void			SendData(const std::vector<uint8_t>& data);
    const CEventStatus& Status() const;
};