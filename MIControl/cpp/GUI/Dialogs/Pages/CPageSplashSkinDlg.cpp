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
#include <shellapi.h>

constexpr auto CSkinResourcePosDlg_width = 250;
constexpr auto CSkinResourcePosDlg_height = 250;
constexpr USHORT CSkinResourcePosDlg_uid = 0x100;

CSkinResourcePosDlg::CSkinResourcePosDlg(HINSTANCE hInstance, POINT pt)
    : CDialog(hInstance, CSkinResourcePosDlg_width, CSkinResourcePosDlg_height, TEXT("Splash position"), TRUE)
    , m_fr(hInstance, TEXT("Set position for this image"), 7, 7, -7, -(7 + 15 + 7))
    , m_pt{ .x = pt.x, .y = pt.y }
{
    constexpr auto xMargin = 15;
    constexpr auto yMargin = 15;
    constexpr auto uWidth = (CSkinResourcePosDlg_width - 2 * 7);
    constexpr auto uHeight = (CSkinResourcePosDlg_height - 7 - (7 + 15 + 7));
    
    int x = xMargin, y = yMargin;
    auto xStep = (uWidth - xMargin) / SPLASH_POS_COUNT_X;
    auto yStep = (uHeight - yMargin) / SPLASH_POS_COUNT_Y;
    USHORT uId = CSkinResourcePosDlg_uid;

    pushctl(m_fr);

    for (auto& rdHorz : m_rdPos)
    {
        for (auto& rd : rdHorz)
        {
            rd.PostInit(
                hInstance,
                uId++, WC_BUTTON,
                TEXT(""),
                x, y, xStep - 1, yStep - 1,
                WS_VISIBLE | WS_TABSTOP | BS_AUTORADIOBUTTON
            );

            pushctl(rd);
            x += xStep;
        }

        x = xMargin;
        y += yStep;
    }
}

BOOL CSkinResourcePosDlg::OnInit(LPARAM lParam)
{
    USHORT uSelectedId =
        LOWORD(CSkinResourcePosDlg_uid)
        + LOWORD((m_pt.x - SPLASH_POS_MARGIN_X - SPLASH_POS_STEP_X / 2) / SPLASH_POS_STEP_X)
        + SPLASH_POS_COUNT_X * LOWORD((m_pt.y - SPLASH_POS_MARGIN_Y - SPLASH_POS_STEP_Y / 2) / SPLASH_POS_STEP_Y);

    CheckRadioButton(*this, CSkinResourcePosDlg_uid, CSkinResourcePosDlg_uid + SPLASH_POS_COUNT_X * SPLASH_POS_COUNT_Y, uSelectedId);

    SetFocus(Ctl(uSelectedId));

    return FALSE;
}

void CSkinResourcePosDlg::OnButton(USHORT uId)
{
    auto c = uId - CSkinResourcePosDlg_uid;
    m_pt.x = SPLASH_POS_MARGIN_X + SPLASH_POS_STEP_X / 2 + (c % SPLASH_POS_COUNT_X) * SPLASH_POS_STEP_X;
    m_pt.y = SPLASH_POS_MARGIN_Y + SPLASH_POS_STEP_Y / 2 + (c / SPLASH_POS_COUNT_Y) * SPLASH_POS_STEP_Y;
}

void CSkinResourcePosDlg::OnOK()
{
    Close(1);
}

POINT CSkinResourcePosDlg::QueryPt() const
{
    return m_pt;
}

CSkinImagesListDlg::CSkinImagesListDlg(HINSTANCE hInstance, CSplashScreenView& view)
    : CDialog(hInstance, 300, 200, TEXT("Images"))
    , m_view(view)
    , m_frBmp(hInstance, TEXT("Loaded images"), 7, 7, -7, -(7 + 15 + 7))
    , m_lvBmp(hInstance, 0x10, 15, 20, -15, -(15 + 7 + 15 + 7), WS_VISIBLE | WS_TABSTOP | LVS_SHOWSELALWAYS | LVS_REPORT | LVS_NOSORTHEADER, WS_EX_STATICEDGE)
    , m_btnAdd(hInstance, 0x11, TEXT("Import images..."), 7, -(7 + 15), 70, 15)
    , m_btnDel(hInstance, 0x12, TEXT("Delete checked..."), 87, -(7 + 15), 70, 15)
{
    pushctl(m_frBmp);
    pushctl(m_lvBmp);
    pushctl(m_btnAdd);
    pushctl(m_btnDel);
}

BOOL CSkinImagesListDlg::OnInit(LPARAM lParam)
{
    m_lvBmp.AddColumn(TEXT("Image name"), 200);
    m_lvBmp.AddColumn(TEXT("Resolution"));
    m_lvBmp.ExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
    UpdateLv();
    SetFocus(m_lvBmp);
    return FALSE;
}

