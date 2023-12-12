#include "BatteryData.h"

BatteryData BatteryData::getBatteryData() {
    SYSTEM_POWER_STATUS powerStatus;
    if (GetSystemPowerStatus(&powerStatus)) {
        BYTE batteryPercentage = powerStatus.BatteryLifePercent;
        bool isCharging = (powerStatus.ACLineStatus == 1);
        DWORD batteryLifeTime = powerStatus.BatteryLifeTime;
        return BatteryData(batteryPercentage, isCharging, batteryLifeTime);
    }
    else {
        return BatteryData(0, false, 0);
    }
}