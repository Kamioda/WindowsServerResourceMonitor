#include "PDHQuery.hpp"
#include <stdexcept>

PDHQuery::PDHQuery() 
	: hQuery(nullptr, [](PDH_HQUERY& query) { PdhCloseQuery(query); }) {
	if (PdhOpenQuery(NULL, NULL, &this->hQuery) != ERROR_SUCCESS) throw std::runtime_error("Failed to open PDH Query");
}

void PDHQuery::Update() {
	PdhCollectQueryData(this->hQuery);
}

PDHQuery::operator const PDH_HQUERY& () const noexcept { return this->hQuery; }