void CSkinImagesListDlg::OnButton(USHORT uId)
{
    switch (uId)
    {
    case 0x11: return OnAdd();
    case 0x12: return OnDel();
    }
}

void CSkinImagesListDlg::OnAdd()
{
    OPENFILENAME ofn = { 0 };

    tstring imList[] =
    {
        TEXT("Images files (*.bmp, *.png, *.ico)"), TEXT("*.bmp;*.png;*.ico"),
        TEXT("Bitmap files (*.bmp)"), TEXT("*.bmp"),
        TEXT("Icon files (*.ico)"), TEXT("*.ico"),
        TEXT("PNG files (*.png)"), TEXT("*.png")
    };

    tstring szList, szFiles(32767, 0);
    for (const auto& t : imList)
    {
        szList.insert(szList.end(), t.cbegin(), t.cend());
        szList.push_back(0);
    }
    szList.push_back(0);

    ofn.lStructSize = sizeof(ofn);
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT;
    ofn.hInstance = GetInstance();
    ofn.hwndOwner = *this;
    ofn.lpstrFile = szFiles.data();
    ofn.lpstrFilter = szList.data();
    ofn.nMaxFile = static_cast<DWORD>(szFiles.size());

    if (!GetOpenFileName(&ofn))
        return;

    std::vector<tstring> strFiles;
    szList.clear();

    auto psz = szFiles.data();
    auto l = lstrlen(psz);
    while (l > 0)
    {
        if (szList.empty())
            szList = tstring(psz);
        else
            strFiles.push_back(szList + TEXT("\\") + psz);

        psz += l + 1;
        l = lstrlen(psz);
    }

    if (strFiles.empty())
        strFiles.push_back(szList);

    szFiles.clear();
    CBitmap bmFile;
    CSplashBitmap bmSplash;

    for (const auto& f : strFiles)
    {
        auto nExt = f.find_last_of(_T('.'));
        if (nExt == f.npos)
            continue;

        auto strName = f.substr(f.find_last_of(TEXT("\\")) + 1);

        LPCTSTR pszExt = f.c_str() + nExt;
        if (!lstrcmpi(TEXT(".png"), pszExt))
        {
            CPngLoader png(f.c_str());
            if (!png.FixedBitmap(bmFile))
                continue;

            bmSplash.load(bmFile.Width(), bmFile.Height(), strName, bmFile.Bits(), bmFile.Width() * bmFile.Height());
        }
        else if (!lstrcmpi(TEXT(".ico"), pszExt))
        {
            HICON hIco = reinterpret_cast<HICON>(LoadImage(nullptr, f.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE));
            if (!hIco)
                continue;

            CIconBits ico(hIco);
            DestroyIcon(hIco);
            bmSplash.load(ico.Width(), ico.Height(), strName, ico.Bits(), static_cast<size_t>(ico.Size().cx) * ico.Size().cy);
        }
        else if (!lstrcmpi(TEXT(".bmp"), pszExt))
        {
            HBITMAP hBmp = reinterpret_cast<HBITMAP>(LoadImage(nullptr, f.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION));
            if (!hBmp)
                continue;

            bmFile.CreateFromBitmap(hBmp);
            bmSplash.load(bmFile.Width(), bmFile.Height(), strName, bmFile.Bits(), static_cast<size_t>(bmFile.Width()) * bmFile.Height());
        }

        m_view.AppendBitmap(bmSplash);
    }

    UpdateLv();
}

void CSkinImagesListDlg::OnDel()
{
    std::vector<USHORT> vId;
    const auto lCount = static_cast<int>(m_lvBmp.Count());
    const auto& bmpList = m_view.ListBitmaps();

    for (int i = 0; i < lCount; ++i)
    {
        if (!m_lvBmp.Check(i))
            continue;

        auto id = static_cast<USHORT>(m_lvBmp.Param(i));
        if (!id || id > bmpList.size())
            continue;

        vId.push_back(id);
    }

    if (vId.empty())
        return;

    if (!Confirm(TEXT("Delete checked images from the list?")))
        return;

    std::sort(vId.begin(), vId.end(), std::greater<>());

    for (const auto& id : vId)
        m_view.RemoveBitmap(id);

    UpdateLv();
}

INT_PTR CSkinImagesListDlg::OnNotify(LPNMHDR lpnm)
{
    return INT_PTR();
}

void CSkinImagesListDlg::UpdateLv()
{
    m_lvBmp.ResetContent();
    auto idPtr = static_cast<INT>(m_lvBmp.AddItem(TEXT("<hidden>"), 0));
    m_lvBmp.SetItem(idPtr, 1, TEXT("<none>"));

    const auto& bmpList = m_view.ListBitmaps();
    LPARAM l = 1;
    for (const auto& bmp : bmpList)
    {
        CString str;
        str.Format(TEXT("%d x %d"), bmp.Width(), bmp.Height());
        idPtr = static_cast<INT>(m_lvBmp.AddItem(bmp.Name().c_str(), l++));
        m_lvBmp.SetItem(idPtr, 1, str);
    }
}

