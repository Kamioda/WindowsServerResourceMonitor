#pragma once
#include "../Server/httplib.h"
#include <picojson.h>
#include <chrono>

class RequestManager {
protected:
	httplib::Client client;
	long long RequestInterval;
	std::chrono::milliseconds LastRequest;
	picojson::object Response;
	int ErrorCount;
	int MaxErrorCount;
	int LastStatus;
	static constexpr std::chrono::milliseconds GetCurrentClock() {
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	}
public:
	RequestManager(const std::string& Host, const int port, const long long Interval = 1000, const int ErrorMax = 5)
		: client(Host, port), RequestInterval(Interval), LastRequest(), Response(), ErrorCount(), MaxErrorCount(ErrorMax), LastStatus(200) {}
	picojson::object GetAll(const std::string& Path) {
		const std::chrono::microseconds Now = GetCurrentClock();
		if ((this->LastRequest - Now).count() < this->RequestInterval) return this->Response;
		auto res = this->client.Get(Path.c_str());
		this->LastStatus = res->status;
		if (res->status != 200) {
			if (this->LastStatus == 503) return this->Response; // 503はサービスが一時停止中にも来るのでエラーカウントしない
			this->ErrorCount++;
			if (this->MaxErrorCount == this->ErrorCount) 
				throw std::runtime_error("複数回にわたってエラーが発生しました。サーバーを確認して下さい。");
			return this->Response;
		}
		this->ErrorCount = 0;
		picojson::value val{};
		if (const std::string err = picojson::parse(val, res->body); !err.empty()) throw std::runtime_error(err);
		this->Response = val.get<picojson::object>();
		return this->Response;
	}
	void Post(const std::string& Path, const std::string& Body = std::string(), const std::string& ContentType = std::string()) {
		this->client.Post(Path.c_str(), Body, ContentType.c_str());
	}
};
