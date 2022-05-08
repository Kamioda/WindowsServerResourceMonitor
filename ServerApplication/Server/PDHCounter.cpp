#include "PDHCounter.hpp"
#include "PDHAssistFunctions.hpp"
#include <stdexcept>
#pragma comment(lib, "pdh.lib")

PDHCounter::PDHCounter(PDHQuery& query, const std::string& CategoryName, const std::string& CounterName, const std::string& instanceName)
	: hQuery(query), CategoryName(CategoryName), CounterName(CounterName), instanceName(instanceName), hCounter(nullptr, [](PDH_HCOUNTER& counter) { PdhRemoveCounter(counter); }) {
	if (const PDH_STATUS ErrorCode = PdhAddCounterA(this->hQuery.get(), ("\\" + CategoryName + "(" + instanceName + ")\\" + CounterName).c_str(), NULL, &this->hCounter);
		ERROR_SUCCESS != ErrorCode)
		throw std::runtime_error("Failed to add PDH counter\nErrorCode : " + std::to_string(ErrorCode));
}

PDHCounter::PDHCounter(PDHQuery& query, const std::string& CategoryName, const std::string& CounterName)
	: hQuery(query), CategoryName(CategoryName), CounterName(CounterName), instanceName(), hCounter(nullptr, [](PDH_HCOUNTER& counter) { PdhRemoveCounter(counter); }) {
	if (const PDH_STATUS ErrorCode = PdhAddCounterA(this->hQuery.get(), ("\\" + CategoryName + "\\" + CounterName).c_str(), NULL, &this->hCounter);
		ERROR_SUCCESS != ErrorCode)
		throw std::runtime_error("Failed to add PDH counter\nErrorCode : " + std::to_string(ErrorCode));
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
