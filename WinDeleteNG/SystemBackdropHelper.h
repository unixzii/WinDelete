#pragma once

#include "pch.h"

namespace WinDelete
{
	class SystemBackdropHelper
	{
	public:
		void SetTarget(const winrt::Microsoft::UI::Xaml::IWindow& window)
		{
			m_window = window;
		}

		void EnableMicaBackdrop();

	private:
		static void EnsureWindowsSystemDispatcherQueueController();

		void HandleWindowActivated(
			const winrt::Windows::Foundation::IInspectable& sender,
			const winrt::Microsoft::UI::Xaml::WindowActivatedEventArgs& args);

		void HandleWindowClosed(
			const winrt::Windows::Foundation::IInspectable& sender,
			const winrt::Microsoft::UI::Xaml::WindowEventArgs& args);

		winrt::Microsoft::UI::Xaml::IWindow m_window{ nullptr };

		winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropConfiguration m_conf;
		winrt::Microsoft::UI::Composition::SystemBackdrops::MicaController m_micaController;
	};
}