CSkinResourceEditorDlg::CSkinResourceEditorDlg(HINSTANCE hInstance, SPLASH_ID spl, const SplashBmpArray& bmArray, bool bCanChangeResId)
    : CDialog(hInstance, 300, 250, TEXT("Resource editor"), true)
    , m_id(spl)
    , m_bmArray(bmArray)
    , m_stResId(hInstance, TEXT("Resource:"), 7, 9, 75, 10)
    , m_cbResId(hInstance, 0x10, WC_COMBOBOX, TEXT(""), 90, 7, -7, -7, WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL | (bCanChangeResId ? 0 : WS_DISABLED))
    , m_chkAlphaSrc(hInstance, 0x11, TEXT("Use source alpha channel"), 90, 27, -7, 13)
    , m_chkDPI(hInstance, 0x12, TEXT("Adjust scale to DPI"), 90, 47, -7, 13)
    , m_stTimer(hInstance, TEXT("Show delay:"), 7, 69, 75, 10)
    , m_cbTimer(hInstance, 0x13, WC_COMBOBOX, TEXT(""), 90, 67, -7, -7, WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL)
    , m_stAlpha(hInstance, TEXT("Transparency (%):"), 7, 89, 75, 10)
    , m_edAlpha(hInstance, 0x14, nullptr, 90, 87, 40, 13)
    , m_udAlpha(hInstance, 0x15)
    , m_stBmp(hInstance, TEXT("Image"), 7, 109, 75, 10)
    , m_lvRes(hInstance, 0x16, 7, 125, -7, -(7 + 15 + 7), WS_VISIBLE | WS_TABSTOP | WS_BORDER | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOCOLUMNHEADER | LVS_SINGLESEL)
    , m_btnPreview(hInstance, 0x17, TEXT("Preview"), 7, -(7 + 15), 50, 15)
    , m_btnPosition(hInstance, 0x18, TEXT("Position..."), 7 + 50 + 7, -(7 + 15), 50, 15)
{
    pushctl(m_stResId);
    pushctl(m_cbResId);
    pushctl(m_chkAlphaSrc);
    pushctl(m_chkDPI);
    pushctl(m_stTimer);
    pushctl(m_cbTimer);
    pushctl(m_stAlpha);
    pushctl(m_edAlpha);
    pushctl(m_udAlpha);
    pushctl(m_stBmp);
    pushctl(m_lvRes);
    pushctl(m_btnPreview);
    pushctl(m_btnPosition);
}

BOOL CSkinResourceEditorDlg::OnInit(LPARAM lParam)
{
    auto id = static_cast<UINT>(m_cbTimer.cSendMessage(CB_ADDSTRING, 0, (LPARAM)TEXT("Quick")));
    m_cbTimer.cSendMessage(CB_SETITEMDATA, id, 50);
    id = static_cast<UINT>(m_cbTimer.cSendMessage(CB_ADDSTRING, 0, (LPARAM)TEXT("Medium")));
    m_cbTimer.cSendMessage(CB_SETITEMDATA, id, 500);
    id = static_cast<UINT>(m_cbTimer.cSendMessage(CB_ADDSTRING, 0, (LPARAM)TEXT("Long")));
    m_cbTimer.cSendMessage(CB_SETITEMDATA, id, 5000);
    

    size_t k = 1;
    m_lvRes.AddColumn(TEXT("Image name"));
    m_lvRes.AddItem(TEXT("<hidden>"), 0);
    for (const auto& bm : m_bmArray)
        m_lvRes.AddItem(bm.Name().c_str(), k++);


    for (const auto& id : CPageSplashSkinDlg::ResourceNames())
    {
        auto _id = m_cbResId.cSendMessage(CB_ADDSTRING, 0, (LPARAM)id.second.c_str());
        m_cbResId.cSendMessage(CB_SETITEMDATA, _id, id.first);

        if (id.first == m_id.idRes)
            m_cbResId.cSendMessage(CB_SETCURSEL, _id);
    }

    m_lvRes.CurSel(m_id.idImage);
    m_lvRes.ScrollToVisible();
    m_udAlpha.Value(MulDiv(m_id.bInitialAlpha, 100, 255));
    switch (m_id.uTimeToShow)
    {
    case 50: m_cbTimer.cSendMessage(CB_SETCURSEL, 0); break;
    case 5000: m_cbTimer.cSendMessage(CB_SETCURSEL, 2); break;
    default: m_cbTimer.cSendMessage(CB_SETCURSEL, 1); break;
    }

    m_chkAlphaSrc.Check(m_id.flags.bAlphaSrc);
    m_chkDPI.Check(m_id.flags.bUseDPI);

    SetFocus(m_cbResId);
    OnCommand(CBN_SELENDOK, 0x13, nullptr);

    return FALSE;
}

