#pragma once
#include "ServiceController.h"
#include "ConfigLoader.hpp"
#include <nlohmann/json.hpp>
#include <unordered_map>

class ServiceMonitor : private ServiceController {
private:
	static std::unordered_map<DWORD, std::string> StatusList;
	static std::unordered_map<DWORD, std::string> ServiceTypeList;
	std::string ServiceDisplayName;
	std::string ServiceType;
	std::string ServiceStatus;
	std::string ShowStatus();
	std::string ShowServiceType();
	std::string GetTargetServiceDisplayName();
public:
	ServiceMonitor(ServiceControlManager& SCManager, const std::string& MonitorService);
	static void InitStatusList(ConfigLoader& conf);
	static void InitServiceTypeList(ConfigLoader& conf);
	void Run();
	void Stop();
	void Pause();
	void Continue();
	void Update();
	nlohmann::json Get() const;
	std::string GetKey() const noexcept;
};
