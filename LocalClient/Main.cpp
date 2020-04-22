#include "RequestManager.hpp"
#include "ResponseProcessingManager.hpp"
#include <thread>
#include <mutex>
std::mutex mutex;
picojson::object res;

namespace Config {
	constexpr const TCHAR* WindowTitle = _T("リソースマネージャー");
	constexpr int WindowWidth = 1280;
	constexpr int WindowHeight = 720;
	constexpr int StringSize = 18;
}

inline void InitDxLib() {
	if (-1 == DxLib::SetMultiThreadFlag(TRUE)) throw "Error in SetMultiThreadFlag function";
	if (-1 == DxLib::SetMainWindowText(Config::WindowTitle)) throw "Error in SetMainWindowText function";
	if (-1 == DxLib::SetOutApplicationLogValidFlag(FALSE)) throw "Error in SetMainWindowText function";
	if (-1 == DxLib::SetDoubleStartValidFlag(TRUE)) throw "Error in DoubleStartValidFlag function";
	if (-1 == DxLib::SetGraphMode(Config::WindowWidth, Config::WindowHeight, 32)) throw "Error in SetGraphMode function";
	if (-1 == DxLib::ChangeWindowMode(true)) throw "Error in ChangeWindowMode function";
	if (-1 == DxLib::SetBackgroundColor(255, 255, 255)) throw "Error in SetBackgroundColor function";
	if (-1 == DxLib::SetAlwaysRunFlag(TRUE)) throw "Error in SetAlwaysRunFlag function";
	if (-1 == DxLib::DxLib_Init()) throw "Failed to initialize.";
	if (-1 == DxLib::SetTransColor(255, 255, 255)) throw "Error in SetTransColor function";
	if (-1 == DxLib::SetDrawScreen(DX_SCREEN_BACK)) throw "Error in SetDrawScreen function";
}

void GetResourceInformation() {
	RequestManager request("localhost", 32768);
	picojson::object resVal{};
	while (1) {
		if (const int Result = request.GetAll(resVal, "/v1/"); Result == 0) {
			std::lock_guard<std::mutex> lock(mutex);
			res = std::move(resVal);
		}
		if (ProcessMessage() == -1) break;
	}
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	try {
		InitDxLib();
		StringManager string = StringManager("Font", Config::StringSize, Color("#000000"));
		ResponseProcessingManager resmgr(string);
		std::thread th(GetResourceInformation);
		th.detach();
		while (ProcessMessage() != -1) {
			ClearDrawScreen();
			resmgr.Draw();
			ScreenFlip();
			resmgr.ApplyViewParameter();
			if (res.size() != 0) resmgr.Update(res);
		}
	}
	catch (const std::exception& er) {
		std::string s = er.what();
		UNREFERENCED_PARAMETER(s);
	}
}

