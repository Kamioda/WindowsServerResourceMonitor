#pragma once
#include "PDHCounter.hpp"
#include "json.hpp"

namespace impl {
	class NetworkReceive : public PDHCounter {
	public:
		NetworkReceive(PDHQuery& query, const std::string& NetworkDeviceName = "Realtek PCIe GBE Family Controller");
		double Get() const;
	};

	class NetworkSend : public PDHCounter {
	public:
		NetworkSend(PDHQuery& query, const std::string& NetworkDeviceName = "Realtek PCIe GBE Family Controller");
		double Get() const;
	};
}

class Network {
private:
	std::string DeviceName;
	impl::NetworkReceive netReceive;
	impl::NetworkSend netSend;
public:
	Network(PDHQuery& query, const std::string& NetworkDeviceName = "Realtek PCIe GBE Family Controller");
	nlohmann::json Get() const;
};
