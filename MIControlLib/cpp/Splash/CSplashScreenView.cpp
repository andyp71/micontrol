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
#include <ShlObj_core.h>

#pragma comment(lib, "cabinet.lib")

CSplashBitmap::CSplashBitmap()
    : m_sz{ .cx = 0, .cy = 0 }
{
}

LONG CSplashBitmap::Width() const
{
    return m_sz.cx;
}

LONG CSplashBitmap::Height() const
{
    return m_sz.cy;
}

RGBQUAD* CSplashBitmap::Bits()
{
    return m_pix.data();
}

const RGBQUAD* CSplashBitmap::Bits() const
{
    return m_pix.data();
}

PBYTE CSplashBitmap::data()
{
    return reinterpret_cast<PBYTE>(Bits());
}

const BYTE* CSplashBitmap::data() const
{
    return reinterpret_cast<const BYTE*>(Bits());
}

size_t CSplashBitmap::size() const
{
    return m_pix.size();
}

size_t CSplashBitmap::datasize() const
{
    return size() * sizeof(RGBQUAD);
}

bool CSplashBitmap::IsNull() const
{
    return (!m_sz.cx || !m_sz.cy);
}

void CSplashBitmap::store(IOStreamBase& stream) const
{
    if (IsNull())
        return;

    auto strName = m_strName;
    strName.resize(64);
    stream.write((const BYTE*)strName.data(), strName.size());
    stream.twrite(Width());
    stream.twrite(Height());
    stream.write((const BYTE*)m_pix.data(), m_pix.size() * sizeof(RGBQUAD));
}

void CSplashBitmap::load(IOStreamBase& stream)
{
    tstring name(64, 0);
    stream.read((PBYTE)name.data(), name.size());
    name.push_back(0); m_strName = name.c_str();
    
    LONG cx = 0, cy = 0;
    stream.tread(cx);
    stream.tread(cy);

    if (cx <= 0 || cx > 32767)
        throw std::runtime_error("image is too big");

    if (cy <= 0 || cy > 32767)
        throw std::runtime_error("image is too big");

    m_sz.cx = cx;
    m_sz.cy = cy;
    m_pix.resize(static_cast<size_t>(m_sz.cx) * m_sz.cy);
    stream.read((PBYTE)m_pix.data(), m_pix.size() * sizeof(RGBQUAD));
}

void CSplashBitmap::load(LONG cx, LONG cy, const tstring& strName, const RGBQUAD* pData, size_t uSize)
{
    if (cx <= 0 || cx > 32767)
        return;

    if (cy <= 0 || cy > 32767)
        return;

    m_sz.cx = cx;
    m_sz.cy = cy;
    m_strName = strName;
    m_pix.resize(uSize);
    CopyMemory(m_pix.data(), pData, datasize());
}

void CSplashBitmap::load(LONG cx, LONG cy, const tstring& strName, const std::vector<RGBQUAD>& data)
{
    load(cx, cy, strName, data.data(), data.size());
}

const tstring& CSplashBitmap::Name() const
{
    return m_strName;
}

void CSplashBitmap::Name(const tstring& strName)
{
    m_strName = strName;
}

CSplashImage::CSplashImage(const SplashBmpArray& bm, const CSplashBitmap& bmNull, SPLASH_ID& id)
    : m_bmpArr(bm)
    , m_bmNull(bmNull)
    , m_id(id)
{

}

CSplashImage::CSplashImage(const SplashBmpArray& bm, const CSplashBitmap& bmNull)
    : m_bmpArr(bm)
    , m_bmNull(bmNull)
    , m_id(m_lid)
{
}

bool CSplashImage::IsNull() const
{
    return Bitmap().IsNull();
}

CSplashImage::operator bool() const
{
    return !IsNull();
}

const SPLASH_ID& CSplashImage::Info() const
{
    return m_id;
}

void CSplashImage::UpdateId(const SPLASH_ID& id)
{
    auto uRes = m_id.idRes;
    m_id = id;
    m_id.idRes = uRes;
}

void CSplashImage::UpdateResId(USHORT uId)
{
    m_id.idRes = uId;
}

const CSplashBitmap& CSplashImage::Bitmap() const
{
    if (m_id.idImage < 1 || m_id.idImage > m_bmpArr.size())
        return m_bmNull;

    return m_bmpArr[m_id.idImage - 1];
}

