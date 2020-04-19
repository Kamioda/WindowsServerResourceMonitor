#include "ServiceMonitor.hpp"

std::unordered_map<DWORD, std::string> ServiceMonitor::StatusList;

void ServiceMonitor::InitStatusList(const IniRead& ini) {
	const DWORD ServiceStatusList[7] = { SERVICE_RUNNING, SERVICE_STOPPED, SERVICE_PAUSED, SERVICE_START_PENDING, SERVICE_PAUSE_PENDING, SERVICE_CONTINUE_PENDING, SERVICE_STOP_PENDING };
	const std::string ServiceStatusLoadKeyList[7] = { "running", "stopped", "paused", "startpending", "pausepending", "continuepending", "stoppending" };
	const std::string ServiceStatusDefaultValue[7] = { "RUNNING", "STOPPED", "PAUSED", "START_PENDING", "PAUSE_PENDING", "CONTINUE_PENDING", "STOP_PENDING" };
	for (int i = 0; i < 7; i++) StatusList.emplace(std::make_pair(ServiceStatusList[i], ini.GetString("services", ServiceStatusLoadKeyList[i], ServiceStatusDefaultValue[i])));
}

ServiceMonitor::ServiceMonitor(const std::string& MonitorService) : ServiceController(MonitorService) {}

void ServiceMonitor::Run() { ServiceController::Run(); }

void ServiceMonitor::Stop() { ServiceController::Stop(); }

void ServiceMonitor::Pause() { ServiceController::Pause(); }

void ServiceMonitor::Continue() { ServiceController::Continue(); }

std::string ServiceMonitor::Show() { return StatusList.at(ServiceController::Show()); }
