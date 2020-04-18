#include "IniRead.hpp"
#include <Windows.h>
constexpr size_t ArrayBufferSize = 1024;

int IniRead::GetNum(const std::string& Section, const std::string& Key, const int DefaultValue) const {
	return GetPrivateProfileIntA(Section.c_str(), Key.c_str(), DefaultValue, this->IniFilePath.c_str());
}
std::string IniRead::GetString(const std::string& Section, const std::string& Key, const std::string& Default) const {
	char Buffer[ArrayBufferSize];
	GetPrivateProfileStringA(Section.c_str(), Key.c_str(), Default.c_str(), Buffer, ArrayBufferSize, this->IniFilePath.c_str());
	return std::string(Buffer);
}
