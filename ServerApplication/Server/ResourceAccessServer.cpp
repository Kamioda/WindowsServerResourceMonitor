#include "ResourceAccessServer.hpp"
#include "Base64Converter.hpp"
#include "ServiceStatus.h"
#include "Split.hpp"
#include <concepts>
#include <algorithm>
#include <chrono>
std::shared_ptr<ResourceAccessServer> Server;

std::shared_ptr<ServiceProcess> GetServiceProcessInstance(const Service_CommandLineManager::CommandLineType& args) {
#if defined(_DEBUG) && !defined(CONSOLE)
	Sleep(10000);
#endif
	return Server = std::make_shared<ResourceAccessServer>(args);
}

struct PerSocketData {};

inline std::string ToString(const nlohmann::json& json) {
	std::stringstream ss{};
	ss << json;
	return ss.str();
}

inline std::string ToString(const std::string_view& v) {
	return std::string(v.data(), v.size());
}

inline std::unordered_map<std::string_view, std::string_view> GetQueries(uWS::HttpRequest* req) {
	const std::string_view QueryParameterView = req->getQuery();
	if (QueryParameterView.size() == 0) return {};
	std::unordered_map<std::string_view, std::string_view> Ret{};
	const std::vector<std::string_view> SplitData = SplitString(QueryParameterView, '&');
	for (const std::string_view& i : SplitData) {
		const std::array<std::string_view, 2> QueryParameter = SplitString<2>(i, '=');
		Ret.emplace(QueryParameter.at(0), QueryParameter.at(1));
	}
	return Ret;
}

static uWS::App CreateWSServer(ConfigLoader& conf, const AuthManager& auth, us_listen_socket_t* dstSocket) {
	return uWS::App{}.get(
		conf.GetString("configuration/url/authoricate", "/v1/auth"),
		[&auth](uWS::HttpResponse<false>* res, uWS::HttpRequest* req) {
			const std::string_view v = req->getHeader("authorization");
			res->writeStatus(auth.Auth(std::string(v.data(), v.size())) ? uWS::HTTP_200_OK : "401 Unauthorized");
		}
	).ws<PerSocketData>(
		"/ws/*",
		{
			.compression = uWS::SHARED_COMPRESSOR,
			.maxPayloadLength = 16 * 1024 * 1024,
			.idleTimeout = 16,
			.maxBackpressure = 1 * 1024 * 1024,
			.open = [](uWS::WebSocket<false, true>* ws, uWS::HttpRequest*) {
				ws->subscribe("resource server");
			},
			.message = [](uWS::WebSocket<false, true>*, std::string_view message, uWS::OpCode) {

			},
			.drain = [](uWS::WebSocket<false, true>* ws) {
				/* Check ws->getBufferedAmount() here */
			},
			.ping = [](uWS::WebSocket<false, true>* ws) {

			},
			.pong = [](uWS::WebSocket<false, true>* ws) {

			},
			.close = [](uWS::WebSocket<false, true>* ws, int /*code*/, std::string_view /*message*/) {
				/* You may access ws->getUserData() here */
			}
		}
	).listen(
		conf.GetNum<int>("configuration/url/port", 9900), 
		[&dstSocket](us_listen_socket_t* sock) { dstSocket = sock; }
	);
}

ResourceAccessServer::ResourceAccessServer(const Service_CommandLineManager::CommandLineType& args)
	: ServiceProcess(args), commgr(),
	conf(BaseClass::ChangeFullPath(".\\server.xml")),
	resource(conf.GetJson("configuration/resourcefile", ".\\auth.json")), 
	auth(conf.GetJson("configuration/authfile", ".\\resources.json")), 
	ListenSocket(), app(CreateWSServer(conf, auth, ListenSocket)) {
	SvcStatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
	SetServiceStatusInfo();
}

uWS::App& ResourceAccessServer::GetApp() noexcept { return this->app; }

ResourceManager& ResourceAccessServer::GetResources() noexcept { return this->resource; }

us_listen_socket_t* ResourceAccessServer::GetSocket() const noexcept { return this->ListenSocket; }

void ResourceAccessServer::Service_MainProcess() {
	us_loop_t* loop = reinterpret_cast<us_loop_t*>(uWS::Loop::get());
	us_timer_t* delayTimer = us_create_timer(loop, 0, 0);
	us_timer_set(
		delayTimer,
		[](us_timer_t* timer) {
			if (SvcStatus.dwCurrentState == SERVICE_STOP_PENDING) {
				us_listen_socket_close(0, Server->GetSocket());
				us_timer_close(timer);
				return;
			}
			ResourceManager& resource = Server->GetResources();
			resource.Update();
			Server->GetApp().publish("resource server", ToString(resource.GetAllResources()), uWS::OpCode::TEXT);
		}, 0, this->conf.GetNum<int>("configuration/loop", 500)
	);
	this->app.run();
	SvcStatus.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatusInfo(); // 停止はサーバーが完全に停止してから停止扱いにするためここで変更
}
