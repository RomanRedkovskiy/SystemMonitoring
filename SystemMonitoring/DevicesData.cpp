#include "DevicesData.h"

#include <vector>
#include "setupapi.h"
#include "devguid.h"
#include "Usbiodef.h"

#include <map>

#pragma comment(lib, "setupapi.lib")


std::vector<std::string> DevicesData::GetPCIDevices() {
    std::vector<std::string> deviceNames;

    HDEVINFO hDevInfo;
    SP_DEVINFO_DATA deviceInfoData;
    DWORD deviceIndex = 0;

    hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_SYSTEM, NULL, NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);

    if (hDevInfo == INVALID_HANDLE_VALUE) {
        return deviceNames; // Return an empty vector
    }

    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    while (SetupDiEnumDeviceInfo(hDevInfo, deviceIndex, &deviceInfoData)) {
        char deviceName[MAX_PATH];
        DWORD deviceNameSize = sizeof(deviceName);

        if (SetupDiGetDeviceRegistryPropertyA(hDevInfo, &deviceInfoData, SPDRP_DEVICEDESC, NULL, (BYTE*)deviceName, deviceNameSize, NULL)) {
            deviceNames.push_back(deviceName);
        }

        deviceIndex++;
    }

    if (GetLastError() != NO_ERROR && GetLastError() != ERROR_NO_MORE_ITEMS) {
        return deviceNames; // Return the collected device names even if an error occurred
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);

    return deviceNames;
}

std::vector<std::string> DevicesData::GetUSBDevices() {
    std::vector<std::string> deviceNames;

    HDEVINFO hDevInfo;
    SP_DEVINFO_DATA deviceInfoData;
    DWORD deviceIndex = 0;

    hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_USB_DEVICE, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (hDevInfo == INVALID_HANDLE_VALUE) {
        return deviceNames; // Return an empty vector
    }

    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    while (SetupDiEnumDeviceInfo(hDevInfo, deviceIndex, &deviceInfoData)) {
        char deviceName[MAX_PATH];
        DWORD deviceNameSize = sizeof(deviceName);

        if (SetupDiGetDeviceRegistryPropertyA(hDevInfo, &deviceInfoData, SPDRP_DEVICEDESC, NULL, (BYTE*)deviceName, deviceNameSize, NULL)) {
            deviceNames.push_back(deviceName);
        }

        deviceIndex++;
    }

    if (GetLastError() != NO_ERROR && GetLastError() != ERROR_NO_MORE_ITEMS) {
        return deviceNames; // Return the collected device names even if an error occurred
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);

    return deviceNames;
}
