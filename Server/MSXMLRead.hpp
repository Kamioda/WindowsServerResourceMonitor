#pragma once
#include "XmlDomElement.hpp"
#include "XmlDataManager.hpp"
#include <unordered_map>

namespace MSXML {
	class Read : public std::unordered_map<std::wstring, XmlDataManager> {
	private:
		using Base = std::unordered_map<std::wstring, XmlDataManager>;
		XmlDomDocument lpXmlDoc;
	public:
		Read(const std::wstring& FilePath);
		XmlDataManager Load(const std::wstring& Path);
	};
}
