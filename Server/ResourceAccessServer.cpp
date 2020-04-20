#include "ResourceAccessServer.hpp"
#include "ServiceStatus.h"
#include "Split.hpp"
#include "JsonObject.hpp"
#include "JsonArray.hpp"
#include <chrono>
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
	const std::vector<std::string> NameList = SplitString(ini.GetString("system", "drives", "C:"), ',');
	for (const auto& i : NameList) {
		try {
			RetVal.emplace(std::make_pair(i, Disk(i)));
		}
		catch (std::exception&) { // ないドライブの時にエラー起こすのでここでcatch

		}
	}
	return RetVal;
}

inline std::unordered_map<std::string, ServiceMonitor> GetServiceInformations(const IniRead& ini, ServiceControlManager& SCM) {
	const std::string LoadTargets = ini.GetString("services", "target", "");
	if (LoadTargets.empty()) return std::unordered_map<std::string, ServiceMonitor>();
	ServiceMonitor::InitStatusList(ini);
	ServiceMonitor::InitServiceTypeList(ini);
	std::unordered_map<std::string, ServiceMonitor> RetVal{};
	const std::vector<std::string> NameList = SplitString(LoadTargets, ',');
	for (const auto& i : NameList) {
		try {
			std::string s = i;
			std::pair<std::string, ServiceMonitor> pair(std::move(s), std::move(ServiceMonitor(SCM, i)));
;			RetVal.emplace(std::move(pair));
		}
		catch (std::exception&) { // ないドライブの時にエラー起こすのでここでcatch

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
	if (SvcStatus.dwCurrentState == SERVICE_PAUSED) {
		res.status = 503;
		res.set_content("service is paused", "text/plain");
	}
	else {
		try { process(); }
		catch (...) { res.status = 500; }
	}
};

ResourceAccessServer::ResourceAccessServer(const Service_CommandLineManager::CommandLineType& args)
	: ServiceProcess(args), 
	ini(BaseClass::ChangeFullPath(".\\server.ini")), 
	SCM(),
	processor(), 
	memory(), 
	disk(GetDiskResourceInformations(this->ini)), 
	network(GetNetworkResourceInformations(this->ini)), 
	services(GetServiceInformations(this->ini, this->SCM)),
	server(),
	looptime(static_cast<DWORD>(this->GetConfInt("application", "looptime", 1000))) {
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
	this->server.Get(GetConfStr("url", "allservice", "/v1/service/").c_str(), [&](Req, Res res) { reqproc(res, [&] {res.set_content(ToJsonText(this->AllServiceToObject()), "text/json"); }); });
	this->server.Get(GetConfStr("url", "service", "/v1/service/[0-9a-zA-Z\\-_.re%]{1,}").c_str(), 
		[&](Req req, Res res) {
			reqproc(res,
				[&] {
					if (const std::string matchstr = (req.matches[0].str()), service = matchstr.substr(matchstr.find_last_of('/') + 1); this->services.find(service) == this->services.end()) res.status = 404;
					else res.set_content(ToJsonText(this->services.at(service).Get()), "text/json");
				}
			);
		}
	);
	this->server.Post(GetConfStr("url", "stop", "/v1/stop").c_str(), [](Req, Res res) { reqproc(res, [] { SvcStatus.dwCurrentState = SERVICE_STOP_PENDING; }); });
	this->server.Post(GetConfStr("url", "pause", "/v1/pause").c_str(), [](Req, Res res) { reqproc(res, [] { SvcStatus.dwCurrentState = SERVICE_PAUSE_PENDING; }); });
	this->server.Post(GetConfStr("url", "continue", "/v1/continue").c_str(), 
		[](Req, Res res) {
			if (SvcStatus.dwCurrentState == SERVICE_PAUSED) SvcStatus.dwCurrentState = SERVICE_CONTINUE_PENDING;
			else res.status = 400;
		}
	);
}

std::string ResourceAccessServer::GetConfStr(const std::string& Section, const std::string& Key, const std::string& Default) const { return this->ini.GetString(Section, Key, Default); };

int ResourceAccessServer::GetConfInt(const std::string& Section, const std::string& Key, const int& Default) const { return this->ini.GetNum(Section, Key, Default); };

template<class ResourceClass>
inline void InsertArray(const std::unordered_map<std::string, ResourceClass>& list, JsonObject& obj, const std::string& key) {
	JsonArray arr{};
	for (const auto& i : list) arr.insert(i.second.Get());
	obj.insert(key, arr);
}

picojson::object ResourceAccessServer::AllResourceToObject() const {
	JsonObject obj{};
	JsonArray diskinfo{};
	JsonArray netinfo{};
	JsonArray serviceinfo{};
	obj.insert("cpu", this->processor.Get());
	obj.insert("memory", this->memory.Get());
	InsertArray(this->disk, obj, "disk");
	for (const auto& i : this->network) netinfo.insert(i.Get());
	obj.insert("network", netinfo);
	InsertArray(this->services, obj, "service");
	return obj;
}

picojson::object ResourceAccessServer::AllNetworkResourceToObject() const {
	JsonObject obj{};
	if (this->network.size() == 1) obj.insert("network", this->network.front().Get());
	else {
		JsonArray netinfo{};
		for (const auto& i : this->network) netinfo.insert(i.Get());
		obj.insert("network", netinfo);
	}
	return obj;
}

template<class ResourceClass>
inline picojson::object AllResourceToObjectImpl(const std::unordered_map<std::string, ResourceClass>& Resource, const std::string& Key) {
	JsonObject obj{};
	if (Resource.size() == 1) obj.insert("disk", (*Resource.begin()).second.Get());
	else {
		JsonArray diskinfo{};
		for (const auto& i : Resource) diskinfo.insert(i.second.Get());
		obj.insert(Key, diskinfo);
	}
	return obj;
}

picojson::object ResourceAccessServer::AllDiskResourceToObject() const {
	return AllResourceToObjectImpl(this->disk, "disk");
}


picojson::object ResourceAccessServer::AllServiceToObject() const {
	return AllResourceToObjectImpl(this->services, "service");
}

void ResourceAccessServer::UpdateResources() {
	this->processor.Update();
	this->memory.Update();
	for (const auto& i : this->disk) i.second.Update();
	for (const auto& i : this->network) i.Update();
	for (auto& i : this->services) i.second.Update();
}

void ResourceAccessServer::Service_MainProcess() {
	auto ChangeSvcStatus = [](const DWORD NewStatus) {
		SvcStatus.dwCurrentState = NewStatus;
		SetServiceStatusInfo();
	};
	while (SvcStatus.dwCurrentState != SERVICE_STOP_PENDING) {
		std::chrono::milliseconds CountStart{};
		try {
			this->server.listen(GetConfStr("url", "domain", "localhost").c_str(), GetConfInt("url", "port", 8080), 0,
				[&] {
					const std::chrono::milliseconds CountEnd = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
					if (const DWORD elapsed = static_cast<DWORD>((CountEnd - CountStart).count()); elapsed < this->looptime) Sleep(this->looptime - elapsed);
					if (SvcStatus.dwCurrentState == SERVICE_STOP_PENDING) this->server.stop();
					else if (SvcStatus.dwCurrentState != SERVICE_PAUSED) {
						this->UpdateResources();
						if (SvcStatus.dwCurrentState == SERVICE_START_PENDING || SvcStatus.dwCurrentState == SERVICE_CONTINUE_PENDING) ChangeSvcStatus(SERVICE_RUNNING);
						else if (SvcStatus.dwCurrentState == SERVICE_PAUSE_PENDING) ChangeSvcStatus(SERVICE_PAUSED);
					}
					CountStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
				}
			);
		}
		catch(...) {} // listen中に例外が発生した際は再起動させる
	}
	ChangeSvcStatus(SERVICE_STOPPED); // 停止はサーバーが完全に停止してから停止扱いにするためここで変更
}
