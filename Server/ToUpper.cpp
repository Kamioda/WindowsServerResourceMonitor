#include <string>
#include <algorithm>
#pragma warning(disable: 4244)

std::string ToUpper(const std::string& str) {
	std::string s = str;
	std::transform(s.begin(), s.end(), s.begin(), std::toupper);
	return s;
}