void CSkinResourceEditorDlg::OnOK()
{
    if (!m_id.idRes)
    {
        ShowError(TEXT("You should select a resource id!"));
        return;
    }

    Close(1);
}

SPLASH_ID CSkinResourceEditorDlg::QueryId() const
{
    return m_id;
}

void CSkinResourceEditorDlg::OnButton(USHORT uId)
{
    switch (uId)
    {
    case 0x11: m_id.flags.bAlphaSrc = m_chkAlphaSrc.Check(); break;
    case 0x12: m_id.flags.bUseDPI = m_chkDPI.Check(); break;
    case 0x17:
    {
        if (!m_wndSplash)
        {
            m_wndSplash = std::make_unique<CSplashScreen>(GetInstance());
            m_wndSplash->Create(nullptr);
        }

        CSplashBitmap bmNull;
        CSplashImagePtr pimg = std::make_shared<CSplashImage>(m_bmArray, bmNull, m_id);
        m_wndSplash->SetSplash(pimg);
        break;
    }
    case 0x18:
    {
        auto pdlg = std::make_unique<CSkinResourcePosDlg>(GetInstance(), m_id.ptScreen);
        if (pdlg->Show(this))
            m_id.ptScreen = pdlg->QueryPt();
        
        break;
    }
    }
}

void CSkinResourceEditorDlg::OnCommand(USHORT uCmd, USHORT uId, HWND hCtl)
{
    switch (uCmd)
    {
    case CBN_SELENDOK:
    {
        switch (uId)
        {
        case 0x10:
        {
            auto _id = m_cbResId.cSendMessage(CB_GETCURSEL);
            if (_id < 0)
                break;

            auto _res = m_cbResId.cSendMessage(CB_GETITEMDATA, _id);
            m_id.idRes = LOWORD(_res);
            break;
        }
        case 0x13:
        {
            auto _id = m_cbTimer.cSendMessage(CB_GETCURSEL);
            if (_id < 0)
                break;

            auto _timer = m_cbTimer.cSendMessage(CB_GETITEMDATA, _id);
            m_id.uTimeToShow = LOWORD(_timer);
            break;
        }
        }
        break;
    }
    case EN_CHANGE:
    {
        switch (uId)
        {
        case 0x14:
        {
            if (m_udAlpha.IsError())
                return;

            m_id.bInitialAlpha = LOBYTE(MulDiv(m_udAlpha.Value(), 255, 100));
            break;
        }
        }
        break;
    }
    }
}

INT_PTR CSkinResourceEditorDlg::OnNotify(LPNMHDR lpnm)
{
    switch (lpnm->idFrom)
    {
    case 0x16:
    {
        switch (lpnm->code)
        {
        case LVN_ITEMCHANGED:
        {
            if (!(LPNMLISTVIEW(lpnm)->uNewState & LVIS_SELECTED))
                return 0;

            auto _id = static_cast<int>(m_lvRes.CurSel());
            if (_id < 0)
            {
                m_lvRes.CurSel(m_id.idImage);
                break;
            }

            auto _idImage = LOWORD(m_lvRes.Param(_id));
            m_id.idImage = _idImage;
            break;
        }
        }
        break;
    }
    }

    return 0;
}

CSkinEditorDlg::CSkinEditorDlg(HINSTANCE hInstance, CSplashScreenView& view, bool bCanRename)
    : CDialog(hInstance, 300, 270, TEXT("Splash editor"), TRUE)
    , m_stName(hInstance, TEXT("Name of splash:"), 7, 9, 100, 10)
    , m_edName(hInstance, 0x1001, nullptr, 110, 7, -7, 13)
    , m_lvRes(hInstance, 0x10, 7, 30, -7, 170, WS_VISIBLE | WS_TABSTOP | WS_BORDER | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL | LVS_NOSORTHEADER, WS_EX_CLIENTEDGE)
    , m_btnAdd(hInstance, 0x100, TEXT("New..."), 7, 215, 50, 15)
    , m_btnMod(hInstance, 0x101, TEXT("Modify..."), 67, 215, 50, 15)
    , m_btnDel(hInstance, 0x102, TEXT("Delete..."), 127, 215, 50, 15)
    , m_btnAddBitmap(hInstance, 0x103, TEXT("Images..."), 187, 215, 75, 15)
    , m_stLine(hInstance, 0xFFFF, WC_STATIC, TEXT(""), 5, -(7 + 15 + 7), -5, 2, WS_VISIBLE | SS_ETCHEDHORZ)
    , m_view(view)
    , m_bCanRename(bCanRename)
{
    pushctl(m_stName);
    pushctl(m_edName);
    pushctl(m_lvRes);
    pushctl(m_btnAdd);
    pushctl(m_btnMod);
    pushctl(m_btnDel);
    pushctl(m_btnAddBitmap);
    pushctl(m_stLine);
}

