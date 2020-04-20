#pragma once
#include "HandleManager.h"
#include <Pdh.h>

class PDHQuery {
private:
	HandleManager<PDH_HQUERY> hQuery;
public:
	PDHQuery();
	void Update();
	operator const PDH_HQUERY& () const noexcept;
};
