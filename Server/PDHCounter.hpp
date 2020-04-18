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
