#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include "SystemBackdropHelper.h"
#include "AboutDialogContent.xaml.h"
#include "HandleProcessItem.h"
#include "../ntutils.h"
#include "../global.h"

#include <CommCtrl.h>
#include <Psapi.h>
#include <shcore.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace WinDelete;

static const int kDefaultDpi = 96;

static LRESULT CALLBACK 
MainWindowSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(uIdSubclass);
    UNREFERENCED_PARAMETER(dwRefData);

    if (uMsg == WM_GETMINMAXINFO)
    {
        int dpi = GetDpiForWindow(hWnd);

        MINMAXINFO* mmi = (MINMAXINFO*) lParam;
        mmi->ptMinTrackSize.x = MulDiv(460, dpi, kDefaultDpi);
        mmi->ptMinTrackSize.y = MulDiv(300, dpi, kDefaultDpi);
        return 0;
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

static winrt::Windows::Foundation::IAsyncOperation<winrt::Microsoft::UI::Xaml::Media::ImageSource>
CreateImageSourceFromHICONAsync(HICON icon, int width, int height)
{
    winrt::Microsoft::UI::Xaml::Media::Imaging::SoftwareBitmapSource bitmapSource;

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
        co_return bitmapSource;
    }

    HBITMAP oldBitmap = (HBITMAP)SelectObject(hdc, bitmap);
    DrawIconEx(hdc, 0, 0, icon, width, height, 0, NULL, DI_NORMAL);

    winrt::Windows::Graphics::Imaging::SoftwareBitmap softwareBitmap(
        winrt::Windows::Graphics::Imaging::BitmapPixelFormat::Bgra8,
        width, height,
        winrt::Windows::Graphics::Imaging::BitmapAlphaMode::Premultiplied);
    auto buffer = softwareBitmap.LockBuffer(winrt::Windows::Graphics::Imaging::BitmapBufferAccessMode::Write);
    auto bufferRef = buffer.CreateReference();
    DWORD* bufferData = (DWORD*)bufferRef.data();

    for (int scanline = height - 1; scanline >= 0; --scanline)
    {
        DWORD* scanlineBits = &bits[(height - scanline - 1) * width];
        DWORD* scanlineData = &bufferData[scanline * width];
        for (int x = 0; x < width; ++x)
        {
            scanlineData[x] = scanlineBits[x];
        }
    }

    bufferRef.Close();
    buffer.Close();
    co_await bitmapSource.SetBitmapAsync(softwareBitmap);

    SelectObject(hdc, oldBitmap);
    DeleteObject(bitmap);
    DeleteDC(hdc);
    ReleaseDC(NULL, screen_hdc);

    co_return bitmapSource;
}

namespace winrt::WinDeleteNG::implementation
{
    MainWindow::MainWindow()
    {
        m_viewModel = winrt::make<MainViewModel>();

        InitializeComponent();

        Title(L"WinDelete");
        ExtendsContentIntoTitleBar(true);
        SetTitleBar(TitleBarDragView());

        m_backdropHelper.SetTarget(this->try_as<Microsoft::UI::Xaml::IWindow>());
        m_backdropHelper.EnableMicaBackdrop();

        HandleProcessListView().ItemsSource(ViewModel().HandleProcessItems());

        // Limit the minimum size of the window.
        HWND hwnd;
        try_as<::IWindowNative>()->get_WindowHandle(&hwnd);
        SetWindowSubclass(hwnd, &MainWindowSubclassProc, 0, 0);
        {
            int dpi = GetDpiForWindow(hwnd);
            RECT windowRect;
            GetWindowRect(hwnd, &windowRect);
            SetWindowPos(hwnd, NULL, windowRect.left, windowRect.top,
                MulDiv(500, dpi, kDefaultDpi), MulDiv(600, dpi, kDefaultDpi), 0);
        }
    }

    WinDeleteNG::MainViewModel MainWindow::ViewModel()
    {
        return m_viewModel;
    }

    void MainWindow::AboutButton_Click(const IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& args)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(args);

