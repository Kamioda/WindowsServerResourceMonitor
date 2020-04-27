#include "Auth.hpp"
#include "Split.hpp"
#include "../Common/StringCvt.h"
#include "../Common/MSXMLRead.hpp"
#include "MSXMLWrite.hpp"
#include "SHA512.hpp"
#include <Shlwapi.h>
#include <algorithm>
#include <functional>
#pragma comment(lib, "shlwapi.lib")

std::mt19937 InitEngine();
std::chrono::milliseconds GetCurrent();

AuthManager::AuthManager(const std::wstring& AuthInfoFilePath, const std::wstring& Root, const std::wstring DefaultUserInfoPath, const long long MaxExpirationTimeOfToken)
	: AuthInfoFilePath(AuthInfoFilePath), Root(Root), AuthInformation(), mt(InitEngine()), StartAuthDataSize(), MaxTokenExpirationTime(MaxExpirationTimeOfToken) {
	MSXML::Read xml = MSXML::Read(AuthInfoFilePath);
	for(const auto& i : xml.Get<std::string>(Root)) this->AuthInformation.emplace_back(i);
	if (const auto defuser = xml.Get<std::wstring>(DefaultUserInfoPath); !defuser.empty()) this->DefaultUser = defuser.front();
	this->StartAuthDataSize = this->AuthInformation.size();
}

AuthManager::~AuthManager() {
	if (this->AuthInfoFilePath.empty() || this->AuthInformation.size() == this->StartAuthDataSize) return;
	if (FALSE == PathFileExistsW(this->AuthInfoFilePath.c_str())) DeleteFileW(this->AuthInfoFilePath.c_str());
	const auto rootinfo = SplitString(this->Root, L'/');
	MSXML::Write writer(rootinfo.at(0));
	for (const auto& i : this->AuthInformation) writer.AddToRootElement(writer.GenerateElement(rootinfo.at(1), string::converter::stl::from_bytes(i)));
	writer.Output(this->AuthInfoFilePath);
}

AuthManager::AuthManager(AuthManager&& a) noexcept
	: AuthInfoFilePath(a.AuthInfoFilePath), AuthInformation(std::move(a.AuthInformation)), AccessToken(std::move(a.AccessToken)),
	mt(std::move(a.mt)), StartAuthDataSize(a.StartAuthDataSize), MaxTokenExpirationTime(a.MaxTokenExpirationTime) { a.AuthInfoFilePath.clear(); }

AuthManager& AuthManager::operator = (AuthManager&& a) noexcept {
	this->AuthInfoFilePath = a.AuthInfoFilePath;
	this->AuthInformation = std::move(a.AuthInformation);
	this->AccessToken = std::move(a.AccessToken);
	this->mt = std::move(a.mt);
	this->StartAuthDataSize = a.StartAuthDataSize;
	this->MaxTokenExpirationTime = a.MaxTokenExpirationTime;
	a.AuthInfoFilePath.clear();
	return *this;
}

std::string AuthManager::GenerateRandomString(const size_t length) const {
	std::string text = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	while (length > text.size()) text += text;
	std::shuffle(text.begin(), text.end(), this->mt);
	return text.substr(0, length);
}

std::string AuthManager::GenerateAuthKey(const std::string& User, const std::string& Pass) const {
	std::string str1 = User + Pass;
	std::string str2 = str1;
	std::string str3 = str1;
	std::string str4 = str1;
	std::reverse(str2.begin(), str2.end());
	std::sort(str3.begin(), str3.end(), std::less<char>());
	std::sort(str4.begin(), str4.end(), std::greater<char>());
	std::string genstr = str2 + str4 + str1 + str3;
	for (size_t i = 0; i < 35; i++) {
		genstr = ToSHA512String(genstr);
		std::reverse(genstr.begin(), genstr.end());
		str1 = this->GenerateRandomString(str1.size());
		str2 = this->GenerateRandomString(str2.size());
		str3 = this->GenerateRandomString(str3.size());
		str4 = this->GenerateRandomString(str4.size());
	}
	return User + ";" + genstr;
}

bool AuthManager::Auth(const std::string& User, const std::string& Pass) const noexcept {
	if (this->AuthInformation.empty()) return true;
	const std::string authkey = this->GenerateAuthKey(User, Pass);
	return std::find(this->AuthInformation.begin(), this->AuthInformation.end(), authkey) != this->AuthInformation.end();
}

auto AuthManager::GetUserPos(const std::string& ID) const {
	return std::find_if(this->AuthInformation.begin(), this->AuthInformation.end(), [&ID](const std::string& str) { return SplitString(str, ';')[0] == ID; });
}

bool AuthManager::UserExist(const std::string& ID) const {
	return this->GetUserPos(ID) != this->AuthInformation.end();
}

auto AuthManager::GetReservedAccessTokenPos(const std::string& Token) const noexcept {
	return std::find_if(this->AccessToken.begin(), this->AccessToken.end(),	[&Token](const AccessTokenType& a) { return a.first == Token; });
}

bool AuthManager::Auth(const std::string& ReceivedAccessToken) noexcept {
	// const_iteratorだと困るのでここでstd::find_if実行
	auto it = std::find_if(this->AccessToken.begin(), this->AccessToken.end(), [&ReceivedAccessToken](const AccessTokenType& a) { return a.first == ReceivedAccessToken; });
	const bool Flag = (it != this->AccessToken.end());
	if (Flag) it->second = GetCurrent();
	return Flag;
}

