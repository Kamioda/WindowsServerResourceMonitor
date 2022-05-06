#include "ConfigLoader.hpp"
#include "../Common/StringCvt.h"
#include <Windows.h>
#include <fstream>
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

nlohmann::json ConfigLoader::GetJson(const std::filesystem::path& JsonFile) {
	if (std::filesystem::status(JsonFile).type() != std::filesystem::file_type::regular)
		throw std::runtime_error(JsonFile.string() + " : File is not found.");
	std::ifstream ifs(JsonFile);
	return nlohmann::json::parse(std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>()));
}

nlohmann::json ConfigLoader::GetJson(const std::string& route, const std::string& DefaultFile) {
	return GetJson(this->GetString(route, DefaultFile));
}

nlohmann::json ConfigLoader::GetJson(const std::wstring& route, const std::wstring& DefaultFile) {
	return GetJson(this->GetString(route, DefaultFile));
}
