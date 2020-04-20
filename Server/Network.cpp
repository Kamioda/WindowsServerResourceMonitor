#include "Network.hpp"
#include "PDHAssistFunctions.hpp"
#include "JsonObject.hpp"

namespace impl {
	NetworkReceive::NetworkReceive(PDHQuery& query, const std::string& NetworkDeviceName) : PDHCounter(query, "Network Adapter", "Bytes Received/sec", NetworkDeviceName) {}
	double NetworkReceive::Get() const { return digit(PDHCounter::GetDoubleValue()); }

	NetworkSend::NetworkSend(PDHQuery& query, const std::string& NetworkDeviceName) : PDHCounter(query, "Network Adapter", "Bytes Sent/sec", NetworkDeviceName) {}
	double NetworkSend::Get() const { return digit(PDHCounter::GetDoubleValue()); }
}

Network::Network(PDHQuery& query, const std::string& NetworkDeviceName)
	: DeviceName(NetworkDeviceName), netReceive(query, NetworkDeviceName), netSend(query, NetworkDeviceName) {}

picojson::object Network::Get() const {
	JsonObject obj{};
	obj.insert("device", this->DeviceName);
	obj.insert("receive", this->netReceive.Get());
	obj.insert("send", this->netSend.Get());
	return obj;
}

