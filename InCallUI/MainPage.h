#pragma once

#include "MainPage.g.h"

namespace winrt::InCallUI::implementation
{
	struct MainPage : MainPageT<MainPage>
	{
		MainPage();

		/*void OnPageLoaded(const Windows::Foundation::IInspectable &, const Windows::UI::Xaml::RoutedEventArgs &);
		void ClickHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);*/
	};
}

namespace winrt::InCallUI::factory_implementation
{
	struct MainPage : MainPageT<MainPage, implementation::MainPage>
	{
	};
}