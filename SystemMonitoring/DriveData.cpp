#include "DriveData.h"
#include "Tools.h"
#include <Windows.h>
#include <vector>


std::vector<DriveData> DriveData::getDriveData() {
    std::vector<DriveData> driveDataVector;

    DWORD drives = GetLogicalDrives();

    if (drives == 0) {
        return driveDataVector;
    }

    for (char drive = 'A'; drive <= 'Z'; ++drive) {
        if ((drives & 1) != 0) {
            std::wstring rootPath = std::wstring(1, drive) + L":\\";

            ULARGE_INTEGER totalFreeBytes;
            ULARGE_INTEGER totalBytes;
            ULARGE_INTEGER totalFreeBytesEx;

            if (GetDiskFreeSpaceExW(rootPath.c_str(), &totalFreeBytes, &totalBytes, &totalFreeBytesEx)) {
                std::string totalDiskSpace = Tools::BytesToReadableSize(static_cast<DWORDLONG>(totalBytes.QuadPart));
                std::string freeDiskSpace = Tools::BytesToReadableSize(static_cast<DWORDLONG>(totalFreeBytes.QuadPart));

                DriveData driveData(rootPath[0], totalDiskSpace, freeDiskSpace);
                driveDataVector.push_back(driveData);
            }
        }

        drives >>= 1;
    }

    return driveDataVector;
}