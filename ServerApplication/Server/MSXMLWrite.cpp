#include "../Common/SafeRelease.hpp"
#include "../Common/GetErrorMessage.h"
#include "MSXMLWrite.hpp"
#include "../Common/ComString.hpp"
#include <comdef.h>

namespace MSXML {
	Write::Write(const std::wstring& Root) : lpXmlDoc() {
		IXMLDOMProcessingInstruction* lpProcInst;
		ComString Target(L"xml");
		ComString Xml(L"version='1.0' encoding='UTF-8'");
		if (const HRESULT hr = this->lpXmlDoc->createProcessingInstruction(Target.get(), Xml.get(), &lpProcInst); FAILED(hr)) throw std::runtime_error(GetErrorMessageA(hr));
		if (const HRESULT hr = this->lpXmlDoc->appendChild(lpProcInst, NULL); FAILED(hr)) throw std::runtime_error(GetErrorMessageA(hr));
		SafeRelease(lpProcInst);
		this->lpRoot = XmlDomElement(this->lpXmlDoc, Root);
	}

	XmlDomElement Write::GenerateElement(const std::wstring& key) {
		return XmlDomElement(this->lpXmlDoc, key);
	}

	XmlDomElement Write::GenerateElement(const std::wstring& key, const std::wstring& data) {
		return XmlDomElement(this->lpXmlDoc, key, data);
	}

	XmlDomText Write::GenerateText(const std::wstring& text) {
		return XmlDomText(this->lpXmlDoc, text);
	}

	void Write::AddToRootElement(const XmlDomElement& elem) {
		this->lpRoot->appendChild(elem, NULL);
	}

	void Write::AddToRootElement(const XmlDomText& text) {
		this->lpRoot->appendChild(text, NULL);
	}

	void Write::Output(const std::wstring& FilePath) {
		if (const HRESULT hr = this->lpXmlDoc->appendChild(this->lpRoot, NULL); FAILED(hr)) throw std::runtime_error(GetErrorMessageA(hr));
		if (const HRESULT hr = this->lpXmlDoc->put_async(VARIANT_FALSE); FAILED(hr)) throw std::runtime_error(GetErrorMessageA(hr));
		if (const HRESULT hr = this->lpXmlDoc->save(_variant_t(FilePath.c_str())); FAILED(hr)) throw std::runtime_error(GetErrorMessageA(hr));
	}
}
