#include "pch.h"
#include "MainPage.h"
#include "MainPage.g.cpp"
#include "../Background/Background.h"

using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::Foundation;

namespace winrt::InCallUI::implementation
{
	MainPage::MainPage()
	{
		InitializeComponent();
		Background::RegisterBackgroundTask();
	}

	/*void MainPage::OnPageLoaded(const IInspectable &, const RoutedEventArgs &)
	{
		Button().Content(winrt::box_value(L"Test"));
	}

	void MainPage::ClickHandler(IInspectable const&, RoutedEventArgs const&)
	{
	}*/
}