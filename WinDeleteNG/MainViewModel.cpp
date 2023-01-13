#include "pch.h"
#include "MainViewModel.h"
#if __has_include("MainViewModel.g.cpp")
#include "MainViewModel.g.cpp"
#endif

namespace winrt::WinDeleteNG::implementation
{
    hstring MainViewModel::SelectedFileName()
    {
        return hstring(m_fileName.c_str());
    }

    void MainViewModel::SelectedFileName(const hstring& value)
    {
        SelectedFileName(std::wstring(value.c_str()));
    }

    void MainViewModel::SelectedFileName(std::wstring&& value)
    {
        m_fileName = std::move(value);
        m_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ L"SelectedFileName" });
    }

    winrt::event_token MainViewModel::PropertyChanged(const Microsoft::UI::Xaml::Data::PropertyChangedEventHandler& handler)
    {
        return m_propertyChanged.add(handler);
    }

    void MainViewModel::PropertyChanged(const winrt::event_token& token) noexcept
    {
        m_propertyChanged.remove(token);
    }
}
