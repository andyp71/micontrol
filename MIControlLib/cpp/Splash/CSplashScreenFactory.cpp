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
#include <atlbase.h>

CSplashScreenFactory::CSplashScreenFactory()
    :
    m_cb
    {
        .strPath = CSplashScreenFactory::AppPath(),
    }
{
    CreateDirectory(m_cb.strPath.c_str(), nullptr);

    PreloadFiles();
}

CSplashScreenFactory::~CSplashScreenFactory()
{
    FreeAll();
}

void CSplashScreenFactory::PreloadFiles()
{
    const tstring strAppPath = AppPath();

    WIN32_FIND_DATA wfd{};
    HANDLE hf = FindFirstFile((strAppPath + TEXT("*.mcz")).c_str(), &wfd);
    if (!hf)
        return;

    do
    {
        if (FILE_ATTRIBUTE_DIRECTORY != (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            tstring strName = wfd.cFileName;
            if (auto nExt = strName.find_last_of(L'.'); nExt != strName.npos)
                m_vNames.emplace_back(strName.begin(), strName.begin() + nExt);
        }
    } while (FindNextFile(hf, &wfd));

    FindClose(hf);
}

void CSplashScreenFactory::FreeAll()
{
    
}

tstring CSplashScreenFactory::AppPath()
{
    tstring strPath(32767, 0);

    ExpandEnvironmentStrings(TEXT("%APPDATA%\\MiControl\\"), strPath.data(), static_cast<DWORD>(strPath.size()));
    strPath.resize(lstrlen(strPath.c_str()));
    return strPath;
}

CSplashScreenPtr CSplashScreenFactory::operator[](const tstring& strName) const
{
    if (IsNamePresent(strName))
        return CSplashScreenView::make_shared(m_cb, strName);

    return nullptr;
}

decltype(CSplashScreenFactory::m_vNames.begin()) CSplashScreenFactory::begin()
{
    return m_vNames.begin();
}

decltype(CSplashScreenFactory::m_vNames.cbegin()) CSplashScreenFactory::cbegin()
{
    return m_vNames.cbegin();
}

decltype(CSplashScreenFactory::m_vNames.end()) CSplashScreenFactory::end()
{
    return m_vNames.end();
}

decltype(CSplashScreenFactory::m_vNames.cend()) CSplashScreenFactory::cend()
{
    return m_vNames.cend();
}

bool CSplashScreenFactory::IsNamePresent(const tstring& strName) const
{
    auto it = std::find_if(
        m_vNames.begin(),
        m_vNames.end(),
        [&strName](const auto& spl)
        {
            return (!lstrcmpi(spl.c_str(), strName.c_str()));
        }
    );

    return (it != m_vNames.end());
}

CSplashScreenPtr CSplashScreenFactory::Create()
{
    return CSplashScreenView::make_shared(m_cb, tstring());
}

void CSplashScreenFactory::Save(CSplashScreenPtr pSplash)
{
    if (!pSplash)
        return;

    pSplash->Save();

    if (!IsNamePresent(pSplash->Name()))
        m_vNames.emplace_back(pSplash->Name());
}

bool CSplashScreenFactory::Delete(const tstring& strName)
{
    auto it = std::find_if(m_vNames.begin(), m_vNames.end(), [&strName](const auto& name) { return !lstrcmpi(name.c_str(), strName.c_str()); });
    if (it == m_vNames.end())
        return false;

    m_vNames.erase(it);
    return ::DeleteFile((AppPath() + TEXT("\\") + strName + TEXT(".mcz")).c_str());
}

void CSplashScreenFactory::SetDefault(CSplashScreenPtr pSplash)
{
    CRegKey rApp;
    if (ERROR_SUCCESS != rApp.Create(HKEY_CURRENT_USER, TEXT("Software\\MiControl"), nullptr, 0, KEY_SET_VALUE | KEY_QUERY_VALUE))
        return;

    if (!pSplash)
    {
        rApp.DeleteValue(TEXT("Skin"));
        return;
    }

    if (!IsNamePresent(pSplash->Name()))
        return;


    rApp.SetStringValue(TEXT("Skin"), pSplash->Name().c_str());
}

tstring CSplashScreenFactory::GetDefaultName() const
{
    CRegKey rApp;
    if (ERROR_SUCCESS != rApp.Create(HKEY_CURRENT_USER, TEXT("Software\\MiControl"), nullptr, 0, KEY_SET_VALUE | KEY_QUERY_VALUE))
        return tstring();

    ULONG uChars = MAX_PATH;
    tstring strName(uChars, 0);
    if (ERROR_SUCCESS != rApp.QueryStringValue(TEXT("Skin"), strName.data(), &uChars))
        return tstring();

    strName.resize(lstrlen(strName.c_str()));
    if (!IsNamePresent(strName))
        return tstring();

    return strName;
}

CSplashScreenPtr CSplashScreenFactory::GetDefault() const
{
    auto strName = GetDefaultName();
    if (strName.empty())
        return nullptr;

    CSplashScreenPtr pSplash = operator[](strName);
    try
    {
        pSplash->Load();
        return pSplash;
    }
    catch (...)
    {}

    return nullptr;
}