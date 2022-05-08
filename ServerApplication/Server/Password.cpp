#include "SHA512.hpp"
constexpr const char* salt = "gdiasivkjaldvbadsu";

std::string HashPassword(const std::string& RawPassword) {
	std::string Ret = RawPassword + salt;
	for (int i = 0; i < 20; i++) Ret = ToSHA512String(Ret);
	return Ret;
}
