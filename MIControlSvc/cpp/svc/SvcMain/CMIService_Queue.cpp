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

void CMIService::SendNotify(const std::vector<uint8_t>& data)
{
	CGuard lock(m_cs);
	m_queue.emplace_back(data.begin(), data.end());
	SetEvent(m_hQueueEvent);
}

void CMIService::QueueHandler()
{
	BOOL fContinueLoop = TRUE;

	while (fContinueLoop)
	{
		auto res = ControlEvent().Wait(m_hQueueEvent);
		switch (res)
		{
		case 2:	// fired local event
			{
				decltype(m_queue) queue;
				{
					CGuard lock(m_cs);
					queue = m_queue;
					m_queue.clear();
				}

				while (!queue.empty())
				{
					auto& data = queue.front();
					if (m_pipe)
						m_pipe->Send(data);

					queue.pop_front();
				}
				break;
			}
		default:
			{
				fContinueLoop = FALSE;
				continue;
			}
		}
	}
}