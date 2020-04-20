#include "Processor.hpp"
#include "PDHAssistFunctions.hpp"
#include "JsonObject.hpp"
#include <Psapi.h>

Processor::Processor(PDHQuery& query) : PDHCounter(query, "Processor", "% Processor Time", "_Total"), ProcessNum() {}

double Processor::GetUsage() const { return digit(PDHCounter::GetDoubleValue()); }

int Processor::GetProcessNum() const { return this->ProcessNum; }

picojson::object Processor::Get() const {
	JsonObject obj{};
	obj.insert("usage", this->GetUsage());
	obj.insert("process", this->GetProcessNum());
	return obj;
}
