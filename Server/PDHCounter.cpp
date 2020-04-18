#include "PDHCounter.hpp"
#include "PDHAssistFunctions.hpp"
#include <stdexcept>

PDHCounter::PDHCounter(const std::string& CategoryName, const std::string& CounterName, const std::string& instanceName)
	: CategoryName(CategoryName), CounterName(CounterName), instanceName(instanceName) {
	if (PdhOpenQuery(NULL, NULL, &this->hQuery) != ERROR_SUCCESS) throw std::runtime_error("Failed to open PDH Query");
	PdhAddCounterA(this->hQuery, ("\\" + CategoryName + "(" + instanceName + ")\\" + CounterName).c_str(), NULL, &this->hCounter);
}

PDHCounter::PDHCounter(const std::string& CategoryName, const std::string& CounterName)
	: CategoryName(CategoryName), CounterName(CounterName), instanceName() {
	if (PdhOpenQuery(NULL, NULL, &this->hQuery) != ERROR_SUCCESS) throw std::runtime_error("Failed to open PDH Query");
	PdhAddCounterA(this->hQuery, ("\\" + CategoryName + "\\" + CounterName).c_str(), NULL, &this->hCounter);
}

void PDHCounter::Update() const {
	PdhCollectQueryData(this->hQuery);
}

double PDHCounter::GetDoubleValue() const {
	PDH_FMT_COUNTERVALUE val{};
	PdhGetFormattedCounterValue(this->hCounter, PDH_FMT_DOUBLE, NULL, &val);
	return val.doubleValue;
}

LONGLONG PDHCounter::GetInt64Value() const {
	PDH_FMT_COUNTERVALUE val{};
	PdhGetFormattedCounterValue(this->hCounter, PDH_FMT_LARGE, NULL, &val);
	return val.largeValue;
}

long PDHCounter::GetLongValue() const {
	PDH_FMT_COUNTERVALUE val{};
	PdhGetFormattedCounterValue(this->hCounter, PDH_FMT_LARGE, NULL, &val);
	return val.longValue;
}