        auto defaultDialogStyle = winrt::Microsoft::UI::Xaml::Application::Current()
            .Resources()
            .Lookup(winrt::box_value(L"DefaultContentDialogStyle"));

        auto dialog = winrt::Microsoft::UI::Xaml::Controls::ContentDialog();
        dialog.XamlRoot(this->Content().XamlRoot());
        dialog.Style(defaultDialogStyle.try_as<winrt::Microsoft::UI::Xaml::Style>());
        dialog.Title(winrt::box_value(L"About WinDelete"));
        dialog.CloseButtonText(L"Close");
        dialog.DefaultButton(winrt::Microsoft::UI::Xaml::Controls::ContentDialogButton::Close);
        dialog.Content(make<AboutDialogContent>());

        dialog.ShowAsync();
    }

    void MainWindow::SelectFileButton_Click(const IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& args)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(args);

        WCHAR path_buf[1024] = { 0 };

        OPENFILENAMEW ofn{ 0 };
        ofn.lStructSize = sizeof(ofn);
        try_as<::IWindowNative>()->get_WindowHandle(&ofn.hwndOwner);
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

        ViewModel().SelectedFileName(path_buf);

        VisualStateManager::GoToState(MainPage().try_as<winrt::Microsoft::UI::Xaml::Controls::Control>(), L"FileSelected", true);
    }

#define OPERATION_PROLOGUE \
    /* Get the file path. */ \
    std::wstring file_path = ViewModel().SelectedFileName().c_str(); \
    \
    /* Convert it to the kernel name for matching. */ \
    auto kernel_name = ntutils::FilePathToKernelName(file_path); \
    if (kernel_name.empty()) \
    { \
        return; \
    }

    winrt::Windows::Foundation::IAsyncAction
    MainWindow::InspectButton_Click(const IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& args)
    {
        OPERATION_PROLOGUE;

        struct ProcessItem
        {
            int32_t pid;
            std::wstring processName;
            HICON processIcon;
        };

        // Save context and switch to background thread.
        VisualStateManager::GoToState((winrt::Microsoft::UI::Xaml::Controls::Control)MainPage(), L"Working", true);
        winrt::apartment_context uiThread;
        co_await winrt::resume_background();

        // Find and collect the opened file handles.
        std::unordered_set<std::wstring> added_proc_paths;
        std::vector<ProcessItem> proc_items;
        ntutils::FindOpenedFileHandles(kernel_name, [&](HANDLE proc_handle, HANDLE file_handle)
            {
                UNREFERENCED_PARAMETER(file_handle);

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

                // Get the PID.
                item.pid = GetProcessId(proc_handle);

                // Get the process name.
                buf_len = GetModuleBaseNameW(proc_handle, 0, buf, MAX_PATH);
                item.processName.assign(buf, buf_len);

                // Get the module file icon.
                WORD icon = 0;
                HICON file_icon = ExtractAssociatedIconW(WdInstanceHandle, const_cast<LPWSTR>(proc_path.c_str()), &icon);
                item.processIcon = file_icon;

                added_proc_paths.insert(proc_path);
                proc_items.emplace_back(std::move(item));
            });

        std::sort(proc_items.begin(), proc_items.end(), [](const auto& lhs, const auto& rhs)
            {
                return lhs.pid > rhs.pid;
            });

        // Switch back to UI thread and update.
        co_await uiThread;

        auto items = ViewModel().HandleProcessItems();
        items.Clear();
        for (const auto& proc_item : proc_items)
        {
            WinDeleteNG::HandleProcessItem item = winrt::make<HandleProcessItem>();
            item.Pid(proc_item.pid);
            item.ProcessName(proc_item.processName);
            item.IconImageSource(co_await CreateImageSourceFromHICONAsync(proc_item.processIcon, 64, 64));
            items.Append(item);

            DestroyIcon(proc_item.processIcon);
        }

        VisualStateManager::GoToState(MainPage().try_as<winrt::Microsoft::UI::Xaml::Controls::Control>(), L"Inspected", true);
    }
}
