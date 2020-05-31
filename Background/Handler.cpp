#include "pch.h"
#include "Handler.h"

using namespace winrt;
using namespace Windows::UI::Notifications;
using namespace Windows::ApplicationModel::Calls;
using namespace Windows::ApplicationModel::Contacts;
using namespace Windows::ApplicationModel::Background;
using namespace Windows::ApplicationModel::Calls::Background;

Handler::Handler(const Windows::ApplicationModel::Background::IBackgroundTaskInstance & Instance) :
	ListenerHandle(Handler::INVALID_HPHONELISTENER),
	Deferral(Instance.GetDeferral()),
	History(PhoneCallHistoryManager::RequestStoreAsync(PhoneCallHistoryStoreAccessType::AppEntriesReadWrite).get()),
	Contacts(ContactManager::RequestStoreAsync().get()),
	Toaster(ToastNotificationManager::CreateToastNotifier())
{
	MEMORY_BASIC_INFORMATION Information;

	if (::VirtualQuery(&VirtualQuery, &Information, sizeof(Information)) == 0)
	{
		throw std::system_error(::GetLastError(), std::system_category());
	}

	const auto SyscallBegin = static_cast<HMODULE>(Information.AllocationBase);
	const auto LoadLibrary = GetFunctionAddress<LoadLibraryExW>(SyscallBegin, "LoadLibraryExW");

	const auto PhoneOmAddress = static_cast<HMODULE>(LoadLibrary(L"PhoneOm.dll", nullptr, 0));
	if (PhoneOmAddress == nullptr)
	{
		throw std::system_error(::GetLastError(), std::system_category());
	}

	PhoneAPIInitialize_ = GetFunctionAddress<PhoneAPIInitialize>(PhoneOmAddress, "PhoneAPIInitialize");
	PhoneWaitForAPIReady_ = GetFunctionAddress<PhoneWaitForAPIReady>(PhoneOmAddress, "PhoneWaitForAPIReady");
	PhoneGetState_ = GetFunctionAddress<PhoneGetState>(PhoneOmAddress, "PhoneGetState");
	PhoneEnd_ = GetFunctionAddress<PhoneEnd>(PhoneOmAddress, "PhoneEnd");
	PhoneFreeCallInfo_ = GetFunctionAddress<PhoneFreeCallInfo>(PhoneOmAddress, "PhoneFreeCallInfo");
	PhoneAddListener_ = GetFunctionAddress<PhoneAddListener>(PhoneOmAddress, "PhoneAddListener");
	PhoneClearIdleCallsFromController_ = GetFunctionAddress<PhoneClearIdleCallsFromController>(PhoneOmAddress, "PhoneClearIdleCallsFromController");
	PhoneRemoveListener_ = GetFunctionAddress<PhoneRemoveListener>(PhoneOmAddress, "PhoneRemoveListener");
	PhoneAPIUninitialize_ = GetFunctionAddress<PhoneAPIUninitialize>(PhoneOmAddress, "PhoneAPIUninitialize");

	Initialise();
}

Handler::~Handler()
{
	CheckedAPICall(PhoneRemoveListener_, ListenerHandle);
	CheckedAPICall(PhoneAPIUninitialize_);
}

void Handler::Initialise()
{
	CheckedAPICall(PhoneAPIInitialize_);
	CheckedAPICall(PhoneWaitForAPIReady_);

	const std::array<PH_CHANGEEVENT, 1> Subscribe = { PH_CHANGEEVENT::PhoneStateChanged };
	CheckedAPICall(
		PhoneAddListener_,
		&Handler::NotificationHandler,
		Subscribe.data(), static_cast<UINT>(Subscribe.size()),
		reinterpret_cast<DWORD_PTR>(this), &ListenerHandle
	);

	std::lock_guard<std::mutex> Guard(HandlerMutex);
	OnPhoneEvent();
}

UINT Handler::NotificationHandler(const HPHONELISTENER, DWORD_PTR UserData, const PH_CHANGEEVENT & Event)
{
	Handler * Background = reinterpret_cast<Handler *>(UserData);
	{
		std::lock_guard<std::mutex> Guard(Background->HandlerMutex);
		Background->OnPhoneEvent();
	}
	return 0;
}

void Handler::OnPhoneEvent()
{
	PH_CALL_INFO * Calls = nullptr;
	UINT CallCount;
	PH_PHONE_CALL_COUNTS Counts;

	CheckedAPICall(PhoneGetState_, &Calls, &CallCount, &Counts);
	if (CallCount > 0)
	{
		ProcessPhoneEvent(Calls[0]);
	}
	PhoneFreeCallInfo_(Calls);

	// Activate proximity if there are any active calls
	if ((Counts.ActiveTalkingCalls > 0) || (Counts.DialingCalls > 0))
	{
		Controller.Sense();
	}
	else
	{
		Controller.Stop();
	}

	if ((Counts.DisconnectedCalls > 0) || (Counts.IndeterminateCalls > 0) || (Counts.UpgradingCalls > 0))
	{
		CheckedAPICall(PhoneClearIdleCallsFromController_);
	}
}

