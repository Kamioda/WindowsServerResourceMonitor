#include "ResourceAccessServer.hpp"
#include "DllFunctionCallManager.hpp"
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
	const DllFunctionCallManager dll(BaseClass::ChangeFullPath(ini.GetString("dll", "path", ".\\KgOSResDrv.dll")));
	std::string ResourceInformation{};
	this->server.Get(ini.GetString("url", "all", "/v1/").c_str(),
		[](const httplib::Request& req, httplib::Response& res) {

		}
	);
}

ServiceProcess* GetServiceProcessInstance(const Service_CommandLineManager::CommandLineType& args) {
	return new ResourceAccessServer(args);
}
