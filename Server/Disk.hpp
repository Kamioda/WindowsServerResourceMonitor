#pragma once
#include "PDHCounter.hpp"
#include <picojson.h>

namespace impl {
	class DiskUsage : public PDHCounter {
	public:
		DiskUsage(const std::string& TargetDrive = "C:") : PDHCounter("LogicalDisk", "% Free Space", TargetDrive) {}
		double Get() const { return digit(100.0 - PDHCounter::GetDoubleValue()); }
	};

	class DiskRead : public PDHCounter {
	public:
		DiskRead(const std::string& TargetDrive = "C:") : PDHCounter("LogicalDisk", "Disk Reads/sec", TargetDrive) {}
		double Get() const { return digit(PDHCounter::GetDoubleValue()); }
	};

	class DiskWrite : public PDHCounter {
	public:
		DiskWrite(const std::string& TargetDrive = "C:") : PDHCounter("LogicalDisk", "Disk Writes/sec", TargetDrive) {}
		double Get() const { return digit(PDHCounter::GetDoubleValue()); }
	};
}

class Disk {
private:
	impl::DiskUsage diskUse;
	impl::DiskRead diskRead;
	impl::DiskWrite diskWrite;
public:
	Disk(const std::string& TargetDrive = "C:");
	void Update() const;
	picojson::object Get() const;
};
