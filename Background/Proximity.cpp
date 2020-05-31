#include "pch.h"
#include "Proximity.h"

using namespace winrt;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Sensors;

Proximity::Proximity() :
	Sensor(
		ProximitySensor::FromId(
			DeviceInformation::FindAllAsync(ProximitySensor::GetDeviceSelector()).get().GetAt(0).Id()
		)
	),
	Controller(nullptr)
{
}

Proximity::~Proximity()
{
	Stop();
}

void Proximity::Sense()
{
	if (Controller != nullptr)
	{
		return;
	}

	Controller = Sensor.CreateDisplayOnOffController();
}

void Proximity::Stop()
{
	if (Controller == nullptr)
	{
		return;
	}

	Controller.Close();
	Controller = nullptr;
}