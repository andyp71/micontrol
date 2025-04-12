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

using CLIENT_CALLBACK = std::function<void(const std::vector<uint8_t>&)>;

#define MI_SERVICE_FLAG_CAMERA_PENDING		1

class CMIServiceStateFlag
{
	std::atomic_char32_t		m_uFlags = 0;

public:
	bool		IsFlagSet(uint8_t flag) const;
	bool		IsFlagSetLock(uint8_t flag);
	bool		SetFlag(uint8_t flag, bool fState);
};

class CMIService : public CService
{
	CCriticalSection				m_cs;
	CMIServiceStateFlag				m_flags;
	using QUEUE_ITEM = std::vector<uint8_t>;
	std::atomic_char8_t				m_perfMode = 0;
	std::deque<QUEUE_ITEM>		m_queue;
	std::unique_ptr<CNamedPipeFactory> m_pipe;
	std::shared_ptr<CMiLowLevelCommand> m_cmd;
	std::unique_ptr<CPowerPowerChargeProtectGuard>	m_pcGuard;
	std::unique_ptr<CSvcParameters>		m_settings;
	HEVENT	m_hQueueEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	std::map<uint8_t, CLIENT_CALLBACK>	m_mClientCallback;

	virtual DWORD HandlerEx(
		__in DWORD dwControl,
		__in DWORD dwEventType,
		__in LPVOID lpEventData
	) override;

	virtual  LARGE_INTEGER ServiceMain(
		__in const std::vector<tstring> m_vArgs
	) override;


	void	OnWMIEvent(WbemClassItem& event);
	void	HandleFnKey(uint8_t cmd, uint8_t arg);
	void	SendDisplayDescription();
	void	QueueHandler();
	void	OnDataReceived(const std::vector<uint8_t>& data);
	void	OnDataReceived_03(const std::vector<uint8_t>& data);
	void	OnDataReceived_C0(const std::vector<uint8_t>& data);
	void	OnDataReceived_C1(const std::vector<uint8_t>& data);
	void	OnClientRequest(const tstring& strReq, const tstring& strType);
	void	SendNotify(const std::vector<uint8_t>&);

	template<class T>
	bool	SendCommand(uint8_t id, const T& protobuf)
	{
		std::string serialized;
		if (protobuf.SerializeToString(&serialized))
		{
			std::vector<uint8_t> data_to_send = { 0x00, 0x00, 0x00, 0x00, id };
			data_to_send.insert(data_to_send.end(), serialized.begin(), serialized.end());
			SendNotify(data_to_send);
			return true;
		}

		return false;
	}

public:
	CMIService();

};