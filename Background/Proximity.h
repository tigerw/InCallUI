#pragma once

#include "pch.h"

class Proximity
{
public:
	Proximity();
	~Proximity();

	void Sense();
	void Stop();

private:
	winrt::Windows::Devices::Sensors::ProximitySensor Sensor;
	winrt::Windows::Devices::Sensors::ProximitySensorDisplayOnOffController Controller;
};