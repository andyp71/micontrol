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

INT_PTR CHostDlg::OnNotify(LPNMHDR lpnm)
{
    switch (lpnm->idFrom)
    {
    case 0x13:
    {
        switch (lpnm->code)
        {
        case LVN_ITEMCHANGED:
        {
            auto sel = m_lvSelector.CurSel();
            if (sel >= 0)
                SelectPage(static_cast<CHostDlg::Page>(m_lvSelector.Param(static_cast<INT>(sel))));

            break;
        }
        case NM_CUSTOMDRAW:
        {
            return OnSelectorCustomDraw((LPNMLVCUSTOMDRAW)lpnm);
        }
        }
        break;
    }
    }
    return INT_PTR();
}

INT_PTR CHostDlg::OnSelectorCustomDraw(LPNMLVCUSTOMDRAW lpnm)
{
    switch (lpnm->nmcd.dwDrawStage)
    {
    case CDDS_PREPAINT: return CDRF_NOTIFYITEMDRAW;
    case CDDS_ITEMPREPAINT:
    {
        return CDRF_NOTIFYSUBITEMDRAW;
    }
    case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
    {
        auto it = m_mPage.find(static_cast<Page>(lpnm->nmcd.lItemlParam));
        if (it == m_mPage.end())
            return 0;

        auto& dlg = it->second;
        COLORREF clr = CLR_INVALID;
        UINT iconId;
        if (!dlg->GetLineColor(clr, iconId))
            return 0;

        if (clr != CLR_INVALID)
            lpnm->clrText = clr;

        break;
    }
    }

    return 0;
}
