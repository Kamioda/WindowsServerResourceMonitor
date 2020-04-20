#pragma once
#include "ServiceController.h"
#include "IniRead.hpp"
#include <unordered_map>

class ServiceMonitor : private ServiceController {
private:
	static std::unordered_map<DWORD, std::string> StatusList;
	static std::unordered_map<DWORD, std::string> ServiceTypeList;
	std::string ServiceType;
	std::string ServiceStatus;
	std::string Show();
public:
	ServiceMonitor(const std::string& MonitorService);
	static void InitStatusList(const IniRead& ini);
	static void InitServiceTypeList(const IniRead& ini);
	void Run();
	void Stop();
	void Pause();
	void Continue();
	void Update();
};
