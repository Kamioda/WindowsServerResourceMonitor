#pragma once
#include <picojson/picojson.h>

class JsonArray {
private:
	picojson::array arr;
public:
	JsonArray() = default;
	void insert(const picojson::object& obj);
	operator const picojson::array& () const noexcept;
};
