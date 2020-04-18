#include "ResourceAccessServer.hpp"
#include "ServiceStatus.h"
#include "Split.hpp"
#include "JsonObject.hpp"

ServiceProcess* GetServiceProcessInstance(const Service_CommandLineManager::CommandLineType& args) {
	return new ResourceAccessServer(args);
}

inline std::unordered_map<std::string, Disk> GetDiskResourceInformations(const IniRead& ini) {
	std::unordered_map<std::string, Disk> RetVal{};
	const std::vector<std::string> DriveList = SplitString(ini.GetString("resource", "drives", "C:"), ',');
	for (const auto& i : DriveList) {
		try {
			RetVal.emplace(std::make_pair(i, Disk(i)));
		}
		catch(std::exception&) { // ないドライブの時にエラー起こすのでここでcatch
			
		}
	}
}

ResourceAccessServer::ResourceAccessServer(const Service_CommandLineManager::CommandLineType& args)
	: ServiceProcess(args), server() {
	SvcStatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
	SetServiceStatusInfo();
}

void ResourceAccessServer::Service_MainProcess() {
	// using

	using Req = const httplib::Request&;
	using Res = httplib::Response&;

	// 変数とラムダ

	IniRead ini(BaseClass::ChangeFullPath(".\\server.ini"));
	auto GetConfStr = [&ini](const std::string& Section, const std::string& Key, const std::string& Default) { return ini.GetString(Section, Key, Default).c_str(); };
	auto GetConfInt = [&ini](const std::string& Section, const std::string& Key, const int& Default) { return ini.GetNum(Section, Key, Default); };
	Processor processor{};
	Memory memory{};
	std::unordered_map<std::string, Disk> disk = GetDiskResourceInformations(ini);
	Network network(GetConfStr("system", "netdevice", "Realtek PCIe GBE Family Controller"));
	auto reqproc = [](Res res, const std::function<void()>& process) {
		try { process(); }
		catch (...) { res.status = 500; }
	};
	auto ChangeSvcStatus = [](const DWORD NewStatus) {
		SvcStatus.dwCurrentState = NewStatus;
		SetServiceStatusInfo();
	};
	auto alljsonobj = [&]() {
		JsonObject obj{};
		JsonObject diskinfo{};
		obj.insert("cpu", processor.Get());
		obj.insert("memory", memory.Get());
		for (const auto& i : disk) diskinfo.insert(i.first.substr(0, 1), i.second.Get());
		obj.insert("disk", diskinfo);
		obj.insert("network", network.Get());
		return obj;
	};
	auto json = [](const picojson::object& obj) { return picojson::value(obj).to_str(); };

	// メインプロセス
	while (SvcStatus.dwCurrentState != SERVICE_STOP_PENDING) {
		if (SvcStatus.dwCurrentState == SERVICE_PAUSED)	Sleep(1000);
		else {
			this->server.Get(GetConfStr("url", "all", "/v1/"), [&](Req, Res res) { reqproc(res, [&] { res.set_content(json(alljsonobj()), "text/json"); }); });
			this->server.Get(GetConfStr("url", "cpu", "/v1/cpu"), [&](Req, Res res) { reqproc(res, [&] { res.set_content(json(processor.Get()), "text/json"); }); });
			this->server.Get(GetConfStr("url", "memory", "/v1/mem"), [&](Req, Res res) { reqproc(res, [&] { res.set_content(json(memory.Get()), "text/json"); }); });
			this->server.Get(GetConfStr("url", "storage", "/v1/disk/[A-Z]"),
				[&](Req req, Res res) {
					reqproc(res, 
						[&] {
							if (const std::string drive = req.matches[1].str() + ":"; disk.find(drive) == disk.end()) res.status = 404;
							else res.set_content(json(disk.at(drive).Get()), "text/json");
						}
					);
				}
			);
			this->server.Get(GetConfStr("url", "network", "/v1/network"), [&](Req, Res res) { reqproc(res, [&] { res.set_content(json(network.Get()), "text/json"); }); });
			this->server.listen(GetConfStr("url", "domain", "localhost"), GetConfInt("url", "port", 8080), 0,
				[&]{
					switch (SvcStatus.dwCurrentState) {
						case SERVICE_START_PENDING:
						case SERVICE_CONTINUE_PENDING:
							processor.Update();
							memory.Update();
							for (const auto& i : disk) i.second.Update();
							network.Update();
							ChangeSvcStatus(SERVICE_RUNNING);
							break;
						case SERVICE_RUNNING:
							processor.Update();
							memory.Update();
							for (const auto& i : disk) i.second.Update();
							network.Update();
							break;
						case SERVICE_STOP_PENDING:
						case SERVICE_PAUSE_PENDING:
							this->server.stop();
							break;
					}
				}
			);	
			if (SvcStatus.dwCurrentState == SERVICE_PAUSE_PENDING) ChangeSvcStatus(SERVICE_PAUSED);
		}
	}
	SvcStatus.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatusInfo();
}
