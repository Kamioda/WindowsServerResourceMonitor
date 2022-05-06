#include "Memory.hpp"
#include "PDHAssistFunctions.hpp"

Memory::Memory() : ms({ sizeof(MEMORYSTATUSEX) }) {}

long long Memory::GetPhysicalTotal() const { return this->ms.ullTotalPhys; }

long long Memory::GetPhysicalAvail() const { return this->ms.ullAvailPhys; }

long long Memory::GetPhysicalUsed() const { return this->GetPhysicalTotal() - this->GetPhysicalAvail(); }

long long Memory::GetCommitTotal() const { return this->ms.ullTotalPageFile; }

long long Memory::GetCommitAvail() const { return this->ms.ullAvailPageFile; }

long long Memory::GetCommitUsed() const { return this->GetCommitTotal() - this->GetCommitAvail(); }

void Memory::Update() { GlobalMemoryStatusEx(&this->ms); }

double Memory::GetPhysicalMaxMemSize() const { return digit(ByteToMegaByte(this->GetPhysicalTotal())); }

double Memory::GetPhysicalAvailable() const { return digit(ByteToMegaByte(this->GetPhysicalAvail())); }

double Memory::GetPhysicalUsage() const { return digit(ByteToMegaByte(this->GetPhysicalUsed())); }

double Memory::GetPhysicalUsagePer() const { return digit(ToPercent(this->GetPhysicalUsed(), this->GetPhysicalTotal())); }

double Memory::GetCommitMaxMemSize() const { return digit(ByteToMegaByte(this->GetCommitTotal())); }

double Memory::GetCommitAvailable() const { return digit(ByteToMegaByte(this->GetCommitAvail())); }

double Memory::GetCommitUsage() const { return digit(ByteToMegaByte(this->GetCommitUsed())); }

double Memory::GetCommitUsagePer() const { return digit(ToPercent(this->GetCommitUsed(), this->GetCommitTotal())); }

nlohmann::json Memory::GetPhysical() const {
	nlohmann::json json{};
	json["total"] = this->GetPhysicalMaxMemSize();
	json["available"] = this->GetPhysicalAvailable();
	json["used"] = this->GetPhysicalUsage();
	json["usedper"] = this->GetPhysicalUsagePer();
	return json;
}
nlohmann::json Memory::GetCommit() const {
	nlohmann::json json{};
	json["total"] = this->GetCommitMaxMemSize();
	json["available"] = this->GetCommitAvailable();
	json["used"] = this->GetCommitUsage();
	json["usedper"] = this->GetCommitUsagePer();
	return json;
}
nlohmann::json Memory::Get() const {
	nlohmann::json json{};
	json["physical"] = this->GetPhysical();
	json["commit"] = this->GetCommit();
	return json;
}
