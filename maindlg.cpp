#include <Windows.h>
#include <Psapi.h>
#include <CommCtrl.h>
#include <memory>
#include <unordered_set>

#include "maindlg.h"
#include "ntutils.h"
#include "global.h"
#include "resource.h"

static void EnableVistaThemeForControls(HWND hwnds[])
{
    HMODULE module_handle = LoadLibrary(L"uxtheme.dll");
    if (!module_handle)
    {
        return;
    }

    auto fn_addr = GetProcAddress(module_handle, "SetWindowTheme");
    if (!fn_addr)
    {
        return;
    }

    typedef int(*SetWindowThemeProc)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
    HWND* hwnd = hwnds;
    while (*hwnd)
    {
        ((SetWindowThemeProc)fn_addr)(*hwnd, L"explorer", NULL);
        ++hwnd;
    }
}

static HBITMAP CreateBitmapFromIcon(HICON icon, int width, int height)
{
    HDC screen_hdc = GetDC(NULL);
    HDC hdc = CreateCompatibleDC(screen_hdc);

    BITMAPINFO bmi = {};
    auto& bih = bmi.bmiHeader;
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = width;
    bih.biHeight = height;
    bih.biPlanes = 1;
    bih.biBitCount = 32;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = width * height * 4;

    DWORD* bits = NULL;
    HBITMAP bitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (VOID**)&bits, NULL, 0);
    if (!bitmap)
    {
        DeleteDC(hdc);
        ReleaseDC(NULL, screen_hdc);
        return NULL;
    }

    HBITMAP oldBitmap = (HBITMAP)SelectObject(hdc, bitmap);
    DrawIconEx(hdc, 0, 0, icon, width, height, 0, NULL, DI_NORMAL);
    
    SelectObject(hdc, oldBitmap);
    DeleteDC(hdc);
    ReleaseDC(NULL, screen_hdc);

    return bitmap;
}

static void UpdateButtons(HWND hwnd)
{
    WORD file_path_len = (WORD)SendDlgItemMessage(hwnd, IDC_FILE_PATH_EDIT, EM_LINELENGTH, 0, 0);
    BOOL enabled = file_path_len > 0;

    HWND inspect_button = GetDlgItem(hwnd, IDC_INSPECT);
    EnableWindow(inspect_button, enabled);

    HWND unlock_button = GetDlgItem(hwnd, IDC_UNLOCK);
    EnableWindow(unlock_button, enabled);
}

static void HandleSysLinkNotifications(HWND hwnd, LPARAM l_param)
{
    switch (((LPNMHDR)l_param)->code)
    {
    case NM_CLICK:
    {
        PNMLINK nm_link = (PNMLINK)l_param;
        LITEM item = nm_link->item;

        if (item.iLink == 0)
        {
            ShellExecute(NULL, L"open", item.szUrl, NULL, NULL, SW_SHOW);
        }
        break;
    }
    }
}

static void HandleEditNotifications(HWND hwnd, WPARAM w_param)
{
    int notification = HIWORD(w_param);
    if (notification == EN_UPDATE)
    {
        UpdateButtons(hwnd);
    }
}

static std::wstring GetEditValue(HWND hwnd)
{
    WORD len = (WORD)SendDlgItemMessage(hwnd, IDC_FILE_PATH_EDIT, EM_LINELENGTH, 0, 0);
    std::unique_ptr<WCHAR> buf(new WCHAR[max(len + 1, 16)]);
    *reinterpret_cast<LPWORD>(buf.get()) = len;
    SendDlgItemMessageW(hwnd, IDC_FILE_PATH_EDIT, EM_GETLINE, 0, (LPARAM)buf.get());
    buf.get()[len] = 0;

    return std::wstring(buf.get());
}

