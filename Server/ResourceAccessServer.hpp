#pragma once
#include "httplib.h"
#include "ServiceMainProcess.h"
#include "ServiceMonitor.hpp"
#include "Processor.hpp"
#include "Memory.hpp"
#include "Disk.hpp"
#include "Network.hpp"
#include "IniRead.hpp"
#include <unordered_map>

class ResourceAccessServer : public ServiceProcess {
private:
	ServiceControlManager SCM;
	IniRead ini;
	Processor processor;
	Memory memory;
	std::unordered_map<std::string, Disk> disk;
	std::vector<Network> network;
	std::unordered_map<std::string, ServiceMonitor> services;
	httplib::Server server;
	DWORD looptime;
	using BaseClass = ServiceProcess;
	std::string GetConfStr(const std::string& Section, const std::string& Key, const std::string& Default) const;
	int GetConfInt(const std::string& Section, const std::string& Key, const int& Default) const;
	picojson::object AllResourceToObject() const;
	picojson::object AllDiskResourceToObject() const;
	picojson::object AllNetworkResourceToObject() const;
	picojson::object AllServiceToObject() const;
	void UpdateResources();
public:
	ResourceAccessServer(const Service_CommandLineManager::CommandLineType& args);
	void Service_MainProcess() override;
};
