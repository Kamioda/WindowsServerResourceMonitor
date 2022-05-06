﻿#pragma once
#include "PDHCounter.hpp"
#include <picojson/picojson.h>

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
	void UpdateProcessNum();
	picojson::object Get() const;
};
