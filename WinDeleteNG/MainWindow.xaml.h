#pragma once

#include "MainWindow.g.h"
#include "SystemBackdropHelper.h"
#include "MainViewModel.h"

#include <string>

namespace winrt::WinDeleteNG::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        WinDeleteNG::MainViewModel ViewModel();

        void AboutButton_Click(const IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& args);
        void SelectFileButton_Click(const IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& args);
        winrt::Windows::Foundation::IAsyncAction
        InspectButton_Click(const IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& args);

    private:
        WinDelete::SystemBackdropHelper m_backdropHelper;
        WinDeleteNG::MainViewModel m_viewModel{ nullptr };
    };
}

namespace winrt::WinDeleteNG::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
