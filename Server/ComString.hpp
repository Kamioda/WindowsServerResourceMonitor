#pragma once
#include <comdef.h>
#include <string>

class ComString {
private:
	BSTR str;
public:
	ComString() = default;
	ComString(std::nullptr_t);
	ComString(const wchar_t* str) noexcept;
	explicit ComString(const std::wstring& str) noexcept;
	~ComString() noexcept;
	ComString(const ComString&) = delete;
	ComString(ComString&& c) noexcept;
	ComString& operator = (const ComString&) = delete;
	ComString& operator = (ComString&& c) noexcept;
	BSTR get() const noexcept;
	bool operator == (std::nullptr_t) const noexcept;
	bool operator != (std::nullptr_t) const noexcept;
};
