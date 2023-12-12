#pragma once

#include "Windows.h"

class SystemParameters
{
public:
	time_t time;
	BYTE batteryPercentage;
	SIZE_T memoryUsage;

	SystemParameters(time_t time, BYTE batteryPercentage, SIZE_T memoryUsage)
		: time(time), batteryPercentage(batteryPercentage), memoryUsage(memoryUsage) {}
};

