#pragma once
#include <msxml.h>
#include <string>
#include <type_traits>

namespace XmlWriteEngine {
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

class MSXMLWrite {
private:
	XmlWriteEngine::XmlDomDocument lpXmlDoc;
	XmlWriteEngine::XmlDomElement lpRoot;
public:
	MSXMLWrite(const std::wstring& Root);
	XmlWriteEngine::XmlDomElement GenerateElement(const std::wstring& key);
	XmlWriteEngine::XmlDomElement GenerateElement(const std::wstring& key, const std::wstring& data);
	template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, std::nullptr_t> = nullptr>
	XmlWriteEngine::XmlDomElement GenerateElement(const std::wstring& key, const T& data) {
		return this->GenerateElement(key, std::to_wstring(data));
	}
	void AddToRootElement(const XmlWriteEngine::XmlDomElement& elem);
	void Output(const std::wstring& FilePath);
};
