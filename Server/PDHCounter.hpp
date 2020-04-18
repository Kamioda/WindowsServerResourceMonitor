#pragma once
#include <Pdh.h>
#include <string>

class PDHCounter {
public:
	std::string CategoryName;
	std::string CounterName;
	std::string instanceName;
private:
	PDH_HQUERY hQuery;
	PDH_HCOUNTER hCounter;
public:
	PDHCounter(const std::string& CategoryName, const std::string& CounterName, const std::string& instanceName);
	PDHCounter(const std::string& CategoryName, const std::string& CounterName);
	void Update() const;
	double GetDoubleValue() const;
	LONGLONG GetInt64Value() const;
	long GetLongValue() const;
};
