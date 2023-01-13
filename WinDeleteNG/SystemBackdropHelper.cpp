#include "pch.h"
#include "SystemBackdropHelper.h"

#include <DispatcherQueue.h>

static PDISPATCHERQUEUECONTROLLER sDispatcherQueueController = nullptr;

namespace WinDelete
{
	void SystemBackdropHelper::EnableMicaBackdrop()
	{
		EnsureWindowsSystemDispatcherQueueController();

		if (!winrt::Microsoft::UI::Composition::SystemBackdrops::MicaController::IsSupported())
		{
			return;
		}

		m_conf.IsInputActive(true);
		m_conf.Theme(winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropTheme::Default);
		m_window.Activated({ this, &SystemBackdropHelper::HandleWindowActivated });
		m_window.Closed({ this, &SystemBackdropHelper::HandleWindowClosed });

		m_micaController.Kind(winrt::Microsoft::UI::Composition::SystemBackdrops::MicaKind::Base);
		m_micaController.AddSystemBackdropTarget(m_window.try_as<winrt::Microsoft::UI::Composition::ICompositionSupportsSystemBackdrop>());
		m_micaController.SetSystemBackdropConfiguration(m_conf);
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

	void SystemBackdropHelper::HandleWindowActivated(
		const winrt::Windows::Foundation::IInspectable& sender,
		const winrt::Microsoft::UI::Xaml::WindowActivatedEventArgs& args)
	{
		UNREFERENCED_PARAMETER(sender);
		UNREFERENCED_PARAMETER(args);

		m_conf.IsInputActive(args.WindowActivationState() != winrt::Microsoft::UI::Xaml::WindowActivationState::Deactivated);
	}

	void SystemBackdropHelper::HandleWindowClosed(
		const winrt::Windows::Foundation::IInspectable& sender,
		const winrt::Microsoft::UI::Xaml::WindowEventArgs& args)
	{
		UNREFERENCED_PARAMETER(sender);
		UNREFERENCED_PARAMETER(args);

		if (m_micaController)
		{
			m_micaController.Close();
			m_micaController = nullptr;
		}
	}
}
