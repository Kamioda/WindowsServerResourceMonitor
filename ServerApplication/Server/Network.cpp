#include "Network.hpp"
#include "PDHAssistFunctions.hpp"

namespace impl {
	NetworkReceive::NetworkReceive(PDHQuery& query, const std::string& NetworkDeviceName) : PDHCounter(query, "Network Adapter", "Bytes Received/sec", NetworkDeviceName) {}
	double NetworkReceive::Get() const { return digit(PDHCounter::GetDoubleValue()); }

	NetworkSend::NetworkSend(PDHQuery& query, const std::string& NetworkDeviceName) : PDHCounter(query, "Network Adapter", "Bytes Sent/sec", NetworkDeviceName) {}
	double NetworkSend::Get() const { return digit(PDHCounter::GetDoubleValue()); }
}

Network::Network(PDHQuery& query, const std::string& NetworkDeviceName)
	: DeviceName(NetworkDeviceName), netReceive(query, NetworkDeviceName), netSend(query, NetworkDeviceName) {}

nlohmann::json Network::Get() const {
	nlohmann::json json{};
	json["device"] = this->DeviceName;
	json["receive"] = this->netReceive.Get();
	json["send"] = this->netSend.Get();
	return json;
}