CSplashScreenView::CSplashScreenView(SplashCB cb, const tstring& strPreName)
    : m_cb(cb), m_strName(strPreName)
{

}

const tstring& CSplashScreenView::Name() const
{
    return m_strName;
}

void CSplashScreenView::Name(const tstring& strName)
{
    m_strName = strName.substr(0, strName.find_last_not_of(L'\x00') + 1);
}

void CSplashScreenView::Save() const
{
    SPLASH_HEADER header{};
    header.dwSignature = SPLASH_SIGNATURE;

    SPLASH_SUBHEADER    subheader{};
    IOStreamMemory      stream;
    
    size_t uCountOfRes = 0;
    std::for_each(m_splash.id.begin(), m_splash.id.end(), [&uCountOfRes](auto& it) { uCountOfRes += ((it.idRes != 0xFFFF) ? 1 : 0); });
    subheader.uBmpCount = LOWORD(m_splash.bmp.size());
    subheader.uResCount = LOWORD(uCountOfRes);
    stream.twrite(subheader);

    for (const auto& bmp : m_splash.bmp)
    {
        bmp.store(stream);
    }

    auto bmCount = m_splash.bmp.size();

    for (const auto& id : m_splash.id)
    {
        if (0xFFFF == id.idRes)
            continue;

        auto _id = id;
        if (_id.idImage < 1 || id.idImage > bmCount)
            _id.idImage = 0;

        stream.twrite(_id);
    }

    using HCOMPRESSOR_HANDLE = CAutoHandle<COMPRESSOR_HANDLE, nullptr, CloseCompressor>;
    HCOMPRESSOR_HANDLE h;
    if (!CreateCompressor(COMPRESS_ALGORITHM_LZMS, nullptr, &h))
        throw std::runtime_error("failed to allocate compressor");

    SIZE_T uCompressedSize = 10;
    std::vector<BYTE> output(uCompressedSize);

    if (!Compress(h, stream.Buffer().data(), stream.Buffer().size(), output.data(), uCompressedSize, &uCompressedSize))
        if (ERROR_INSUFFICIENT_BUFFER != GetLastError())
            throw std::runtime_error("failed to calculate compressed size");

    output.resize(uCompressedSize);
    if (!Compress(h, stream.Buffer().data(), stream.Buffer().size(), output.data(), output.size(), &uCompressedSize))
        throw std::runtime_error("failed to compress buffer");

    output.resize(uCompressedSize);

    header.uSize = static_cast<DWORD>(output.size());
    CopyMemory(header.md5, MD5Calc(output).data(), std::size(header.md5));

    IOStreamFile file(Filename(), GENERIC_WRITE, CREATE_ALWAYS);
    file.twrite(header);
    file.write(output.data(), output.size());
}

void CSplashScreenView::Load()
{
    if (m_bLoaded)
        return;

    IOStreamFile file(Filename(), GENERIC_READ, OPEN_EXISTING);

    SPLASH_HEADER header{};
    file.tread(header);
    if (header.dwSignature != SPLASH_SIGNATURE)
        throw std::runtime_error("type of splash file is not correct");

    if (file.size() > static_cast<size_t>(10) * 1024 * 1024)
        throw std::runtime_error("file is too big");

    size_t uSize = file.size() - sizeof(header);
    std::vector<BYTE> input(uSize, 0);
    file.read(input.data(), input.size());

    if (std::size(header.md5) != RtlCompareMemory(header.md5, MD5Calc(input).data(), std::size(header.md5)))
        throw std::runtime_error("corrupted splash file");

    using HDECOMPRESSOR_HANDLE = CAutoHandle<DECOMPRESSOR_HANDLE, nullptr, CloseDecompressor>;
    HDECOMPRESSOR_HANDLE h;
    if (!CreateDecompressor(COMPRESS_ALGORITHM_LZMS, nullptr, &h))
        throw std::runtime_error("failed to allocate decompressor");

    SIZE_T uSizeDecompressed = 10;
    std::vector<BYTE> buffer(uSizeDecompressed, 0);

    if (!Decompress(h, input.data(), input.size(), buffer.data(), uSizeDecompressed, &uSizeDecompressed))
        if (ERROR_INSUFFICIENT_BUFFER != GetLastError())
            throw std::runtime_error("failed to calculate decompressed size");

    buffer.resize(uSizeDecompressed);
    if (!Decompress(h, input.data(), input.size(), buffer.data(), buffer.size(), &uSizeDecompressed))
        throw std::runtime_error("failed to decompress data");

    IOStreamMemory stream;
    stream = std::move(buffer);

    SPLASH_SUBHEADER subheader{};
    stream.tread(subheader);

    for (decltype(subheader.uBmpCount)i = 0;i < subheader.uBmpCount;++i)
    {
        m_splash.bmp.emplace_back();
        m_splash.bmp.back().load(stream);
    }

    for (decltype(subheader.uResCount)i = 0;i < subheader.uResCount;++i)
    {
        SPLASH_ID id{};
        stream.tread(id);
        if (id.idImage < 1 || id.idImage > subheader.uBmpCount)
            id.idImage = 0;

        m_splash.id.push_back(id);
    }

    m_bLoaded = true;
}

