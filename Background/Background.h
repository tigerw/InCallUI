#pragma once

#include "pch.h"

namespace Background
{
	using namespace winrt;
	using namespace Windows::Foundation;
	using namespace Windows::ApplicationModel::Background;
	using namespace Windows::ApplicationModel::Calls::Background;

	static const auto BackgroundTaskName = L"Phone";

	IAsyncAction RegisterBackgroundTask()
	{
		co_await BackgroundExecutionManager::RequestAccessAsync();

		for (const auto & Task : BackgroundTaskRegistration::AllTasks())
		{
			if (Task.Value().Name() == BackgroundTaskName)
			{
				return;
			}
		}

		BackgroundTaskBuilder Task;
		Task.Name(BackgroundTaskName);
		Task.TaskEntryPoint(L"Background.Listener");
		Task.SetTrigger(PhoneTrigger(PhoneTriggerType::LineChanged, false));
		Task.Register();
	}
}