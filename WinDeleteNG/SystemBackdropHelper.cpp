#include "pch.h"
#include "SystemBackdropHelper.h"

#include <DispatcherQueue.h>

static PDISPATCHERQUEUECONTROLLER sDispatcherQueueController = nullptr;

namespace WinDelete
{
	void SystemBackdropHelper::EnableMicaBackdrop(const winrt::Microsoft::UI::Xaml::IWindow& window)
	{
		EnsureWindowsSystemDispatcherQueueController();

		if (!winrt::Microsoft::UI::Composition::SystemBackdrops::MicaController::IsSupported())
		{
			return;
		}

		auto conf = winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropConfiguration();
		conf.IsInputActive(true);
		conf.Theme(winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropTheme::Default);

		m_micaController.Kind(winrt::Microsoft::UI::Composition::SystemBackdrops::MicaKind::Base);
		m_micaController.AddSystemBackdropTarget(window.try_as<winrt::Microsoft::UI::Composition::ICompositionSupportsSystemBackdrop>());
		m_micaController.SetSystemBackdropConfiguration(conf);
	}

	void SystemBackdropHelper::EnsureWindowsSystemDispatcherQueueController()
	{
		auto current = winrt::Windows::System::DispatcherQueue::GetForCurrentThread();
		if (current)
		{
			return;
		}

		if (sDispatcherQueueController)
		{
			return;
		}

		DispatcherQueueOptions options;
		options.dwSize = sizeof(options);
		options.threadType = DQTYPE_THREAD_CURRENT;
		options.apartmentType = DQTAT_COM_STA;

		CreateDispatcherQueueController(options, &sDispatcherQueueController);
	}
}
