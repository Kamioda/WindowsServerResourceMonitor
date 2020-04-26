#pragma once
#include "../Common/GetErrorMessage.hpp"
#include <Windows.h>
#include <string>
#include <stdexcept>

namespace string {
	namespace converter {
		namespace stl {
			inline std::string to_bytes(const std::wstring& str) {
				const int iBufferSize = WideCharToMultiByte(CP_OEMCP, 0, str.c_str(), -1, (char*)NULL, 0, NULL, NULL);
				if (0 == iBufferSize) throw std::runtime_error(GetErrorMessageA());
				std::string oRet;
				oRet.resize(iBufferSize);
				WideCharToMultiByte(CP_OEMCP, 0, str.c_str(), -1, oRet.data(), iBufferSize, NULL, NULL);
				oRet.resize(std::char_traits<char>::length(oRet.c_str()));
				return oRet;
			}

			inline std::wstring from_bytes(const std::string& str) {
				const int iBufferSize = MultiByteToWideChar(CP_OEMCP, 0, str.c_str(), -1, (wchar_t*)NULL, 0);
				if (0 == iBufferSize) throw std::runtime_error(GetErrorMessageA());
				std::wstring oRet;
				oRet.resize(iBufferSize);
				MultiByteToWideChar(CP_OEMCP, 0, str.c_str(), -1, oRet.data(), iBufferSize);
				oRet.resize(std::char_traits<wchar_t>::length(oRet.c_str()));
				return oRet;
			}
		}
	}
}