std::string AuthManager::CreateAccessToken(const std::string& ID) {
	if (!this->UserExist(ID)) throw std::runtime_error("This ID user doesn't exist");
	std::string str = this->GenerateRandomString(30) + *this->GetUserPos(ID) + this->GenerateRandomString(30);
	for (size_t i = 0; i < 35; i++) str = ToSHA512String(str);
	this->AccessToken.emplace_back(std::make_pair(str, GetCurrent()));
	return str;
}

void AuthManager::DeleteAccessToken(const std::string& TargetAccessToken) {
	if (const auto it = this->GetReservedAccessTokenPos(TargetAccessToken); it != this->AccessToken.end()) this->AccessToken.erase(it);
}

bool AuthManager::IsDefaultUser(const std::string& ID) const noexcept {
	return this->DefaultUser.empty() ? false : (string::converter::stl::to_bytes(this->DefaultUser) == ID);
}

void AuthManager::AddUser(const std::string& NewID, const std::string& NewPass, const std::string& AuthUserAccessToken) {
	if (!this->Auth(AuthUserAccessToken)) throw AuthException(401, "incorrect user id or password");
	if (this->UserExist(NewID)) throw AuthException(400, "This user exists");
	const std::string str = this->GenerateAuthKey(NewID, NewPass);
	// 初期ユーザー登録が完了後はデフォルトアカウントは削除
	const std::string DefaultUserA = string::converter::stl::to_bytes(this->DefaultUser);
	if (auto it = std::find_if(this->AuthInformation.begin(), this->AuthInformation.end(), 
		[&DefaultUserA](const std::string& str) { return DefaultUserA == SplitString(str, ';').front(); }); it != this->AuthInformation.end()) this->AuthInformation.erase(it);
	this->AuthInformation.emplace_back(str);
}

void AuthManager::DeleteExpiredAccessToken() {
	std::sort(this->AccessToken.begin(), this->AccessToken.end(), [](const AccessTokenType& a, const AccessTokenType& b) { return a.second > b.second; });
	const std::chrono::milliseconds Now = GetCurrent();
	this->AccessToken.erase(
		std::find_if(this->AccessToken.begin(), this->AccessToken.end(), [&Now, this](const AccessTokenType& token) { return (Now - token.second).count() > this->MaxTokenExpirationTime; }),
		this->AccessToken.end()
	);
}

#include <array>

#if defined(__MINGW32__) && defined(__GNUC__) && !defined(__clang__)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <system_error>
#include <limits>
#include <string>
#include <Windows.h>
#include <wincrypt.h>

namespace workaround_mingw_gcc {
	class random_device {
	private:
		class crypt_context {
		private:
			HCRYPTPROV prov_;
		public:
			crypt_context(DWORD prov_type, LPCTSTR container = nullptr, LPCTSTR provider = nullptr, DWORD flags = 0) {
				const auto success = ::CryptAcquireContext(&this->prov_, container, provider, prov_type, flags);
				if (!success) {
					throw std::system_error(
						std::error_code(::GetLastError(), std::system_category()),
						"CryptAcquireContext:(" + std::to_string(success) + ')'
					);
				}
			}
			crypt_context(const crypt_context&) = delete;
			void operator=(const crypt_context&) = delete;
			~crypt_context() noexcept {
				::CryptReleaseContext(this->prov_, 0);
			}
			//HCRYPTPROV& get() noexcept { return this->prov_; }
			const HCRYPTPROV& get() const noexcept { return this->prov_; }
		};
		crypt_context prov_;

	public:
		using result_type = unsigned int;
		explicit random_device(const std::string & /*token*/ = "workaround_mingw_gcc ")
			: prov_(PROV_RSA_FULL)
		{}
		random_device(const random_device&) = delete;
		void operator=(const random_device&) = delete;
		//~random_device() = default;
		double entropy() const noexcept { return 0.0; }
		result_type operator()() {
			result_type re;
			const auto success = ::CryptGenRandom(this->prov_.get(), sizeof(re), reinterpret_cast<BYTE*>(&re));
			if (!success) {
				throw std::system_error(
					std::error_code(::GetLastError(), std::system_category()),
					"CryptGenRandom:(" + std::to_string(success) + ')'
				);
			}
			return re;
		}
		static constexpr result_type min() { return std::numeric_limits<result_type>::min(); }
		static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }
	};
} // namespace workaround_mingw_gcc

namespace cpprefjp {
	using workaround_mingw_gcc::random_device;
}

#else //defined(__MINGW32__) && defined(__GNUC__) && !defined(__clang__)

namespace cpprefjp {
	using std::random_device;
}

#endif //defined(__MINGW32__) && defined(__GNUC__) && !defined(__clang__)

std::mt19937 InitEngine() {
	cpprefjp::random_device rand;
	std::array<std::uint_least32_t, sizeof(std::mt19937) / sizeof(std::uint_least32_t)> v;
	std::generate(v.begin(), v.end(), std::ref(rand));
	std::seed_seq seed(v.begin(), v.end());
	return std::mt19937(seed);
}

std::chrono::milliseconds GetCurrent() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());; }
