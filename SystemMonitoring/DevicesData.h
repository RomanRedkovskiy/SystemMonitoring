#pragma

#include <Windows.h>
#include <vector>
#include <string>

class DevicesData
{
public:
	static std::vector<std::string> GetPCIDevices();
	static std::vector<std::string> GetUSBDevices();
};

