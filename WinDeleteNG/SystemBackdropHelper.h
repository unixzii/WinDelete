#pragma once

namespace WinDelete
{
	class SystemBackdropHelper
	{
	public:
		void EnableMicaBackdrop(const winrt::Microsoft::UI::Xaml::IWindow& window);

	private:
		static void EnsureWindowsSystemDispatcherQueueController();

		winrt::Microsoft::UI::Composition::SystemBackdrops::MicaController m_micaController;
	};
}
