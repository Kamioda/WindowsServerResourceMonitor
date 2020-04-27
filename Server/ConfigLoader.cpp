#include "ConfigLoader.hpp"
#include "../Common/CommandLineManager.h"
#include <Windows.h>
constexpr size_t ArrayBufferSize = 1024;
constexpr const wchar_t* CommonRoot = L"configuration/";

ConfigLoader::ConfigLoader(const std::string& ConfigFilePath) : xml(CommandLineManagerW::AlignCmdLineStrType(ConfigFilePath)) {}

int ConfigLoader::GetNum(const std::string& Section, const std::string& Key, const int DefaultValue) {
	try {
		const std::wstring TargetRoot = CommonRoot + CommandLineManagerW::AlignCmdLineStrType(Section + "/" + Key);
		return this->xml.Get<int>(TargetRoot).front();
	}
	catch (std::exception) {
		return DefaultValue;
	}
}

std::string ConfigLoader::GetString(const std::string& Section, const std::string& Key, const std::string& Default) {
	try {
		const std::wstring TargetRoot = CommonRoot + CommandLineManagerW::AlignCmdLineStrType(Section + "/" + Key);
		return this->xml.Get<std::string>(TargetRoot).front();
	}
	catch (std::exception) {
		return Default;
	}

}
