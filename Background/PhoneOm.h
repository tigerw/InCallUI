#pragma once

#include "pch.h"

enum class PH_CHANGEEVENT
{
	PhoneStateChanged = 1,
	ActionAvailabilityChanged = 2,
	RecordingStateChanged = 2,
	AudioStateChanged4 = 2,
	AudioStateChanged = 3,
	AudioStateChanged2 = 4,
	AudioStateChanged3 = 5,
	LineRegistrationInfoChanged = 7,
	LineConfigurationChanged = 8,
	LineVoicemailCountChanged = 8,
	LineSignalStrengthChanged = 9,
	LineVoicemailConnectivityChanged = 10,
	Error = 11,
	AlertMessage = 12,
	ModifyLineSettingComplete = 13,
	ModifyLineSettingComplete2 = 14,
	ModifyLineSettingComplete3 = 15,
	LineComponentsChanged = 16,
	LineLockInfoChanged = 17,
	NewLineCreated3 = 22,
	LineConfigurationChanged2 = 22,
	LineRegistrationInfoChanged2 = 22,
	ModifyLineSettingComplete4 = 23,
	NewLineCreated = 24,
	ReleaseLine = 25,
	WifiCallDropWarning = 27,
	LineBluetoothHandsFreeDeviceStateChanged = 28,
	EmergencyCallDialerRequired = 34,
	NewLineCreated2 = -1
};

enum class PH_CALL_STATE
{
	Indeterminate = 0,
	Incoming = 1,
	Dialing = 2,
	ActiveTalking = 3,
	OnHold = 4,
	Disconnected = 5,
	Transferring = 6,
	Count = 7
};

enum class PH_CALL_STATE_REASON
{
	Other = 0,
	Busy = 1,
	Ended = 4,
	NetworkCongestion = 9,
	Dropped = 10,
	CallBarred = 11,
	FDNRestricted = 12,
	ServiceOff = 16,
	RoamRestricted = 24,
	CallUpgradeInitiated = 25,
	VideoCallingOff = 26
};

enum class PH_CALL_DIRECTION
{
	Incoming = 1,
	Outgoing = 2
};

enum class PH_CALL_TRANSPORT
{
	Unknown = 0,
	Cellular = 1,
	VoIP = 2
};

using PhoneCallId = unsigned;

struct PH_CALL_INFO
{
	wchar_t Name[80]; // sizeof(SMS message limit) / sizeof(wchar_t) ??
	wchar_t Number[128];
	wchar_t Details[160];
	wchar_t Unknown_0[64];
	wchar_t ForwardNumber[64];
	wchar_t Unknown_1[80];
	wchar_t Unknown_2[80];
	wchar_t Unknown_3[40];
	wchar_t ContactPicture[260];
	wchar_t Ringtone[266];
	long long Unknown_4;
	int Unknown_5;
	int Unknown_6;
	wchar_t MeContactPicture[260];
	long long StartTime;
	long long EndTime;
	long long Unknown_7;
	long long LastFlashedTime;
	PH_CALL_STATE State;
	PH_CALL_STATE_REASON StateReason;
	PH_CALL_DIRECTION Direction;
	int FindStatus;
	int Unknown_8;
	int Unknown_9;
	PH_CALL_TRANSPORT Transport;
	PhoneCallId Id;
	unsigned ConferenceId;
	int Unknown_10;
	int Unknown_11;
	winrt::guid PhoneLineGuid;
};

struct PH_PHONE_CALL_COUNTS
{
	unsigned ActiveTalkingCalls;
	unsigned OnHoldCalls;
	unsigned DisconnectedCalls;
	unsigned IncomingCalls;
	unsigned DialingCalls;
	unsigned IndeterminateCalls;
	unsigned TransferingCalls;
	unsigned InvalidCalls;
	unsigned ConferenceCalls;
	unsigned UpgradingCalls;
	unsigned AllCalls;
	unsigned BluetoothCalls;
};

DECLARE_HANDLE(HPHONELISTENER);
typedef UINT(CALLBACK *PH_CHANGE_EVENT_NOTIFY_FUNCTION)(
	HPHONELISTENER hListener,
	DWORD_PTR dwUserData,
	const PH_CHANGEEVENT & data
);

using LoadLibraryExW = HMODULE WINAPI(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
using PhoneAPIInitialize = HRESULT WINAPI(void);
using PhoneWaitForAPIReady = HRESULT WINAPI(void);
using PhoneGetState = HRESULT WINAPI(
	_Outptr_result_buffer_(*pCallInfoCount) PH_CALL_INFO **ppCallInfo,
	_Out_ UINT *pCallInfoCount,
	_Out_opt_ PH_PHONE_CALL_COUNTS *pCallCounts
);
using PhoneEnd = HRESULT WINAPI(_In_ const PhoneCallId & callId);
using PhoneFreeCallInfo = void WINAPI(_In_opt_ _Post_invalid_ PH_CALL_INFO * pCallInfos);
using PhoneAddListener = HRESULT WINAPI(
	_In_ PH_CHANGE_EVENT_NOTIFY_FUNCTION pfnChangeEventNotify,
	_In_reads_(cEvents) const PH_CHANGEEVENT *prgchangeevent,
	UINT cEvents,
	DWORD_PTR dwUserData,
	_Out_ HPHONELISTENER *phListener
);
using PhoneClearIdleCallsFromController = HRESULT WINAPI();
using PhoneRemoveListener = HRESULT WINAPI(_In_ HPHONELISTENER hListener);
using PhoneAPIUninitialize = HRESULT WINAPI();