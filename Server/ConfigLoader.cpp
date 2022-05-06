#include "ConfigLoader.hpp"
#include "../Common/StringCvt.h"
#include <Windows.h>
constexpr size_t ArrayBufferSize = 1024;

ConfigLoader::ConfigLoader(const std::string& ConfigFilePath) : xml(string::converter::stl::from_bytes(ConfigFilePath)) {}

int ConfigLoader::GetNum(const std::string& route, const int& DefaultValue) {
	try {
		return this->xml.Get<int>(string::converter::stl::from_bytes(route)).front();
	}
	catch (std::exception) {
		return DefaultValue;
	}
}

int ConfigLoader::GetNum(const std::wstring& route, const int& DefaultValue) {
	try {
		return this->xml.Get<int>(route).front();
	}
	catch (std::exception) {
		return DefaultValue;
	}
}

std::string ConfigLoader::GetString(const std::string& route, const std::string& Default) {
	try {
		return this->xml.Get<std::string>(string::converter::stl::from_bytes(route)).front();
	}
	catch (std::exception) {
		return Default;
	}

}

std::wstring ConfigLoader::GetString(const std::wstring& route, const std::wstring& Default) {
	try {
		return this->xml.Get<std::wstring>(route).front();
	}
	catch (std::exception) {
		return Default;
	}

}
