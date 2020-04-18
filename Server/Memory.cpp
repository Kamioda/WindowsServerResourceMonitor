#include "Memory.hpp"
#include "PDHAssistFunctions.hpp"
#include "JsonObject.hpp"

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

picojson::object Memory::GetPhysical() const {
	JsonObject physical{};
	physical.insert("total", this->GetPhysicalMaxMemSize());
	physical.insert("available", this->GetPhysicalAvailable());
	physical.insert("used", this->GetPhysicalUsage());
	physical.insert("usedper", this->GetPhysicalUsagePer());
	return physical;
}
picojson::object Memory::GetCommit() const {
	JsonObject commit{};
	commit.insert("total", this->GetCommitMaxMemSize());
	commit.insert("available", this->GetCommitAvailable());
	commit.insert("used", this->GetCommitUsage());
	commit.insert("usedper", this->GetCommitUsagePer());
	return commit;
}
picojson::object Memory::Get() const {
	JsonObject obj{};
	obj.insert("physical", this->GetPhysical());
	obj.insert("commit", this->GetCommit());
	return obj;
}
