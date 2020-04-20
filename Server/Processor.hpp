#pragma once
#include "PDHCounter.hpp"
#include <picojson.h>

class Processor : public PDHCounter {
private:
	typedef int (*GetProcessNumFunc)();
	DWORD ProcessNum;
public:
	Processor(PDHQuery& query);
private:
	double GetUsage() const;
	int GetProcessNum() const;
public:
	picojson::object Get() const;
};
