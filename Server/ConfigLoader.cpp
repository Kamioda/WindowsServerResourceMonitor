#include "ConfigLoader.hpp"
#include "CommandLineManager.h"
#include <Windows.h>
constexpr size_t ArrayBufferSize = 1024;
constexpr const wchar_t* CommonRoot = L"configuration/";

ConfigLoader::ConfigLoader(const std::string& ConfigFilePath) : xml(CommandLineManagerW::AlignCmdLineStrType(ConfigFilePath)) {}

std::vector<Node>::const_iterator ConfigLoader::find(const std::wstring& root) const {
	return std::find_if(this->xml.begin(), this->xml.end(), [&root](const Node& n) { return n.NodePath == root; });
}

int ConfigLoader::GetNum(const std::string& Section, const std::string& Key, const int DefaultValue) const {
	try {
		const std::wstring TargetRoot = CommonRoot + CommandLineManagerW::AlignCmdLineStrType(Section + "/" + Key);
		if (this->find(TargetRoot) == this->xml.end()) this->xml.Load(TargetRoot);
		return std::stoi(this->xml[TargetRoot][0]);
	}
	catch (std::exception) {
		return DefaultValue;
	}
}

std::string ConfigLoader::GetString(const std::string& Section, const std::string& Key, const std::string& Default) const {
	try {
		const std::wstring TargetRoot = CommonRoot + CommandLineManagerW::AlignCmdLineStrType(Section + "/" + Key);
		if (this->find(TargetRoot) == this->xml.end()) this->xml.Load(TargetRoot);
		std::string str{};
		str.resize(1024);
		str = CommandLineManagerA::AlignCmdLineStrType(this->xml[TargetRoot][0]);
		str.resize(std::strlen(str.c_str()));
		return str;
	}
	catch (std::exception) {
		return Default;
	}

}
