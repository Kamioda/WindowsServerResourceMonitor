#pragma once
#include "../Common/MSXMLRead.hpp"
#include <string>

class ConfigLoader {
private:
	MSXML::Read xml;
public:
	ConfigLoader() = default;
	ConfigLoader(const std::string& ConfigFilePath);
	int GetNum(const std::string& route, const int& DefaultValue);
	int GetNum(const std::wstring& route, const int& DefaultValue);
	std::string GetString(const std::string& route, const std::string& Default);
	std::wstring GetString(const std::wstring& route, const std::wstring& Default);
};
