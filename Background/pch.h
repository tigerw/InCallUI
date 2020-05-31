#pragma once

#include <SDKDDKVer.h>

#ifndef WINRT_LEAN_AND_MEAN
#define WINRT_LEAN_AND_MEAN
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <mutex>

#include <windows.h>
#include <unknwn.h>
#include <restrictederrorinfo.h>
#include <hstring.h>
#include <activation.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.ApplicationModel.Background.h>
#include <winrt/Windows.ApplicationModel.Calls.h>
#include <winrt/Windows.ApplicationModel.Calls.Background.h>
#include <winrt/Windows.ApplicationModel.Contacts.h>
#include <winrt/Windows.Data.Xml.Dom.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Sensors.h>
#include <winrt/Windows.UI.Notifications.h>