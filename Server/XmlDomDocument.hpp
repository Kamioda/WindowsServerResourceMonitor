#pragma once
#include <msxml.h>
#include <string>

namespace MSXML {
	class XmlDomDocument {
	private:
		IXMLDOMDocument* lpXmlDoc;
	public:
		XmlDomDocument();
		XmlDomDocument(std::nullptr_t);
		~XmlDomDocument();
		XmlDomDocument(const XmlDomDocument&) = delete;
		XmlDomDocument(XmlDomDocument&& x) noexcept;
		XmlDomDocument& operator = (const XmlDomDocument&) = delete;
		XmlDomDocument& operator = (XmlDomDocument&& x) noexcept;
		IXMLDOMDocument* operator -> () noexcept;
		IXMLDOMDocument* operator -> () const noexcept;
		operator IXMLDOMDocument* () const noexcept;
	};
}
