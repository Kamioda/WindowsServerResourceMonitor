#pragma once
#include "StringCvt.hpp"
#include <vector>
#include <functional>
#include <type_traits>

namespace CmdLineMgrStringConverter {
	template<typename OutCharType, typename InCharType> constexpr bool InAndOutAreSame = std::is_same_v<OutCharType, InCharType>;
	template<typename OutCharType, typename InCharType> constexpr bool InIsWCharAndOutIsChar = std::is_same_v<OutCharType, char> && std::is_same_v<InCharType, wchar_t>;
	template<typename OutCharType, typename InCharType> constexpr bool InIsCharAndOutIsWChar = std::is_same_v<OutCharType, wchar_t> && std::is_same_v<InCharType, char>;

	template<typename OutCharType, typename InCharType, std::enable_if_t<InAndOutAreSame<OutCharType, InCharType>, std::nullptr_t> = nullptr>
	inline std::basic_string<OutCharType> Convert(const InCharType* str) {
		return std::basic_string<OutCharType>(str);
	}

	template<typename OutCharType, typename InCharType, std::enable_if_t<InIsWCharAndOutIsChar<OutCharType, InCharType>, std::nullptr_t> = nullptr>
	inline std::basic_string<OutCharType> Convert(const InCharType* str) {
		return string::converter::stl::to_bytes(str);
	}

	template<typename OutCharType, typename InCharType, std::enable_if_t<InIsCharAndOutIsWChar<OutCharType, InCharType>, std::nullptr_t> = nullptr>
	inline std::basic_string<OutCharType> Convert(const InCharType* str) {
		return string::converter::stl::from_bytes(str);
	}

	template<typename OutCharType, typename InCharType, std::enable_if_t<!std::is_same_v<InCharType, OutCharType>, std::nullptr_t> = nullptr>
	inline std::vector<std::basic_string<OutCharType>> Convert(const std::vector<std::basic_string<InCharType>>& arr, const std::function<std::basic_string<OutCharType>(const std::basic_string<InCharType>&)> ConvertFunc) {
		std::vector<std::basic_string<OutCharType>> RetVal{};
		for (const auto& i : arr) RetVal.emplace_back(ConvertFunc(i));
		return RetVal;
	}

	template<typename OutCharType, typename InCharType, std::enable_if_t<InIsWCharAndOutIsChar<OutCharType, InCharType>, std::nullptr_t> = nullptr>
	inline std::vector<std::basic_string<OutCharType>> Convert(const std::vector<std::basic_string<InCharType>>& arr) {
		return Convert<OutCharType, InCharType>(arr, string::converter::stl::to_bytes);
	}

	template<typename OutCharType, typename InCharType, std::enable_if_t<InIsCharAndOutIsWChar<OutCharType, InCharType>, std::nullptr_t> = nullptr>
	inline std::vector<std::basic_string<OutCharType>> Convert(const std::vector<std::basic_string<InCharType>>& arr) {
		return Convert<OutCharType, InCharType>(arr, string::converter::stl::from_bytes);
	}

}

namespace CommandLine {
	namespace impl {
		template<typename OutCharType, typename InCharType>
		std::vector<std::basic_string<OutCharType>> GetCommandLineArg(const InCharType* str) {
			int ArgSize = 0;
			LPWSTR* lpConvertedCmdLine = CommandLineToArgvW(CmdLineMgrStringConverter::Convert<wchar_t, InCharType>(str).c_str(), &ArgSize);
			std::vector<std::basic_string<OutCharType>> Arr{};
			for (int i = 0; i < ArgSize; i++)
				Arr.emplace_back(CmdLineMgrStringConverter::Convert<OutCharType, wchar_t>(lpConvertedCmdLine[i]));
			return Arr;
		}
	}
	namespace CharArg {
		inline std::vector<std::string> GetCommandLineArg(const char* lpCmdLine) { return impl::GetCommandLineArg<char, char>(lpCmdLine); }
		inline std::vector<std::string> GetCommandLineArg(const wchar_t* lpCmdLine) { return impl::GetCommandLineArg<char, wchar_t>(lpCmdLine); }
		inline std::vector<std::string> GetCommandLineArg(const std::vector<std::string>& args) { return args; }
		inline std::vector<std::string> GetCommandLineArg(const std::vector<std::wstring>& args) { return CmdLineMgrStringConverter::Convert<char, wchar_t>(args); }
		inline std::string AlignCmdLineStrType(const std::string& str) { return str; }
		inline std::string AlignCmdLineStrType(const std::wstring& str) { return string::converter::stl::to_bytes(str); }
		using CommandLineStringType = std::string;
		using CommandLineType = std::vector<std::string>;
	}
	namespace WCharArg {
		inline std::vector<std::wstring> GetCommandLineArg(const char* lpCmdLine) { return impl::GetCommandLineArg<wchar_t, char>(lpCmdLine); }
		inline std::vector<std::wstring> GetCommandLineArg(const wchar_t* lpCmdLine) { return impl::GetCommandLineArg<wchar_t, wchar_t>(lpCmdLine); }
		inline std::vector<std::wstring> GetCommandLineArg(const std::vector<std::string>& args) { return CmdLineMgrStringConverter::Convert<wchar_t, char>(args); }
		inline std::vector<std::wstring> GetCommandLineArg(const std::vector<std::wstring>& args) { return args; }
		inline std::wstring AlignCmdLineStrType(const std::string& str) { return string::converter::stl::from_bytes(str); }
		inline std::wstring AlignCmdLineStrType(const std::wstring& str) { return str; }
		using CommandLineStringType = std::wstring;
		using CommandLineType = std::vector<std::wstring>;
	}
}
namespace CommandLineManagerA = CommandLine::CharArg;
namespace CommandLineManagerW = CommandLine::WCharArg;
