#pragma once
#include "PDHCounter.hpp"
#include <picojson.h>
#include <utility>

namespace impl {
	class DiskSpaceInformation {
	private:
		std::string Drive;
		unsigned long long Total;
		unsigned long long Used;
		unsigned long long Free;
	public:
		DiskSpaceInformation(const std::string& TargetDrive = "C:");
		void Update();
		picojson::object GetTotal() const noexcept;
		picojson::object GetUsed() const noexcept;
		picojson::object GetFree() const noexcept;
		double GetUsedPer() const noexcept;
		double GetFreePer() const noexcept;
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

	class DiskReadAverage : public PDHCounter {
	public:
		DiskReadAverage(PDHQuery& query, const std::string& TargetDrive = "C:");
		double Get() const;
	};

	class DiskWriteAverage : public PDHCounter {
	public:
		DiskWriteAverage(PDHQuery& query, const std::string& TargetDrive = "C:");
		double Get() const;
	};
}

class Disk {
private:
	std::string Drive;
	impl::DiskSpaceInformation diskSpace;
	impl::DiskRead diskRead;
	impl::DiskWrite diskWrite;
public:
	Disk(PDHQuery& query, const std::string& TargetDrive = "C:");
	picojson::object Get() const;
	std::string GetKey() const noexcept;
	void Update();
};
