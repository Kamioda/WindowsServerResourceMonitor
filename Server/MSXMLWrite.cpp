#include "SafeRelease.hpp"
#include "../Common/GetErrorMessage.h"
#include "MSXMLWrite.hpp"
#include "ComString.hpp"
#include <comdef.h>
#define _B(str) SysAllocString(str)
#define _BFREE(str) SysFreeString(str)

namespace XmlWriteEngine {
	XmlDomDocument::XmlDomDocument() {
		if (
			const HRESULT hr = CoCreateInstance(CLSID_DOMDocument, nullptr, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&this->lpXmlDoc);
			this->lpXmlDoc == nullptr
			) throw std::runtime_error(GetErrorMessageA(hr));
		IXMLDOMProcessingInstruction* lpProcInst;
		ComString Target(L"xml");
		ComString Xml(L"version='1.0' encoding='UTF-8'");
		if (const HRESULT hr = this->lpXmlDoc->createProcessingInstruction(Target.get(), Xml.get(), &lpProcInst); FAILED(hr)) throw std::runtime_error(GetErrorMessageA(hr));
		if (const HRESULT hr = this->lpXmlDoc->appendChild(lpProcInst, NULL); FAILED(hr)) throw std::runtime_error(GetErrorMessageA(hr));
		SafeRelease(lpProcInst);
	}

	XmlDomDocument::XmlDomDocument(std::nullptr_t) : lpXmlDoc(nullptr) {}
	
	XmlDomDocument::~XmlDomDocument() { SafeRelease(this->lpXmlDoc); }
	
	XmlDomDocument::XmlDomDocument(XmlDomDocument&& x) noexcept : lpXmlDoc(x.lpXmlDoc) { x.lpXmlDoc = nullptr; }
	
	XmlDomDocument& XmlDomDocument::operator = (XmlDomDocument&& x) noexcept {
		this->lpXmlDoc = x.lpXmlDoc;
		x.lpXmlDoc = nullptr;
		return *this;
	}
	
	IXMLDOMDocument* XmlDomDocument::operator -> () noexcept { return this->lpXmlDoc; }
	
	IXMLDOMDocument* XmlDomDocument::operator -> () const noexcept { return this->lpXmlDoc; }
	
	XmlDomDocument::operator IXMLDOMDocument* () const noexcept { return this->lpXmlDoc; }

	XmlDomElement::XmlDomElement() : XmlDomElement(nullptr) {}

	XmlDomElement::XmlDomElement(std::nullptr_t) : element(nullptr) {}

	XmlDomElement::XmlDomElement(const XmlDomDocument& lpXmlDoc, const std::wstring& key) {
		ComString str(key);
		if (const HRESULT hr = lpXmlDoc->createElement(str.get(), &this->element); FAILED(hr)) throw std::runtime_error(GetErrorMessageA(hr));
	}

	XmlDomElement::XmlDomElement(const XmlDomDocument& lpXmlDoc, const std::wstring& key, const std::wstring& data) : XmlDomElement(lpXmlDoc, key){
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

MSXMLWrite::MSXMLWrite(const std::wstring& Root) : lpXmlDoc(), lpRoot(this->lpXmlDoc, Root) {}

XmlWriteEngine::XmlDomElement MSXMLWrite::GenerateElement(const std::wstring& key) {
	return XmlWriteEngine::XmlDomElement(this->lpXmlDoc, key);
}

XmlWriteEngine::XmlDomElement MSXMLWrite::GenerateElement(const std::wstring& key, const std::wstring& data) {
	return XmlWriteEngine::XmlDomElement(this->lpXmlDoc, key, data);
}

void MSXMLWrite::AddToRootElement(const XmlWriteEngine::XmlDomElement& elem) {
	this->lpRoot->appendChild(elem, NULL);
}

void MSXMLWrite::Output(const std::wstring& FilePath) {
	if (const HRESULT hr = this->lpXmlDoc->appendChild(this->lpRoot, NULL); FAILED(hr)) throw std::runtime_error(GetErrorMessageA(hr));
	if (const HRESULT hr = this->lpXmlDoc->put_async(VARIANT_FALSE); FAILED(hr)) throw std::runtime_error(GetErrorMessageA(hr));
	if (const HRESULT hr = this->lpXmlDoc->save(_variant_t(FilePath.c_str())); FAILED(hr)) throw std::runtime_error(GetErrorMessageA(hr));
}
