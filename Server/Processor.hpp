#pragma once
#include "PDHCounter.hpp"
#include <picojson.h>

class Processor : public PDHCounter {
private:
	typedef int (*GetProcessNumFunc)();
	DWORD ProcessNum;
	std::string CPUName;
public:
	Processor(PDHQuery& query);
private:
	double GetUsage() const;
	int GetProcessNum() const;
public:
	void Update();
	picojson::object Get() const;
};
