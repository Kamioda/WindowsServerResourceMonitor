#include "XmlDomDocument.hpp"
#include "GetErrorMessage.hpp"
#include "SafeRelease.hpp"

namespace MSXML {
	XmlDomDocument::XmlDomDocument() {
		if (
			const HRESULT hr = CoCreateInstance(CLSID_DOMDocument, nullptr, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&this->lpXmlDoc);
			this->lpXmlDoc == nullptr
			) throw std::runtime_error(GetErrorMessageA(hr));
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
}
