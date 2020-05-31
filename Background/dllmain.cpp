#include "pch.h"
#include "Listener.h"

bool Created = false;

BOOL APIENTRY DllMain(HMODULE /* hModule */, DWORD ul_reason_for_call, LPVOID /* lpReserved */)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

STDAPI DllCanUnloadNow(void)
{
	if (winrt::get_module_lock())
	{
		return S_FALSE;
	}

	winrt::clear_factory_cache();
	return S_OK;
}

STDAPI DllGetActivationFactory(
	_In_  HSTRING            activatableClassId,
	_Out_ IActivationFactory **factory
)
{
	if (!Created)
	{
		Created = true;
		*factory = winrt::make<winrt::Background::factory_implementation::Listener>().as<IActivationFactory>().detach();
		return S_OK;
	}

	*factory = nullptr;
	return E_NOT_VALID_STATE;
}