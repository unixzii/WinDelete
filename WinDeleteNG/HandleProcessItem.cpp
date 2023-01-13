#include "pch.h"
#include "HandleProcessItem.h"
#if __has_include("HandleProcessItem.g.cpp")
#include "HandleProcessItem.g.cpp"
#endif

namespace winrt::WinDeleteNG::implementation
{
    int32_t HandleProcessItem::Pid()
    {
        return m_pid;
    }

    void HandleProcessItem::Pid(int32_t value)
    {
        m_pid = value;
        m_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ L"Pid" });
    }

    hstring HandleProcessItem::ProcessName()
    {
        return hstring(m_processName);
    }

    void HandleProcessItem::ProcessName(const hstring& value)
    {
        ProcessName(std::wstring(value.c_str()));
    }

    void HandleProcessItem::ProcessName(std::wstring&& value)
    {
        m_processName = std::move(value);
        m_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ L"ProcessName" });
    }

    winrt::Microsoft::UI::Xaml::Media::ImageSource HandleProcessItem::IconImageSource()
    {
        return m_iconImageSource;
    }

    void HandleProcessItem::IconImageSource(winrt::Microsoft::UI::Xaml::Media::ImageSource value)
    {
        m_iconImageSource = value;
        m_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ L"IconImageSource" });
    }

    winrt::event_token HandleProcessItem::PropertyChanged(const Microsoft::UI::Xaml::Data::PropertyChangedEventHandler& handler)
    {
        return m_propertyChanged.add(handler);
    }

    void HandleProcessItem::PropertyChanged(const winrt::event_token& token) noexcept
    {
        m_propertyChanged.remove(token);
    }
}
