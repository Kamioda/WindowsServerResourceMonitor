#include "ServiceMonitor.hpp"
#include <array>

std::unordered_map<DWORD, std::string> ServiceMonitor::StatusList;

std::unordered_map<DWORD, std::string> ServiceMonitor::ServiceTypeList;

void ServiceMonitor::InitStatusList(ConfigLoader& conf) {
	constexpr int statusNum = 8;
	const std::array<DWORD, statusNum> ServiceStatusList = { SERVICE_RUNNING, SERVICE_STOPPED, SERVICE_PAUSED, SERVICE_START_PENDING, SERVICE_PAUSE_PENDING, SERVICE_CONTINUE_PENDING, SERVICE_STOP_PENDING, 0 };
	const std::array<std::string, statusNum> ServiceStatusLoadKeyList = { "status/running", "status/stopped", "status/paused", "status/startpending", "status/pausepending", "status/continuepending", "status/stoppending", "status/unsupported" };
	const std::array<std::string, statusNum> ServiceStatusDefaultValue = { "RUNNING", "STOPPED", "PAUSED", "START_PENDING", "PAUSE_PENDING", "CONTINUE_PENDING", "STOP_PENDING", "Unsupported status on resource monitor server" };
	for (int i = 0; i < statusNum; i++) StatusList.emplace(std::make_pair(ServiceStatusList[i], conf.GetString("services", ServiceStatusLoadKeyList[i], ServiceStatusDefaultValue[i])));
}

void ServiceMonitor::InitServiceTypeList(ConfigLoader& conf) {
	constexpr int typeNum = 9;
	const std::array<DWORD, typeNum> ServiceTypeNumberList = {
		SERVICE_WIN32, SERVICE_WIN32_OWN_PROCESS, SERVICE_WIN32_SHARE_PROCESS, SERVICE_KERNEL_DRIVER, SERVICE_FILE_SYSTEM_DRIVER,
		SERVICE_ADAPTER, SERVICE_USER_OWN_PROCESS, SERVICE_USER_SHARE_PROCESS, 0
	};
	const std::array<std::string, typeNum> ServiceTypeLoadKeyList = { "type/win32", "type/own", "type/share", "type/kernel", "type/filesys", "type/adapt", "type/userown", "type/usershare", "type/unsupported" };
	const std::array<std::string, typeNum> ServiceTypeDefaultValue = { "Win32", "Win32 Own Process", "Win32 Share Process", "Kernel Driver", "File System", "Adapter", "User Own Process",	"User Share Process", "Unsupported type on resource monitor server" };
	for (int i = 0; i < typeNum; i++) StatusList.emplace(std::make_pair(ServiceTypeNumberList[i], conf.GetString("services", ServiceTypeLoadKeyList[i], ServiceTypeDefaultValue[i])));
}

ServiceMonitor::ServiceMonitor(ServiceControlManager& SCManager, const std::string& MonitorService) 
	: ServiceController(SCManager, MonitorService) {
	ServiceController::Update();
	this->ServiceStatus = this->ShowStatus();
	this->ServiceType = this->ShowServiceType();
	this->ServiceDisplayName = this->GetTargetServiceDisplayName();
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
	ServiceController::Update();
	this->ServiceStatus = this->ShowStatus();
	// サーバー動作中にAPIやsc.exeを使って変更されることも考慮して更新
	this->ServiceType = this->ShowServiceType();
	this->ServiceDisplayName = this->GetTargetServiceDisplayName();
}

nlohmann::json ServiceMonitor::Get() const {
	nlohmann::json json{};
	json["name"] = ServiceController::ServiceName;
	json["display"] = this->ServiceDisplayName;
	json["type"] = this->ServiceType;
	json["status"] = this->ServiceStatus;
	return json;
}

#include "../Common/GetErrorMessage.h"
#include "../Common/CommandLineManager.h"

std::string ServiceMonitor::GetTargetServiceDisplayName() {
	DWORD Size{};
	const std::wstring lpServiceName = CommandLineManagerW::AlignCmdLineStrType(this->ServiceName).c_str();
	GetServiceDisplayNameW(ServiceController::SCM.get(), lpServiceName.c_str(), nullptr, &Size);
	if (Size > 0) {
		std::wstring buf{};
		buf.resize(++Size);
		if (FALSE != GetServiceDisplayNameW(ServiceController::SCM.get(), lpServiceName.c_str(), &buf[0], &Size)) {
			int iBufferSize = WideCharToMultiByte(CP_UTF8, 0, buf.c_str(), -1, NULL, 0, NULL, NULL);
			if (0 == iBufferSize) throw std::runtime_error(GetErrorMessageA());
			iBufferSize++;
			std::string strbuf{};
			strbuf.resize(iBufferSize);
			WideCharToMultiByte(CP_UTF8, 0, buf.c_str(), -1, &strbuf[0], iBufferSize, NULL, NULL);
			strbuf.resize(std::strlen(strbuf.c_str()));
			return strbuf;
		}
	}
	return std::string();
}

std::string ServiceMonitor::GetKey() const noexcept { return ServiceController::ServiceName; }
