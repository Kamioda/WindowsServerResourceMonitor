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

	}
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
