#pragma once
#include <string>

class IniRead {
private:
	std::string IniFilePath;
public:
	IniRead() = default;
	IniRead(const std::string& IniFilePath) : IniFilePath(IniFilePath) {}
	int GetNum(const std::string& Section, const std::string& Key, const int DefaultValue) const;
	std::string GetString(const std::string& Section, const std::string& Key, const std::string& Default) const;
};
