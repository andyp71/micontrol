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

#include <Splash/splash.h>

typedef struct _tagHOST_DATA
{
    const CEventStatus& status;
    struct
    {
        CSplashScreenFactory& splash_mgr;
        tstring               strDefault;
        CSplashScreenPtr      pview;
    } splash;
} HOST_DATA, * PHOST_DATA;

class CPageDlg;

using CPageDlgPtr = std::shared_ptr<CPageDlg>;
using SendCommand_t = std::function<bool(uint8_t type, uint8_t cmd, uint8_t arg)>;

class CPageDlg : public CDialog
{
    SendCommand_t				m_fnSendCommand;
    std::atomic_bool			m_bStatus;

    void						OnSize(INT dw, USHORT x, USHORT y) override;
    void						OnSizing(INT dw, USHORT x, USHORT y) override;
    std::vector<CControl*>		m_vAutoSizeCtl;
    VOID						OnCancel() override;
    BOOL						OnClose() override;
    VOID						OnTimer(DWORD_PTR idTimer, FARPROC pTimerProc) override;
    BOOL						OnInit(LPARAM lParam) override;
    virtual BOOL				OnPageInit(LPARAM lParam);

protected:
    HOST_DATA&			        m_hostdata;
    const CEventStatus&         m_status;

    CPageDlg(HINSTANCE hInstance, SendCommand_t fnSendCommand, HOST_DATA& data);
    CBitmap& MergeBitmap(CBitmap& bmTarget, const std::vector<UINT>& bmList, bool fConvert = true);
    virtual			void OnSizeChanged(INT, USHORT, USHORT);
    virtual			void OnServiceChange(bool bState);
    void			AutosizeCtl(CControl& ctl);
    bool			SendCommand(uint8_t type, uint8_t cmd, uint8_t arg);
    bool			SendCommandPut(uint8_t command, uint8_t arg);
    bool			SendCommandGet(uint8_t command, uint8_t arg);

public:
    virtual void	OnEventDetail(uint8_t cmd, uint8_t arg);
    virtual void	OnEventCommon(uint8_t cmd, const std::vector<uint8_t>& data);
    virtual void	OnShow();
    virtual bool	GetLineColor(COLORREF& clr, UINT& iconId) const;
    virtual bool    IsCanQuit() const;
};