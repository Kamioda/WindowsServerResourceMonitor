#include "Disk.hpp"
#include "JsonObject.hpp"
#include "PDHAssistFunctions.hpp"

namespace impl {
	DiskSpaceInformation::DiskSpaceInformation(const std::string& TargetDrive) 
		: Drive(TargetDrive), Total(), Used(), Free() {}
	
	void DiskSpaceInformation::Update() {
		ULARGE_INTEGER TotalInfo{}, Avail{};
		if (FALSE == GetDiskFreeSpaceExA(this->Drive.c_str(), NULL, &TotalInfo, &Avail)) return;
		this->Total = TotalInfo.QuadPart;
		this->Used = TotalInfo.QuadPart - Avail.QuadPart;
		this->Free = Avail.QuadPart;
	}
	
	nlohmann::json DiskSpaceInformation::GetTotal() const noexcept {
		nlohmann::json json{};
		json["capacity"] = this->Total;
		return json;
	}
	
	nlohmann::json DiskSpaceInformation::GetUsed() const noexcept {
		nlohmann::json json{};
		json["capacity"] = this->Used;
		json["per"] = this->GetUsedPer();
		return json;
	}
	
	nlohmann::json DiskSpaceInformation::GetFree() const noexcept {
		nlohmann::json json{};
		json["capacity"] = this->Free;
		json["per"] = this->GetFreePer();
		return json;
	}

	double DiskSpaceInformation::GetUsedPer() const noexcept {
		return digit(static_cast<double>(this->Used) * 100.0 / static_cast<double>(this->Total));
	}

	double DiskSpaceInformation::GetFreePer() const noexcept {
		return digit(static_cast<double>(this->Free) * 100.0 / static_cast<double>(this->Total));
	}

	DiskRead::DiskRead(PDHQuery& query, const std::string& TargetDrive) : PDHCounter(query, "LogicalDisk", "Disk Read Bytes/sec", TargetDrive) {}

	double DiskRead::Get() const { return digit(PDHCounter::GetDoubleValue()); }

	DiskWrite::DiskWrite(PDHQuery& query, const std::string& TargetDrive) : PDHCounter(query, "LogicalDisk", "Disk Write Bytes/sec", TargetDrive) {}

	double DiskWrite::Get() const { return digit(PDHCounter::GetDoubleValue()); }
}

Disk::Disk(PDHQuery& query, const std::string& TargetDrive)
	: Drive(TargetDrive), diskSpace(TargetDrive), diskRead(query, TargetDrive), diskWrite(query, TargetDrive) {}

nlohmann::json Disk::Get() const {
	nlohmann::json json{};
	json["drive"] = this->Drive;
	json["total"] = this->diskSpace.GetTotal();
	json["used"] = this->diskSpace.GetUsed();
	json["free"] = this->diskSpace.GetFree();
	json["read"] = this->diskRead.Get();
	json["write"] = this->diskWrite.Get();
	return json;
}

void Disk::Update() {
	this->diskSpace.Update();
}

std::string Disk::GetKey() const noexcept { return this->Drive; }
