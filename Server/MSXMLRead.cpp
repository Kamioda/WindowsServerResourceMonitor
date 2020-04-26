#include "../Common/GetErrorMessage.hpp"
#include "../Common/StringCvt.hpp"
#include "../Common/CommandLineManager.hpp"
#include "SafeRelease.hpp"
#include "MSXMLRead.hpp"
#include "ComString.hpp"
#include <Shlwapi.h>
#include <type_traits>
#include <stdexcept>
#pragma comment(lib, "Shlwapi.lib")

namespace Replace {
	// code copy from make_array.h in repositry xml_text_cooking_quiz(Author:yumetodo)
	std::basic_string<TCHAR> LF(std::basic_string<TCHAR>&& Str) {
		if (_T("empty") == Str) return _T("");
		const std::basic_string<TCHAR> ReplaceTarget = _T("\\n");
		const std::basic_string<TCHAR> AfterReplace = _T("\n");
		for (size_t pos = Str.find(ReplaceTarget); std::string::npos != pos; pos = Str.find(ReplaceTarget, pos + AfterReplace.length())) {
			Str.replace(pos, ReplaceTarget.length(), AfterReplace);
		}
		return Str;
	}
}

std::basic_string<TCHAR> Node::operator [] (const long Count) const {
	IXMLDOMNode* lpItem;
	this->NodeList->get_item(Count, &lpItem);
	BSTR Strings;
	lpItem->get_text(&Strings);
	std::basic_string<TCHAR> Str = Replace::LF(
#if defined(UNICODE)
		Strings
#else
		Win32LetterConvert::WStringToString(Strings)
#endif
	);
	Str.resize(std::char_traits<TCHAR>::length(Str.c_str()));
	return Str;
}

Node::~Node() {
	SafeRelease(this->NodeList);
}

MSXMLRead::MSXMLRead(const std::basic_string<TCHAR> FileName, const std::basic_string<TCHAR> CommonPath) {
	if (FALSE == PathFileExists(FileName.c_str())) throw std::runtime_error(
#if defined(UNICODE)
		string::converter::stl::to_bytes(FileName)
#else
		FileName
#endif
		+ " : file is not found.");
	const HRESULT ErrorCode = CoCreateInstance(CLSID_DOMDocument, nullptr, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&this->lpXmlDoc);
	if (this->lpXmlDoc == nullptr) throw std::runtime_error("xml open failed.\nCause : " + GetErrorMessageA(ErrorCode));
	VARIANT_BOOL Result;
	this->lpXmlDoc->put_async(VARIANT_FALSE);
	this->lpXmlDoc->load(_variant_t(FileName.c_str()), &Result);
	if (0 == Result) {
		this->lpXmlDoc->Release();
		throw std::runtime_error("xml read failed");
	}
	this->CommonPath = CommonPath;
}

void MSXMLRead::Release() {
	SafeRelease(this->lpXmlDoc);
	this->Data.clear();
}

MSXMLRead::~MSXMLRead() {
	this->Release();
}

IXMLDOMNodeList* MSXMLRead::XmlSetNodeList(const std::basic_string<TCHAR> NodePath, long &Length) {
	ComString Path( 
#if defined(UNICODE)
		this->CommonPath + NodePath
#else
		Win32LetterConvert::StringToWString(this->CommonPath + NodePath)
#endif
	);
	if (NodePath.empty() || 0 == SysStringLen(Path.get())) return NULL;
	IXMLDOMNodeList* lpNodeList;
	if (E_INVALIDARG == this->lpXmlDoc->selectNodes(Path.get(), &lpNodeList)) {
		lpNodeList->Release();
		return NULL;
	}
	lpNodeList->get_length(&Length);
	return lpNodeList;
}

long MSXMLRead::CheckLength(const std::basic_string<TCHAR> NodePath) {
	long Length = 0;
	IXMLDOMNodeList* list = XmlSetNodeList(this->CommonPath + NodePath, Length);
	if (NULL == list) {
		list->Release();
		throw std::runtime_error("xml read failed");
	}
	list->Release();
	return Length;
}

void MSXMLRead::LoadFromFile(const std::basic_string<TCHAR> NodePath) {
	Node node{};
	node.NodePath = this->CommonPath + NodePath;
	node.NodePath.resize(std::char_traits<TCHAR>::length(node.NodePath.c_str()));
	node.NodeList = this->XmlSetNodeList(NodePath, node.Length);
	this->Data.emplace_back(node);
}

void MSXMLRead::ChangeCommonPath(const std::basic_string<TCHAR> NewRoot) {
	this->clear();
	this->CommonPath = NewRoot;
}

Node MSXMLRead::operator [] (const std::basic_string<TCHAR> NodePath) const {
	for (const auto& i : this->Data) if (i.NodePath == this->CommonPath + NodePath) return i;
	throw std::runtime_error(
#if defined(UNICODE)
		string::converter::stl::to_bytes(this->CommonPath + NodePath)
#else
		this->CommonPath + NodePath
#endif
		+ " : not found such node.");
}
