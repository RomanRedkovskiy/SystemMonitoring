#pragma once

#include <Windows.h>
#include <string>

class Tools {
public:

    static std::string BytesToReadableSize(DWORDLONG bytes);
    static LPWSTR ConvertToWideString(const char* narrowStr);
    static wchar_t* ConvertStringToWCharT(const std::string& str);
    static wchar_t* ConcatByteToConstWCharT(const wchar_t* str, BYTE number);
    static std::string ConvertSizeTToString(SIZE_T sizeValue);
    static std::string ConvertByteToString(BYTE byteValue);
    static std::string ConvertFileTimeToHumanReadable(ULONGLONG fileTime);
    static std::string ConvertCurrentTimeToHumanReadable(time_t currentTime);
    static std::string ConvertBatteryLifeTimeToHumanReadable(DWORD batteryLifeTime);
};