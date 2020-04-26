#pragma once
#include "XmlDomDocument.hpp"

namespace MSXML {
	class XmlDomElement {
	private:
		IXMLDOMElement* element;
	public:
		XmlDomElement();
		XmlDomElement(std::nullptr_t);
		XmlDomElement(const XmlDomDocument& lpXmlDoc, const std::wstring& key);
		XmlDomElement(const XmlDomDocument& lpXmlDoc, const std::wstring& key, const std::wstring& data);
		~XmlDomElement();
		XmlDomElement(const XmlDomElement&) = delete;
		XmlDomElement(XmlDomElement&& x) noexcept;
		XmlDomElement& operator = (const XmlDomElement&) = delete;
		XmlDomElement& operator = (XmlDomElement&& x) noexcept;
		IXMLDOMElement* operator -> () noexcept;
		IXMLDOMElement* operator -> () const noexcept;
		operator IXMLDOMElement* () const noexcept;
	};
}
