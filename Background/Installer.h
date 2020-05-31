#pragma once

#include "pch.h"
#include "Installer.g.h"

namespace winrt::Background::implementation
{
	using namespace winrt;

    struct Installer : InstallerT<Installer>
	{
	public:
		Windows::Foundation::IAsyncAction Run(const Windows::ApplicationModel::Background::IBackgroundTaskInstance &) const;
    };
}

namespace winrt::Background::factory_implementation
{
    struct Installer : InstallerT<Installer, implementation::Installer>
    {
    };
}