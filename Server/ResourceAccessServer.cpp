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

class IniRead {
private:
	std::string IniFilePath;
public:
	IniRead() = default;
	IniRead(const std::string& IniFilePath) : IniFilePath(IniFilePath) {}
	int GetNum(const std::string& Section, const std::string& Key, const int DefaultValue) const {
		return GetPrivateProfileIntA(Section.c_str(), Key.c_str(), DefaultValue, this->IniFilePath.c_str());
	}
	std::string GetString(const std::string& Section, const std::string& Key, const std::string& Default) const {
		char Buffer[ArrayBufferSize];
		GetPrivateProfileStringA(Section.c_str(), Key.c_str(), Default.c_str(), Buffer, ArrayBufferSize, this->IniFilePath.c_str());
		return std::string(Buffer);
	}
};

constexpr double ByteToKiloByte(const long long& ByteVal) { return static_cast<double>(ByteVal) / 1024.0; }
constexpr double ByteToMegaByte(const long long& ByteVal) { return ByteToKiloByte(ByteVal) / 1024.0; }
constexpr double ByteToGigaByte(const long long& ByteVal) { return ByteToMegaByte(ByteVal) / 1024.0; }
constexpr double ToPercentBase(const long long& AvailOrUsage, const long long& Total) { return static_cast<double>(AvailOrUsage) * 100.0 / static_cast<double>(Total); }
constexpr double ToPercentCheckLower(const long long& AvailOrUsage, const long long& Total) { return AvailOrUsage > 0 && static_cast<int>(ToPercentBase(AvailOrUsage, Total) * 100) == 0; }
constexpr double ToPercentCheckUpper(const long long& AvailOrUsage, const long long& Total) { return AvailOrUsage != Total && static_cast<int>(ToPercentBase(AvailOrUsage, Total) * 100) == 10000; }
constexpr double ToPercent(const long long& AvailOrUsage, const long long& Total) {	return ToPercentCheckLower(AvailOrUsage, Total) ? 0.01 : (ToPercentCheckUpper(AvailOrUsage, Total) ? 99.99 : ToPercentBase(AvailOrUsage, Total)); }
constexpr double digit(const double d) { return static_cast<double>(static_cast<LONGLONG>(d * 100.0 + 0.5)) / 100.0; }

class PDHCounter {
public:
	std::string CategoryName;
	std::string CounterName;
	std::string instanceName;
private:
	PDH_HQUERY hQuery;
	PDH_HCOUNTER hCounter;
public:
	PDHCounter(const std::string& CategoryName, const std::string& CounterName, const std::string& instanceName)
		: CategoryName(CategoryName), CounterName(CounterName), instanceName(instanceName) {
		if (PdhOpenQuery(NULL, NULL, &this->hQuery) != ERROR_SUCCESS) throw std::runtime_error("Failed to open PDH Query");
		PdhAddCounterA(this->hQuery, ("\\" + CategoryName + "(" + instanceName + ")\\" + CounterName).c_str(), NULL, &this->hCounter);
	}
	PDHCounter(const std::string& CategoryName, const std::string& CounterName)
		: CategoryName(CategoryName), CounterName(CounterName), instanceName() {
		if (PdhOpenQuery(NULL, NULL, &this->hQuery) != ERROR_SUCCESS) throw std::runtime_error("Failed to open PDH Query");
		PdhAddCounterA(this->hQuery, ("\\" + CategoryName + "\\" + CounterName).c_str(), NULL, &this->hCounter);
	}
	void Update() const {
		PdhCollectQueryData(this->hQuery);
	}
	double GetDoubleValue() const {
		PDH_FMT_COUNTERVALUE val{};
		PdhGetFormattedCounterValue(this->hCounter, PDH_FMT_DOUBLE, NULL, &val);
		return digit(val.doubleValue);
	}
	LONGLONG GetInt64Value() const {
		PDH_FMT_COUNTERVALUE val{};
		PdhGetFormattedCounterValue(this->hCounter, PDH_FMT_LARGE, NULL, &val);
		return val.largeValue;
	}
	long GetLongValue() const {
		PDH_FMT_COUNTERVALUE val{};
		PdhGetFormattedCounterValue(this->hCounter, PDH_FMT_LARGE, NULL, &val);
		return val.longValue;
	}
};

