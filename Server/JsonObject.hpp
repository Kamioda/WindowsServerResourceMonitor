#pragma once
#include <picojson/picojson.h>

class JsonObject {
private:
	picojson::object obj;
public:
	JsonObject() = default;
	void insert(const std::string& key, const std::string& value);
	void insert(const std::string& key, const double& value);
	void insert(const std::string& key, const picojson::object& InsertObject);
	void insert(const std::string& key, const picojson::array& InsertObject);
	operator const picojson::object& () const noexcept;
};
