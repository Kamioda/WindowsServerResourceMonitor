#include "Processor.hpp"
#include "PDHAssistFunctions.hpp"
#include "JsonObject.hpp"
#include <Psapi.h>

Processor::Processor() : PDHCounter("Processor", "% Processor Time", "_Total"), ProcessNum() {}

double Processor::GetUsage() const { return digit(PDHCounter::GetDoubleValue()); }

int Processor::GetProcessNum() const { return this->ProcessNum; }

void Processor::Update() {
	PDHCounter::Update();
	constexpr DWORD BufferSize = 1024;
	DWORD Buffer[BufferSize];
	EnumProcesses(Buffer, sizeof(Buffer), &this->ProcessNum);
	this->ProcessNum /= sizeof(DWORD);
}

picojson::object Processor::Get() const {
	JsonObject obj{};
	obj.insert("usage", this->GetUsage());
	obj.insert("process", this->GetProcessNum());
}
