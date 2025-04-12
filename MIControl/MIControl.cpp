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

#define MAX_LOADSTRING 100

#include <shellapi.h>
#include <Common.h>

extern "C" __declspec(dllimport) bool InitializeMI(HINSTANCE hInstance);
extern "C" __declspec(dllimport) bool UninitializeMI(HINSTANCE hInstance);

int InstallService()
{
    return InitializeMI(GetModuleHandle(TEXT("MIControlSvc"))) ? 0 : -1;
}

int UninstallService()
{
    CRegKey keyMI, keyMI2;

    if (ERROR_SUCCESS == keyMI.Open(HKEY_CURRENT_USER, TEXT("Software\\MiControl"), DELETE | KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE))
    {
        RegDeleteTree(keyMI, nullptr);
        keyMI.Close();

        if (ERROR_SUCCESS == keyMI2.Open(HKEY_CURRENT_USER, TEXT("Software"), DELETE))
            keyMI2.DeleteSubKey(TEXT("MiControl"));
    }

    return UninitializeMI(GetModuleHandle(TEXT("MIControlSvc"))) ? 0 : -1;
}

int MyWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    ULONG_PTR token;
    Gdiplus::GdiplusStartupInput input;
    Gdiplus::GdiplusStartup(&token, &input, 0);
    WMIService wmi;
    auto dpiDefault = SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    INT_PTR retval = 0;
    // CMainWindow wnd(hInstance);
    // CMiControlDlg wnd(hInstance);
    CHostDlg wnd(hInstance);

    CString s;
    std::ignore = s.LoadString(hInstance, IDS_APP_TITLE, 0);

    switch (nCmdShow)
    {
    case SW_HIDE:
        {
            wnd.Create();
            retval = wnd.MessageLoop();
            break;
        }
    default:
        {
            retval = wnd.Show();
            break;
        }
    }

    SetThreadDpiAwarenessContext(dpiDefault);

    Gdiplus::GdiplusShutdown(token);

    return (INT)retval;
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    std::vector<tstring> vArgs;
    int nArgs = 0;
    auto pszArgs = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if (pszArgs)
    {
        for (int i = 0; i < nArgs; ++i)
            vArgs.push_back(pszArgs[i]);

        LocalFree(pszArgs);
    }

    if (nArgs > 1 && !lstrcmpi(vArgs[1].c_str(), TEXT("/install")))
    {
        return InstallService();
    }
    else if (nArgs > 1 && !lstrcmpi(vArgs[1].c_str(), TEXT("/uninstall")))
    {
        return UninstallService();
    }
    else
    {
        nCmdShow = SW_SHOW;
        if (nArgs > 1 && !lstrcmpi(vArgs[1].c_str(), TEXT("/tray")))
            nCmdShow = SW_HIDE;

        return MyWinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
    }
}

int main(int argc, char** argv)
{
    return _tWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOW);
}
