#pragma once

#include "pch.h"
#include "Listener.g.h"

namespace winrt::Background::implementation
{
	using namespace winrt;

    struct Listener : ListenerT<Listener>
    {
		void Run(const Windows::ApplicationModel::Background::IBackgroundTaskInstance & Instance);

		Windows::ApplicationModel::Background::BackgroundTaskDeferral Deferral = nullptr;
    };
}

namespace winrt::Background::factory_implementation
{
    struct Listener : ListenerT<Listener, implementation::Listener>
    {
    };
}