#include "ConsoleMainProcess.h"
#include "Install.h"
#include "Uninstall.h"
#include <Windows.h>
#include <string>
#include <vector>

void Main_Console(HINSTANCE hInstance, const CommandLineManager::CommandLineType& CommandLines, int nCmdShow) {
	if (CommandLines.at(0) == CommandLineManager::AlignCmdLineStrType("install")) Install();
	else if (CommandLines.at(0) == CommandLineManager::AlignCmdLineStrType("uninstall")) Uninstall();
	else {
		// ここに他のコマンドライン引数が与えられた時の処理を書く

	}
}
