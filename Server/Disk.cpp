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
	
	std::pair<double, std::string> convertUnit(const unsigned long long ByteVal) {
		if (const double d = ByteToTeraByte(ByteVal); d >= 1.0) return std::make_pair(d, "TB");
		if (const double d = ByteToGigaByte(ByteVal); d >= 1.0) return std::make_pair(d, "GB");
		if (const double d = ByteToMegaByte(ByteVal); d >= 1.0) return std::make_pair(d, "MB");
		if (const double d = ByteToKiloByte(ByteVal); d >= 1.0) return std::make_pair(d, "KB");
		return std::make_pair(static_cast<double>(ByteVal), "B");
	}

	inline void ConvertDiskInfoToJsonObject(JsonObject& obj, const std::pair<double, std::string>& val) {
		obj.insert("capacity", val.first);
		obj.insert("unit", val.second);
	}

	picojson::object DiskSpaceInformation::GetTotal() const noexcept {
		JsonObject obj{};
		ConvertDiskInfoToJsonObject(obj, convertUnit(this->Total));
		return obj;
	}
	
	picojson::object DiskSpaceInformation::GetUsed() const noexcept {
		JsonObject obj{};
		ConvertDiskInfoToJsonObject(obj, convertUnit(this->Used));
		obj.insert("per", this->GetUsedPer());
		return obj;
	}
	
	picojson::object DiskSpaceInformation::GetFree() const noexcept {
		JsonObject obj{};
		ConvertDiskInfoToJsonObject(obj, convertUnit(this->Free));
		obj.insert("per", this->GetFreePer());
		return obj;
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

picojson::object Disk::Get() const {
	JsonObject obj{};
	obj.insert("drive", this->Drive);
	obj.insert("total", this->diskSpace.GetTotal());
	obj.insert("used", this->diskSpace.GetUsed());
	obj.insert("free", this->diskSpace.GetFree());
	obj.insert("read", this->diskRead.Get());
	obj.insert("write", this->diskWrite.Get());
	return obj;
}

void Disk::Update() {
	this->diskSpace.Update();
}

std::string Disk::GetKey() const noexcept { return this->Drive; }
