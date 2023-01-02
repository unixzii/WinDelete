#include <windows.h>

#include "global.h"
#include "maindlg.h"
#include "resource.h"

static int WdMessageLoop(void);

int APIENTRY wWinMain(
    _In_ HINSTANCE instance,
    _In_opt_ HINSTANCE prev_instance,
    _In_ LPWSTR cmd_line,
    _In_ int cmd_show)
{
    UNREFERENCED_PARAMETER(prev_instance);
    UNREFERENCED_PARAMETER(cmd_line);

    WdInstanceHandle = instance;

    HWND main_window = CreateMainDialog();
    ShowWindow(main_window, cmd_show);

    return WdMessageLoop();
}

static int WdMessageLoop(void)
{
    HACCEL hAccelTable = LoadAccelerators(WdInstanceHandle, MAKEINTRESOURCE(IDC_WINDELETE));
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}
