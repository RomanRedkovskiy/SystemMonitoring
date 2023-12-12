#include "ProcessData.h"
#include <vector>
#include <algorithm>
#include "setupapi.h"
#include "devguid.h"
#include "initguid.h"
#include "Usbiodef.h"
#include <Psapi.h>
#include <map>

#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "setupapi.lib")

std::string toLowerCase(const std::string& str) {
    std::string lowerStr;
    lowerStr.reserve(str.length());

    for (char c : str) {
        lowerStr.push_back(std::tolower(c));
    }

    return lowerStr;
}

bool compareByProcessName(const ProcessData& processData1, const ProcessData& processData2) {
    if (processData1.name.size() < 3) {
        return false;
    } 
    if (processData2.name.size() < 3) {
        return true;
    }
    return toLowerCase(processData1.name) < toLowerCase(processData2.name);
}

bool compareByProcessCount(const ProcessData& processData1, const ProcessData& processData2) {
    return processData1.processCount > processData2.processCount;
}

bool compareByMemoryUsage(const ProcessData& processData1, const ProcessData& processData2) {
    return processData1.memoryUsage > processData2.memoryUsage;
}

bool compareByCreationTime(const ProcessData& processData1, const ProcessData& processData2) {
    return processData1.creationTime > processData2.creationTime;
}

void ProcessData::SortProcessesBySortingMethod(std::vector<ProcessData>& runningProcesses, SortEnum::SortingMethod sortingMethod) {
    switch (sortingMethod) {
    case SortEnum::NAME:
        std::sort(runningProcesses.begin(), runningProcesses.end(), compareByProcessName);
        break;
    case SortEnum::COUNT:
        std::sort(runningProcesses.begin(), runningProcesses.end(), compareByProcessCount);
        break;
    case SortEnum::MEMORY:
        std::sort(runningProcesses.begin(), runningProcesses.end(), compareByMemoryUsage);
        break;
    case SortEnum::DATE:
        std::sort(runningProcesses.begin(), runningProcesses.end(), compareByCreationTime);
        break;
    }
}

int ProcessData::CheckIfProcessNameUnique(const std::vector<ProcessData>& runningProcesses, const std::string& name) {
    for (size_t i = 0; i < runningProcesses.size(); ++i) {
        if (runningProcesses[i].name == name) {
            return i;  // Found a matching process name, return the index
        }
    }
    return -1;  // Process name is unique
}

// Helper function to retrieve CPU/memory usage of a process
bool ProcessData::GetProcessData(DWORD processId, SIZE_T& memoryUsage, SIZE_T& overallMemory, ULONGLONG& creationTime) {
    FILETIME createTime, exitTime, kernelTime, userTime;
    // Get the memory usage
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess != nullptr) {
        if (GetProcessTimes(hProcess, &createTime, &exitTime, &kernelTime, &userTime)) {
            static std::map<DWORD, ULONGLONG> processStartTime;

            ULONGLONG currentCreationTime = (static_cast<ULONGLONG>(createTime.dwHighDateTime) << 32) | createTime.dwLowDateTime;
            creationTime = currentCreationTime;

            PROCESS_MEMORY_COUNTERS_EX pmc;
            if (GetProcessMemoryInfo(hProcess, reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
                memoryUsage = pmc.WorkingSetSize;
                overallMemory += memoryUsage;
                CloseHandle(hProcess);
                return true;
            }
            else {
                CloseHandle(hProcess);
                return false;
            }
        }
        else {
            CloseHandle(hProcess);
            return false;
        }
    }
    else {
        return false;
    }
}

// Helper function to get a list of running processes
std::vector<ProcessData> ProcessData::GetRunningProcesses(SIZE_T& overallMemory) {
    std::vector<ProcessData> processesData;
    overallMemory = 0;
    DWORD processes[1024], cbNeeded, cProcesses;
    if (EnumProcesses(processes, sizeof(processes), &cbNeeded)) {
        cProcesses = cbNeeded / sizeof(DWORD);

        for (DWORD i = 0; i < cProcesses; i++) {
            SIZE_T memoryUsage;
            ULONGLONG creationTime = 0;

            if (processes[i] != 0) { // Skip IDLE process (PID 0)
                if (GetProcessData(processes[i], memoryUsage, overallMemory, creationTime)) {
                    char processName[MAX_PATH] = { 0 };
                    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
                    if (hProcess != nullptr) {
                        GetModuleBaseNameA(hProcess, nullptr, processName, sizeof(processName) / sizeof(char));
                        CloseHandle(hProcess);
                    }
                    std::string name = processName;
                    int position = ProcessData::CheckIfProcessNameUnique(processesData, name);
                    if (position == -1) {
                        // Process name is unique, create a new ProcessData object and add it to the vector
                        ProcessData processData(name, memoryUsage, creationTime, 1);
                        processesData.push_back(processData);
                    }
                    else {
                        // Process name already exists, update memoryUsage and creationTime of the existing element
                        processesData[position].memoryUsage += memoryUsage;
                        processesData[position].processCount++;                       
                    }
                }
            }
        }
    }

    return processesData;
}