#include "ResourceAccessServer.hpp"
#include "Base64Converter.hpp"
#include "ServiceStatus.h"
#include "Split.hpp"
#include "JsonObject.hpp"
#include "JsonArray.hpp"
#include <algorithm>
#include <chrono>
using Req = const httplib::Request&;
using Res = httplib::Response&;

std::unique_ptr<ServiceProcess> GetServiceProcessInstance(const Service_CommandLineManager::CommandLineType& args) {
#if defined(_DEBUG) && !defined(CONSOLE)
	Sleep(10000);
#endif
	return std::make_unique<ResourceAccessServer>(args);
}

void ResourceAccessServer::GetDiskResourceInformations() {
	std::vector<std::string> NameList = SplitString(this->conf.GetString("system", "drives", "C:"), ',');
	for (auto& i : NameList) {
		try {
			this->disk.emplace_back(this->query, i);
		}
		catch (std::exception&) { // ないドライブの時にエラー起こすのでここでcatch

		}
	}
}

void ResourceAccessServer::GetNetworkResourceInformations() {
	const std::vector<std::string> DriveList = SplitString(this->conf.GetString("system", "netdevice", "Realtek PCIe GBE Family Controller"), ',');
	for (const auto& i : DriveList) {
		try {
			this->network.emplace_back(this->query, i);
		}
		catch (std::exception&) { // ないネットワークデバイスの時にエラー起こすのでここでcatch

		}
	}
}

void ResourceAccessServer::GetServiceInformations() {
	const std::string LoadTargets = this->conf.GetString("services", "target", "");
	if (LoadTargets.empty()) return;
	ServiceMonitor::InitStatusList(this->conf);
	ServiceMonitor::InitServiceTypeList(this->conf);
	const std::vector<std::string> NameList = SplitString(LoadTargets, ',');
	for (const auto& i : NameList) {
		try {
			this->services.emplace_back(this->SCM, i);
		}
		catch (std::exception&) { // ないサービスの時にエラー起こすのでここでcatch

		}
	}
}

std::string ResourceAccessServer::GetConfStr(const std::string& Section, const std::string& Key, const std::string& Default) { return this->conf.GetString(Section, Key, Default); };

std::wstring ResourceAccessServer::GetConfStr(const std::wstring& Section, const std::wstring& Key, const std::wstring& Default) { return this->conf.GetString(Section, Key, Default); };

int ResourceAccessServer::GetConfInt(const std::string& Section, const std::string& Key, const int& Default) { return this->conf.GetNum(Section, Key, Default); };

