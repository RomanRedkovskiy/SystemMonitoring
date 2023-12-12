#include "FileProcessing.h"

#include "Tools.h"
#include "SystemParameters.h"

#include <fstream>
#include <string>

size_t skipUntilSpaceOrEndOfLine(const std::string& line, size_t currentIndex) {
    while (currentIndex < line.length() && line[currentIndex] != ' ') {
        ++currentIndex;
    }
    return currentIndex;
}

SystemParameters parseLineIntoSystemParameter(std::string line) {
    size_t i = 0;

    i = skipUntilSpaceOrEndOfLine(line, i);
    std::string timeStr = line.substr(0, i);
    time_t time = static_cast<time_t>(std::stoll(timeStr));

    i = skipUntilSpaceOrEndOfLine(line, i + 1);
    std::string batteryStr = line.substr(timeStr.length() + 1, i - (timeStr.length() + 1));
    BYTE batteryPercentage = static_cast<unsigned char>(std::stoi(batteryStr));

    std::string memoryStr = line.substr(timeStr.length() + batteryStr.length() + 2);
    SIZE_T memoryUsage = static_cast<size_t>(std::stoull(memoryStr));

    return SystemParameters(time, batteryPercentage, memoryUsage);
}

std::vector<SystemParameters> FileProcessing::ReadSystemParametersFromFile(const std::string& filename) {
    std::vector<SystemParameters> systemParameters;
    std::ifstream inFile(filename);
    std::string line;

    if (inFile.is_open()) {
        while (std::getline(inFile, line)) {
            systemParameters.push_back(parseLineIntoSystemParameter(line));
        }
        inFile.close();
    }

    return systemParameters;
}

void FileProcessing::AppendSystemParametersToFile(const SystemParameters& parameters, const std::string& filename) {
    std::ofstream outFile(filename, std::ios::binary | std::ios::app);
    std::string time = std::to_string(parameters.time);
    std::string percentage = Tools::ConvertByteToString(parameters.batteryPercentage);
    std::string memory = Tools::ConvertSizeTToString(parameters.memoryUsage);
    if (outFile.is_open()) {
        outFile.write(time.c_str(), time.length());  // Write the string to the file
        outFile.write(" ", 1);  // Write a space to the file
        outFile.write(percentage.c_str(), percentage.length());  // Write the string to the file
        outFile.write(" ", 1);  // Write a space to the file
        outFile.write(memory.c_str(), memory.length());  // Write the string to the file
        outFile.write("\n", 1);  // Write a new line to the file
        outFile.close();
    }
}