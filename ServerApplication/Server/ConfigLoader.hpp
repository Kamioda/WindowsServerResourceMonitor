#pragma once
#include "../Common/MSXMLRead.hpp"
#include "../Common/StringCvt.h"
#include "json.hpp"
#include <string>
#include <type_traits>
#include <fstream>
#include <filesystem>

class ConfigLoader {
private:
	MSXML::Read xml;
public:
	ConfigLoader() = default;
	ConfigLoader(const std::string& ConfigFilePath) : xml(string::converter::stl::from_bytes(ConfigFilePath)) {}
	template<std::integral T>
	T GetNum(const std::string& route, const T& DefaultValue) {
		try {
			return this->xml.Get<int>(string::converter::stl::from_bytes(route)).front();
		}
		catch (std::exception) {
			return DefaultValue;
		}
	}
	template<std::integral T>
	int GetNum(const std::wstring& route, const int& DefaultValue) {
		try {
			return this->xml.Get<int>(route).front();
		}
		catch (std::exception) {
			return DefaultValue;
		}
	}
	std::string GetString(const std::string& route, const std::string& Default) {
		try {
			return this->xml.Get<std::string>(string::converter::stl::from_bytes(route)).front();
		}
		catch (std::exception) {
			return Default;
		}
	}
	std::wstring GetString(const std::wstring& route, const std::wstring& Default) {
		try {
			return this->xml.Get<std::wstring>(route).front();
		}
		catch (std::exception) {
			return Default;
		}
	}
private:
	static nlohmann::json GetJson(const std::filesystem::path& JsonFile) {
		if (std::filesystem::status(JsonFile).type() != std::filesystem::file_type::regular)
			throw std::runtime_error(JsonFile.string() + " : File is not found.");
		std::ifstream ifs(JsonFile);
		return nlohmann::json::parse(std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>()));
	}
public:
	nlohmann::json GetJson(const std::string& route, const std::string& DefaultFile) {
		return GetJson(this->GetString(route, DefaultFile));
	}
	nlohmann::json GetJson(const std::wstring& route, const std::wstring& DefaultFile) {
		return GetJson(this->GetString(route, DefaultFile));
	}
};
