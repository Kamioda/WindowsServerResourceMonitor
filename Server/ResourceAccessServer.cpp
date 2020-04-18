#include "ResourceAccessServer.hpp"
#include <picojson.h>
#include <Windows.h>
#include <Pdh.h>
#include <Psapi.h>
#include <vector>
#include <filesystem>
constexpr size_t ArrayBufferSize = 1024;
#pragma comment(lib, "pdh.lib")

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

ResourceAccessServer::ResourceAccessServer(const Service_CommandLineManager::CommandLineType& args)
	: ServiceProcess(args), server() {}

void ResourceAccessServer::Service_MainProcess() {
	IniRead ini(BaseClass::ChangeFullPath(".\\server.ini"));
	std::string ResourceInformation{};
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
	server.listen(ini.GetString("url", "domain", "localhost").c_str(), ini.GetNum("url", "port", 8080), 0, 
		[&]() {
			
		}
	);
}

ServiceProcess* GetServiceProcessInstance(const Service_CommandLineManager::CommandLineType& args) {
	return new ResourceAccessServer(args);
}

constexpr double ByteToKiloByte(const long long ByteVal) { return static_cast<double>(ByteVal) / 1024.0; }
constexpr double ByteToMegaByte(const long long ByteVal) { return ByteToKiloByte(ByteVal) / 1024.0; }
constexpr double ByteToGigaByte(const long long ByteVal) { return ByteToMegaByte(ByteVal) / 1024.0; }
constexpr double ToPercent(const long long AvailOrUsage, const long long Total) { return static_cast<double>(AvailOrUsage) * 100.0 / static_cast<double>(Total); }

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
	class DiskFreeSpace : public PDHCounter {
	public:
		DiskFreeSpace(const std::string& TargetDrive = "C:") : PDHCounter("LogicalDisk", "% Free Space", TargetDrive) {}
		double Get() const { return PDHCounter::GetDoubleValue(); }
	};

	class DiskRead : public PDHCounter {
	public:
		DiskRead(const std::string& TargetDrive = "C:") : PDHCounter("LogicalDisk", "Disk Reads/sec", TargetDrive) {}
		double Get() const { return PDHCounter::GetDoubleValue(); }
	};

	class DiskWrite : public PDHCounter {
	public:
		DiskWrite(const std::string& TargetDrive = "C:") : PDHCounter("LogicalDisk", "Disk Writes/sec", TargetDrive) {}
		double Get() const { return PDHCounter::GetDoubleValue(); }
	};

	class NetworkReceive : public PDHCounter {
	public:
		NetworkReceive(const std::string& NetworkDeviceName = "Realtek PCIe GBE Family Controller") : PDHCounter("Network Adapter", "Bytes Received/sec", NetworkDeviceName) {}
		double Get() const { return PDHCounter::GetDoubleValue(); }
	};

	class NetworkSend : public PDHCounter {
	public:
		NetworkSend(const std::string& NetworkDeviceName = "Realtek PCIe GBE Family Controller") : PDHCounter("Network Adapter", "Bytes Sent/sec", NetworkDeviceName) {}
		double Get() const { return PDHCounter::GetDoubleValue(); }
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
	void insert(const std::string& key, const picojson::object& obj) {
		this->obj.insert(std::make_pair(key, obj));
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
	double GetUsage() const { return PDHCounter::GetDoubleValue(); }
	int GetProcessNum() const { return this->ProcessNum; }
public:
	void Update() {
		PDHCounter::Update();
		constexpr DWORD BufferSize = 1024;
		DWORD Buffer[BufferSize];
		EnumProcesses(Buffer, sizeof(Buffer), &this->ProcessNum);
		this->ProcessNum /= sizeof(DWORD);
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
	double GetPhysicalMaxMemSize() const { return digit(ByteToMegaByte(this->GetPhysicalTotal())); }
	double GetPhysicalAvailable() const { return digit(ByteToMegaByte(this->GetPhysicalAvail())); }
	double GetPhysicalUsage() const { return digit(ByteToMegaByte(this->GetPhysicalUsed())); }
	double GetPhysicalUsagePer() const { return digit(ToPercent(this->GetPhysicalUsed(), GetDiviveNum(this->GetPhysicalTotal()))); }
	double GetCommitMaxMemSize() const { return digit(ByteToMegaByte(this->GetCommitTotal())); }
	double GetCommitAvailable() const { return digit(ByteToMegaByte(this->GetCommitAvail())); }
	double GetCommitUsage() const { return digit(ByteToMegaByte(this->GetCommitUsed())); }
	double GetCommitUsagePer() const { return digit(ToPercent(this->GetCommitUsed(), GetDiviveNum(this->GetCommitTotal()))); }
public:
};

public:
};

