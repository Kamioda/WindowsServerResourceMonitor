#include "JsonObject.hpp"

void JsonObject::insert(const std::string& key, const std::string& value) {
	this->obj.insert(std::make_pair(key, value));
}

void JsonObject::insert(const std::string& key, const double& value) {
	this->obj.insert(std::make_pair(key, value));
}

void JsonObject::insert(const std::string& key, const picojson::object& InsertObject) {
	this->obj.insert(std::make_pair(key, InsertObject));
}

void JsonObject::insert(const std::string& key, const picojson::array& InsertObject) {
	this->obj.insert(std::make_pair(key, InsertObject));
}

JsonObject::operator const picojson::object& () const noexcept { return this->obj; }
