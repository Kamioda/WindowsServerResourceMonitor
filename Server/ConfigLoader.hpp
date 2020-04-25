#pragma once
#include "MSXMLRead.hpp"
#include <string>

class ConfigLoader {
private:
	mutable MSXMLRead xml;
	std::vector<Node>::const_iterator find(const std::wstring& root) const;
public:
	ConfigLoader() = default;
	ConfigLoader(const std::string& ConfigFilePath);
	int GetNum(const std::string& Section, const std::string& Key, const int DefaultValue) const;
	std::string GetString(const std::string& Section, const std::string& Key, const std::string& Default) const;
};
