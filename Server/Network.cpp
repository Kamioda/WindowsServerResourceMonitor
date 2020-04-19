#include "Network.hpp"
#include "PDHAssistFunctions.hpp"
#include "JsonObject.hpp"

namespace impl {
	NetworkReceive::NetworkReceive(const std::string& NetworkDeviceName) : PDHCounter("Network Adapter", "Bytes Received/sec", NetworkDeviceName) {}
	double NetworkReceive::Get() const { return digit(PDHCounter::GetDoubleValue()); }

	NetworkSend::NetworkSend(const std::string& NetworkDeviceName) : PDHCounter("Network Adapter", "Bytes Sent/sec", NetworkDeviceName) {}
	double NetworkSend::Get() const { return digit(PDHCounter::GetDoubleValue()); }
}

Network::Network(const std::string& NetworkDeviceName)
	: DeviceName(NetworkDeviceName), netReceive(NetworkDeviceName), netSend(NetworkDeviceName) {}

void Network::Update() const {
	this->netReceive.Update();
	this->netSend.Update();
}

picojson::object Network::Get() const {
	JsonObject obj{};
	obj.insert("device", this->DeviceName);
	obj.insert("receive", this->netReceive.Get());
	obj.insert("send", this->netSend.Get());
	return obj;
}

