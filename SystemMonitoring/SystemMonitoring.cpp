// SystemMonitoring.cpp : Defines the entry point for the application.
#include "framework.h"
#include "SystemMonitoring.h"
#include <Windows.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include "time.h"
#include "commctrl.h"

#include "ProcessData.h"
#include "Tools.h"
#include "DriveData.h"
#include "DevicesData.h"
#include "BatteryData.h"
#include "SystemParameters.h"
#include "FileProcessing.h"

#pragma comment(lib, "Comctl32.lib")


enum Constants {
     DEFAULT_OFFSET = 1,
     DEFAULT_DROPDOWN_POS = 2,
     TIMER = 5,
     DROPDOWN_COUNT = 7,
     DROPDOWN_HEIGHT = 17,
     SCROLL_LENGTH = 17,
     PANEL_HEIGHT = 22,
     MAX_LOADSTRING = 100,

     DROPDOWN_LENGTH = 195,
     BUTTON_LENGTH = 290,

     PROCESS_COLUMN_NAME_LENGTH = 450,
     PROCESS_COLUMN_COUNT_LENGTH = 120,
     PROCESS_COLUMN_MEMORY_LENGTH = 200,
     PROCESS_COLUMN_DATE_LENGTH = 300,

     LOGGER_COLUMN_LENGTH = 260,

     ADDITIONAL_OFFSET = 50,
     ADDITIONAL_COLUMN_NAME_LENGTH = 350,
     ADDITIONAL_COLUMN_NUMBER_LENGTH = 100,

     LABEL_OFFSET = 10,
     SMALL_LABEL_LENGTH = 30,
     HUGE_LABEL_LENGTH = 100,

     PCI_COLUMN_OFFSET = 20,

     PROCESS_TABLE_HEIGHT = 498,
     PCI_TABLE_HEIGHT = 464,
     USB_TABLE_HEIGHT = 193,
     DRIVE_TABLE_HEIGHT = 91,

     MILLIS_IN_SEC = 1000,

     ID_PROCESS_BUTTON = 600,
     ID_LOGGER_BUTTON = 601,
     ID_ADDITIONAL_BUTTON = 602,

     ID_REFRESH_DROPDOWN = 610,
     ID_SORT_DROPDOWN = 611,
     ID_LOGGER_DROPDOWN = 612,

     ID_PROCESS_TABLE = 620,
     ID_USB_TABLE = 621,
     ID_PCI_TABLE = 622,
     ID_DRIVE_TABLE = 623
};

//map to store a key-value for refreshDropdown row number and frequency of the update
const std::map<int, int> frequencyMap = { {0, 5}, {1, 30}, {2, 60}, {3, 300}, {4, 1800}, {5, 3600} };
//map to store a key-value for sortDropdown row number and sort method for the displaying processes in the process table
const std::map<int, SortEnum::SortingMethod> sortingMap = { {0, SortEnum::NAME}, {1, SortEnum::COUNT}, {2, SortEnum::MEMORY}, {3, SortEnum::DATE} };

// Global Variables:
HINSTANCE hInst;                                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                   // the title bar text
WCHAR szWindowClass[MAX_LOADSTRING];    // the main window class name

HWND processTable;                                           // global variable to store the handle to the processTable window
HWND usbTable;                                                 // global variable to store the handle to the usbTable window
HWND pciTable;                                                  // global variable to store the handle to the pciTable window
HWND driveTable;                                               // global variable to store the handle to the driveTable window
HWND loggerTable;                                            // global variable to store the handle to the loggerTable window

HWND processButton;                                        // global variable to store the handle to the processButton window
HWND loggerButton;                                          // global variable to store the handle to the loggerButton window
HWND additionalButton;                                    // global variable to store the handle to the additionalButton window

HWND refreshDropdown;                                   // global variable to store the handle to the refreshDropdown window
HWND sortDropdown;                                        // global variable to store the handle to the sortDropdown window
HWND loggerDropdown;                                    // global variable to store the handle to the loggerDropdown window

HWND labelArea;                                                 // global variable to store the handle to the labelArea window
HWND percentageLabel;                                     // global variable to store the handle to the percentageLabel window
HWND statusLabel;                                              // global variable to store the handle to the statusLabel window
HWND timeLabel;                                                // global variable to store the handle to the timeLabel window

std::string filePath = "SystemData.txt";                           // system data file path

BatteryData batteryData(0, false, 0);                               //object that stores battery data
int lastPressedButton = ID_PROCESS_BUTTON;            //stores id of the last pressed panel button 
SIZE_T overallMemory = 0;                                             //number that stores overall memory usage
std::vector<std::string> pciDevices;                                //vector that stores all PCI devices
std::vector<std::string> usbDevices;                               //vector that stores all USB devices
std::vector<ProcessData> runningProcesses;                //vector that stores data of all running processes
std::vector<DriveData> drives;                                       //vector that stores data of all current drives      
std::vector<SystemParameters> systemParameters;     //vector that stores system's memory usage and battery percentage at a particular times

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

