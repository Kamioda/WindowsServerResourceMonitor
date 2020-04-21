#include "Processor.hpp"
#include "PDHAssistFunctions.hpp"
#include "JsonObject.hpp"
#include <Psapi.h>
#include <intrin.h>

Processor::Processor(PDHQuery& query) 
	: PDHCounter(query, "Processor", "% Processor Time", "_Total"), ProcessNum() {
	int CPUInfo[4] = { -1 };
	__cpuid(CPUInfo, 0x80000000);
	if (CPUInfo[0] >= 0x80000004) {
		CHAR szCPUBrandString[0x40] = { 0 };
		__cpuid(CPUInfo, 0x80000002);
		memcpy(szCPUBrandString, CPUInfo, sizeof(CPUInfo));
		__cpuid(CPUInfo, 0x80000003);
		memcpy(szCPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
		__cpuid(CPUInfo, 0x80000004);
		memcpy(szCPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
		this->CPUName = std::string(szCPUBrandString);
	}
}

double Processor::GetUsage() const { return digit(PDHCounter::GetDoubleValue()); }

int Processor::GetProcessNum() const { return this->ProcessNum; }

void Processor::Update() {
	constexpr DWORD BufferSize = 1024;
	DWORD Buffer[BufferSize];
	EnumProcesses(Buffer, sizeof(Buffer), &this->ProcessNum);
	this->ProcessNum /= sizeof(DWORD);
}

picojson::object Processor::Get() const {
	JsonObject obj{};
	obj.insert("name", this->CPUName);
	obj.insert("usage", this->GetUsage());
	obj.insert("process", this->GetProcessNum());
	return obj;
}
