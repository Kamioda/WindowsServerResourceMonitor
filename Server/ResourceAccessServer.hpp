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
public:
	ResourceAccessServer(const Service_CommandLineManager::CommandLineType& args);
	void Service_MainProcess() override;
};
