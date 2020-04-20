﻿#include "ServiceMonitor.hpp"
#include "JsonObject.hpp"

std::unordered_map<DWORD, std::string> ServiceMonitor::StatusList;

std::unordered_map<DWORD, std::string> ServiceMonitor::ServiceTypeList;

void ServiceMonitor::InitStatusList(const IniRead& ini) {
	const DWORD ServiceStatusList[8] = { SERVICE_RUNNING, SERVICE_STOPPED, SERVICE_PAUSED, SERVICE_START_PENDING, SERVICE_PAUSE_PENDING, SERVICE_CONTINUE_PENDING, SERVICE_STOP_PENDING, 0 };
	const std::string ServiceStatusLoadKeyList[8] = { "status-running", "status-stopped", "status-paused", "status-startpending", "status-pausepending", "status-continuepending", "status-stoppending", "status-unsupported" };
	const std::string ServiceStatusDefaultValue[8] = { "RUNNING", "STOPPED", "PAUSED", "START_PENDING", "PAUSE_PENDING", "CONTINUE_PENDING", "STOP_PENDING", "Unsupported status on resource monitor server" };
	for (int i = 0; i < 8; i++) StatusList.emplace(std::make_pair(ServiceStatusList[i], ini.GetString("services", ServiceStatusLoadKeyList[i], ServiceStatusDefaultValue[i])));
}

void ServiceMonitor::InitServiceTypeList(const IniRead& ini) {
	const DWORD ServiceTypeList[8] = {
		SERVICE_WIN32_OWN_PROCESS, 	SERVICE_WIN32_SHARE_PROCESS, SERVICE_KERNEL_DRIVER, SERVICE_FILE_SYSTEM_DRIVER,
		SERVICE_ADAPTER, SERVICE_USER_OWN_PROCESS, SERVICE_USER_SHARE_PROCESS, 0
	};
	const std::string ServiceTypeLoadKeyList[8] = { "type-own", "type-share", "type-kernel", "type-filesys", "type-adapt", "type-userown", "type-usershare", "type-unsupported" };
	const std::string ServiceTypeDefaultValue[8] = { "Win32 Own Process", "Win32 Share Process", "Kernel Driver", "File System", "Adapter", "User Own Process",	"User Share Process", "Unsupported type on resource monitor server" };
	for (int i = 0; i < 8; i++) StatusList.emplace(std::make_pair(ServiceTypeList[i], ini.GetString("services", ServiceTypeLoadKeyList[i], ServiceTypeDefaultValue[i])));
}

ServiceMonitor::ServiceMonitor(const std::string& MonitorService) 
	: ServiceController(MonitorService) {
	ServiceController::Update();
	this->ServiceStatus = this->ShowStatus();
	this->ServiceType = this->ShowServiceType();
}

void ServiceMonitor::Run() { ServiceController::Run(); }

void ServiceMonitor::Stop() { ServiceController::Stop(); }

void ServiceMonitor::Pause() { ServiceController::Pause(); }

void ServiceMonitor::Continue() { ServiceController::Continue(); }

std::string ServiceMonitor::ShowStatus() { 
	const DWORD Code = StatusList.find(ServiceController::Status.dwCurrentState) == StatusList.end() ? 0 : ServiceController::Status.dwCurrentState;
	return StatusList.at(Code);
}

std::string ServiceMonitor::ShowServiceType() {
	const DWORD Code = StatusList.find(ServiceController::Status.dwServiceType) == StatusList.end() ? 0 : ServiceController::Status.dwServiceType;
	return StatusList.at(Code);
}

void ServiceMonitor::Update() {
	this->Update();
	this->ServiceStatus = this->ShowStatus();
	this->ServiceType = this->ShowServiceType();
}

picojson::object ServiceMonitor::Get() const {
	JsonObject obj{};
	obj.insert("name", ServiceController::ServiceName);
	obj.insert("type", this->ServiceType);
	obj.insert("status", this->ServiceStatus);
	return obj;
}
