#pragma once
#include <string>
#include <vector>
#include <array>
#include <type_traits>
#include <algorithm>

template<typename T, std::enable_if_t<std::is_same<T, char>::value || std::is_same<T, wchar_t>::value, std::nullptr_t> = nullptr>
std::vector<std::basic_string<T>> SplitString(const std::basic_string<T> &str, const T sep) {
	std::vector<std::basic_string<T>> Arr;
	std::basic_string<T> Item;
	for (T i : str) {
		if (i == sep) {
			if (!Item.empty()) Arr.push_back(Item);
			Item.clear();
		}
		else Item += i;
	}
	if (!Item.empty()) Arr.push_back(Item);
	return Arr;
}

template<typename T, std::enable_if_t<std::is_same<T, char>::value || std::is_same<T, wchar_t>::value, std::nullptr_t> = nullptr>
std::vector<std::basic_string<T>> SplitString(const T* str, const T sep) {
	return SplitString(std::basic_string<T>(str), sep);
}

template<size_t ArrSize, typename T, std::enable_if_t<std::is_same<T, char>::value || std::is_same<T, wchar_t>::value, std::nullptr_t> = nullptr>
std::array<std::basic_string<T>, ArrSize> SplitString(const std::basic_string<T>& str, const T sep) {
	const std::vector<std::basic_string<T>> List = SplitString<T>(str, sep);
	if (List.size() > ArrSize) throw std::runtime_error("Split data is over.");
	std::array<std::basic_string<T>, ArrSize> RetData{};
	RetData.fill(T{});
	for (size_t i = 0; i < std::min(ArrSize, List.size()); i++) RetData[i] = std::move(List[i]);
	return RetData;
}

template<size_t ArrSize, typename T, std::enable_if_t<std::is_same<T, char>::value || std::is_same<T, wchar_t>::value, std::nullptr_t> = nullptr>
std::array<std::basic_string<T>, ArrSize> SplitString(const T*& str, const T sep) {
	return SplitString(std::basic_string<T>(str), sep);
}

#if _HAS_CXX17
#include <string_view>

#if _HAS_CXX20
#include <utility>

template<typename T>
constexpr T exchange(T& data, T&& newVal) {
	return std::exchange(data, newVal);
}
#else
template<typename T>
inline T exchange(T& data, T&& newVal) {
	const T ret = data;
	data = std::move(newVal);
	return ret;
}
#endif

template<typename T, std::enable_if_t<std::is_same<T, char>::value || std::is_same<T, wchar_t>::value, std::nullptr_t> = nullptr>
std::vector<std::basic_string_view<T>> SplitString(const std::basic_string_view<T>& str, const T sep) {
	std::vector<std::basic_string_view<T>> Arr{};
	size_t start = 0, last = 0;
	for (const T& i : str) {
		if (i == sep) Arr.push_back(std::string_view(str.begin() + exchange(start, last + 1), str.begin() + last));
		last++;
	}
	if (last > start) Arr.push_back(std::string_view(str.begin() + start, str.begin() + last));
	return Arr;
}

template<typename T, std::enable_if_t<std::is_same<T, char>::value || std::is_same<T, wchar_t>::value, std::nullptr_t> = nullptr>
std::vector<std::basic_string_view<T>> SplitString(const T* str, const T sep) {
	return SplitString(std::basic_string_view<T>(str), sep);
}

template<size_t ArrSize, typename T, std::enable_if_t<std::is_same<T, char>::value || std::is_same<T, wchar_t>::value, std::nullptr_t> = nullptr>
std::array<std::basic_string_view<T>, ArrSize> SplitString(const std::basic_string_view<T>& str, const T sep) {
	const std::vector<std::basic_string_view<T>> List = SplitString<T>(str, sep);
	if (List.size() > ArrSize) throw std::runtime_error("Split data is over.");
	std::array<std::basic_string_view<T>, ArrSize> RetData{};
	RetData.fill(T{});
	for (size_t i = 0; i < std::min(ArrSize, List.size()); i++) RetData[i] = std::move(List[i]);
	return RetData;
}

template<size_t ArrSize, typename T, std::enable_if_t<std::is_same<T, char>::value || std::is_same<T, wchar_t>::value, std::nullptr_t> = nullptr>
std::array<std::basic_string_view<T>, ArrSize> SplitString(const T*& str, const T sep) {
	return SplitString(std::basic_string_view<T>(str), sep);
}
#endif
