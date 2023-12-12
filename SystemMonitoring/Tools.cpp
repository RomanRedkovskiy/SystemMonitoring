#include "Tools.h"
#include <cstdio>
#include <sstream>
#include <iomanip>

enum Constants {
    HOUR = 3600,
    MINUTE = 60,
    BYTES = 1024
};

std::string Tools::BytesToReadableSize(DWORDLONG bytes) {
    const int bufferSize = BYTES;
    char buffer[bufferSize];

    double kilobytes = static_cast<double>(bytes) / BYTES;
    double megabytes = kilobytes / BYTES;
    double gigabytes = megabytes / BYTES;

    if (gigabytes >= 1.0)
        snprintf(buffer, bufferSize, "%.3f GB", gigabytes);
    else if (megabytes >= 1.0)
        snprintf(buffer, bufferSize, "%.3f MB", megabytes);
    else if (kilobytes >= 1.0)
        snprintf(buffer, bufferSize, "%.3f KB", kilobytes);
    else
        snprintf(buffer, bufferSize, "%llu B", bytes);

    return std::string(buffer);
}

LPWSTR Tools::ConvertToWideString(const char* narrowStr) {
    int wideStrLength = MultiByteToWideChar(CP_UTF8, 0, narrowStr, -1, nullptr, 0);
    wchar_t* wideStr = new wchar_t[wideStrLength];
    MultiByteToWideChar(CP_UTF8, 0, narrowStr, -1, wideStr, wideStrLength);
    return wideStr;
}

wchar_t* Tools::ConvertStringToWCharT(const std::string& str) {
    int length = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    wchar_t* wstr = new wchar_t[length];
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr, length);
    return wstr;
}

wchar_t* Tools::ConcatByteToConstWCharT(const wchar_t* str, BYTE number) {
    // Calculate the length of the input string
    size_t strLength = wcslen(str);
    // Calculate the length of the resulting string
    size_t resultLength = strLength + 4; // 4 is the maximum number of characters needed for a BYTE (0-255)
    // Allocate memory for the resulting string
    wchar_t* result = new wchar_t[resultLength + 1]; // +1 for the null terminator
    // Copy the input string to the resulting string
    wcscpy_s(result, resultLength + 1, str);
    // Concatenate the BYTE number to the resulting string
    swprintf_s(result + strLength, resultLength - strLength + 1, L"%u", number);

    return result;
}

std::string Tools::ConvertSizeTToString(SIZE_T sizeValue) {
    std::stringstream ss;
    ss << sizeValue; // Insert the SIZE_T value into the stringstream
    return ss.str(); // Return the stringstream content as a string
}

std::string Tools::ConvertByteToString(BYTE byteValue) {
    std::stringstream ss;
    ss << static_cast<int>(byteValue); // Convert the BYTE value to an integer and insert into the stringstream
    return ss.str(); // Return the stringstream content as a string
}

// Helper function to convert creation time in seconds to a human-readable format (d/m/y h:m:s)
std::string Tools::ConvertFileTimeToHumanReadable(ULONGLONG fileTime) {
    FILETIME ft;
    ft.dwLowDateTime = static_cast<DWORD>(fileTime);
    ft.dwHighDateTime = fileTime >> 32;

    SYSTEMTIME stUTC, stLocal;
    FileTimeToSystemTime(&ft, &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

    std::ostringstream oss;
    oss << stLocal.wDay << "/" << stLocal.wMonth << "/" << stLocal.wYear << " ";

    // Add leading zeros for hour, minute, and second
    oss << std::setfill('0') << std::setw(2) << stLocal.wHour << ":"
        << std::setfill('0') << std::setw(2) << stLocal.wMinute << ":"
        << std::setfill('0') << std::setw(2) << stLocal.wSecond;

    return oss.str();
}

std::string Tools::ConvertCurrentTimeToHumanReadable(time_t currentTime) {
    std::tm timeInfo;
    localtime_s(&timeInfo, &currentTime);
    std::stringstream ss;

    ss << std::setfill('0') << std::setw(2) << timeInfo.tm_mday << '/'  // Day of the month (d)
        << std::setw(2) << (timeInfo.tm_mon + 1) << '/'  // Month (m)
        << (timeInfo.tm_year + 1900) << ' '  // Year (y)
        << std::setw(2) << std::setfill('0') << timeInfo.tm_hour << ':'  // Hour (h)
        << std::setw(2) << std::setfill('0') << timeInfo.tm_min << ':'  // Minute (m)
        << std::setw(2) << std::setfill('0') << timeInfo.tm_sec;  // Second (s)

    return ss.str();
}

//Helper function to convert seconds into human readable (x hours y minutes)
std::string Tools::ConvertBatteryLifeTimeToHumanReadable(DWORD batteryLifeTime) {
    if (batteryLifeTime != DWORD(-1)) {
        int hours = batteryLifeTime / HOUR;
        int minutes = (batteryLifeTime % HOUR) / MINUTE;
        return "Estimated Time Left: " + std::to_string(hours) + " hours, " + std::to_string(minutes) + " minutes";
    }
    else {
        return "Estimated Time Left: Not available";
    }
}