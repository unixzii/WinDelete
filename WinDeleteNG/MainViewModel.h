#pragma once

#include "MainViewModel.g.h"

namespace winrt::WinDeleteNG::implementation
{
    struct MainViewModel : MainViewModelT<MainViewModel>
    {
        MainViewModel() = default;

        hstring SelectedFileName();
        void SelectedFileName(const hstring& value);
        void SelectedFileName(std::wstring&& value);

        winrt::event_token PropertyChanged(const Microsoft::UI::Xaml::Data::PropertyChangedEventHandler& handler);
        void PropertyChanged(const winrt::event_token& token) noexcept;

    private:
        std::wstring m_fileName;
        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
    };
}

namespace winrt::WinDeleteNG::factory_implementation
{
    struct MainViewModel : MainViewModelT<MainViewModel, implementation::MainViewModel>
    {
    };
}
