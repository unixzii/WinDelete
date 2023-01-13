#pragma once

#include "MainViewModel.g.h"
#include "HandleProcessItem.h"

namespace winrt::WinDeleteNG::implementation
{
    struct MainViewModel : MainViewModelT<MainViewModel>
    {
        MainViewModel()
            : m_handleProcessItems(winrt::single_threaded_observable_vector<WinDeleteNG::HandleProcessItem>())
        { }

        hstring SelectedFileName();
        void SelectedFileName(const hstring& value);
        void SelectedFileName(std::wstring&& value);

        winrt::Windows::Foundation::Collections::IObservableVector<WinDeleteNG::HandleProcessItem> HandleProcessItems();

        winrt::event_token PropertyChanged(const Microsoft::UI::Xaml::Data::PropertyChangedEventHandler& handler);
        void PropertyChanged(const winrt::event_token& token) noexcept;

    private:
        std::wstring m_fileName;
        winrt::Windows::Foundation::Collections::IObservableVector<WinDeleteNG::HandleProcessItem> m_handleProcessItems;
        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
    };
}

namespace winrt::WinDeleteNG::factory_implementation
{
    struct MainViewModel : MainViewModelT<MainViewModel, implementation::MainViewModel>
    {
    };
}
