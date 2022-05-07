#pragma once
#include "PDHCounter.hpp"
#include "json.hpp"
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
		nlohmann::json GetTotal() const noexcept;
		nlohmann::json GetUsed() const noexcept;
		nlohmann::json GetFree() const noexcept;
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
}

class Disk {
private:
	std::string Drive;
	impl::DiskSpaceInformation diskSpace;
	impl::DiskRead diskRead;
	impl::DiskWrite diskWrite;
public:
	Disk(PDHQuery& query, const std::string& TargetDrive = "C:");
	nlohmann::json Get() const;
	std::string GetKey() const noexcept;
	void Update();
};
