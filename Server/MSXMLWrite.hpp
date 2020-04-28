#pragma once
#include "../Common/XmlDomElement.hpp"
#include "../Common/XmlDomText.hpp"
#include <type_traits>

namespace MSXML {
	class Write {
	private:
		XmlDomDocument lpXmlDoc;
		XmlDomElement lpRoot;
	public:
		Write(const std::wstring& Root);
		XmlDomElement GenerateElement(const std::wstring& key);
		XmlDomElement GenerateElement(const std::wstring& key, const std::wstring& data);
		XmlDomText GenerateText(const std::wstring& text);
		template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, std::nullptr_t> = nullptr>
		XmlDomElement GenerateElement(const std::wstring& key, const T& data) {
			return this->GenerateElement(key, std::to_wstring(data));
		}
		void AddToRootElement(const XmlDomElement& elem);
		void AddToRootElement(const XmlDomText& text);
		void Output(const std::wstring& FilePath);
	};
}
