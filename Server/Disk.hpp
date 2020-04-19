#pragma once
#include "PDHCounter.hpp"
#include <picojson.h>

namespace impl {
	class DiskUsage : public PDHCounter {
	public:
		DiskUsage(const std::string& TargetDrive = "C:");
		double Get() const;
	};

	class DiskRead : public PDHCounter {
	public:
		DiskRead(const std::string& TargetDrive = "C:");
		double Get() const;
	};

	class DiskWrite : public PDHCounter {
	public:
		DiskWrite(const std::string& TargetDrive = "C:");
		double Get() const;
	};
}

class Disk {
private:
	std::string Drive;
	impl::DiskUsage diskUse;
	impl::DiskRead diskRead;
	impl::DiskWrite diskWrite;
public:
	Disk(const std::string& TargetDrive = "C:");
	void Update() const;
	picojson::object Get() const;
};