tstring CSplashScreenView::Filename() const
{
    if (m_strName.empty())
        throw std::runtime_error("name for file was not specified");

    tstring strResult = Name() + TEXT(".mcz");
    PathCleanupSpec(m_cb.strPath.c_str(), strResult.data());

    return m_cb.strPath + strResult;
}

CSplashImagePtr CSplashScreenView::operator[](USHORT res)
{
    auto itID = std::find_if(m_splash.id.begin(), m_splash.id.end(), [&res](const auto& id) { return id.idRes == res; });
    if (itID == m_splash.id.end())
        return nullptr;

    auto& id = *itID;
    return std::make_shared<CSplashImage>(ListBitmaps(), m_splash.bmNull, id);
}

const SplashBmpArray& CSplashScreenView::ListBitmaps() const
{
    return m_splash.bmp;
}

const SplashArray& CSplashScreenView::ListRes() const
{
    return m_splash.id;
}

CSplashImagePtr CSplashScreenView::Create()
{
    return std::make_shared<CSplashImage>(ListBitmaps(), m_splash.bmNull);
}

void CSplashScreenView::Update(CSplashImagePtr pImage)
{
    auto id = pImage->Info();
    if (id.idImage < 1 || id.idImage > m_splash.bmp.size())
        id.idImage = 0;

    auto it = std::find_if(
        m_splash.id.begin(),
        m_splash.id.end(),
        [&id](const auto& _id)
        {
            return id.idRes == _id.idRes;
        }
    );

    if (it == m_splash.id.end())
        m_splash.id.emplace_back(id);
    else
        *it = id;
}

void CSplashScreenView::RemoveBitmap(USHORT uId)
{
    if (uId < 1 || uId > m_splash.bmp.size())
        return;

    std::for_each(
        m_splash.id.begin(),
        m_splash.id.end(),
        [&uId](auto& id)
        {
            if (id.idImage > uId)
                id.idImage--;
            else if (id.idImage == uId)
                id.idImage = 0;
        }
    );

    m_splash.bmp.erase(m_splash.bmp.begin() + uId - 1);
}

USHORT CSplashScreenView::AppendBitmap(const CSplashBitmap& bm)
{
    auto uId = LOWORD(m_splash.bmp.size());
    m_splash.bmp.emplace_back();
    m_splash.bmp.back().load(bm.Width(), bm.Height(), bm.Name(), bm.Bits(), bm.size());
    return uId;
}

void CSplashScreenView::ReplaceBitmap(USHORT uId, const CSplashBitmap& bm)
{
    if (uId < 1 || uId > m_splash.bmp.size())
        return;

    auto& _bm = m_splash.bmp[uId - 1];
    _bm.load(bm.Width(), bm.Height(), bm.Name(), bm.Bits(), bm.size());
}

void CSplashScreenView::CopyFrom(const CSplashScreenView& view)
{
    if (this == &view)
        return;

    m_splash = view.m_splash;
    m_strName = view.m_strName;
    m_bLoaded = view.m_bLoaded;
}

void CSplashScreenView::MoveFrom(CSplashScreenView&& view) noexcept
{
    if (this == &view)
        return;

    m_splash = std::move(view.m_splash);
    m_strName = std::move(view.m_strName);
    m_bLoaded = view.m_bLoaded;
    view.m_bLoaded = false;
}

CSplashScreenPtr CSplashScreenView::Copy() const
{
    auto pSplash = make_shared(m_cb, m_strName);
    pSplash->m_splash = m_splash;

    return pSplash;
}