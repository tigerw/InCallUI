#include "pch.h"
#include "Installer.h"
#include "Background.h"

namespace winrt::Background::implementation
{
	using namespace Windows::Foundation;
	using namespace Windows::ApplicationModel::Background;

	IAsyncAction Installer::Run(const IBackgroundTaskInstance &) const
	{
		co_await ::Background::RegisterBackgroundTask();
	}
}