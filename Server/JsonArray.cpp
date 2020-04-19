#include "JsonArray.hpp"

void JsonArray::insert(const picojson::object& obj) {
	this->arr.push_back(picojson::value(obj));
}

JsonArray::operator const picojson::array& () const noexcept { return this->arr; }
