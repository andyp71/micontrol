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

#define WM_CUSTOM_SHOW_SPLASH		(WM_USER+120)
#define WM_CUSTOM_TRAY				(WM_USER+121)

#define MI_CONTROL_MUTEX			TEXT("WM_MiControlHandler")

#include <Splash/splash.h>

using HMUTEX = CAutoHandle<HANDLE, nullptr, CloseHandle>;

template <class PAGE>
    concept PAGE_D = std::derived_from<PAGE, CPageDlg>;

class CHostDlg : public CDialog
{
public:
    enum Page
    {
        pagePerfomance,
        pageMicrophone,
        pageTouchpad,
        pageChargeProtection,
        pageDisplayRate,
        pageMiButton,
        pageICCProfile,
        pageServiceStatus,
        pageSysInfo,
        pageSplashEdit,
        pageAbout
    };

private:
    CListView						m_lvSelector;
    std::map<Page, CPageDlgPtr>		m_mPage;
    std::map<USHORT, std::map<USHORT, std::function<void()>>>				m_cmdEv;
    std::map<USHORT, std::function<void()>>									m_btnEv;
    std::map<uint8_t, std::function<void(uint8_t)>>							m_dtlEv;
    std::map<uint8_t, std::function<void(const std::vector<uint8_t>&)>>		m_getEv;
    std::unique_ptr<CSplashScreen>	m_splash;
    CEventStatus					m_status;
    CSplashScreenFactory            m_spmgr;
    UINT							m_uIconsCount = 0;
    HMUTEX							m_hMutex;
    const UINT                      m_wmActivateMessage = RegisterWindowMessage(MI_CONTROL_MUTEX);
    HOST_DATA                       m_host;

    struct
    {
        std::unique_ptr<CEventListener>	listener;
        bool			fPipeAvailable = false;
        size_t			counter = 0;
    } m_pipe;

    template <PAGE_D PAGE>
    CPageDlgPtr		Add(Page pageId, const tstring& strCaption)
    {
        auto it = m_mPage.find(pageId);
        if (it != m_mPage.end())
            it->second.reset();

        auto page = std::make_shared<PAGE>(
            GetInstance(), 
            std::bind(&CHostDlg::SendCommand, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
            m_host);
        m_mPage[pageId] = page;

        m_lvSelector.SetItemImage(
            static_cast<int>(m_lvSelector.AddItem(strCaption.c_str(), pageId)),
            pageId);
        return page;
    }

    CBitmap&	MergeIcons(CBitmap& dest, std::initializer_list<UINT> icons);
    BOOL		OnInit(LPARAM lParam) override;
    INT_PTR		OnNotify(LPNMHDR) override;
    VOID		OnTimer(DWORD_PTR idTimer, FARPROC pTimerProc) override;
    LRESULT		OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandle) override;
    BOOL		OnClose() override;
    VOID		OnCancel() override;
    void		DoClose();

    void		UpdateLvIcons();
    INT_PTR		OnSelectorCustomDraw(LPNMLVCUSTOMDRAW lpnm);
    void		OnPipeStartStop(bool);
    VOID		OnEventDetail(const CEventHandler<EventDetail>& ev);
    VOID		OnCommonRequestByte(const CEventHandler<CommonRequestByte>& ev);
    VOID		OnEventWorkload(uint8_t arg);
    VOID		OnEventMiclock(uint8_t arg);
    VOID		OnWorkloadModeGet(const std::vector<uint8_t>& data);
    VOID		OnPowerModeGet(const std::vector<uint8_t>& data);
    VOID		OnCameraModeGet(const std::vector<uint8_t>& data);
    VOID		OnMicLockGet(const std::vector<uint8_t>& data);

    VOID		Broadcast_EventDetail(uint8_t cmd, uint8_t arg);
    VOID		Broadcast_Common(uint8_t cmd, const std::vector<uint8_t>& data);

    void		InitPages();
    void		SelectPage(Page pageId);
    bool		SendCommand(uint8_t type, uint8_t command, uint8_t arg);
    bool		SendCommandPut(uint8_t command, uint8_t arg);
    bool		SendCommandGet(uint8_t command, uint8_t arg);

    void		OnSpecialButtonPressed(uint8_t cmd, uint8_t arg);

    UINT		OnScreenSettings();
    UINT		OnScreenCut();
    UINT		OnSystemSettings();
    UINT		OnMiButton(BOOL fReleased);
    UINT		OnDisplateRateChange();
    UINT		OnTouchpadChange();
    UINT		OnUnknownButton(uint8_t, uint8_t);

    void        OnMiButtonDown();
    void        OnMiButtonUp();
    void        OnSplashShow(UINT res);
    void        OnUpdateEvent();
    void        OnTrayLBtn();
    void        OnTrayRBtn();
    void        OnWmMyMessage(UINT pid);
    void        OnPeriodic();



public:
    CHostDlg(HINSTANCE hInstance);
};