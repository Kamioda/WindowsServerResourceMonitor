#pragma once
#include "ServiceMainProcess.h"
#include "httplib.h"

class ResourceAccessServer : public ServiceProcess {
private:
	httplib::Server server;
	using BaseClass = ServiceProcess;
public:
	ResourceAccessServer(const Service_CommandLineManager::CommandLineType& args);
	void Service_MainProcess() override;
};