//Setting visability for top panel (buttons and refresh dropdown)
void SetPanelStates(int activeButtonId) {
    ShowWindow(processButton, SW_SHOW);
    EnableWindow(processButton, activeButtonId != ID_PROCESS_BUTTON);

    ShowWindow(loggerButton, SW_SHOW);
    EnableWindow(loggerButton, activeButtonId != ID_LOGGER_BUTTON);

    ShowWindow(additionalButton, SW_SHOW);
    EnableWindow(additionalButton, activeButtonId != ID_ADDITIONAL_BUTTON);

    ShowWindow(refreshDropdown, SW_SHOW);
    EnableWindow(refreshDropdown, SW_SHOW);
}


/*
****************************************************************************************
*****Functions that create tables by setting their size, position, column name etc.****
****************************************************************************************
*/


void CreateProcessTable(HWND hWndParent) {
    // Create the ListView control
    processTable = CreateWindowW(WC_LISTVIEW, L"",
        WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_NOSORTHEADER,
        DEFAULT_OFFSET * 3, PANEL_HEIGHT * 3, PROCESS_COLUMN_NAME_LENGTH * 4, PROCESS_TABLE_HEIGHT, hWndParent, NULL, NULL, NULL);

    // Set extended styles for the ListView control
    ListView_SetExtendedListViewStyle(processTable, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    // Add columns to the ListView control
    LVCOLUMN lvColumn;
    lvColumn.mask = LVCF_TEXT | LVCF_WIDTH;

    // Column 0: Name
    lvColumn.pszText = _wcsdup(L"Name");
    lvColumn.cx = PROCESS_COLUMN_NAME_LENGTH;
    ListView_InsertColumn(processTable, 0, &lvColumn);

    // Column 1: Process count
    lvColumn.pszText = _wcsdup(L"Process count");
    lvColumn.cx = PROCESS_COLUMN_COUNT_LENGTH;
    ListView_InsertColumn(processTable, 1, &lvColumn);

    // Column 2: Memory Usage
    lvColumn.pszText = _wcsdup(L"Memory Usage");
    lvColumn.cx = PROCESS_COLUMN_MEMORY_LENGTH;
    ListView_InsertColumn(processTable, 2, &lvColumn);

    // Column 3: Creation Time
    lvColumn.pszText = _wcsdup(L"Creation Time");
    lvColumn.cx = PROCESS_COLUMN_DATE_LENGTH;
    ListView_InsertColumn(processTable, 3, &lvColumn);

    free(lvColumn.pszText);
}

void CreateLoggerTable(HWND hWndParent) {
    // Create the ListView control
    loggerTable = CreateWindowW(WC_LISTVIEW, L"",
        WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_NOSORTHEADER,
        DEFAULT_OFFSET * 3, PANEL_HEIGHT * 3, LOGGER_COLUMN_LENGTH * 4 + SCROLL_LENGTH, PROCESS_TABLE_HEIGHT, hWndParent, NULL, NULL, NULL);

    // Set extended styles for the ListView control
    ListView_SetExtendedListViewStyle(loggerTable, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    // Add columns to the ListView control
    LVCOLUMN lvColumn;
    lvColumn.mask = LVCF_TEXT | LVCF_WIDTH;

    // Column 0: Number
    lvColumn.pszText = _wcsdup(L"Number");
    lvColumn.cx = LOGGER_COLUMN_LENGTH;
    ListView_InsertColumn(loggerTable, 0, &lvColumn);

    // Column 1: Overall Memory Usage
    lvColumn.pszText = _wcsdup(L"Overall Memory Usage");
    lvColumn.cx = LOGGER_COLUMN_LENGTH;
    ListView_InsertColumn(loggerTable, 1, &lvColumn);

    // Column 2: Battery Percentage
    lvColumn.pszText = _wcsdup(L"Battery Percentage");
    lvColumn.cx = LOGGER_COLUMN_LENGTH;
    ListView_InsertColumn(loggerTable, 2, &lvColumn);

    // Column 3: Log Time
    lvColumn.pszText = _wcsdup(L"Log Time");
    lvColumn.cx = LOGGER_COLUMN_LENGTH;
    ListView_InsertColumn(loggerTable, 3, &lvColumn);

    free(lvColumn.pszText);
}

void CreateDriveTable(HWND hWndParent) {
    // Create the ListView control
    driveTable = CreateWindowW(WC_LISTVIEW, L"", WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_NOSORTHEADER,
        ADDITIONAL_OFFSET, ADDITIONAL_OFFSET * 4, ADDITIONAL_COLUMN_NUMBER_LENGTH + ADDITIONAL_COLUMN_NAME_LENGTH,
        DRIVE_TABLE_HEIGHT, hWndParent, NULL, NULL, NULL);

    // Set extended styles for the ListView control
    ListView_SetExtendedListViewStyle(driveTable, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    // Add columns to the ListView control
    LVCOLUMN lvColumn;
    lvColumn.mask = LVCF_TEXT | LVCF_WIDTH;

    // Column 0: Disk
    lvColumn.pszText = _wcsdup(L"Disk");
    lvColumn.cx = ADDITIONAL_COLUMN_NUMBER_LENGTH;
    ListView_InsertColumn(driveTable, 0, &lvColumn);

    // Column 1: Free Disk Space
    lvColumn.pszText = _wcsdup(L"Free Disk Space");
    lvColumn.cx = ADDITIONAL_COLUMN_NAME_LENGTH / 2;
    ListView_InsertColumn(driveTable, 1, &lvColumn);

    // Column 2: Total Disk Space
    lvColumn.pszText = _wcsdup(L"Total Disk Space");
    lvColumn.cx = ADDITIONAL_COLUMN_NAME_LENGTH / 2;
    ListView_InsertColumn(driveTable, 2, &lvColumn);

    free(lvColumn.pszText);
}

void CreateUsbTable(HWND hWndParent) {
    // Create the ListView control
    usbTable = CreateWindowW(WC_LISTVIEW, L"", WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_NOSORTHEADER,
        ADDITIONAL_OFFSET, ADDITIONAL_OFFSET * 5 + DRIVE_TABLE_HEIGHT, ADDITIONAL_COLUMN_NUMBER_LENGTH + ADDITIONAL_COLUMN_NAME_LENGTH,
        USB_TABLE_HEIGHT, hWndParent, NULL, NULL, NULL);

    // Set extended styles for the ListView control
    ListView_SetExtendedListViewStyle(usbTable, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    // Add columns to the ListView control
    LVCOLUMN lvColumn;
    lvColumn.mask = LVCF_TEXT | LVCF_WIDTH;

    // Column 0: Number
    lvColumn.pszText = _wcsdup(L"Number");
    lvColumn.cx = ADDITIONAL_COLUMN_NUMBER_LENGTH;
    ListView_InsertColumn(usbTable, 0, &lvColumn);

    // Column 1: USB Device Name
    lvColumn.pszText = _wcsdup(L"USB Device Name");
    lvColumn.cx = ADDITIONAL_COLUMN_NAME_LENGTH;
    ListView_InsertColumn(usbTable, 1, &lvColumn);

    free(lvColumn.pszText);
}

void CreatePciTable(HWND hWndParent) {
    // Create the ListView control
    pciTable = CreateWindowW(WC_LISTVIEW, L"", WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_NOSORTHEADER, ADDITIONAL_COLUMN_NUMBER_LENGTH +
        ADDITIONAL_COLUMN_NAME_LENGTH + ADDITIONAL_OFFSET * 2, (PANEL_HEIGHT + DEFAULT_OFFSET) * 3, ADDITIONAL_COLUMN_NUMBER_LENGTH +
        PCI_COLUMN_OFFSET + SCROLL_LENGTH + ADDITIONAL_COLUMN_NAME_LENGTH, PCI_TABLE_HEIGHT, hWndParent, NULL, NULL, NULL);

    // Set extended styles for the ListView control
    ListView_SetExtendedListViewStyle(pciTable, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    // Add columns to the ListView control
    LVCOLUMN lvColumn;
    lvColumn.mask = LVCF_TEXT | LVCF_WIDTH;

    // Column 0: Number
    lvColumn.pszText = _wcsdup(L"Number");
    lvColumn.cx = ADDITIONAL_COLUMN_NUMBER_LENGTH - PCI_COLUMN_OFFSET;
    ListView_InsertColumn(pciTable, 0, &lvColumn);

    // Column 1: PCI Device Name
    lvColumn.pszText = _wcsdup(L"PCI Device Name");
    lvColumn.cx = ADDITIONAL_COLUMN_NAME_LENGTH + PCI_COLUMN_OFFSET * 2;
    ListView_InsertColumn(pciTable, 1, &lvColumn);

    free(lvColumn.pszText);
}


/*
****************************************************************************************
**Functions that fill created tables with certain values (taken from certain structures)*
****************************************************************************************
*/


void FillProcessTable() {
    // Clear existing items from the ListView
    ListView_DeleteAllItems(processTable);

    // Iterate over the runningProcesses vector and add items to the ListView
    for (size_t i = 0; i < runningProcesses.size(); i++) {
        // Create a new item
        LVITEM lvItem;
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = i;
        lvItem.iSubItem = 0;

        lvItem.pszText = Tools::ConvertToWideString(runningProcesses[i].name.c_str());

        ListView_InsertItem(processTable, &lvItem);

        // Set the subitems (columns) for the item
        char buffer[256]; // Temporary buffer for converting numeric values to string
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = i;

        // Column 1:  Process Count 
        lvItem.iSubItem = 1;
        sprintf_s(buffer, "%zu", runningProcesses[i].processCount);
        lvItem.pszText = Tools::ConvertToWideString(buffer);
        ListView_SetItem(processTable, &lvItem);

        // Column 2: Memory Usage
        lvItem.iSubItem = 2;
        sprintf_s(buffer, "%s", Tools::BytesToReadableSize(runningProcesses[i].memoryUsage).c_str());
        lvItem.pszText = Tools::ConvertToWideString(buffer);
        ListView_SetItem(processTable, &lvItem);

        // Column 3: Creation Time
        lvItem.iSubItem = 3;
        sprintf_s(buffer, "%s", Tools::ConvertFileTimeToHumanReadable(runningProcesses[i].creationTime).c_str());
        lvItem.pszText = Tools::ConvertToWideString(buffer);
        ListView_SetItem(processTable, &lvItem);
    }
}

void FillLoggerTable(int reverse) {
    // Clear existing items from the ListView
    ListView_DeleteAllItems(loggerTable);

    size_t itemIndex = 0;
    size_t startIndex = 0;
    size_t endIndex = systemParameters.size();
    int increment = 1;

    if (reverse != 0) {
        startIndex = systemParameters.size() - 1;
        endIndex = -1;
        increment = -1;
    }

    for (size_t i = startIndex; i != endIndex; i += increment, itemIndex++) {
        // Create a new item
        LVITEM lvItem;
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = itemIndex;
        lvItem.iSubItem = 0;
        lvItem.pszText = Tools::ConvertToWideString(std::to_string(i).c_str());

        ListView_InsertItem(loggerTable, &lvItem);

        // Set the subitems (columns) for the item
        char buffer[256]; // Temporary buffer for converting numeric values to string
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = itemIndex;

        // Column 1: Overall Memory Usage
        lvItem.iSubItem = 1;
        sprintf_s(buffer, "%s", Tools::BytesToReadableSize(systemParameters[i].memoryUsage).c_str());
        lvItem.pszText = Tools::ConvertToWideString(buffer);
        ListView_SetItem(loggerTable, &lvItem);

        // Column 2: Battery Percentage
        lvItem.iSubItem = 2;
        sprintf_s(buffer, "%u", systemParameters[i].batteryPercentage);
        lvItem.pszText = Tools::ConvertToWideString(buffer);
        ListView_SetItem(loggerTable, &lvItem);

        // Column 3: Log Time
        lvItem.iSubItem = 3;
        sprintf_s(buffer, "%s", Tools::ConvertCurrentTimeToHumanReadable(systemParameters[i].time).c_str());
        lvItem.pszText = Tools::ConvertToWideString(buffer);
        ListView_SetItem(loggerTable, &lvItem);
    }
}

void FillDriveTable()
{
    // Clear existing items from the ListView
    ListView_DeleteAllItems(driveTable);

    // Iterate over the usbDevices vector and add items to the ListView
    for (size_t i = 0; i < drives.size(); i++) {
        // Create a new item
        LVITEM lvItem;
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = i;
        lvItem.iSubItem = 0;
        std::wstring wideDriveName(1, drives[i].driveName); // Construct a std::wstring with a single character
        lvItem.pszText = const_cast<LPWSTR>(wideDriveName.c_str());

        ListView_InsertItem(driveTable, &lvItem);

        // Set the subitems (columns) for the item
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = i;

        // Column 1: Free Disk Space
        lvItem.iSubItem = 1;
        lvItem.pszText = Tools::ConvertToWideString(drives[i].freeDiskSpace.c_str());
        ListView_SetItem(driveTable, &lvItem);

        // Column 2: Total Disk Space
        lvItem.iSubItem = 2;
        lvItem.pszText = Tools::ConvertToWideString(drives[i].totalDiskSpace.c_str());
        ListView_SetItem(driveTable, &lvItem);
    }
}

void FillUsbTable()
{
    // Clear existing items from the ListView
    ListView_DeleteAllItems(usbTable);

    // Iterate over the usbDevices vector and add items to the ListView
    for (size_t i = 0; i < usbDevices.size(); i++) {
        // Create a new item
        LVITEM lvItem;
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = i;
        lvItem.iSubItem = 0;
        lvItem.pszText = Tools::ConvertToWideString(std::to_string(i).c_str());

        ListView_InsertItem(usbTable, &lvItem);

        // Set the subitems (columns) for the item
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = i;

        // Column 1: USB Device Name
        lvItem.iSubItem = 1;
        lvItem.pszText = Tools::ConvertToWideString(usbDevices[i].c_str());
        ListView_SetItem(usbTable, &lvItem);
    }
}

void FillPciTable() {
    // Clear existing items from the ListView
    ListView_DeleteAllItems(pciTable);

    // Iterate over the usbDevices vector and add items to the ListView
    for (size_t i = 0; i < pciDevices.size(); i++) {
        // Create a new item
        LVITEM lvItem;
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = i;
        lvItem.iSubItem = 0;
        lvItem.pszText = Tools::ConvertToWideString(std::to_string(i).c_str());

        ListView_InsertItem(pciTable, &lvItem);

        // Set the subitems (columns) for the item
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = i;

        // Column 1: PCI Device Name
        lvItem.iSubItem = 1;
        lvItem.pszText = Tools::ConvertToWideString(pciDevices[i].c_str());
        ListView_SetItem(pciTable, &lvItem);
    }
}

void FillTables() {
    FillProcessTable();
    FillLoggerTable(SendMessage(loggerDropdown, CB_GETCURSEL, 0, 0));
    FillDriveTable();
    FillUsbTable();
    FillPciTable();
}

/*
****************************************************************************************
*************Functions that create custom controllers by their parameters************
****************************************************************************************
*/


HWND CreateCustomButton(HWND hWndParent, HINSTANCE hInstance, LPCWSTR buttonText, int x, int y, int width, int height, bool isEnabled)
{
    DWORD buttonStyles = WS_VISIBLE | WS_CHILD | (isEnabled ? 0 : WS_DISABLED) | BS_PUSHBUTTON;

    return CreateWindowW(
        L"BUTTON",           // Predefined class name for a button control
        buttonText,            // Button text
        buttonStyles,         // Button styles
        x,                           // X-coordinate
        y,                           // Y-coordinate
        width,                    // Width
        height,                   // Height
        hWndParent,         // Parent window handle
        nullptr,                  // No menu handle
        hInstance,             // Instance handle
        nullptr                   // No additional parameters
    );
}

HWND CreateCustomDropdown(HWND hWndParent, HINSTANCE hInstance, int x, int y, int width, int height)
{
    return CreateWindowW(
        WC_COMBOBOXW,                                              // Window class name for the combobox control
        NULL,                                                                    // Window title (not used for dropdown)
        CBS_DROPDOWNLIST | CBS_HASSTRINGS |       // Combobox style: dropdown list, items with strings
        WS_CHILD | WS_VISIBLE | WS_VSCROLL,            // Window styles: child, visible, vertical scroll bar
        x,                                                                          // X-coordinate of the control's position
        y,                                                                          // Y-coordinate of the control's position
        width,                                                                   // Width of the control
        height * DROPDOWN_COUNT,                           // Height of the control (scaled by DROPDOWN_COUNT)
        hWndParent,                                                        // Parent window handle
        nullptr,                                                                  // Menu handle (not used for dropdown)
        hInstance,                                                             // Instance handle of the application
        nullptr                                                                   // Additional creation parameters (not used for dropdown)
    );
}

HWND CreateCustomLabel(HWND hWndParent, HINSTANCE hInstance, LPCWSTR text, int x, int y, int width, int height) {
    return CreateWindowW(L"STATIC", text, WS_VISIBLE | WS_CHILD, x, y, width, height, hWndParent, NULL, hInstance, NULL);
}

HFONT CreateDefaultFont() {
    return CreateFontW(
        16,                                                   // Font height in pixels
        0,                                                     // Font width (0 = default)
        0,                                                     // Font escapement angle
        0,                                                     // Font orientation angle
        FW_NORMAL,                                 // Font weight (e.g., FW_NORMAL, FW_BOLD)
        FALSE,                                             // Font italic (TRUE or FALSE)
        FALSE,                                             // Font underline (TRUE or FALSE)
        FALSE,                                             // Font strikeout (TRUE or FALSE)
        DEFAULT_CHARSET,                        // Character set identifier
        OUT_DEFAULT_PRECIS,                   // Output precision
        CLIP_DEFAULT_PRECIS,                   // Clipping precision
        CLEARTYPE_QUALITY,                    // Font quality
        DEFAULT_PITCH | FF_DONTCARE,  // Pitch and family
        L"Arial"                                            // Font name
    );
}


/*
****************************************************************************************
***Functions that change state of controllers according to last system data update***
****************************************************************************************
*/


void ChangeLabels() {
    SetWindowText(percentageLabel, Tools::ConcatByteToConstWCharT(L"Battery Percentage: ", batteryData.batteryPercentage));
    SetWindowText(statusLabel, batteryData.isCharging ? L"Status: Is Charging" : L"Status: Not Charging");
    SetWindowText(timeLabel, Tools::ConvertStringToWCharT(Tools::ConvertBatteryLifeTimeToHumanReadable(batteryData.batteryLifeTime)));
}

void ProcessDropDownChange(HWND hWnd, int dropdownId) {
    int selectedIndex;
    if (dropdownId == ID_REFRESH_DROPDOWN) {
        selectedIndex = SendMessage(refreshDropdown, CB_GETCURSEL, 0, 0);
        auto it = frequencyMap.find(selectedIndex);
        if (it != frequencyMap.end()) {
            SetTimer(hWnd, TIMER, it->second * MILLIS_IN_SEC, NULL);
        }
        else {
            KillTimer(hWnd, TIMER);
        }
    }
    else if (dropdownId == ID_SORT_DROPDOWN) {
        selectedIndex = SendMessage(sortDropdown, CB_GETCURSEL, 0, 0);
        auto it = sortingMap.find(selectedIndex);
        if (it != sortingMap.end()) {
            ProcessData::SortProcessesBySortingMethod(runningProcesses, it->second);
        }
        else {
            ProcessData::SortProcessesBySortingMethod(runningProcesses, sortingMap.at(DEFAULT_DROPDOWN_POS));
        }
        FillProcessTable();
    }
    else if (dropdownId == ID_LOGGER_DROPDOWN) {
        FillLoggerTable(SendMessage(loggerDropdown, CB_GETCURSEL, 0, 0));
    }
}

void ChangeControllersVisability(HWND hWnd, int activeButtonId) {
    lastPressedButton = activeButtonId;
    // Enumerate child windows and hide them
    EnumChildWindows(hWnd, [](HWND hWnd, LPARAM lParam) -> BOOL {
        ShowWindow(hWnd, SW_HIDE);
        return TRUE;
        }, reinterpret_cast<LPARAM>(nullptr));

    switch (activeButtonId) {
    case ID_PROCESS_BUTTON:
        ShowWindow(sortDropdown, SW_SHOW);
        ShowWindow(processTable, SW_SHOW);
        break;
    case ID_LOGGER_BUTTON:
        ShowWindow(loggerTable, SW_SHOW);
        ShowWindow(loggerDropdown, SW_SHOW);
        break;
    case ID_ADDITIONAL_BUTTON:
        ShowWindow(pciTable, SW_SHOW);
        ShowWindow(usbTable, SW_SHOW);
        ShowWindow(driveTable, SW_SHOW);
        ShowWindow(labelArea, SW_SHOW);
        ShowWindow(percentageLabel, SW_SHOW);
        ShowWindow(statusLabel, SW_SHOW);
        ShowWindow(timeLabel, SW_SHOW);
        break;
    }
    SetPanelStates(activeButtonId);
}

//Main function, which is responsible for system data update (works on every timer tick)
void UpdateSystemData(HWND hwnd) {

    //Get all running processes and sort them according to the sort method, chosen by user
    runningProcesses = ProcessData::GetRunningProcesses(overallMemory);
    ProcessData::SortProcessesBySortingMethod(runningProcesses, sortingMap.at(SendMessage(sortDropdown, CB_GETCURSEL, 0, 0)));

    //Get all PCI and USB devices
    pciDevices = DevicesData::GetPCIDevices();
    usbDevices = DevicesData::GetUSBDevices();

    //Get Drives and Battery data 
    drives = DriveData::getDriveData();
    batteryData = BatteryData::getBatteryData();

    //Creating log object for current system parameters
    SystemParameters parameters(time(NULL), batteryData.batteryPercentage, overallMemory);
    //Add log to the log file
    FileProcessing::AppendSystemParametersToFile(parameters, filePath);
    //Read the whole file (with added log including) and store logs as a systemParameters vector
    systemParameters = FileProcessing::ReadSystemParametersFromFile(filePath);

    FillTables();
    ChangeLabels();
}


/*
****************************************************************************************
*************Functions that initialize controllers and set their parameters*************
****************************************************************************************
*/


void AddOptionsForLoggerDropDown() {
    SendMessageW(loggerDropdown, CB_ADDSTRING, 0, (LPARAM)L"Show first logs before");
    SendMessageW(loggerDropdown, CB_ADDSTRING, 0, (LPARAM)L"Show last logs before");
}

void AddOptionsForSortDropDown() {
    SendMessageW(sortDropdown, CB_ADDSTRING, 0, (LPARAM)L"Sort by process name");
    SendMessageW(sortDropdown, CB_ADDSTRING, 0, (LPARAM)L"Sort by process count");
    SendMessageW(sortDropdown, CB_ADDSTRING, 0, (LPARAM)L"Sort by memory usage");
    SendMessageW(sortDropdown, CB_ADDSTRING, 0, (LPARAM)L"Sort by date");
}

void AddOptionsForRefreshDropDown() {
    SendMessageW(refreshDropdown, CB_ADDSTRING, 0, (LPARAM)L"Refresh every 5 second");
    SendMessageW(refreshDropdown, CB_ADDSTRING, 0, (LPARAM)L"Refresh every 30 seconds");
    SendMessageW(refreshDropdown, CB_ADDSTRING, 0, (LPARAM)L"Refresh every minute");
    SendMessageW(refreshDropdown, CB_ADDSTRING, 0, (LPARAM)L"Refresh every 5 minutes");
    SendMessageW(refreshDropdown, CB_ADDSTRING, 0, (LPARAM)L"Refresh every 30 minutes");
    SendMessageW(refreshDropdown, CB_ADDSTRING, 0, (LPARAM)L"Refresh every hour");
    SendMessageW(refreshDropdown, CB_ADDSTRING, 0, (LPARAM)L"Don't refresh");
}

void SetDropDownHeight(int itemHeight) {
    SendMessageW(refreshDropdown, CB_SETITEMHEIGHT, (WPARAM)-1, (LPARAM)itemHeight);
    SendMessageW(sortDropdown, CB_SETITEMHEIGHT, (WPARAM)-1, (LPARAM)itemHeight);
    SendMessageW(loggerDropdown, CB_SETITEMHEIGHT, (WPARAM)-1, (LPARAM)itemHeight);
}

void SetDropDownParameters() {
    SetDropDownHeight(DROPDOWN_HEIGHT);

    AddOptionsForRefreshDropDown();
    AddOptionsForSortDropDown();
    AddOptionsForLoggerDropDown();

    SendMessageW(refreshDropdown, CB_SETCURSEL, 2, 0);
    SendMessageW(sortDropdown, CB_SETCURSEL, 2, 0);
    SendMessageW(loggerDropdown, CB_SETCURSEL, 0, 0);
}

void SetIdsForChildWindows() {
    //Set IDS for buttons
    SetWindowLongPtr(processButton, GWLP_ID, ID_PROCESS_BUTTON);
    SetWindowLongPtr(loggerButton, GWLP_ID, ID_LOGGER_BUTTON);
    SetWindowLongPtr(additionalButton, GWLP_ID, ID_ADDITIONAL_BUTTON);
    //Set IDs for dropdowns
    SetWindowLongPtr(refreshDropdown, GWL_ID, ID_REFRESH_DROPDOWN);
    SetWindowLongPtr(sortDropdown, GWL_ID, ID_SORT_DROPDOWN);
    SetWindowLongPtr(loggerDropdown, GWL_ID, ID_LOGGER_DROPDOWN);
    //Set ProcessTable ID
    SetWindowLongPtr(processTable, GWL_ID, ID_PROCESS_TABLE);
}

void SetTableFonts(HFONT defaultFont) {
    SendMessageW(processTable, WM_SETFONT, reinterpret_cast<WPARAM>(defaultFont), TRUE);
    SendMessageW(loggerTable,   WM_SETFONT, reinterpret_cast<WPARAM>(defaultFont), TRUE);
    SendMessageW(pciTable,         WM_SETFONT, reinterpret_cast<WPARAM>(defaultFont), TRUE);
    SendMessageW(usbTable,        WM_SETFONT, reinterpret_cast<WPARAM>(defaultFont), TRUE);
    SendMessageW(driveTable,      WM_SETFONT, reinterpret_cast<WPARAM>(defaultFont), TRUE);
}

void SetFonts(HWND hWnd) {
    HFONT defaultFont = CreateDefaultFont();
    //Set fonts for hWnd
    SendMessageW(hWnd, WM_SETFONT, reinterpret_cast<WPARAM>(defaultFont), TRUE);
    //Set fonts for hWnd children elements
    SendMessageW(hWnd, WM_SETFONT, reinterpret_cast<WPARAM>(defaultFont), MAKELPARAM(TRUE, 0));
    //Set fonts for tables
    SetTableFonts(defaultFont);
    DeleteObject(defaultFont);
}

BOOL CreateLabels(HWND hWnd, HINSTANCE hInstance) {
    labelArea = CreateCustomLabel(hWnd, hInstance, L"", ADDITIONAL_OFFSET, (PANEL_HEIGHT + DEFAULT_OFFSET) * 3,
        ADDITIONAL_COLUMN_NUMBER_LENGTH + ADDITIONAL_COLUMN_NAME_LENGTH, HUGE_LABEL_LENGTH);
    percentageLabel = CreateCustomLabel(hWnd, hInstance, L"Battery Percentage:", ADDITIONAL_OFFSET + LABEL_OFFSET, (PANEL_HEIGHT + DEFAULT_OFFSET) * 3 +
        LABEL_OFFSET, ADDITIONAL_COLUMN_NAME_LENGTH, SMALL_LABEL_LENGTH);
    statusLabel = CreateCustomLabel(hWnd, hInstance, L"Status:", ADDITIONAL_OFFSET + LABEL_OFFSET, (PANEL_HEIGHT + DEFAULT_OFFSET) * 4.3 +
        LABEL_OFFSET, ADDITIONAL_COLUMN_NAME_LENGTH, SMALL_LABEL_LENGTH);
    timeLabel = CreateCustomLabel(hWnd, hInstance, L"Estimated time left: ", ADDITIONAL_OFFSET + LABEL_OFFSET, (PANEL_HEIGHT + DEFAULT_OFFSET) * 5.6 +
        LABEL_OFFSET, ADDITIONAL_COLUMN_NAME_LENGTH, SMALL_LABEL_LENGTH);
    return (!labelArea || !percentageLabel || !statusLabel || !timeLabel) ? FALSE : TRUE;
}

BOOL CreateDropdowns(HWND hWnd, HINSTANCE hInstance) {
    refreshDropdown = CreateCustomDropdown
        (hWnd, hInstance, (BUTTON_LENGTH - 2) * 3 + DEFAULT_OFFSET * 6, DEFAULT_OFFSET, DROPDOWN_LENGTH, PANEL_HEIGHT);
    sortDropdown = CreateCustomDropdown
        (hWnd, hInstance, (BUTTON_LENGTH - 2) * 3 + DEFAULT_OFFSET * 6, DROPDOWN_HEIGHT * 2.4 + DEFAULT_OFFSET * 2, DROPDOWN_LENGTH, PANEL_HEIGHT);
    loggerDropdown = CreateCustomDropdown
        (hWnd, hInstance, (BUTTON_LENGTH - 2) * 3 + DEFAULT_OFFSET * 6, DROPDOWN_HEIGHT * 2.4 + DEFAULT_OFFSET * 2, DROPDOWN_LENGTH, PANEL_HEIGHT);
    return (!refreshDropdown || !sortDropdown || !loggerDropdown) ? FALSE : TRUE;
}

BOOL CreateButtons(HWND hWnd, HINSTANCE hInstance) {
    processButton = CreateCustomButton
        (hWnd, hInstance, L"Running Processes", DEFAULT_OFFSET, DEFAULT_OFFSET, BUTTON_LENGTH - 2, PANEL_HEIGHT, false);
    loggerButton = CreateCustomButton
        (hWnd, hInstance, L"Logger Data", BUTTON_LENGTH - 2 + DEFAULT_OFFSET * 2, DEFAULT_OFFSET, BUTTON_LENGTH, PANEL_HEIGHT, true);
    additionalButton = CreateCustomButton
        (hWnd, hInstance, L"Additional Data", (BUTTON_LENGTH - 2) * 2 + DEFAULT_OFFSET * 4, DEFAULT_OFFSET, BUTTON_LENGTH, PANEL_HEIGHT, true);
    return (!processButton || !loggerButton || !additionalButton) ? FALSE : TRUE;

}

BOOL CreateChildTables(HWND hWnd) {
    CreateProcessTable(hWnd);
    CreateUsbTable(hWnd);
    CreateDriveTable(hWnd);
    CreatePciTable(hWnd);
    CreateLoggerTable(hWnd);
    return (!processTable || !usbTable || !driveTable || !pciTable || !loggerTable) ? FALSE : TRUE;
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SYSTEMMONITORING, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow)) {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SYSTEMMONITORING));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SYSTEMMONITORING));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    hInst = hInstance; // Store instance handle in our global variable

    //Creating main window
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
    
    //Checking if main window was created properly
    if (!hWnd) {
        return FALSE;
    }

    //Create function return BOOL
    //Creating child windows while checking if they were created properly
    if (!CreateChildTables(hWnd) || 
        !CreateButtons(hWnd, hInstance) ||
        !CreateDropdowns(hWnd, hInstance) ||
        !CreateLabels(hWnd, hInstance)) {
        return FALSE;
    }

    //Changing default font for instances
    SetFonts(hWnd);

    //binding IDS to every child window
    SetIdsForChildWindows();

    //Function:
    //  changes default height
    //  adds options for every dropdown
    //  sets initial dropdown value
    SetDropDownParameters();

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    UpdateSystemData(hWnd);
    ChangeControllersVisability(hWnd, ID_PROCESS_BUTTON);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED) { // Button click event
            ChangeControllersVisability(hWnd, LOWORD(wParam));
        }
        if (HIWORD(wParam) == CBN_SELCHANGE) {
            ProcessDropDownChange(hWnd, LOWORD(wParam));
        }
        break;
    case WM_CREATE:
        SetTimer(hWnd, TIMER, frequencyMap.at(DEFAULT_DROPDOWN_POS) * MILLIS_IN_SEC, NULL);
        break;
    case WM_TIMER:
        UpdateSystemData(hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
