#pragma once

#include "HandleProcessItem.g.h"

namespace winrt::WinDeleteNG::implementation
{
    struct HandleProcessItem : HandleProcessItemT<HandleProcessItem>
    {
        HandleProcessItem() = default;

        int32_t Pid();
        void Pid(int32_t value);

        hstring ProcessName();
        void ProcessName(const hstring& value);
        void ProcessName(std::wstring&& value);

        winrt::Microsoft::UI::Xaml::Media::ImageSource IconImageSource();
        void IconImageSource(winrt::Microsoft::UI::Xaml::Media::ImageSource value);

        winrt::event_token PropertyChanged(const Microsoft::UI::Xaml::Data::PropertyChangedEventHandler& handler);
        void PropertyChanged(const winrt::event_token& token) noexcept;

    private:
        int32_t m_pid{ 0 };
        std::wstring m_processName;
        winrt::Microsoft::UI::Xaml::Media::ImageSource m_iconImageSource{ nullptr };
        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
    };
}

namespace winrt::WinDeleteNG::factory_implementation
{
    struct HandleProcessItem : HandleProcessItemT<HandleProcessItem, implementation::HandleProcessItem>
    {
    };
}
