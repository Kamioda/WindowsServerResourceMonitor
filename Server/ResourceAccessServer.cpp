#include "ResourceAccessServer.hpp"
#include "ServiceStatus.h"
#include "Split.hpp"
#include "JsonObject.hpp"
#include "JsonArray.hpp"
using Req = const httplib::Request&;
using Res = httplib::Response&;

ServiceProcess* GetServiceProcessInstance(const Service_CommandLineManager::CommandLineType& args) {
	return new ResourceAccessServer(args);
}

inline std::vector<Network> GetNetworkResourceInformations(const IniRead& ini) {
	std::vector<Network> RetVal{};
	const std::vector<std::string> DriveList = SplitString(ini.GetString("system", "netdevice", "Realtek PCIe GBE Family Controller"), ',');
	for (const auto& i : DriveList) {
		try {
			RetVal.emplace_back(i);
		}
		catch (std::exception&) { // ないネットワークデバイスの時にエラー起こすのでここでcatch

		}
	}
	return RetVal;
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
	network(GetNetworkResourceInformations(this->ini)), 
	server() {
	SvcStatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
	SetServiceStatusInfo();
	this->server.Get(GetConfStr("url", "all", "/v1/").c_str(), [&](Req, Res res) { reqproc(res, [&] { res.set_content(ToJsonText(this->AllResourceToObject()), "text/json"); }); });
	this->server.Get(GetConfStr("url", "cpu", "/v1/cpu").c_str(), [&](Req, Res res) { reqproc(res, [&] { res.set_content(ToJsonText(this->processor.Get()), "text/json"); }); });
	this->server.Get(GetConfStr("url", "memory", "/v1/mem").c_str(), [&](Req, Res res) { reqproc(res, [&] { res.set_content(ToJsonText(this->memory.Get()), "text/json"); }); });
	this->server.Get(GetConfStr("url", "allstorage", "/v1/disk/").c_str(), [&](Req, Res res) { reqproc(res, [&] { res.set_content(ToJsonText(this->AllDiskResourceToObject()), "text/json"); }); });
	this->server.Get(GetConfStr("url", "storage", "/v1/disk/[A-Z]").c_str(),
		[&](Req req, Res res) {
			reqproc(res,
				[&] {
					if (const std::string matchstr = (req.matches[0].str() + ":"), drive = matchstr.substr(matchstr.size() - 2); this->disk.find(drive) == this->disk.end()) res.status = 404;
					else res.set_content(ToJsonText(this->disk.at(drive).Get()), "text/json");
				}
			);
		}
	);
	this->server.Get(GetConfStr("url", "allnetwork", "/v1/network/").c_str(), [&](Req, Res res) { reqproc(res, [&] { res.set_content(ToJsonText(this->AllNetworkResourceToObject()), "text/json"); }); });
	this->server.Get(GetConfStr("url", "network", "/v1/network/eth[0-9]{1,}").c_str(), [&](Req req, Res res) {
		reqproc(res, 
			[&] { 
				const std::string matchstr = (req.matches[0].str());
				if (const size_t pos = std::stoul(matchstr.substr(matchstr.find_last_of('/') + 4)); pos >= this->network.size()) res.status = 404;
				else res.set_content(ToJsonText(this->network.at(pos).Get()), "text/json");
			}
		);
		}
	);
	this->server.Post(GetConfStr("url", "stop", "/v1/stop").c_str(), [](Req, Res) { SvcStatus.dwCurrentState = SERVICE_STOP_PENDING; });
	this->server.Post(GetConfStr("url", "pause", "/v1/pause").c_str(), [](Req, Res) { SvcStatus.dwCurrentState = SERVICE_PAUSE_PENDING; });
}

std::string ResourceAccessServer::GetConfStr(const std::string& Section, const std::string& Key, const std::string& Default) const { return this->ini.GetString(Section, Key, Default); };

int ResourceAccessServer::GetConfInt(const std::string& Section, const std::string& Key, const int& Default) const { return this->ini.GetNum(Section, Key, Default); };

picojson::object ResourceAccessServer::AllResourceToObject() const {
	JsonObject obj{};
	JsonArray diskinfo{};
	JsonArray netinfo{};
	obj.insert("cpu", this->processor.Get());
	obj.insert("memory", this->memory.Get());
	for (const auto& i : this->disk) diskinfo.insert(i.second.Get());
	obj.insert("disk", diskinfo);
	for (const auto& i : this->network) netinfo.insert(i.Get());
	obj.insert("network", netinfo);
	return obj;
}

picojson::object ResourceAccessServer::AllDiskResourceToObject() const {
	JsonObject obj{};
	if (this->disk.size() == 1) obj.insert("disk", (*this->disk.begin()).second.Get());
	else {
		JsonArray diskinfo{};
		for (const auto& i : this->disk) diskinfo.insert(i.second.Get());
		obj.insert("disk", diskinfo);
	}
	return obj;
}

picojson::object ResourceAccessServer::AllNetworkResourceToObject() const {
	JsonObject obj{};
	JsonArray netinfo{};
	for (const auto& i : this->network) netinfo.insert(i.Get());
	obj.insert("network", netinfo);
	return obj;
}

void ResourceAccessServer::UpdateResources() {
	this->processor.Update();
	this->memory.Update();
	for (const auto& i : this->disk) i.second.Update();
	for (const auto& i : this->network) i.Update();
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
