#include "Disk.hpp"
#include "JsonObject.hpp"
#include "PDHAssistFunctions.hpp"

namespace impl {
	DiskUsage::DiskUsage(PDHQuery& query, const std::string& TargetDrive) : PDHCounter(query, "LogicalDisk", "% Free Space", TargetDrive) {}

	double DiskUsage::Get() const { return digit(100.0 - PDHCounter::GetDoubleValue()); }

	DiskRead::DiskRead(PDHQuery& query, const std::string& TargetDrive) : PDHCounter(query, "LogicalDisk", "Disk Reads/sec", TargetDrive) {}

	double DiskRead::Get() const { return digit(PDHCounter::GetDoubleValue()); }

	DiskWrite::DiskWrite(PDHQuery& query, const std::string& TargetDrive) : PDHCounter(query, "LogicalDisk", "Disk Writes/sec", TargetDrive) {}

	double DiskWrite::Get() const { return digit(PDHCounter::GetDoubleValue()); }
}

Disk::Disk(PDHQuery& query, const std::string& TargetDrive)
	: Drive(TargetDrive), diskUse(query, TargetDrive), diskRead(query, TargetDrive), diskWrite(query, TargetDrive) {}

picojson::object Disk::Get() const {
	JsonObject obj{};
	obj.insert("drive", this->Drive);
	obj.insert("used", this->diskUse.Get());
	obj.insert("read", this->diskRead.Get());
	obj.insert("write", this->diskWrite.Get());
	return obj;
}

std::string Disk::GetKey() const noexcept { return this->Drive; }
