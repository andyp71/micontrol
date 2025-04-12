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

constexpr auto SPLASH_POS_COUNT_X = 5;
constexpr auto SPLASH_POS_COUNT_Y = 5;
constexpr auto SPLASH_POS_WIDTH = 1000;
constexpr auto SPLASH_POS_HEIGHT = 1000;
constexpr auto SPLASH_POS_MARGIN_X = 5;
constexpr auto SPLASH_POS_MARGIN_Y = 10;
constexpr auto SPLASH_POS_STEP_X = (SPLASH_POS_WIDTH - 2 * SPLASH_POS_MARGIN_X) / SPLASH_POS_COUNT_X;
constexpr auto SPLASH_POS_STEP_Y = (SPLASH_POS_HEIGHT - 2 * SPLASH_POS_MARGIN_Y) / SPLASH_POS_COUNT_Y;
constexpr UINT WM_CUSTOM_STOP_LOOP = (WM_USER + 0x5513);

class CSkinResourcePosDlg : public CDialog
{
    POINT           m_pt = {};
    CFrameControl   m_fr;
    CRadioControl   m_rdPos[SPLASH_POS_COUNT_X][SPLASH_POS_COUNT_Y];
    
    BOOL            OnInit(LPARAM lParam) override;
    void            OnButton(USHORT uId) override;
    void            OnOK();

public:
    CSkinResourcePosDlg(HINSTANCE hInstance, POINT pt);
    POINT           QueryPt() const;
};

class CSkinImagesListDlg : public CDialog
{
    CSplashScreenView&      m_view;
    CFrameControl           m_frBmp;
    CListView               m_lvBmp;
    CButtonControl          m_btnAdd, m_btnDel;

    BOOL        OnInit(LPARAM lParam) override;
    void        OnAdd();
    void        OnDel();
    void        OnButton(USHORT uId) override;
    INT_PTR     OnNotify(LPNMHDR lpnm) override;
    void        UpdateLv();

public:
    CSkinImagesListDlg(HINSTANCE hInstance, CSplashScreenView& view);

};

class CSkinResourceEditorDlg : public CDialog
{
    SPLASH_ID               m_id;
    const SplashBmpArray&   m_bmArray;
    CStaticControl          m_stResId;
    CControl                m_cbResId;
    CCheckBoxControl        m_chkAlphaSrc;
    CCheckBoxControl        m_chkDPI;
    CStaticControl          m_stTimer;
    CControl                m_cbTimer;
    CStaticControl          m_stAlpha;
    CEditControl            m_edAlpha;
    CUpDownControl          m_udAlpha;
    CStaticControl          m_stBmp;
    CListView               m_lvRes;
    CButtonControl          m_btnPreview;
    CButtonControl          m_btnPosition;

    std::unique_ptr<CSplashScreen>  m_wndSplash;

    BOOL                    OnInit(LPARAM lParam) override;
    void                    OnOK() override;
    void                    OnButton(USHORT uId) override;
    void                    OnCommand(USHORT uCmd, USHORT uId, HWND hCtl) override;
    INT_PTR                 OnNotify(LPNMHDR lpnm) override;

public:
    CSkinResourceEditorDlg(HINSTANCE hInstance, SPLASH_ID spl, const SplashBmpArray& bmArray, bool bCanChangeResId);
    SPLASH_ID               QueryId() const;
};

class CSkinEditorDlg : public CDialog
{
    CSplashScreenView&      m_view;
    CStaticControl          m_stName;
    CEditControl            m_edName;
    CListView               m_lvRes;
    CButtonControl          m_btnAdd, m_btnMod, m_btnDel, m_btnAddBitmap;
    CControl                m_stLine;
    const bool              m_bCanRename;

    BOOL                    OnInit(LPARAM lParam) override;
    void                    OnCommand(USHORT uCmd, USHORT uId, HWND hCtl) override;
    void                    OnButton(USHORT uId) override;
    void                    OnOK() override;
    void                    UpdateLv(USHORT uId);

    void                    OnNewResource();
    void                    OnModResource();
    void                    OnDelResource();
    void                    OnAddBitmaps();

public:
    CSkinEditorDlg(HINSTANCE hInstance, CSplashScreenView& view, bool bCanRename = true);
};

class CPageSplashSkinDlg : public CPageDlg
{
    CListView				m_lvList;
    CButtonControl          m_btnAdd, m_btnMod, m_btnDel;
    CButtonControl          m_btnSetDefault, m_btnUnset;
    CButtonControl          m_btnOpenFolder;

    BOOL			OnPageInit(LPARAM lParam) override;
    void			OnShow() override;
    void			OnSizeChanged(INT dw, USHORT x, USHORT y) override;

    INT_PTR			OnNotify(LPNMHDR lpnm) override;
    void			OnItemSelected(LPARAM id);
    void            OnButton(USHORT uId) override;

    void            OnNewSkin();
    void            OnModSkin();
    void            OnDelSkin();
    void            OnUseSkin();
    void            OnUseDefault();

    void            UpdateLv();

    INT_PTR         OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandle) override;

    std::atomic_bool    m_bWaiting = false;
    void                ExitLoop();
    void                Wait();
    void                StopLoop();

public:
    CPageSplashSkinDlg(HINSTANCE hInstance, SendCommand_t fn, HOST_DATA& status);
    static  std::map<USHORT, tstring> ResourceNames();
};