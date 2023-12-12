#pragma once

#include "SystemParameters.h"

#include <string>
#include <vector>

class FileProcessing
{
public:
	static void AppendSystemParametersToFile(const SystemParameters& parameters, const std::string& filename);
	static std::vector<SystemParameters> ReadSystemParametersFromFile(const std::string& filename);
	
};

