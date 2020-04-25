#pragma once
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <msxml.h>
#include <tchar.h>

class Node {
public:
	Node() = default;
	~Node();
	std::basic_string<TCHAR> NodePath;
	IXMLDOMNodeList* NodeList;
	long Length;
	std::basic_string<TCHAR> operator [] (const long Count) const; // ちょっと長いからcpp側に定義
};

class MSXMLRead {
private:
	IXMLDOMDocument* lpXmlDoc;
	IXMLDOMNodeList* XmlSetNodeList(const std::basic_string<TCHAR> NodePath, long &Length);
	std::vector<Node> Data;
	std::basic_string<TCHAR> CommonPath;
	void LoadFromFile(const std::basic_string<TCHAR> NodePath);
public:
	MSXMLRead(const std::basic_string<TCHAR> FileName, const std::basic_string<TCHAR> CommonPath = TEXT(""));
	~MSXMLRead();
	long CheckLength(const std::basic_string<TCHAR> NodePath);
	void Check(const std::basic_string<TCHAR> NodePath) { this->CheckLength(NodePath); }
	template<class ...Args> void Check(const std::basic_string<TCHAR> NodePath, Args ...arg) {
		this->CheckLength(NodePath);
		this->Check(std::forward<Args>(arg)...);
	}
	void Load(const std::basic_string<TCHAR> NodePath) { this->LoadFromFile(NodePath); }
	template<class ...Args> void Load(const std::basic_string<TCHAR> NodePath, Args ...arg) {
		this->LoadFromFile(NodePath);
		this->Load(std::forward<Args>(arg)...);
	}
	void Release();
	size_t size() const { return this->Data.size(); }
	Node at(const size_t Count) const { return this->Data[Count]; }
	void clear() { this->Data.clear(); }
	void ChangeCommonPath(const std::basic_string<TCHAR> NewRoot);
	Node operator [] (const size_t Count) const { return this->at(Count); }
	Node operator [] (const std::basic_string<TCHAR> NodePath) const; // WStringToStringを使う関係でcpp側に定義
	auto begin() const noexcept { return this->Data.begin(); }
	auto end() const noexcept { return this->Data.end(); }
};
