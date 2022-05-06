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

inline void ReplaceString(std::string& String1, const std::string& Old , const std::string& New) {
	std::string::size_type Pos(String1.find(Old));
	while (Pos != std::string::npos) {
		String1.replace(Pos, Old.length(), New);
		Pos = String1.find(Old, Pos + New.length());
	}
}

inline std::string ToJsonText(const nlohmann::json& obj) {
	std::stringstream ss{};
	ss << obj;
	return ss.str();
}

template<typename T>
inline auto find(std::vector<T>& v, const std::string& val) { return std::find_if(v.begin(), v.end(), [&val](const T& t) { return t.GetKey() == val; }); }

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
