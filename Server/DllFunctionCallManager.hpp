#ifndef __DLLFUNCTIONCALLMANAGER_HPP__
#define __DLLFUNCTIONCALLMANAGER_HPP__
#include "GetErrorMessage.h"
#include <Windows.h>
#include <stdexcept>

class DllFunctionCallManager {
private:
	HMODULE hModule;
public:
	DllFunctionCallManager() : hModule(nullptr) {}
	DllFunctionCallManager(const std::string ModuleName) : hModule(LoadLibraryA(ModuleName.c_str())) { if (this->hModule == NULL) throw std::runtime_error("Failed to load dll\n" + GetErrorMessageA()); }
	DllFunctionCallManager(const std::wstring ModuleName) : hModule(LoadLibraryW(ModuleName.c_str())) { if (this->hModule == NULL) throw std::runtime_error("Failed to load dll\n" + GetErrorMessageA()); }
	DllFunctionCallManager(const DllFunctionCallManager&) = delete;
	DllFunctionCallManager(DllFunctionCallManager&& dcm) noexcept : hModule(std::move(dcm.hModule)) {}
	~DllFunctionCallManager() { FreeLibrary(this->hModule); }
	template<typename Func>
	Func GetFunctionAddress(const std::string FunctionName) const {
		FARPROC func = GetProcAddress(this->hModule, FunctionName.c_str());
		if (NULL == func) throw std::runtime_error("Failed to load dll\n" + GetErrorMessageA());
		return reinterpret_cast<Func>(func);
	}
};

template<class T>
inline std::string GetStringFromDllFunc(T func) {
	std::string str;
	str.resize(1024);
	char Arr[1024];
	func(Arr);
	str = Arr;
	str.resize(std::strlen(str.c_str()));
	return str;
}

#endif