BOOL CSkinEditorDlg::OnInit(LPARAM lParam)
{
    m_lvRes.ExtendedStyle(LVS_EX_FULLROWSELECT);
    m_lvRes.AddColumn(TEXT("Resource"), 250);
    m_lvRes.AddColumn(TEXT("Image"));

    UpdateLv(0);
    if (!m_view.Name().empty())
    {
        m_edName.Text(m_view.Name());
        m_edName.Enable(false);
    }

    m_edName.Enable(m_bCanRename);
    SetFocus(m_bCanRename ? m_edName : m_btnAdd);
        
    return FALSE;
}

void CSkinEditorDlg::OnCommand(USHORT uCmd, USHORT uId, HWND hCtl)
{
    switch (uCmd)
    {
    case EN_CHANGE:
    {
        switch (uId)
        {
        case 0x1001:
        {
            if (m_bCanRename)
                m_view.Name(m_edName.Text());
            break;
        }
        }
        break;
    }
    }
}

void CSkinEditorDlg::UpdateLv(USHORT uId)
{
    const auto& ids = m_view.ListRes();
    const auto& bms = m_view.ListBitmaps();
    auto mid = CPageSplashSkinDlg::ResourceNames();

    m_lvRes.ResetContent();

    for (const auto& id : ids)
    {
        if (0xFFFF == id.idRes)
            continue;

        auto lid = static_cast<int>(m_lvRes.AddItem(mid[id.idRes].c_str(), id.idRes));
        if (id.idImage >= 1 && id.idImage <= bms.size())
            m_lvRes.SetItem(lid, 1, bms[id.idImage - 1].Name().c_str());
        else
            m_lvRes.SetItem(lid, 1, TEXT("<hidden>"));

        if (id.idRes == uId)
            m_lvRes.CurSel(lid);
    }

    m_lvRes.ScrollToVisible();
}

void CSkinEditorDlg::OnButton(USHORT uId)
{
    switch (uId)
    {
    case 0x100: OnNewResource(); break;
    case 0x101: OnModResource(); break;
    case 0x102: OnDelResource(); break;
    case 0x103: OnAddBitmaps(); break;
    }
}

void CSkinEditorDlg::OnNewResource()
{
    if (m_view.ListBitmaps().empty())
    {
        if (Confirm(TEXT("You have not imported images.\nDo you want import it now?")))
            OnAddBitmaps();
    }

    CSplashImagePtr pImage = m_view.Create();
    
    // setup default values for point
    auto spl = pImage->Info();
    spl.ptScreen.x = SPLASH_POS_MARGIN_X + SPLASH_POS_STEP_X / 2;
    spl.ptScreen.y = SPLASH_POS_MARGIN_Y + SPLASH_POS_STEP_Y / 2;
    spl.uTimeToShow = 500;
    spl.bInitialAlpha = 255;
    spl.flags.bUseDPI = true;
    pImage->UpdateId(spl);

    CSkinResourceEditorDlg dlg(GetInstance(), pImage->Info(), m_view.ListBitmaps(), true);
    if (!dlg.Show(this))
        return;

    pImage->UpdateResId(dlg.QueryId().idRes);
    pImage->UpdateId(dlg.QueryId());
    m_view.Update(pImage);
    UpdateLv(pImage->Info().idRes);
}

void CSkinEditorDlg::OnOK()
{
    if (m_view.Name().empty())
    {
        ShowError(TEXT("You should specify a name of this skin!"));
        return;
    }

    Close(1);
}

void CSkinEditorDlg::OnModResource()
{
    auto lid = static_cast<int>(m_lvRes.CurSel());
    if (lid < 0)
        return;

    auto id = LOWORD(m_lvRes.Param(lid));
    CSplashImagePtr pImage = m_view[id];
    if (!pImage)
        return;

    CSkinResourceEditorDlg dlg(GetInstance(), pImage->Info(), m_view.ListBitmaps(), false);
    if (!dlg.Show(this))
        return;

    pImage->UpdateId(dlg.QueryId());
    m_view.Update(pImage);
    UpdateLv(pImage->Info().idRes);
}

void CSkinEditorDlg::OnDelResource()
{
    auto lid = static_cast<int>(m_lvRes.CurSel());
    if (lid < 0)
        return;

    auto id = LOWORD(m_lvRes.Param(lid));
    CSplashImagePtr pImage = m_view[id];
    if (!pImage)
        return;

    if (!ConfirmF(TEXT("Are you sure that you want to delete resource '%s'?"), false, CPageSplashSkinDlg::ResourceNames()[pImage->Info().idRes].c_str()))
        return;

    pImage->UpdateResId(0xFFFF);
    UpdateLv(0);
}

