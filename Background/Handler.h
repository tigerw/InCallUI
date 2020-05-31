#pragma once

#include "pch.h"
#include "PhoneOm.h"
#include "Proximity.h"

namespace Call
{
	struct Details
	{
		Details() :
			Acknowledged(false),
			Toasted(false)
		{
		}

		winrt::hstring Identifier;
		bool Acknowledged;
		bool Toasted;
	};
}

class Handler
{
public:
	Handler(const winrt::Windows::ApplicationModel::Background::IBackgroundTaskInstance & Instance);
	~Handler();

private:
	static UINT NotificationHandler(const HPHONELISTENER, DWORD_PTR UserData, const PH_CHANGEEVENT & Event);

	void Initialise();
	void OnPhoneEvent();
	void ProcessPhoneEvent(const PH_CALL_INFO & Call);
	winrt::hstring AddCallToHistory(const PH_CALL_INFO & Call) const;
	void MarkCallConnectedInHistory(const winrt::hstring & Id) const;
	void MarkCallCompletedInHistory(const winrt::hstring & Id, bool Missed) const;

	void OnToastDismissed(const winrt::Windows::UI::Notifications::ToastNotification & Notification, const winrt::Windows::UI::Notifications::ToastDismissedEventArgs & Dismissed) const;

	static const inline HPHONELISTENER INVALID_HPHONELISTENER = reinterpret_cast<HPHONELISTENER>(-1);
	HPHONELISTENER ListenerHandle;
	Proximity Controller;

	std::mutex HandlerMutex;

	winrt::Windows::ApplicationModel::Background::BackgroundTaskDeferral Deferral;
	winrt::Windows::ApplicationModel::Calls::PhoneCallHistoryStore History;
	winrt::Windows::ApplicationModel::Contacts::ContactStore Contacts;
	winrt::Windows::UI::Notifications::ToastNotifier Toaster;
	std::unordered_map<PhoneCallId, Call::Details> CurrentCalls;

	PhoneAPIInitialize * PhoneAPIInitialize_;
	PhoneWaitForAPIReady * PhoneWaitForAPIReady_;
	PhoneGetState * PhoneGetState_;
	PhoneEnd * PhoneEnd_;
	PhoneFreeCallInfo * PhoneFreeCallInfo_;
	PhoneAddListener * PhoneAddListener_;
	PhoneClearIdleCallsFromController * PhoneClearIdleCallsFromController_;
	PhoneRemoveListener * PhoneRemoveListener_;
	PhoneAPIUninitialize * PhoneAPIUninitialize_;

	template<typename FunctionType>
	static FunctionType * GetFunctionAddress(HMODULE StartAddress, LPCSTR ExportedName);

	template<typename FunctionType, typename ...ArgumentTypes>
	static void CheckedAPICall(FunctionType Function, ArgumentTypes ...Arguments);
};