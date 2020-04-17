#pragma once
#include "CommandLineManager.h"
#include <Windows.h>
#include <string>
#include <vector>

/*
	CommandLineManager
	CommandLineManagerA
		Command line argument type						：std::vector<std::string>
		Return value of AlignCmdLineStrType function	: std::string
	CommandLineManagerw
		Command line argument type						：std::vector<std::wtring>
		Return value of AlignCmdLineStrType function	: std::wtring

*/

namespace CommandLineManager = CommandLineManagerA;

void Main_Console(HINSTANCE hInstance, const CommandLineManager::CommandLineType& CommandLines, int nCmdShow);
