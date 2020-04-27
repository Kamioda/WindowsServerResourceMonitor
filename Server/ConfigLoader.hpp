#pragma once
#include "MSXMLRead.hpp"
#include <string>

class ConfigLoader {
private:
	MSXML::Read xml;
public:
	ConfigLoader() = default;
	ConfigLoader(const std::string& ConfigFilePath);
	int GetNum(const std::string& Section, const std::string& Key, const int DefaultValue);
	std::string GetString(const std::string& Section, const std::string& Key, const std::string& Default);
};
