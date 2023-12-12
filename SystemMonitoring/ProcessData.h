#pragma once

#include <string>
#include <Windows.h>
#include <vector>

#include "SortEnum.h"

class ProcessData {
public:
    std::string name;
    SIZE_T memoryUsage;
    ULONGLONG creationTime;
    SIZE_T processCount;

    static void SortProcessesBySortingMethod(std::vector<ProcessData>& runningProcesses, SortEnum::SortingMethod sortingMethod);
    static int CheckIfProcessNameUnique(const std::vector<ProcessData>& runningProcesses, const std::string& name);
    static std::vector<ProcessData> GetRunningProcesses(SIZE_T& overallMemory);
    static bool GetProcessData(DWORD processId, SIZE_T& memoryUsage, SIZE_T& overallMemory, ULONGLONG& creationTime);

    ProcessData(const std::string& name, SIZE_T memoryUsage, ULONGLONG creationTime, SIZE_T processCount)
        : name(name), memoryUsage(memoryUsage), creationTime(creationTime), processCount(processCount) {}
};