namespace impl {
	class DiskUsage : public PDHCounter {
	public:
		DiskUsage(const std::string& TargetDrive = "C:") : PDHCounter("LogicalDisk", "% Free Space", TargetDrive) {}
		double Get() const { return digit(100.0 - PDHCounter::GetDoubleValue()); }
	};

	class DiskRead : public PDHCounter {
	public:
		DiskRead(const std::string& TargetDrive = "C:") : PDHCounter("LogicalDisk", "Disk Reads/sec", TargetDrive) {}
		double Get() const { return digit(PDHCounter::GetDoubleValue()); }
	};

	class DiskWrite : public PDHCounter {
	public:
		DiskWrite(const std::string& TargetDrive = "C:") : PDHCounter("LogicalDisk", "Disk Writes/sec", TargetDrive) {}
		double Get() const { return digit(PDHCounter::GetDoubleValue()); }
	};

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

class jsonobject {
private:
	picojson::object obj;
public:
	jsonobject() = default;
	void insert(const std::string& key, const std::string& value) {
		this->obj.insert(std::make_pair(key, value));
	}
	void insert(const std::string& key, const double& value) {
		this->obj.insert(std::make_pair(key, value));
	}
	void insert(const std::string& key, const picojson::object& InsertObject) {
		this->obj.insert(std::make_pair(key, InsertObject));
	}
	operator const picojson::object& () const noexcept { return this->obj; }
};

class Processor : public PDHCounter {
private:
	typedef int (*GetProcessNumFunc)();
	DWORD ProcessNum;
public:
	Processor() : PDHCounter("Processor", "% Processor Time", "_Total"), ProcessNum() {}
private:
	double GetUsage() const { return digit(PDHCounter::GetDoubleValue()); }
	int GetProcessNum() const { return this->ProcessNum; }
public:
	void Update() {
		PDHCounter::Update();
		constexpr DWORD BufferSize = 1024;
		DWORD Buffer[BufferSize];
		EnumProcesses(Buffer, sizeof(Buffer), &this->ProcessNum);
		this->ProcessNum /= sizeof(DWORD);
	}
	picojson::object Get() const {
		jsonobject obj{};
		obj.insert("usage", this->GetUsage());
		obj.insert("process", this->GetProcessNum());
	}
};

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
		jsonobject physical{};
		physical.insert("total", this->GetPhysicalMaxMemSize());
		physical.insert("available", this->GetPhysicalAvailable());
		physical.insert("used", this->GetPhysicalUsage());
		physical.insert("usedper", this->GetPhysicalUsagePer());
		return physical;
	}
	picojson::object GetCommit() const {
		jsonobject commit{};
		commit.insert("total", this->GetCommitMaxMemSize());
		commit.insert("available", this->GetCommitAvailable());
		commit.insert("used", this->GetCommitUsage());
		commit.insert("usedper", this->GetCommitUsagePer());
		return commit;
	}
public:
	picojson::object Get() const {
		jsonobject obj{};
		obj.insert("physical", this->GetPhysical());
		obj.insert("commit", this->GetCommit());
		return obj;
	}
};

class Disk {
private:
	impl::DiskUsage diskUse;
	impl::DiskRead diskRead;
	impl::DiskWrite diskWrite;
public:
	Disk(const std::string& TargetDrive = "C:") 
		: diskUse(TargetDrive), diskRead(TargetDrive), diskWrite(TargetDrive) {}
	void Update() const {
		this->diskUse.Update();
		this->diskRead.Update();
		this->diskWrite.Update();
	}
	picojson::object Get() const {
		jsonobject obj{};
		obj.insert("used", this->diskUse.Get());
		obj.insert("read", this->diskRead.Get());
		obj.insert("write", this->diskWrite.Get());
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
		jsonobject obj{};
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
