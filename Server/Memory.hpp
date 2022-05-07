#pragma once
#include <Windows.h>
#include "json.hpp"

class Memory {
private:
	MEMORYSTATUSEX ms;
	long long GetPhysicalTotal() const;
	long long GetPhysicalAvail() const;
	long long GetPhysicalUsed() const;
	long long GetCommitTotal() const;
	long long GetCommitAvail() const;
	long long GetCommitUsed() const;
public:
	Memory();
	void Update();
private:
	double GetPhysicalMaxMemSize() const;
	double GetPhysicalAvailable() const;
	double GetPhysicalUsage() const;
	double GetPhysicalUsagePer() const;
	double GetCommitMaxMemSize() const;
	double GetCommitAvailable() const;
	double GetCommitUsage() const;
	double GetCommitUsagePer() const;
	nlohmann::json GetPhysical() const;
	nlohmann::json GetCommit() const;
public:
	nlohmann::json Get() const;
};
