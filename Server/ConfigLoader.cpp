#include "ConfigLoader.hpp"
#include "../Common/StringCvt.h"
#include <Windows.h>
constexpr size_t ArrayBufferSize = 1024;
constexpr const wchar_t* CommonRoot = L"configuration/";

ConfigLoader::ConfigLoader(const std::string& ConfigFilePath) : xml(string::converter::stl::from_bytes(ConfigFilePath)) {}

int ConfigLoader::GetNum(const std::string& Section, const std::string& Key, const int DefaultValue) {
	try {
		return this->xml.Get<int>(CommonRoot + string::converter::stl::from_bytes(Section + "/" + Key)).front();
	}
	catch (std::exception) {
		return DefaultValue;
	}
}

std::string ConfigLoader::GetString(const std::string& Section, const std::string& Key, const std::string& Default) {
	try {
		return this->xml.Get<std::string>(CommonRoot + string::converter::stl::from_bytes(Section + "/" + Key)).front();
	}
	catch (std::exception) {
		return Default;
	}

}

std::wstring ConfigLoader::GetString(const std::wstring& Section, const std::wstring& Key, const std::wstring& Default) {
	try {
		return this->xml.Get<std::wstring>(CommonRoot + Section + L"/" + Key).front();
	}
	catch (std::exception) {
		return Default;
	}

}
