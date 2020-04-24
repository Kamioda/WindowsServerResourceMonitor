#pragma once
#include "PDHCounter.hpp"
#include <picojson/picojson.h>

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
	picojson::object Get() const;
};
