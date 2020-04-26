#include "ComString.hpp"

ComString::ComString(std::nullptr_t) : str(nullptr) {}

ComString::ComString(const wchar_t* str) noexcept : str(SysAllocString(str)) {}

ComString::ComString(const std::wstring& str) noexcept : ComString(str.c_str()) {}

ComString::ComString(ComString&& c) noexcept {
	this->str = new wchar_t[sizeof(c.str)];
	wcscpy_s(this->str, sizeof(c.str), c.str);
	c.str = nullptr;
}

ComString& ComString::operator = (ComString&& c) noexcept {
	this->str = new wchar_t[sizeof(c.str)];
	wcscpy_s(this->str, sizeof(c.str), c.str);
	c.str = nullptr;
	return *this;
}

ComString::~ComString() noexcept {
	if (this->str != nullptr) {
		SysFreeString(this->str);
		this->str = nullptr;
	}
}
bool ComString::operator == (std::nullptr_t) const noexcept { return this->str == nullptr; }

bool ComString::operator != (std::nullptr_t) const noexcept { return this->str != nullptr; }

BSTR ComString::get() const noexcept { return this->str; }

BSTR* ComString::operator & () noexcept { return &this->str; }

unsigned int ComString::len() const noexcept { return SysStringLen(this->str); }
