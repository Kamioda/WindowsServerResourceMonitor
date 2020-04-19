#pragma once
#include "ServiceController.h"
#include "IniRead.hpp"
#include <unordered_map>

class ServiceMonitor : private ServiceController {
private:
	static std::unordered_map<DWORD, std::string> StatusList;
public:
	ServiceMonitor(const std::string& MonitorService);
	static void InitStatusList(const IniRead& ini);
	void Run();
	void Stop();
	void Pause();
	void Continue();
	std::string Show();
};
