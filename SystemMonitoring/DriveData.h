#pragma once
#include <string>
#include <vector>

class DriveData
{
public:
    wchar_t driveName;
    std::string totalDiskSpace;
    std::string freeDiskSpace;

    static std::vector<DriveData> getDriveData();

    DriveData(wchar_t driveName, const std::string& totalDiskSpace, const std::string& freeDiskSpace)
        : driveName(driveName), totalDiskSpace(totalDiskSpace), freeDiskSpace(freeDiskSpace){}
};