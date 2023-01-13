#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include "SystemBackdropHelper.h"
#include "AboutDialogContent.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace WinDelete;

namespace winrt::WinDeleteNG::implementation
{
    MainWindow::MainWindow()
    {
        m_viewModel = winrt::make<MainViewModel>();

        InitializeComponent();

        Title(L"WinDelete");
        ExtendsContentIntoTitleBar(true);
        SetTitleBar(TitleBarDragView());

        m_backdropHelper.EnableMicaBackdrop(this->try_as<Microsoft::UI::Xaml::IWindow>());
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
}