static void BrowseFile(HWND hwnd)
{
    WCHAR path_buf[1024] = { 0 };

    OPENFILENAMEW ofn{ 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = path_buf;
    ofn.nMaxFile = sizeof(path_buf);
    ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if (!GetOpenFileNameW(&ofn))
    {
        return;
    }

    HWND edit = GetDlgItem(hwnd, IDC_FILE_PATH_EDIT);
    SetWindowTextW(edit, path_buf);
}

#define OPERATION_PROLOGUE \
    /* Get the file path. */ \
    auto file_path = GetEditValue(hwnd); \
    \
    /* Convert it to the kernel name for matching. */ \
    auto kernel_name = ntutils::FilePathToKernelName(file_path); \
    if (kernel_name.empty()) \
    { \
        MessageBoxW(hwnd, L"The specific file cannot be found.", L"Error", MB_OK | MB_ICONERROR); \
        return; \
    }

static void DoInspect(HWND hwnd)
{
    OPERATION_PROLOGUE;

    struct ProcessItem
    {
        std::wstring process_name;
        HBITMAP process_icon{ NULL };
    };

    // Find and collect the opened file handles.
    std::unordered_set<std::wstring> added_proc_paths;
    std::vector<ProcessItem> proc_items;
    ntutils::FindOpenedFileHandles(kernel_name, [&](HANDLE proc_handle, HANDLE file_handle)
        {
            WCHAR buf[MAX_PATH + 1];
            DWORD buf_len = GetModuleFileNameExW(proc_handle, 0, buf, MAX_PATH);
#pragma warning( push )
#pragma warning( disable : 6386 )
            buf[buf_len] = 0;
#pragma warning( pop )
            std::wstring proc_path(buf, buf_len);

            auto iter = added_proc_paths.find(proc_path);
            if (iter != added_proc_paths.end())
            {
                return;
            }

            ProcessItem item;

            // Get the process name.
            buf_len = GetModuleBaseNameW(proc_handle, 0, buf, MAX_PATH);
            item.process_name.assign(buf, buf_len);

            // Get the module file icon.
            WORD icon = 0;
            HICON file_icon = ExtractAssociatedIconW(WdInstanceHandle, const_cast<LPWSTR>(proc_path.c_str()), &icon);
            item.process_icon = CreateBitmapFromIcon(file_icon, 32, 32);
            DestroyIcon(file_icon);

            added_proc_paths.insert(proc_path);
            proc_items.emplace_back(std::move(item));
        });

    // Create an icon image list and update the process tree view.
    HIMAGELIST himl = ImageList_Create(32, 32, ILC_COLOR32, (int)proc_items.size(), 0);
    for (const auto& item : proc_items)
    {
        ImageList_Add(himl, item.process_icon, NULL);
    }

    HWND tree_view = GetDlgItem(hwnd, IDC_PROC_TREE);
    TreeView_DeleteAllItems(tree_view);
    HIMAGELIST old_himl = TreeView_GetImageList(tree_view, TVSIL_NORMAL);
    if (old_himl)
    {
        ImageList_Destroy(old_himl);
    }
    TreeView_SetImageList(tree_view, himl, TVSIL_NORMAL);
    int image_idx = 0;
    for (const auto& item : proc_items)
    {
        TVINSERTSTRUCTW tvis;
        tvis.hParent = TVI_ROOT;
        tvis.hInsertAfter = TVI_LAST;
        tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        tvis.item.pszText = const_cast<LPWSTR>(item.process_name.c_str());
        tvis.item.cchTextMax = (int)item.process_name.size();
        tvis.item.iImage = image_idx++;
        tvis.item.iSelectedImage = tvis.item.iImage;
        TreeView_InsertItem(tree_view, &tvis);
    }

    // Destory icon bitmaps.
    for (const auto& item : proc_items)
    {
        DeleteObject(item.process_icon);
    }
}

static void DoUnlock(HWND hwnd)
{
    OPERATION_PROLOGUE;

    int result =
    MessageBoxW(hwnd, L"Are you sure to force unlocking the file? This may cause programs that are using this file unstable or crash.", L"Confirm", MB_YESNO | MB_ICONWARNING);

    if (result != IDYES)
    {
        return;
    }

    ntutils::FindOpenedFileHandles(kernel_name, [&](HANDLE proc_handle, HANDLE file_handle)
        {
            ntutils::CloseHandle(proc_handle, file_handle);
        });

    // Check again.
    DoInspect(hwnd);
}

static INT_PTR CALLBACK MainDialogProc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
    UNREFERENCED_PARAMETER(l_param);

    switch (message)
    {
    case WM_INITDIALOG:
    {
        // Hack to enable modern theme for the tree view.
        HWND controls[2] =
        {
            GetDlgItem(hwnd, IDC_PROC_TREE),
            NULL
        };
        EnableVistaThemeForControls(controls);

        UpdateButtons(hwnd);
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
    {
        int btn_id = LOWORD(w_param);
        switch (btn_id)
        {
        case IDC_FILE_PATH_EDIT:
            HandleEditNotifications(hwnd, w_param);
            break;
        case IDC_BROWSE:
            BrowseFile(hwnd);
            break;
        case IDC_INSPECT:
            DoInspect(hwnd);
            break;
        case IDC_UNLOCK:
            DoUnlock(hwnd);
            break;
        case IDCANCEL:
            EndDialog(hwnd, 0);
            PostQuitMessage(0);
            break;
        }
        return (INT_PTR)TRUE;
    }
    case WM_NOTIFY:
    {
        auto hdr = (LPNMHDR)l_param;
        if (hdr->idFrom == IDC_GITHUB_SYSLINK)
        {
            HandleSysLinkNotifications(hwnd, l_param);
        }
        break;
    }
    }
    return (INT_PTR)FALSE;
}


extern HWND CreateMainDialog()
{
    HWND hwnd = CreateDialogW(WdInstanceHandle, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDialogProc);
    
    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);

    RECT window_rect;
    GetWindowRect(hwnd, &window_rect);
    int window_width = window_rect.right - window_rect.left;
    int window_height = window_rect.bottom - window_rect.top;
    window_rect.left = (screen_width - window_width) / 2;
    window_rect.top = (screen_height - window_height) / 2;
    window_rect.right = window_rect.left + window_width;
    window_rect.bottom = window_rect.top + window_height;

    SetWindowPos(hwnd, NULL, window_rect.left, window_rect.top, window_width, window_height, 0);

    return hwnd;
}