void Handler::ProcessPhoneEvent(const PH_CALL_INFO & Call)
{
	if ((Call.State == PH_CALL_STATE::Incoming) || (Call.State == PH_CALL_STATE::Dialing))
	{
		const auto & Emplacement = CurrentCalls.try_emplace(Call.Id);
		if (Emplacement.second)
		{
			// Only add entry if it was our first time
			Emplacement.first->second.Identifier = std::move(AddCallToHistory(Call));
		}

		if (Call.Direction == PH_CALL_DIRECTION::Incoming)
		{
			// System displays incoming call toast for us
			return;
		}
	}

	const auto & CurrentIt = CurrentCalls.find(Call.Id);
	if (CurrentIt == CurrentCalls.cend())
	{
		return;
	}
	auto & Current = CurrentIt->second;

	if (Call.State == PH_CALL_STATE::Disconnected)
	{
		ToastNotificationManager::History().Remove(std::to_wstring(Call.Id));
		MarkCallCompletedInHistory(Current.Identifier, !Current.Acknowledged);
		CurrentCalls.erase(Call.Id);
		return;
	}

	if (!Current.Acknowledged && (Call.State == PH_CALL_STATE::ActiveTalking))
	{
		Current.Acknowledged = true;
		MarkCallConnectedInHistory(Current.Identifier);
	}

	if (!Current.Toasted)
	{
		Windows::Data::Xml::Dom::XmlDocument Content;
		Content.LoadXml(
LR"(<toast scenario="reminder">
	<visual>
		<binding template="ToastGeneric">
			<text>{CallText}</text>
			<text>In progress. Dismiss to hang up.</text>
		</binding>
	</visual>
	<actions>
		<action content="Open calling UI" arguments="" imageUri="Assets\Badge.png" activationType="foreground" />
	</actions>
	<audio silent="true" />
</toast>)");

		ToastNotification Toast(Content);
		Toast.Tag(std::to_wstring(Call.Id));
		{
			NotificationData Data;
			Data.Values().Insert(L"CallText", L"Call with " + std::wstring(Call.Number));
			Toast.Data(Data);
		}
		Toast.Priority(ToastNotificationPriority::High);
		Toast.Dismissed({ this, &Handler::OnToastDismissed });
		Toaster.Show(Toast);

		Current.Toasted = true;
	}
}

winrt::hstring Handler::AddCallToHistory(const PH_CALL_INFO & Call) const
{
	PhoneCallHistoryEntry Entry;
	{
		PhoneCallHistoryEntryAddress Address;
		const auto & MatchedContacts = Contacts.FindContactsAsync(Call.Number).get();
		if (MatchedContacts.Size() >= 1)
		{
			const auto & Contact = MatchedContacts.GetAt(0);
			Address.ContactId(Contact.Id());
			Address.DisplayName(Contact.DisplayName());
		}
		Address.RawAddress(Call.Number);
		Address.RawAddressKind(PhoneCallHistoryEntryRawAddressKind::PhoneNumber);
		Entry.Address(Address);
	}
	Entry.IsIncoming(Call.Direction == PH_CALL_DIRECTION::Incoming);
	Entry.IsRinging(true);
	Entry.Media(PhoneCallHistoryEntryMedia::Audio);
	Entry.OtherAppReadAccess(PhoneCallHistoryEntryOtherAppReadAccess::Full);

	//const auto & id = winrt::to_hstring(PhoneCallManager::RequestStoreAsync().get().GetDefaultLineAsync().get());
	//Entry.SourceId(id);
	//Entry.SourceId(L"{B1776703-738E-437D-B891-44555CEB6669}");
	//Entry.SourceIdKind(PhoneCallHistorySourceIdKind::CellularPhoneLineId);

	// Create an entry for a connecting call, incoming or outgoing
	History.SaveEntryAsync(Entry).get();
	return Entry.Id();
}

void Handler::MarkCallConnectedInHistory(const winrt::hstring & Id) const
{
	const auto & Entry = History.GetEntryAsync(Id).get();
	Entry.IsRinging(false);
	Entry.StartTime(Windows::Foundation::DateTime::clock::now());
	History.SaveEntryAsync(Entry).get();
}

void Handler::MarkCallCompletedInHistory(const winrt::hstring & Id, bool Missed) const
{
	const auto & Entry = History.GetEntryAsync(Id).get();
	Entry.IsRinging(false); // Duplicated here because a missed call is never connected
	Entry.IsMissed(Missed);
	if (Missed)
	{
		// Also appears necessary to be set for Phone to show a missed call notification
		// Makes sense, since it needs to show when the call arrived
		Entry.StartTime(Windows::Foundation::DateTime::clock::now());
	}
	Entry.Duration(Windows::Foundation::TimeSpan(std::chrono::seconds(5)));
	History.SaveEntryAsync(Entry).get();
}

void Handler::OnToastDismissed(const ToastNotification & Notification, const ToastDismissedEventArgs & Dismissed) const
{
	if (Dismissed.Reason() == ToastDismissalReason::TimedOut)
	{
		return;
	}

	CheckedAPICall(PhoneEnd_, std::stoi(std::wstring(Notification.Tag())));
}

template<typename FunctionType>
FunctionType * Handler::GetFunctionAddress(HMODULE StartAddress, LPCSTR ExportedName)
{
	const auto FunctionAddress = reinterpret_cast<FunctionType *>(::GetProcAddress(StartAddress, ExportedName));
	if (FunctionAddress == nullptr)
	{
		throw std::system_error(::GetLastError(), std::system_category());
	}

	return FunctionAddress;
}

template<typename FunctionType, typename ...ArgumentTypes>
inline void Handler::CheckedAPICall(FunctionType Function, ArgumentTypes ...Arguments)
{
	const auto Result = Function(Arguments...);
	if (SUCCEEDED(Result))
	{
		return;
	}

	throw std::system_error(Result, std::system_category());
}