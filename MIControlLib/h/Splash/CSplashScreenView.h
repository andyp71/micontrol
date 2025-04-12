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

#define SPLASH_SIGNATURE        static_cast<DWORD>('>CSM')

class CSplashScreenView;

using CSplashScreenPtr = std::shared_ptr<CSplashScreenView>;

using CB_Check = std::function<void(const tstring&,bool)>;

struct SplashCB
{
    const tstring   strPath;
};

#pragma pack(push, 1)
typedef struct _tagSPLASH_HEADER
{
    DWORD           dwSignature;
    BYTE            md5[16];
    DWORD           uSize;
} SPLASH_HEADER, *PSPLASH_HEADER;

typedef struct _tagSPLASH_SUBHEADER
{
    USHORT          uBmpCount;
    USHORT          uResCount;
} SPLASH_SUBHEADER, *PSPLASH_SUBHEADER;

typedef struct _tagSPLASH_ID
{
    USHORT          idRes;
    USHORT          idImage;
    union
    {
        DWORD       dwFlags;
        struct
        {
            BYTE            bAlphaSrc : 1;
            BYTE            bUseDPI : 1;
        } flags;
    };
    POINT           ptScreen;
    USHORT          uTimeToShow;
    BYTE            bInitialAlpha;
    DWORD           dwReserved[8];
} SPLASH_ID, *PSPLASH_ID;
#pragma pack(pop)

using PIXEL_ARRAY = std::vector<RGBQUAD>;

class CSplashBitmap
{
    PIXEL_ARRAY     m_pix;
    SIZE            m_sz;
    tstring         m_strName;

public:
    CSplashBitmap();
    LONG            Width() const;
    LONG            Height() const;
    RGBQUAD*        Bits();
    PBYTE           data();
    const BYTE*     data() const;
    const RGBQUAD*  Bits() const;
    size_t          datasize() const;
    size_t          size() const;
    bool            IsNull() const;
    const tstring&  Name() const;
    void            Name(const tstring& strName);

    void            store(IOStreamBase& stream) const;
    void            load(IOStreamBase& stream);
    void            load(LONG cx, LONG cy, const tstring& strName, const RGBQUAD* pData, size_t uSize);
    void            load(LONG cx, LONG cy, const tstring& strName, const std::vector<RGBQUAD>& data);
};

using SplashArray = std::vector<SPLASH_ID>;
using SplashBmpArray = std::vector<CSplashBitmap>;

class CSplashImage
{
    SPLASH_ID           m_lid{};
    SPLASH_ID&          m_id;
    const SplashBmpArray&     m_bmpArr;
    const CSplashBitmap&      m_bmNull;

public:
    CSplashImage(const SplashBmpArray& bm, const CSplashBitmap& bmNull, SPLASH_ID& id);
    CSplashImage(const SplashBmpArray& bm, const CSplashBitmap& bmNull);

    const SPLASH_ID&        Info() const;
    void                    UpdateId(const SPLASH_ID&);
    void                    UpdateResId(USHORT uId);
    const CSplashBitmap&    Bitmap() const;
    bool                    IsNull() const;
    operator bool() const;
};

enum class CBSplashState
{
    Bitmap,
    Resources,
    Compressing
};

using CSplashImagePtr = std::shared_ptr<CSplashImage>;
class CSplashScreenFactory;

class CSplashScreenView
{
    friend CSplashScreenFactory;
    CSplashScreenView(SplashCB cb, const tstring& strPreName);

    tstring         m_strName;
    SplashCB        m_cb;
    bool            m_bLoaded = false;

    struct
    {
        CSplashBitmap       bmNull;
        SplashArray         id;
        SplashBmpArray      bmp;
    } m_splash;

    static CSplashScreenPtr make_shared(SplashCB cb, const tstring& strPreName)
    {
        struct make_shared_private : public CSplashScreenView { make_shared_private(SplashCB cb, const tstring& strPreName) : CSplashScreenView(cb, strPreName) {} };
        return 
            std::make_shared<make_shared_private>(cb, strPreName);
    }

public:
    const tstring& Name() const;
    void Name(const tstring& strName);
    
    void    Save() const;
    void    Load();

    tstring Filename() const;

    CSplashImagePtr operator[](USHORT res);

    const SplashBmpArray&   ListBitmaps() const;
    const SplashArray&      ListRes() const;

    CSplashImagePtr Create();
    void    Update(CSplashImagePtr pImage);
    USHORT  AppendBitmap(const CSplashBitmap& bm);
    void    ReplaceBitmap(USHORT uId, const CSplashBitmap& bm);
    void    RemoveBitmap(USHORT uId);
    void    CopyFrom(const CSplashScreenView& view);
    void    MoveFrom(CSplashScreenView&& view) noexcept;
    CSplashScreenPtr Copy() const;
};