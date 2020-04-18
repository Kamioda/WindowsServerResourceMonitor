#include "Disk.hpp"
#include "JsonObject.hpp"
#include "PDHAssistFunctions.hpp"

namespace impl {
	DiskUsage::DiskUsage(const std::string& TargetDrive) : PDHCounter("LogicalDisk", "% Free Space", TargetDrive) {}

	double DiskUsage::Get() const { return digit(100.0 - PDHCounter::GetDoubleValue()); }

	DiskRead::DiskRead(const std::string& TargetDrive) : PDHCounter("LogicalDisk", "Disk Reads/sec", TargetDrive) {}

	double DiskRead::Get() const { return digit(PDHCounter::GetDoubleValue()); }

	DiskWrite::DiskWrite(const std::string& TargetDrive) : PDHCounter("LogicalDisk", "Disk Writes/sec", TargetDrive) {}

	double DiskWrite::Get() const { return digit(PDHCounter::GetDoubleValue()); }
}

Disk::Disk(const std::string& TargetDrive)
	: diskUse(TargetDrive), diskRead(TargetDrive), diskWrite(TargetDrive) {}

void Disk::Update() const {
	this->diskUse.Update();
	this->diskRead.Update();
	this->diskWrite.Update();
}

picojson::object Disk::Get() const {
	JsonObject obj{};
	obj.insert("used", this->diskUse.Get());
	obj.insert("read", this->diskRead.Get());
	obj.insert("write", this->diskWrite.Get());
	return obj;
}
