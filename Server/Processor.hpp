#pragma once
#include "PDHCounter.hpp"
#include <picojson.h>

class Processor : public PDHCounter {
private:
	typedef int (*GetProcessNumFunc)();
	DWORD ProcessNum;
public:
	Processor() : PDHCounter("Processor", "% Processor Time", "_Total"), ProcessNum() {}
private:
	double GetUsage() const;
	int GetProcessNum() const;
public:
	void Update();
	picojson::object Get() const;
};