void CSkinEditorDlg::OnAddBitmaps()
{
    CSkinImagesListDlg dlg(GetInstance(), m_view);
    dlg.Show(this);
    UpdateLv(LOWORD(m_lvRes.Param(static_cast<int>(m_lvRes.CurSel()))));
}

CPageSplashSkinDlg::CPageSplashSkinDlg(HINSTANCE hInstance, SendCommand_t fn, HOST_DATA& status)
    : CPageDlg(hInstance, fn, status)
    , m_lvList(hInstance, 0x10, 7, 7, 10, 150, WS_VISIBLE | WS_TABSTOP | WS_BORDER | LVS_REPORT | LVS_NOCOLUMNHEADER | LVS_SHOWSELALWAYS | LVS_SINGLESEL, WS_EX_CLIENTEDGE)
    , m_btnAdd(hInstance, 0x100, TEXT("New..."), 7, 185, 50, 15)
    , m_btnMod(hInstance, 0x101, TEXT("Modify..."), 67, 185, 50, 15)
    , m_btnDel(hInstance, 0x102, TEXT("Delete..."), 127, 185, 50, 15)
    , m_btnSetDefault(hInstance, 0x103, TEXT("Set skin"), 187, 185, 50, 15)
    , m_btnUnset(hInstance, 0x104, TEXT("Use default"), 247, 185, 50, 15)
    , m_btnOpenFolder(hInstance, 0x105, TEXT("Open skins folder..."), 7, 210, 100, 15)
{

    pushctl(m_lvList);
    pushctl(m_btnAdd);
    pushctl(m_btnMod);
    pushctl(m_btnDel);
    pushctl(m_btnSetDefault);
    pushctl(m_btnUnset);
    pushctl(m_btnOpenFolder);
}

BOOL CPageSplashSkinDlg::OnPageInit(LPARAM lParam)
{
    RECT rc({ 0 });
    GetClientRect(m_lvList, &rc);

    CPngLoader png(GetInstance(), MAKEINTRESOURCE(IDB_ICONS), TEXT("PNG"));
    CBitmap bm;
    if (png.FixedBitmap(bm))
    {
        auto dpi = GetDpiForWindow(*this);
        LONG cx = MulDiv(bm.Width(), dpi, 96);
        LONG cy = MulDiv(bm.Height(), dpi, 96);
        if (cx != bm.Width() || cy != bm.Height())
        {
            CBitmap bmNew(cx, cy);
            BLENDFUNCTION bf = { 0 };
            bf.AlphaFormat = AC_SRC_ALPHA;
            bf.BlendOp = AC_SRC_OVER;
            bf.SourceConstantAlpha = 255;
            GdiAlphaBlend(bmNew, 0, 0, cx, cy, bm, 0, 0, bm.Width(), bm.Height(), bf);

            bm.Create(cx, cy);
            bm.CopyRect(bmNew, 0, 0, cx, cy, 0, 0);
        }

        HIMAGELIST himl = ImageList_Create(bm.Height(), bm.Height(), ILC_COLOR32, bm.Width() / bm.Height(), 1);
        if (himl)
        {
            auto hbm = bm.Bitmap();
            ImageList_Add(himl, hbm, nullptr);
            DeleteBitmap(hbm);
            ListView_SetImageList(m_lvList, himl, LVSIL_NORMAL);
            ListView_SetImageList(m_lvList, himl, LVSIL_SMALL);
        }
    }

    m_lvList.AddColumn(TEXT(""), 600);
    UpdateLv();

    SetFocus(m_lvList);
    return FALSE;
}

void CPageSplashSkinDlg::OnShow()
{

}

void CPageSplashSkinDlg::OnSizeChanged(INT dw, USHORT x, USHORT y)
{
    union RCPT
    {
        RECT    rc;
        POINT   pt[2];
    };


    RCPT rc{}, rcLv{}, rcProgress{};
    GetClientRect(*this, &rc.rc);
    GetWindowRect(m_lvList, &rcLv.rc);
    ScreenToClient(*this, &rcLv.pt[0]);
    ScreenToClient(*this, &rcProgress.pt[0]);
    ScreenToClient(*this, &rcLv.pt[1]);
    ScreenToClient(*this, &rcProgress.pt[1]);

    SetWindowPos(m_lvList, nullptr, 0, 0, rc.rc.right - (rcLv.rc.left << 1), rcLv.rc.bottom - rcLv.rc.top, SWP_NOMOVE);
}

INT_PTR CPageSplashSkinDlg::OnNotify(LPNMHDR lpnm)
{
    return INT_PTR();
}

void CPageSplashSkinDlg::OnItemSelected(LPARAM id)
{

}

