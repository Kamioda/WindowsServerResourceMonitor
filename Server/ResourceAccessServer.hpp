#pragma once
#include "Auth.hpp"
#include "ComInitManager.hpp"
#include "ServiceMainProcess.h"
#include "ResourceManager.hpp"
#include "ConfigLoader.hpp"

class ResourceAccessServer : public ServiceProcess {
private:
	ComInitManager commgr;
	ConfigLoader conf;
	ResourceMonitor resource;
	AuthManager auth;
	using BaseClass = ServiceProcess;
	std::string GetConfStr(const std::string& Section, const std::string& Key, const std::string& Default);
	std::wstring GetConfStr(const std::wstring& Section, const std::wstring& Key, const std::wstring& Default);
	int GetConfInt(const std::string& Section, const std::string& Key, const int& Default);
public:
	ResourceAccessServer(const Service_CommandLineManager::CommandLineType& args);
	void Service_MainProcess() override;
};
