#ifndef __SPLIT_HPP__
#define __SPLIT_HPP__
#include <string>
#include <vector>
#include <array>
#include <sstream>
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
	std::array<std::basic_string<T>, ArrSize> RetData;
	RetData.fill(T{});
	for (size_t i = 0; i < std::min(ArrSize, List.size()); i++) RetData[i] = std::move(List[i]);
	return RetData;
}

template<size_t ArrSize, typename T, std::enable_if_t<std::is_same<T, char>::value || std::is_same<T, wchar_t>::value, std::nullptr_t> = nullptr>
std::array<std::basic_string<T>, ArrSize> SplitString(const T*& str, const T sep) {
	return SplitString(std::basic_string<T>(str), sep);
}
#endif
