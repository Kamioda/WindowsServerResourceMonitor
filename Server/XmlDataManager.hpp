#pragma once
#include "XmlDomNodeList.hpp"
#include "../Common/StringCvt.h"
#include <type_traits>
#include <vector>

namespace MSXML {
	namespace XmlDataManager {
		class wstring : public std::vector<std::wstring> {
		private:
			using Base = std::vector<std::wstring>;
		public:
			wstring() : Base() {}
			wstring(const XmlDomDocument& lpXmlDoc, const std::wstring& NodePath) {
				XmlDomNodeList nodeList(lpXmlDoc, NodePath);
				for (long i = 0; i < nodeList.size(); i++) Base::emplace_back(nodeList.Get(i));
			}
			template<typename I = int, std::enable_if_t<std::is_integral_v<I>&& std::is_signed_v<I>, std::nullptr_t> = nullptr>
			inline I at(const size_t count) const { return static_cast<I>(std::stoll(Base::at(count))); }
			template<typename U = unsigned int, std::enable_if_t<std::is_integral_v<U>&& std::is_unsigned_v<U>, std::nullptr_t> = nullptr>
			inline U at(const long count) const { return static_cast<U>(std::stoull(Base::at(count))); }
			template<typename F = double, std::enable_if_t<std::is_floating_point_v<F>, std::nullptr_t> = nullptr>
			inline F at(const long count) const { return static_cast<F>(std::stold(Base::at(count))); }
		};

		class string : public std::vector<std::string> {
		private:
			using Base = std::vector<std::string>;
		public:
			string(const wstring& w) { for (const auto& i : w) Base::emplace_back(::string::converter::stl::to_bytes(i)); }
		};

		template<typename T = int, std::enable_if_t<std::is_signed_v<T> && std::is_integral_v<T>, std::nullptr_t> = nullptr>
		class signed_integer : public std::vector<T> {
		private:
			using Base = std::vector<T>;
		public:
			signed_integer() : Base() {}
			signed_integer(const wstring& w) { for (const auto& i : w) Base::emplace_back(static_cast<T>(std::stoll(i))); }
		};

		template<typename T = unsigned int, std::enable_if_t<std::is_unsigned_v<T>&& std::is_integral_v<T>, std::nullptr_t> = nullptr>
		class unsigned_integer : public std::vector<T> {
		private:
			using Base = std::vector<T>;
		public:
			unsigned_integer() : Base() {}
			unsigned_integer(const wstring& w) { for (const auto& i : w) Base::emplace_back(static_cast<T>(std::stoull(i))); }
		};

		template<typename T = double, std::enable_if_t<std::is_floating_point_v<T>, std::nullptr_t> = nullptr>
		class floating_point : public std::vector<T> {
		private:
			using Base = std::vector<T>;
		public:
			floating_point() : Base() {}
			floating_point(const wstring& w) { for (const auto& i : w) Base::emplace_back(static_cast<T>(std::stold(i))); }
		};
	}
}
