#pragma once
#include "XmlDomDocument.hpp"
#include <type_traits>

namespace MSXML {
	class XmlDomNodeList {
	private:
		IXMLDOMNodeList* list;
		long length;
	public:
		XmlDomNodeList();
		XmlDomNodeList(std::nullptr_t);
		XmlDomNodeList(const XmlDomDocument& lpXmlDoc, const std::wstring& NodePath);
		~XmlDomNodeList();
		XmlDomNodeList(const XmlDomNodeList&) = delete;
		XmlDomNodeList(XmlDomNodeList&& x) noexcept;
		XmlDomNodeList& operator = (const XmlDomNodeList&) = delete;
		XmlDomNodeList& operator = (XmlDomNodeList&& x) noexcept;
		IXMLDOMNodeList* operator -> () noexcept;
		IXMLDOMNodeList* operator -> () const noexcept;
		operator IXMLDOMNodeList* () const noexcept;
		std::wstring Get(const long count) const;
		long size() const noexcept;
	};
}
