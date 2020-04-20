#pragma once
#include "PDHQuery.hpp"
#include <string>
#include <functional>

class PDHCounter {
public:
	std::string CategoryName;
	std::string CounterName;
	std::string instanceName;
private:
	std::reference_wrapper<PDHQuery> hQuery;
	HandleManager<PDH_HCOUNTER> hCounter;
public:
	PDHCounter(PDHQuery& query, const std::string& CategoryName, const std::string& CounterName, const std::string& instanceName);
	PDHCounter(PDHQuery& query, const std::string& CategoryName, const std::string& CounterName);
	PDHCounter(const PDHCounter&) = delete;
	PDHCounter& operator = (const PDHCounter&) = delete;
	double GetDoubleValue() const;
	LONGLONG GetInt64Value() const;
	long GetLongValue() const;
};