template<class ResourceClass>
inline void InsertArray(const std::vector<ResourceClass>& list, JsonObject& obj, const std::string& key) {
	JsonArray arr{};
	for (const auto& i : list) arr.insert(i.Get());
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
	InsertArray(this->network, obj, "network");
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
inline picojson::object AllResourceToObjectImpl(const std::vector<ResourceClass>& Resource, const std::string& Key) {
	JsonObject obj{};
	if (Resource.size() == 1) obj.insert("disk", Resource.front().Get());
	else {
		JsonArray diskinfo{};
		for (const auto& i : Resource) diskinfo.insert(i.Get());
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
	this->query.Update();
	this->memory.Update();
	for (auto& i : this->disk) i.Update();
	for (auto& i : this->services) i.Update();
}

inline void ReplaceString(std::string& String1, const std::string& Old , const std::string& New) {
	std::string::size_type Pos(String1.find(Old));
	while (Pos != std::string::npos) {
		String1.replace(Pos, Old.length(), New);
		Pos = String1.find(Old, Pos + New.length());
	}
}

inline std::string ToJsonText(const picojson::object& obj) {
	std::stringstream ss{};
	ss << picojson::value(obj);
	return ss.str();
}

inline std::string PickupAccessToken(Req req) {
	const auto CookieIt = req.headers.find("Cookie");
	if (CookieIt == req.headers.end()) return {};
	const std::string CookieText = CookieIt->second;
	return CookieText.substr(0, CookieText.find_first_of('=')) == "AccessToken"
		? base64::decode(CookieText.substr(CookieText.find_first_of('=') + 1)) : std::string{};
}

inline void reqproc(AuthManager& auth, Req req, Res res, const std::function<void()>& process, const bool AuthUseAccessToken = true, const bool IgnoreServiceStatus = false) {
	if (!IgnoreServiceStatus && SvcStatus.dwCurrentState == SERVICE_PAUSED) {
		res.status = 503;
		res.set_content("service is paused", "text/plain");
	}
	else {
		try {
			if (AuthUseAccessToken) {
				if (const std::string AccessToken = PickupAccessToken(req); AccessToken.empty()) throw AuthException(401, "AccessToken is not contained.");
				else if (!auth.Auth(AccessToken)) throw AuthException(401, "Invalid AccessToken");
			}
			process();
		}
		catch (const AuthException& aex) {
			res.status = aex.GetErrorCode();
			res.set_content(aex.GetErrorMessage(), "text/plain");
		}
		catch (...) { res.status = 500; }
	}
};

template<typename T>
inline auto find(const std::vector<T>& v, const std::string& val) { return std::find_if(v.begin(), v.end(), [&val](const T& t) { return t.GetKey() == val; }); }

inline std::string Auth(AuthManager& auth, const std::string& RequestBody) {
	picojson::value val{};
	if (const std::string err = picojson::parse(val, RequestBody); !err.empty()) throw AuthException(400, "auth data is wrong");
	picojson::object obj = val.get<picojson::object>();
	const std::string id = obj.at("id").get<std::string>();
	if (!auth.Auth(id, obj.at("pass").get<std::string>())) throw AuthException(401, "invalid ID or Password");
	return auth.CreateAccessToken(id);
}

inline std::string CreateUser(AuthManager& auth, const std::string& RequestBody, const std::string& CurrentUserAccessToken) {
	picojson::value val{};
	if (const std::string err = picojson::parse(val, RequestBody); !err.empty()) throw AuthException(400, "auth data is wrong");
	picojson::object obj = val.get<picojson::object>();
	const std::string ID = obj.at("id").get<std::string>();
	const std::string Pass = obj.at("pass").get<std::string>();
	// 認証していたのがデフォルトユーザーの場合、新ユーザーのアクセストークンを発行して返す
	return auth.AddUser(ID, Pass, CurrentUserAccessToken) ? auth.CreateAccessToken(ID) : CurrentUserAccessToken;
}

ResourceAccessServer::ResourceAccessServer(const Service_CommandLineManager::CommandLineType& args)
	: ServiceProcess(args), commgr(),
	conf(BaseClass::ChangeFullPath(".\\server.xml")),
	auth(
		string::converter::stl::from_bytes(BaseClass::ChangeFullPath(this->GetConfStr("application", "authfile/file", ".\\auth,xml"))),
		this->GetConfStr(L"application", L"authfile/root", L"authinformation"),
		this->GetConfStr(L"application", L"authfile/defaultuser", L"defaultuser"),
		this->GetConfStr(L"application", L"authfile/allowuser", L"allowuser"),
		this->GetConfInt("application", "authmaxtime", 0)
	),
	SCM(), query(), processor(this->query), memory(), disk(), network(), services(), server(),
	looptime(static_cast<DWORD>(this->GetConfInt("application", "looptime", 1000))) {
	SvcStatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
	SetServiceStatusInfo();
	this->GetDiskResourceInformations();
	this->GetNetworkResourceInformations();
	this->GetServiceInformations();
	this->server.Get(this->GetConfStr("url", "all", "/v1/").c_str(), [&](Req req, Res res) { reqproc(this->auth, req, res, [&] { res.set_content(ToJsonText(this->AllResourceToObject()), "application/json"); }); });
	this->server.Get(this->GetConfStr("url", "cpu", "/v1/cpu").c_str(), [&](Req req, Res res) { reqproc(this->auth, req, res, [&] { res.set_content(ToJsonText(this->processor.Get()), "application/json"); }); });
	this->server.Get(this->GetConfStr("url", "memory", "/v1/mem").c_str(), [&](Req req, Res res) { reqproc(this->auth, req, res, [&] { res.set_content(ToJsonText(this->memory.Get()), "application/json"); }); });
	this->server.Get(this->GetConfStr("url", "allstorage", "/v1/disk/").c_str(), [&](Req req, Res res) { reqproc(this->auth, req, res, [&] { res.set_content(ToJsonText(this->AllDiskResourceToObject()), "application/json"); }); });
	this->server.Get(this->GetConfStr("url", "storage", "/v1/disk/[A-Z]").c_str(),
		[&](Req req, Res res) {
			reqproc(this->auth, req, res,
				[&] {
					const std::string matchstr = (req.matches[0].str() + ":"),
						drive = matchstr.substr(matchstr.size() - 2);
					if (auto it = find(this->disk, drive); it == this->disk.end()) res.status = 404;
					else res.set_content(ToJsonText(it->Get()), "application/json");
				}
			);
		}
	);
	this->server.Get(this->GetConfStr("url", "allnetwork", "/v1/network/").c_str(), [&](Req req, Res res) { reqproc(this->auth, req, res, [&] { res.set_content(ToJsonText(this->AllNetworkResourceToObject()), "application/json"); }); });
	this->server.Get(this->GetConfStr("url", "network", "/v1/network/eth[0-9]{1,}").c_str(), [&](Req req, Res res) {
		reqproc(this->auth, req, res,
			[&] {
				const std::string matchstr = (req.matches[0].str());
				if (const size_t pos = std::stoul(matchstr.substr(matchstr.find_last_of('/') + 4)); pos >= this->network.size()) res.status = 404;
				else res.set_content(ToJsonText(this->network.at(pos).Get()), "application/json");
			}
		);
		}
	);
	this->server.Get(this->GetConfStr("url", "allservice", "/v1/service/").c_str(), [&](Req req, Res res) { reqproc(this->auth, req, res, [&] {res.set_content(ToJsonText(this->AllServiceToObject()), "application/json"); }); });
	this->server.Get(this->GetConfStr("url", "service", "/v1/service/[0-9a-zA-Z\\-_.%]{1,}").c_str(),
		[&](Req req, Res res) {
			reqproc(this->auth, req, res,
				[&] {
					const std::string matchstr = (req.matches[0].str());
					std::string service = matchstr.substr(matchstr.find_last_of('/') + 1);
					ReplaceString(service, "%20", " ");
					if (auto it = find(this->services, service); it == this->services.end()) res.status = 404;
					else res.set_content(ToJsonText(it->Get()), "application/json");
				}
			);
		}
	);
	this->server.Post(this->GetConfStr("url", "stop", "/v1/stop").c_str(), [&](Req req, Res res) { reqproc(this->auth, req, res, [] { SvcStatus.dwCurrentState = SERVICE_STOP_PENDING; }); });
	this->server.Post(this->GetConfStr("url", "pause", "/v1/pause").c_str(), [&](Req req, Res res) { reqproc(this->auth, req, res, [] { SvcStatus.dwCurrentState = SERVICE_PAUSE_PENDING; }); });
	this->server.Post(this->GetConfStr("url", "continue", "/v1/continue").c_str(),
		[&](Req req, Res res) {
			reqproc(this->auth, req, res,
				[&res] {
					if (SvcStatus.dwCurrentState == SERVICE_PAUSED) SvcStatus.dwCurrentState = SERVICE_CONTINUE_PENDING;
					else res.status = 400;
				}, true, true
			);
		}
	);
	this->server.Post(this->GetConfStr("url", "authoricate/signin", "/v1/auth").c_str(),
		[&](Req req, Res res) {
			reqproc(this->auth, req, res, 
				[&] {
					const std::string AccessToken = Auth(this->auth, req.body);
					res.set_header("Cookie", "AccessToken=" + AccessToken);
				},
				false
			);
		}
	);
	this->server.Post(this->GetConfStr("url", "authoricate/createuser", "/v1/newuser").c_str(),
		[&](Req req, Res res) {
			reqproc(this->auth, req, res,
				[&] {
					res.set_header("Cookie", "AccessToken=" + CreateUser(this->auth, req.body, PickupAccessToken(req)));
				}
			);
		}
	);
	this->server.Delete(this->GetConfStr("url", "authoricate/signout", "/v1/auth").c_str(),
		[&](Req req, Res res) {
			reqproc(this->auth, req, res, [&] { this->auth.DeleteAccessToken(PickupAccessToken(req)); });
		}
	);

}

void ResourceAccessServer::Service_MainProcess() {
	auto ChangeSvcStatus = [](const DWORD NewStatus) {
		SvcStatus.dwCurrentState = NewStatus;
		SetServiceStatusInfo();
	};
	while (SvcStatus.dwCurrentState != SERVICE_STOP_PENDING) {
		std::chrono::milliseconds CountStart{};
		try {
			this->server.listen(this->GetConfStr("url", "domain", "localhost").c_str(), GetConfInt("url", "port", 8080), 0,
				[&] {
					try {
						const std::chrono::milliseconds CountEnd = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
						if (const DWORD elapsed = static_cast<DWORD>((CountEnd - CountStart).count()); elapsed < this->looptime) Sleep(this->looptime - elapsed);
						if (SvcStatus.dwCurrentState == SERVICE_STOP_PENDING) this->server.stop();
						else if (SvcStatus.dwCurrentState != SERVICE_PAUSED) {
							if (SvcStatus.dwCurrentState == SERVICE_START_PENDING || SvcStatus.dwCurrentState == SERVICE_CONTINUE_PENDING) ChangeSvcStatus(SERVICE_RUNNING);
							else if (SvcStatus.dwCurrentState == SERVICE_PAUSE_PENDING) ChangeSvcStatus(SERVICE_PAUSED);
							this->UpdateResources();
						}
					}
					catch (const std::exception& er) {
						std::ofstream ofs(BaseClass::ChangeFullPath("log4.txt"));
						ofs << er.what() << std::endl;
					}
					this->auth.DeleteExpiredAccessToken();
					CountStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
				}
			);
		}
		catch(...) {} // listen中に例外が発生した際は再起動させる
	}
	ChangeSvcStatus(SERVICE_STOPPED); // 停止はサーバーが完全に停止してから停止扱いにするためここで変更
}
