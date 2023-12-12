#pragma once
#include <iostream>
#include <Windows.h>

class BatteryData {
public:
    BYTE batteryPercentage;
    bool isCharging;
    DWORD batteryLifeTime;

    static BatteryData getBatteryData();

    BatteryData(BYTE percentage, bool charging, DWORD lifeTime)
        : batteryPercentage(percentage), isCharging(charging), batteryLifeTime(lifeTime) {}
};

