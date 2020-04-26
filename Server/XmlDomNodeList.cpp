#include "XmlDomNodeList.hpp"
#include "ComString.hpp"
#include "SafeRelease.hpp"
#include "GetErrorMessage.hpp"

namespace MSXML {
	XmlDomNodeList::XmlDomNodeList() : XmlDomNodeList(nullptr) {}

	XmlDomNodeList::XmlDomNodeList(std::nullptr_t) 
		: list(nullptr), length() {}
	
	XmlDomNodeList::XmlDomNodeList(const XmlDomDocument& lpXmlDoc, const std::wstring& NodePath) 
		: list(nullptr), length() {
		if (NodePath.empty()) throw std::runtime_error("NodePath is empty.");
		ComString Path(NodePath);
		if (const HRESULT hr = lpXmlDoc->selectNodes(Path.get(), &this->list); FAILED(hr)) throw std::runtime_error(GetErrorMessageA(hr));
		this->list->get_length(&this->length);
	}

	XmlDomNodeList::~XmlDomNodeList() {
		SafeRelease(this->list);
	}

	XmlDomNodeList::XmlDomNodeList(XmlDomNodeList&& x)
		: list(x.list), length(x.length) { x.list = nullptr; }

	XmlDomNodeList& XmlDomNodeList::operator = (XmlDomNodeList&& x) {
		this->list = x.list;
		this->length = x.length;
		x.list = nullptr;
		return *this;
	}

	IXMLDOMNodeList* XmlDomNodeList::operator -> () noexcept { return this->list; }

	IXMLDOMNodeList* XmlDomNodeList::operator -> () const noexcept { return this->list; }
	
	XmlDomNodeList::operator IXMLDOMNodeList* () const noexcept { return this->list; }

	std::wstring XmlDomNodeList::Get(const long count) const {
		IXMLDOMNode* lpItem = nullptr;
		try {
			if (const HRESULT hr = this->list->get_item(count, &lpItem); FAILED(hr)) throw std::runtime_error(GetErrorMessageA(hr));
			ComString stringBuffer{};
			if (const HRESULT hr = lpItem->get_text(&stringBuffer); FAILED(hr)) throw std::runtime_error(GetErrorMessageA(hr));
			std::wstring str{};
			str.resize(stringBuffer.len());
			str = stringBuffer.get();
			str.resize(std::wcslen(str.c_str()));
			SafeRelease(lpItem);
			return str;
		}
		catch (const std::exception& er) {
			SafeRelease(lpItem);
			throw er;
		}
	}

	long XmlDomNodeList::size() const noexcept { return this->length; }
}
