#pragma once
#include "PDHCounter.hpp"
#include <picojson.h>

namespace impl {
	class DiskUsage : public PDHCounter {
	public:
		DiskUsage(PDHQuery& query, const std::string& TargetDrive = "C:");
		double Get() const;
	};

	class DiskRead : public PDHCounter {
	public:
		DiskRead(PDHQuery& query, const std::string& TargetDrive = "C:");
		double Get() const;
	};

	class DiskWrite : public PDHCounter {
	public:
		DiskWrite(PDHQuery& query, const std::string& TargetDrive = "C:");
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
	Disk(PDHQuery& query, const std::string& TargetDrive = "C:");
	picojson::object Get() const;
};
