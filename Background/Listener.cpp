#include "pch.h"
#include "Handler.h"
#include "Listener.h"

namespace winrt::Background::implementation
{
	void Listener::Run(const Windows::ApplicationModel::Background::IBackgroundTaskInstance & Instance)
	{
		static Handler HandlerInstance(Instance);

		if (Deferral == nullptr)
		{
			Deferral = Instance.GetDeferral();
		}
	}
}