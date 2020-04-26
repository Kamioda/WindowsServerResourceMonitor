#include "XmlDomElement.hpp"
#include "ComString.hpp"
#include "../Common/GetErrorMessage.h"
#include "SafeRelease.hpp"

namespace MSXML {
	XmlDomElement::XmlDomElement() : XmlDomElement(nullptr) {}

	XmlDomElement::XmlDomElement(std::nullptr_t) : element(nullptr) {}

	XmlDomElement::XmlDomElement(const XmlDomDocument& lpXmlDoc, const std::wstring& key) {
		ComString str(key);
		if (const HRESULT hr = lpXmlDoc->createElement(str.get(), &this->element); FAILED(hr)) throw std::runtime_error(GetErrorMessageA(hr));
	}

	XmlDomElement::XmlDomElement(const XmlDomDocument& lpXmlDoc, const std::wstring& key, const std::wstring& data) : XmlDomElement(lpXmlDoc, key) {
		ComString str(data);
		IXMLDOMText* txt = nullptr;
		if (const HRESULT hr = lpXmlDoc->createTextNode(str.get(), &txt); FAILED(hr)) throw std::runtime_error(GetErrorMessageA(hr));
		this->element->appendChild(txt, NULL);
		SafeRelease(txt);
	}

	XmlDomElement::~XmlDomElement() {
		SafeRelease(this->element);
	}

	XmlDomElement::XmlDomElement(XmlDomElement&& x) noexcept : element(x.element) { x.element = nullptr; }

	XmlDomElement& XmlDomElement::operator = (XmlDomElement&& x) noexcept {
		this->element = x.element;
		x.element = nullptr;
		return *this;
	}

	IXMLDOMElement* XmlDomElement::operator -> () noexcept { return this->element; }

	IXMLDOMElement* XmlDomElement::operator -> () const noexcept { return this->element; }

	XmlDomElement::operator IXMLDOMElement* () const noexcept { return this->element; }

}
