#pragma once
#include "Auth.hpp"
#include "ComInitManager.hpp"
#pragma warning(push)
#pragma warning(disable: 4100)
#pragma warning(disable: 4324)
#pragma warning(disable: 4456)
#pragma warning(disable: 4458)
#pragma warning(disable: 4701)
#pragma warning(disable: 4706)
#include <uwebsockets/App.h>
#pragma warning(pop)
#include "ServiceMainProcess.h"
#include "ResourceManager.hpp"
#include "ConfigLoader.hpp"
#include <memory>
#include <mutex>

class ResourceAccessServer : public ServiceProcess {
private:
	ComInitManager commgr;
	ConfigLoader conf;
	ResourceManager resource;
	AuthManager auth;
	using BaseClass = ServiceProcess;
	us_listen_socket_t* ListenSocket;
	uWS::App app;
public:
	ResourceAccessServer(const Service_CommandLineManager::CommandLineType& args);
	void Service_MainProcess() override;
	uWS::App& GetApp() noexcept;
	ResourceManager& GetResources() noexcept;
	us_listen_socket_t* GetSocket() const noexcept;
};
