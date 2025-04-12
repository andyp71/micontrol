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

CAudioInputControl::CAudioInputControl()
{
    // Создаем экземпляр устройства
    if (FAILED(m_hRes = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), (void**)&m_pEnumerator)))
        return;

    // Получаем устройство вывода по умолчанию
    if (FAILED(m_hRes = m_pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &m_pDevice)))
    {
        m_pEnumerator->Release();
        return;
    }

    // Получаем интерфейс управления громкостью
    if (FAILED(m_hRes = m_pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&m_pAudioEndpointVolume)))
    {
        m_pDevice->Release();
        m_pEnumerator->Release();
        m_pDevice = nullptr;
        m_pEnumerator = nullptr;
        return;
    }
}

CAudioInputControl::~CAudioInputControl()
{
    if (m_pAudioEndpointVolume)
        m_pAudioEndpointVolume->Release();

    if (m_pDevice)
        m_pDevice->Release();

    if (m_pEnumerator)
        m_pEnumerator->Release();
}

HRESULT CAudioInputControl::State() const
{
    return m_hRes;
}

CAudioInputControl::operator bool() const
{
    return SUCCEEDED(m_hRes);
}

bool CAudioInputControl::GetMute() const
{
    if (FAILED(State()))
        return false;

    BOOL fState = FALSE;
    if (SUCCEEDED(m_pAudioEndpointVolume->GetMute(&fState)))
        return !!fState;

    return false;
}

void CAudioInputControl::SetMute(bool fMuteState)
{
    if (FAILED(State()))
        return void();

    if (fMuteState ^ GetMute()) // only if state changed
        m_pAudioEndpointVolume->SetMute(fMuteState, nullptr);
}