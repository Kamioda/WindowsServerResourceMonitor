#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "Processor.hpp"
#include "Memory.hpp"
#include "Disk.hpp"
#include "Network.hpp"
#include "ServiceMonitor.hpp"
#include <unordered_map>
#include <limits>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace {
	template<class T>
	concept ResourceClass = std::same_as<decltype(std::declval<T>().Get()), nlohmann::json>;
	template<class T>
	concept KeyIncludeResourceClass = std::same_as<decltype(std::declval<T>().Get()), nlohmann::json> && std::same_as<decltype(std::declval<T>().GetKey()), std::string>;
	template<KeyIncludeResourceClass Resource>
	nlohmann::json GetKeyIncludeResource(const std::vector<Resource>& resources, const std::string& Target, const std::string& NotFoundMessage) {
		if (std::strlen(Target.c_str()) == 0) return CreateJsonArray(resources);
		const auto it = std::find_if(resources.begin(), resources.end(), [&Target](const Resource& r) { return Target == r.GetKey(); });
		if (it == resources.end()) return CreateError(NotFoundMessage);
		return it->Get();
	}
	template<ResourceClass Resource>
	inline nlohmann::json CreateJsonArray(const std::vector<Resource>& list) {
		if (list.size() == 1) return list.front().Get();
		nlohmann::json json = nlohmann::json::array();
		for (const auto& i : list) json.emplace_back(i.Get());
		return json;
	}
	inline nlohmann::json CreateError(const std::string& Message) {
		nlohmann::json json{};
		json["error"] = Message;
		return json;
	}
}

class ResourceMonitor {
private:
	ServiceControlManager SCM;
	PDHQuery Query;
	Processor processor;
	Memory memory;
	std::vector<Disk> disk;
	std::vector<Network> network;
	std::vector<ServiceMonitor> services;
public:
	ResourceMonitor(const nlohmann::json& resources)
		: SCM(), Query(), processor(Query), memory(), disk(), network(), services() {
		for (const auto& i : resources["disks"]) this->disk.emplace_back(this->Query, i.get<std::string>());
		for (const auto& i : resources["netadapters"]) this->network.emplace_back(this->Query, i.get<std::string>());
		for (const auto& i : resources["services"]) this->services.emplace_back(this->SCM, i.get<std::string>());
	}
	void Update() {
		this->processor.UpdateProcessNum();
		this->memory.Update();
		for (auto& i : this->disk) i.Update();
		for (auto& i : this->services) i.Update();
	}
	nlohmann::json GetAllResources() const {
		nlohmann::json json{};
		json["cpu"] = this->processor.Get();
		json["memory"] = this->memory.Get();
		json["disk"] = CreateJsonArray(this->disk);
		json["network"] = CreateJsonArray(this->network);
		json["services"] = CreateJsonArray(this->services);
		return json;
	}
	nlohmann::json GetCPU() const { return this->processor.Get(); }
	nlohmann::json GetMemory() const { return this->memory.Get(); }
	nlohmann::json GetDisk(const std::string& DriveLetter = "") const {
		return GetKeyIncludeResource(this->disk, DriveLetter, "Drive Letter '" + DriveLetter + "' is not found");
	}
	nlohmann::json GetNetwork(const size_t& DeviceID = std::numeric_limits<size_t>::max()) const {
		if (DeviceID == std::numeric_limits<size_t>::max()) return CreateJsonArray(this->network);
		if (this->network.size() >= DeviceID) return CreateError("Network Adapter " + std::to_string(DeviceID) + " is not found.");
		return this->network.at(DeviceID).Get();
	}
	nlohmann::json GetService(const std::string& ServiceName = "") const {
		return GetKeyIncludeResource(this->services, ServiceName, "Service Name '" + ServiceName + "' is not found");
	}
};
