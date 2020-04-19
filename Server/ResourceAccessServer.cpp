#include "ResourceAccessServer.hpp"
#include "ServiceStatus.h"
#include "Split.hpp"
#include "JsonObject.hpp"
using Req = const httplib::Request&;
using Res = httplib::Response&;

ServiceProcess* GetServiceProcessInstance(const Service_CommandLineManager::CommandLineType& args) {
	return new ResourceAccessServer(args);
}

inline std::unordered_map<std::string, Disk> GetDiskResourceInformations(const IniRead& ini) {
	std::unordered_map<std::string, Disk> RetVal{};
	const std::vector<std::string> DriveList = SplitString(ini.GetString("system", "drives", "C:"), ',');
	for (const auto& i : DriveList) {
		try {
			RetVal.emplace(std::make_pair(i, Disk(i)));
		}
		catch(std::exception&) { // ないドライブの時にエラー起こすのでここでcatch
			
		}
	}
	return RetVal;
}

inline std::string ToJsonText(const picojson::object& obj) { 
	std::stringstream ss{};
	ss << picojson::value(obj);
	return ss.str();
}

inline void reqproc(Res res, const std::function<void()>& process) {
	try { process(); }
	catch (...) { res.status = 500; }
};
ResourceAccessServer::ResourceAccessServer(const Service_CommandLineManager::CommandLineType& args)
	: ServiceProcess(args), 
	ini(BaseClass::ChangeFullPath(".\\server.ini")), 
	processor(), 
	memory(), 
	disk(GetDiskResourceInformations(this->ini)), 
	network(GetConfStr("system", "netdevice", "Realtek PCIe GBE Family Controller")), 
	server() {
	SvcStatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
	SetServiceStatusInfo();
	this->server.Get(GetConfStr("url", "all", "/v1/").c_str(), [&](Req, Res res) { reqproc(res, [&] { res.set_content(ToJsonText(this->AllResourceToObject()), "text/json"); }); });
	this->server.Get(GetConfStr("url", "cpu", "/v1/cpu").c_str(), [&](Req, Res res) { reqproc(res, [&] { res.set_content(ToJsonText(this->processor.Get()), "text/json"); }); });
	this->server.Get(GetConfStr("url", "memory", "/v1/mem").c_str(), [&](Req, Res res) { reqproc(res, [&] { res.set_content(ToJsonText(this->memory.Get()), "text/json"); }); });
	this->server.Get(GetConfStr("url", "storage", "/v1/disk/[A-Z]").c_str(),
		[&](Req req, Res res) {
			reqproc(res,
				[&] {
					if (const std::string drive = req.matches[1].str() + ":"; this->disk.find(drive) == this->disk.end()) res.status = 404;
					else res.set_content(ToJsonText(this->disk.at(drive).Get()), "text/json");
				}
			);
		}
	);
	this->server.Get(GetConfStr("url", "network", "/v1/network").c_str(), [&](Req, Res res) { reqproc(res, [&] { res.set_content(ToJsonText(network.Get()), "text/json"); }); });
	this->server.Post(GetConfStr("url", "stop", "/v1/stop").c_str(), [](Req, Res) { SvcStatus.dwCurrentState = SERVICE_STOP_PENDING; });
	this->server.Post(GetConfStr("url", "pause", "/v1/pause").c_str(), [](Req, Res) { SvcStatus.dwCurrentState = SERVICE_PAUSE_PENDING; });
}

std::string ResourceAccessServer::GetConfStr(const std::string& Section, const std::string& Key, const std::string& Default) const { return this->ini.GetString(Section, Key, Default); };

int ResourceAccessServer::GetConfInt(const std::string& Section, const std::string& Key, const int& Default) const { return this->ini.GetNum(Section, Key, Default); };

picojson::object ResourceAccessServer::AllResourceToObject() const {
	JsonObject obj{};
	JsonObject diskinfo{};
	obj.insert("cpu", this->processor.Get());
	obj.insert("memory", this->memory.Get());
	for (const auto& i : this->disk) diskinfo.insert(i.first.substr(0, 1), i.second.Get());
	obj.insert("disk", diskinfo);
	obj.insert("network", this->network.Get());
	return obj;
}

void ResourceAccessServer::UpdateResources() {
	this->processor.Update();
	this->memory.Update();
	for (const auto& i : this->disk) i.second.Update();
	this->network.Update();
}

void ResourceAccessServer::Service_MainProcess() {
	auto ChangeSvcStatus = [](const DWORD NewStatus) {
		SvcStatus.dwCurrentState = NewStatus;
		SetServiceStatusInfo();
	};
	while (SvcStatus.dwCurrentState != SERVICE_STOP_PENDING) {
		if (SvcStatus.dwCurrentState == SERVICE_PAUSED)	Sleep(1000);
		else {
			this->server.listen(GetConfStr("url", "domain", "localhost").c_str(), GetConfInt("url", "port", 8080), 0,
				[&]{
					if (SvcStatus.dwCurrentState == SERVICE_STOP_PENDING || SvcStatus.dwCurrentState == SERVICE_PAUSE_PENDING) this->server.stop();
					else {
						this->UpdateResources();
						if (SvcStatus.dwCurrentState == SERVICE_START_PENDING || SvcStatus.dwCurrentState == SERVICE_CONTINUE_PENDING) ChangeSvcStatus(SERVICE_RUNNING);
					}
				}
			);	
			if (SvcStatus.dwCurrentState == SERVICE_PAUSE_PENDING) ChangeSvcStatus(SERVICE_PAUSED);
		}
	}
	ChangeSvcStatus(SERVICE_STOPPED); // 停止はサーバーが完全に停止してから停止扱いにするためここで変更
}
