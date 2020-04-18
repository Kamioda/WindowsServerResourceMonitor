#include "ResourceAccessServer.hpp"
#include <picojson.h>
#include <Windows.h>
#include <Pdh.h>
#include <Psapi.h>
#include <vector>
#include <filesystem>
constexpr size_t ArrayBufferSize = 1024;
#pragma comment(lib, "pdh.lib")

ServiceProcess* GetServiceProcessInstance(const Service_CommandLineManager::CommandLineType& args) {
	return new ResourceAccessServer(args);
}

namespace impl {
	class NetworkReceive : public PDHCounter {
	public:
		NetworkReceive(const std::string& NetworkDeviceName = "Realtek PCIe GBE Family Controller") : PDHCounter("Network Adapter", "Bytes Received/sec", NetworkDeviceName) {}
		double Get() const { return digit(PDHCounter::GetDoubleValue()); }
	};

	class NetworkSend : public PDHCounter {
	public:
		NetworkSend(const std::string& NetworkDeviceName = "Realtek PCIe GBE Family Controller") : PDHCounter("Network Adapter", "Bytes Sent/sec", NetworkDeviceName) {}
		double Get() const { return digit(PDHCounter::GetDoubleValue()); }
	};
}

class MemoryManager {
private:
	PDHCounter counter;
	MEMORYSTATUSEX ms;
	long long GetPhysicalTotal() const { return this->ms.ullTotalPhys; }
	long long GetPhysicalAvail() const { return this->ms.ullAvailPhys; }
	long long GetPhysicalUsed() const { return this->GetPhysicalTotal() - this->GetPhysicalAvail(); }

	long long GetCommitTotal() const { return this->ms.ullTotalPageFile; }
	long long GetCommitAvail() const { return this->ms.ullAvailPageFile; }
	long long GetCommitUsed() const { return this->GetCommitTotal() - this->GetCommitAvail(); }

	static constexpr long long GetDiviveNum(const long long v) { return v == 0 ? 1 : v; }
public:
	MemoryManager() : ms({ sizeof(MEMORYSTATUSEX) }), counter(PDHCounter("Memory", "Committed Bytes")) {}
	void Update() {
		GlobalMemoryStatusEx(&this->ms);
		this->counter.Update();
	}
private:
	double GetPhysicalMaxMemSize() const { return digit(ByteToMegaByte(this->GetPhysicalTotal())); }
	double GetPhysicalAvailable() const { return digit(ByteToMegaByte(this->GetPhysicalAvail())); }
	double GetPhysicalUsage() const { return digit(ByteToMegaByte(this->GetPhysicalUsed())); }
	double GetPhysicalUsagePer() const { return digit(ToPercent(this->GetPhysicalUsed(), GetDiviveNum(this->GetPhysicalTotal()))); }
	double GetCommitMaxMemSize() const { return digit(ByteToMegaByte(this->GetCommitTotal())); }
	double GetCommitAvailable() const { return digit(ByteToMegaByte(this->GetCommitAvail())); }
	double GetCommitUsage() const { return digit(ByteToMegaByte(this->GetCommitUsed())); }
	double GetCommitUsagePer() const { return digit(ToPercent(this->GetCommitUsed(), GetDiviveNum(this->GetCommitTotal()))); }
	picojson::object GetPhysical() const {
		JsonObject physical{};
		physical.insert("total", this->GetPhysicalMaxMemSize());
		physical.insert("available", this->GetPhysicalAvailable());
		physical.insert("used", this->GetPhysicalUsage());
		physical.insert("usedper", this->GetPhysicalUsagePer());
		return physical;
	}
	picojson::object GetCommit() const {
		JsonObject commit{};
		commit.insert("total", this->GetCommitMaxMemSize());
		commit.insert("available", this->GetCommitAvailable());
		commit.insert("used", this->GetCommitUsage());
		commit.insert("usedper", this->GetCommitUsagePer());
		return commit;
	}
public:
	picojson::object Get() const {
		JsonObject obj{};
		obj.insert("physical", this->GetPhysical());
		obj.insert("commit", this->GetCommit());
		return obj;
	}
};

class Network {
private:
	impl::NetworkReceive netReceive;
	impl::NetworkSend netSend;
public:
	Network(const std::string& NetworkDeviceName = "Realtek PCIe GBE Family Controller")
		: netReceive(NetworkDeviceName), netSend(NetworkDeviceName) {}
	void Update() const {
		this->netReceive.Update();
		this->netSend.Update();
	}
	picojson::object Get() const {
		JsonObject obj{};
		obj.insert("receive", this->netReceive.Get());
		obj.insert("send", this->netSend.Get());
		return obj;
	}
};

inline std::string GetAllResourceData(const Processor& p, const MemoryManager& m, const Disk& d, const Network& n) {
	jsonobject obj{};
	obj.insert("cpu", p.Get());
	obj.insert("memory", m.Get());
	obj.insert("disk", d.Get());
	obj.insert("network", n.Get());
	return picojson::value(obj).to_str();
}

ResourceAccessServer::ResourceAccessServer(const Service_CommandLineManager::CommandLineType& args)
	: ServiceProcess(args), server() {}

void ResourceAccessServer::Service_MainProcess() {
	IniRead ini(BaseClass::ChangeFullPath(".\\server.ini"));
	Processor processor{};
	MemoryManager memory{};
	Disk disk(ini.GetString("resource", "drive", "C:"));
	Network network(ini.GetString("resource", "network", "Realtek PCIe GBE Family Controller"));
	this->server.Get(ini.GetString("url", "all", "/v1/").c_str(),
		[&](const httplib::Request& req, httplib::Response& res) {

		}
	);
	this->server.Get(ini.GetString("url", "cpu", "/v1/cpu").c_str(),
		[&](const httplib::Request& req, httplib::Response& res) {

		}
	);
	this->server.Get(ini.GetString("url", "memory", "/v1/mem").c_str(),
		[&](const httplib::Request& req, httplib::Response& res) {

		}
	);
	this->server.Get(ini.GetString("url", "storage", "/v1/disk/[A-Z]").c_str(),
		[&](const httplib::Request& req, httplib::Response& res) {
			const std::string drive = req.matches[1].str() + ":";

		}
	);
	this->server.Get(ini.GetString("url", "network", "/v1/network").c_str(),
		[&](const httplib::Request& req, httplib::Response& res) {

		}
	);
	this->server.listen(ini.GetString("url", "domain", "localhost").c_str(), ini.GetNum("url", "port", 8080), 0,
		[&]() {
			processor.Update();
			memory.Update();
			disk.Update();
			network.Update();
		}
	);
}
