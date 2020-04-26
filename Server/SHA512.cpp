#include <openssl/sha.h>
#include "ToUpper.hpp"
#include <sstream>
#include <iomanip>
#ifdef _DEBUG
#pragma comment(lib, "libcrypto.lib")
#else
#pragma comment(lib, "libcrypto.lib")
#endif

std::string ToSHA512String(const std::string& str) {
	SHA512_CTX ctx{};
	unsigned char digest[SHA512_DIGEST_LENGTH];
	SHA512_Init(&ctx);
	SHA512_Update(&ctx, str.data(), str.size());
	SHA512_Final(digest, &ctx);
	std::stringstream ss{};
	for (int i = 0; i < SHA512_DIGEST_LENGTH; i++) ss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
	return ToUpper(ss.str());
}
