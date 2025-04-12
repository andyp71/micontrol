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

#include <Windows.h>
#include "CAutoHandle.h"

class IOStreamBase
{
public:
    virtual void write(const BYTE* pData, size_t uSize) = 0;
    virtual void read(BYTE* pData, size_t uSize) = 0;
    virtual size_t size() const = 0;

    template<class T> void twrite(const T& data)
    {
        write(reinterpret_cast<const BYTE*>(&data), sizeof(T));
    }

    template<class T> void twrite(const std::vector<T>& data)
    {
        for (const auto& i : data)
            write(reinterpret_cast<const BYTE*>(&i), sizeof(T));
    }

    template<class T> void twrite(const std::vector<BYTE>& data)
    {
        write(data.data(), data.size());
    }

    template<class T> void tread(T& data)
    {
        read(reinterpret_cast<PBYTE>(&data), sizeof(T));
    }

    template<class T> void tread(std::vector<T>& data)
    {
        for (auto& i : data)
            read(reinterpret_cast<PBYTE>(&i), sizeof(T));
    }

    virtual void seek(size_t ptr, DWORD seekMode) = 0;
};

class IOStreamFile: public IOStreamBase
{
    CAutoHandle<HANDLE, INVALID_HANDLE_VALUE, ::CloseHandle>    m_hFile;

public:
    IOStreamFile(const tstring& strFile, DWORD uMode, DWORD uOpenMode)
        : m_hFile(
            CreateFile(strFile.c_str(), uMode, FILE_SHARE_READ, nullptr, uOpenMode, 0, nullptr)
        ) 
    {
        if (!m_hFile)
            throw std::runtime_error("open file failed");
    }

    void write(const BYTE* pData, size_t uSize) override
    {
        DWORD dw = 0;
        if (!WriteFile(m_hFile, pData, static_cast<DWORD>(uSize), &dw, nullptr) || dw != uSize)
            throw std::runtime_error("failed to write to file");
    }

    void read(BYTE* pData, size_t uSize) override
    {
        DWORD dw = 0;
        if (!ReadFile(m_hFile, pData, static_cast<DWORD>(uSize), &dw, nullptr) || dw != uSize)
            throw std::runtime_error("failed to read from file");
    }

    void seek(size_t ptr, DWORD seekMode)
    {
        LARGE_INTEGER li{ .QuadPart = static_cast<LONGLONG>(ptr) };
        if (!SetFilePointerEx(m_hFile, li, nullptr, seekMode))
            throw std::runtime_error("failed to set new pointer for file");
    }

    size_t size() const
    {
        LARGE_INTEGER li{};
        if (GetFileSizeEx(m_hFile, &li))
            return li.QuadPart;

        return 0;
    }
};

class IOStreamMemory : public IOStreamBase
{
    size_t                      m_ptr = 0;
    std::vector<BYTE>           m_buffer;

public:
    IOStreamMemory& operator=(std::vector<BYTE>&& data)
    {
        m_buffer = data;
        data.clear();
        return *this;
    }

    void write(const BYTE* pData, size_t uSize) override
    {
        if (m_ptr >= m_buffer.size())
        {
            m_buffer.insert(m_buffer.end(), pData, pData + uSize);
            m_ptr += uSize;
        }
        else
        {
            auto uNewSize = m_ptr + uSize;
            if (uNewSize > m_buffer.size())
            {
                m_buffer.resize(uNewSize);
            }
            CopyMemory(m_buffer.data() + m_ptr, pData, uSize);
            m_ptr += uSize;
        }
    }

    void read(BYTE* pData, size_t uSize) override
    {
        if (m_ptr + uSize > m_buffer.size())
            throw std::runtime_error("read from out of bound of buffer");

        CopyMemory(pData, m_buffer.data() + m_ptr, uSize);
        m_ptr += uSize;
    }

    void seek(size_t ptr, DWORD seekMode)
    {
        decltype(m_ptr) lptr = 0;
        switch (seekMode)
        {
        case SEEK_SET: lptr = ptr; break;
        case SEEK_CUR: lptr = m_ptr + ptr; break;
        case SEEK_END: lptr = m_ptr; break;
        }

        if (lptr > m_buffer.size())
            throw std::runtime_error("position invalid for buffer");

        m_ptr = lptr;
    }

    const std::vector<BYTE>& Buffer() const
    {
        return m_buffer;
    }

    size_t size() const
    {
        return m_buffer.size();
    }
};