#include "ResourceAccessServer.hpp"
#include "Base64Converter.hpp"
#include "ServiceStatus.h"
#include "Split.hpp"
#include <concepts>
#include <algorithm>
#include <chrono>

std::unique_ptr<ServiceProcess> GetServiceProcessInstance(const Service_CommandLineManager::CommandLineType& args) {
#if defined(_DEBUG) && !defined(CONSOLE)
	Sleep(10000);
#endif
	return std::make_unique<ResourceAccessServer>(args);
}

ResourceAccessServer::ResourceAccessServer(const Service_CommandLineManager::CommandLineType& args)
	: ServiceProcess(args), commgr(),
	conf(BaseClass::ChangeFullPath(".\\server.xml")),
	resource(conf.GetJson("configuration/resourcefile", ".\\auth.json")),
	auth(conf.GetJson("configuration/authfile", ".\\resources.json")) {
	SvcStatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
	SetServiceStatusInfo();
}

void ResourceAccessServer::Service_MainProcess() {
	auto ChangeSvcStatus = [](const DWORD NewStatus) {
		SvcStatus.dwCurrentState = NewStatus;
		SetServiceStatusInfo();
	};
	while (SvcStatus.dwCurrentState != SERVICE_STOP_PENDING) {
		
	}
	ChangeSvcStatus(SERVICE_STOPPED); // 停止はサーバーが完全に停止してから停止扱いにするためここで変更
}