void CPageSplashSkinDlg::OnButton(USHORT uId)
{
    if (0x105 == uId)
    {
        ShellExecute(*this, TEXT("open"), TEXT("explorer.exe"), CSplashScreenFactory::AppPath().c_str(), nullptr, SW_SHOW);
        return;
    }

    if (m_bWaiting)
        return;

    switch (uId)
    {
    case 0x100: OnNewSkin(); break;
    case 0x101: OnModSkin(); break;
    case 0x102: OnDelSkin(); break;
    case 0x103: OnUseSkin(); break;
    case 0x104: OnUseDefault(); break;
    }
}

void CPageSplashSkinDlg::OnNewSkin()
{
    auto pSplash = m_hostdata.splash.splash_mgr.Create();
    
    CSkinEditorDlg dlg(GetInstance(), *pSplash);
    
    if (dlg.Show(this))
    {
        try
        {
            m_hostdata.splash.splash_mgr.Save(pSplash);
            m_hostdata.splash.pview.reset();
            UpdateLv();
        }
        catch (std::runtime_error& e)
        {
            ShowErrorF(TEXT("Unable to create skin\n%S"), e.what());
        }
        catch (...)
        {
            ShowError(TEXT("Unable to create skin\nUnknown error"));
        }
    }

    pSplash.reset();
}

void CPageSplashSkinDlg::OnModSkin()
{
    auto idPtr = static_cast<int>(m_lvList.CurSel());
    if (idPtr < 0)
        return;

    auto x = m_lvList.Param(idPtr);
    if (x >= m_hostdata.splash.splash_mgr.end() - m_hostdata.splash.splash_mgr.begin())
        return;

    auto& strName = *(m_hostdata.splash.splash_mgr.begin() + x);

    auto pImage = m_hostdata.splash.splash_mgr[strName];
    if (!pImage)
        return;

    auto pImageCopy = pImage->Copy();
    try
    {
        pImageCopy->Load();
    }
    catch (std::runtime_error& e)
    {
        ShowErrorF(TEXT("Unable to load skin\n%S"), e.what());
        return;
    }
    catch (...)
    {
        ShowError(TEXT("Unable to save skin\nUnknown error"));
        return;
    }

    CSkinEditorDlg dlg(GetInstance(), *pImageCopy, false);
    if (dlg.Show(this))
    {
        pImage->MoveFrom(std::move(*pImageCopy.get()));
        try
        {
            pImage->Save();
            m_hostdata.splash.pview.reset();
        }
        catch (std::runtime_error& e)
        {
            ShowErrorF(TEXT("Unable to save skin\n%S"), e.what());
        }
        catch (...)
        {
            ShowError(TEXT("Unable to save skin\nUnknown error"));
        }
    }
}

void CPageSplashSkinDlg::OnDelSkin()
{
    auto idPtr = static_cast<int>(m_lvList.CurSel());
    if (idPtr < 0)
        return;

    auto x = m_lvList.Param(idPtr);
    if (x >= m_hostdata.splash.splash_mgr.end() - m_hostdata.splash.splash_mgr.begin())
        return;

    tstring strName = *(m_hostdata.splash.splash_mgr.begin() + x);

    if (!ConfirmF(TEXT("Delete skin '%s'?"), false, strName.c_str()))
        return;

    if (!m_hostdata.splash.splash_mgr.Delete(strName))
    {
        Win32Error err;
        ShowErrorF(TEXT("Skin '%s' could not be deleted:\n%s"), strName.c_str(), err.description().c_str());
    }

    m_hostdata.splash.pview.reset();
    UpdateLv();
}

void CPageSplashSkinDlg::OnUseSkin()
{
    auto idPtr = static_cast<int>(m_lvList.CurSel());
    if (idPtr < 0)
        return;

    auto x = m_lvList.Param(idPtr);
    if (x >= m_hostdata.splash.splash_mgr.end() - m_hostdata.splash.splash_mgr.begin())
        return;

    auto& strName = *(m_hostdata.splash.splash_mgr.begin() + x);

    m_hostdata.splash.splash_mgr.SetDefault(m_hostdata.splash.splash_mgr[strName]);
    m_hostdata.splash.pview.reset();
    UpdateLv();
}

void CPageSplashSkinDlg::OnUseDefault()
{
    m_hostdata.splash.splash_mgr.SetDefault(nullptr);
    m_hostdata.splash.pview.reset();
    UpdateLv();
}

void CPageSplashSkinDlg::UpdateLv()
{
    LPARAM x = 0;
    m_lvList.ResetContent();
    m_hostdata.splash.strDefault = m_hostdata.splash.splash_mgr.GetDefaultName();

    for (auto& spl : m_hostdata.splash.splash_mgr)
    {
        auto id = static_cast<int>(m_lvList.AddItem(spl.c_str(), x++));
        if (spl == m_hostdata.splash.strDefault)
        {
            m_lvList.CurSel(id);
            m_lvList.SetItemImage(id, 1);
        }
        else
            m_lvList.SetItemImage(id, 11);
    }

    m_lvList.ScrollToVisible();
}

