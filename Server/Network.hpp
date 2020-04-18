#pragma once
#include "PDHCounter.hpp"
#include <picojson.h>

namespace impl {
	class NetworkReceive : public PDHCounter {
	public:
		NetworkReceive(const std::string& NetworkDeviceName = "Realtek PCIe GBE Family Controller");
		double Get() const;
	};

	class NetworkSend : public PDHCounter {
	public:
		NetworkSend(const std::string& NetworkDeviceName = "Realtek PCIe GBE Family Controller");
		double Get() const;
	};
}

class Network {
private:
	impl::NetworkReceive netReceive;
	impl::NetworkSend netSend;
public:
	Network(const std::string& NetworkDeviceName = "Realtek PCIe GBE Family Controller");
	void Update() const;
	picojson::object Get() const;
};
