#include "ResourceAccessServer.hpp"
#include <filesystem>
constexpr size_t ArrayBufferSize = 1024;

class IniRead {
private:
	std::string IniFilePath;
public:
	IniRead() = default;
	IniRead(const std::string& IniFilePath) : IniFilePath(IniFilePath) {}
	int GetNum(const std::string& Section, const std::string& Key, const int DefaultValue) const {
		return GetPrivateProfileIntA(Section.c_str(), Key.c_str(), DefaultValue, this->IniFilePath.c_str());
	}
	std::string GetString(const std::string& Section, const std::string& Key, const std::string& Default) const {
		char Buffer[ArrayBufferSize];
		GetPrivateProfileStringA(Section.c_str(), Key.c_str(), Default.c_str(), Buffer, ArrayBufferSize, this->IniFilePath.c_str());
		return std::string(Buffer);
	}
};

ResourceAccessServer::ResourceAccessServer(const Service_CommandLineManager::CommandLineType& args)
	: ServiceProcess(args), server() {}

void ResourceAccessServer::Service_MainProcess() {
	IniRead ini(BaseClass::ChangeFullPath(".\\server.ini"));
	std::string ResourceInformation{};
	this->server.Get(ini.GetString("url", "all", "/v1/").c_str(),
		[&](const httplib::Request& req, httplib::Response& res) {

		}
	);
	this->server.Get(ini.GetString("url", "cpu", "/v1/cpu").c_str(),
		[&](const httplib::Request& req, httplib::Response& res) {

		}
	);
	this->server.Get(ini.GetString("url", "memory", "/v1/mem").c_str(),
		[&](const httplib::Request& req, httplib::Response& res) {

		}
	);
	this->server.Get(ini.GetString("url", "storage", "/v1/disk/[A-Z]").c_str(),
		[&](const httplib::Request& req, httplib::Response& res) {
			const std::string drive = req.matches[1].str() + ":";

		}
	);
	this->server.Get(ini.GetString("url", "network", "/v1/network").c_str(),
		[&](const httplib::Request& req, httplib::Response& res) {

		}
	);
	server.listen(ini.GetString("url", "domain", "localhost").c_str(), ini.GetNum("url", "port", 8080), 0, 
		[&]() {
			
		}
	);
}

ServiceProcess* GetServiceProcessInstance(const Service_CommandLineManager::CommandLineType& args) {
	return new ResourceAccessServer(args);
}
