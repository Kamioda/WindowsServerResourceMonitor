#pragma once
#include "ServiceMainProcess.h"
#include "httplib.h"
#include "Processor.hpp"
#include "Memory.hpp"
#include "Disk.hpp"
#include "Network.hpp"
#include "IniRead.hpp"
#include <unordered_map>

class ResourceAccessServer : public ServiceProcess {
private:
	IniRead ini;
	Processor processor;
	Memory memory;
	std::unordered_map<std::string, Disk> disk;
	std::vector<Network> network;
	httplib::Server server;
	using BaseClass = ServiceProcess;
	std::string GetConfStr(const std::string& Section, const std::string& Key, const std::string& Default) const;
	int GetConfInt(const std::string& Section, const std::string& Key, const int& Default) const;
	picojson::object AllResourceToObject() const;
	picojson::object AllDiskResourceToObject() const;
	picojson::object AllNetworkResourceToObject() const;
	void UpdateResources();
public:
	ResourceAccessServer(const Service_CommandLineManager::CommandLineType& args);
	void Service_MainProcess() override;
};