void CPageSplashSkinDlg::Wait()
{
    m_bWaiting = true;
    MessageLoop();
    m_bWaiting = false;
}

void CPageSplashSkinDlg::ExitLoop()
{
    wSendMessage(WM_CUSTOM_STOP_LOOP);
}

void CPageSplashSkinDlg::StopLoop()
{
    if (m_bWaiting)
        PostQuitMessage(0);
}

INT_PTR CPageSplashSkinDlg::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandle)
{
    fHandle = true;
    switch (uMsg)
    {
    case WM_CUSTOM_STOP_LOOP: StopLoop(); break;
    default: fHandle = false; break;
    }
    return 0;
}


std::map<USHORT, tstring> CPageSplashSkinDlg::ResourceNames()
{
    return
    {
        std::make_pair(IDB_BAT_70, TEXT("Charge protect ON")),
        std::make_pair(IDB_BAT_100, TEXT("Charge protect OFF")),
        std::make_pair(IDB_CAPSLOCK_ON, TEXT("Caps Lock ON")),
        std::make_pair(IDB_CAPSLOCK_OFF, TEXT("Caps Lock OFF")),
        std::make_pair(IDB_FNLOCK_ON, TEXT("FN Lock ON")),
        std::make_pair(IDB_FNLOCK_OFF, TEXT("FN Lock OFF")),
        std::make_pair(IDB_KBHL_0, TEXT("Keyboard Light 0%")),
        std::make_pair(IDB_KBHL_1, TEXT("Keyboard Light 10%")),
        std::make_pair(IDB_KBHL_2, TEXT("Keyboard Light 20%")),
        std::make_pair(IDB_KBHL_3, TEXT("Keyboard Light 30%")),
        std::make_pair(IDB_KBHL_4, TEXT("Keyboard Light 40%")),
        std::make_pair(IDB_KBHL_5, TEXT("Keyboard Light 50%")),
        std::make_pair(IDB_KBHL_6, TEXT("Keyboard Light 60%")),
        std::make_pair(IDB_KBHL_7, TEXT("Keyboard Light 70%")),
        std::make_pair(IDB_KBHL_8, TEXT("Keyboard Light 80%")),
        std::make_pair(IDB_KBHL_9, TEXT("Keyboard Light 90%")),
        std::make_pair(IDB_KBHL_10, TEXT("Keyboard Light 100%")),
        std::make_pair(IDB_KBHL_AUTO, TEXT("Keyboard Light AUTO")),
        std::make_pair(IDB_MIC_ON, TEXT("Microphone ON")),
        std::make_pair(IDB_MIC_OFF, TEXT("Microphone OFF")),
        std::make_pair(IDB_NUMLOCK_ON, TEXT("Num Lock ON")),
        std::make_pair(IDB_NUMLOCK_OFF, TEXT("Num Lock OFF")),
        std::make_pair(IDB_PERFOMANCE_SILENCE, TEXT("Perfomance Silence")),
        std::make_pair(IDB_PERFOMANCE_BALANCE, TEXT("Perfomance Balance")),
        std::make_pair(IDB_PERFOMANCE_TURBO, TEXT("Perfomance Turbo")),
        std::make_pair(IDB_PERFOMANCE_FULLSPEED, TEXT("Perfomance Full Speed")),
        std::make_pair(IDB_PERFOMANCE_AUTO, TEXT("Perfomance Intelligent")),
        std::make_pair(IDB_RATE_48, TEXT("48 Hz")),
        std::make_pair(IDB_RATE_60, TEXT("60 Hz")),
        std::make_pair(IDB_RATE_72, TEXT("72 Hz")),
        std::make_pair(IDB_RATE_75, TEXT("75 Hz")),
        std::make_pair(IDB_RATE_90, TEXT("90 Hz")),
        std::make_pair(IDB_RATE_100, TEXT("100 Hz")),
        std::make_pair(IDB_RATE_120, TEXT("120 Hz")),
        std::make_pair(IDB_RATE_144, TEXT("144 Hz")),
        std::make_pair(IDB_RATE_165, TEXT("165 Hz")),
        std::make_pair(IDB_RATE_240, TEXT("240 Hz")),
        std::make_pair(IDB_RATE_FAIL, TEXT("<FAIL> Hz")),
        std::make_pair(IDB_TOUCHPAD_ON, TEXT("Touchpad ON")),
        std::make_pair(IDB_TOUCHPAD_OFF, TEXT("Touchpad OFF")),
        std::make_pair(IDB_WRONG_CHARGER, TEXT("Wrong Charger"))
    };
}