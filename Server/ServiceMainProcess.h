#pragma once
#include <Windows.h>
#include <string>
#include <vector>

class ServiceProcess {
protected:
	DWORD Argc;
	std::vector<std::basic_string<TCHAR>> Argv;
public:
	virtual void Service_MainProcess() = 0;
public:
	ServiceProcess(DWORD argc, const std::vector<std::basic_string<TCHAR>>& args);
	virtual ~ServiceProcess() = 0;
};

ServiceProcess* GetServiceProcessInstance(DWORD argc, const std::vector<std::basic_string<TCHAR>>& args);
