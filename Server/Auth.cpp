#include "Auth.hpp"
#include "Split.hpp"
#include "../Common/CommandLineManager.hpp"
#include "MSXMLRead.hpp"
#include "MSXMLWrite.hpp"
#include "SHA512.hpp"
#include <Shlwapi.h>
#include <algorithm>
#include <functional>
#pragma comment(lib, "shlwapi.lib")

std::mt19937 InitEngine();

AuthManager::AuthManager(const std::string& AuthInfoFilePath, const std::string& Root)
	: AuthManager(CommandLineManagerW::AlignCmdLineStrType(AuthInfoFilePath), CommandLineManagerW::AlignCmdLineStrType(Root)) {}

AuthManager::AuthManager(const std::wstring& AuthInfoFilePath, const std::wstring& Root) 
	: AuthInfoFilePath(AuthInfoFilePath), Root(Root), AuthInformation(), mt(InitEngine()), StartAuthDataSize() {
	if (FALSE == PathFileExistsW(this->AuthInfoFilePath.c_str())) return;
	MSXMLRead xml = MSXMLRead(AuthInfoFilePath);
	xml.Load(Root);
	for (long i = 0; i < xml.at(0).Length; i++) this->AuthInformation.emplace_back(CommandLineManagerA::AlignCmdLineStrType(xml.at(0)[i]));
	this->StartAuthDataSize = this->AuthInformation.size();
}

AuthManager::~AuthManager() {
	if (this->AuthInfoFilePath.empty() || this->AuthInformation.size() == this->StartAuthDataSize) return;
	if (FALSE == PathFileExistsW(this->AuthInfoFilePath.c_str())) DeleteFileW(this->AuthInfoFilePath.c_str());
	const auto rootinfo = SplitString(this->Root, L'/');
	MSXMLWrite writer(rootinfo.at(0));
	for (const auto& i : this->AuthInformation) writer.AddToRootElement(writer.GenerateElement(rootinfo.at(1), CommandLineManagerW::AlignCmdLineStrType(i)));
	writer.Output(this->AuthInfoFilePath);
}

AuthManager::AuthManager(AuthManager&& a) noexcept
	: AuthInfoFilePath(a.AuthInfoFilePath), AuthInformation(std::move(a.AuthInformation)),
	mt(std::move(a.mt)), StartAuthDataSize(a.StartAuthDataSize) { a.AuthInfoFilePath.clear(); }

AuthManager& AuthManager::operator = (AuthManager&& a) noexcept {
	this->AuthInfoFilePath = a.AuthInfoFilePath;
	this->AuthInformation = std::move(a.AuthInformation);
	this->mt = std::move(a.mt);
	this->StartAuthDataSize = a.StartAuthDataSize;
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

bool AuthManager::UserExist(const std::string& ID) const {
	return std::find_if(this->AuthInformation.begin(), this->AuthInformation.end(), [&ID](const std::string& str) { return SplitString(str, ';')[0] == ID; }) != this->AuthInformation.end();
}

void AuthManager::AddUser(const std::string& NewID, const std::string& NewPass, const std::string& AuthUserID, const std::string& AuthUserPass) {
	if (!this->Auth(AuthUserID, AuthUserPass)) throw AuthException(401, "incorrect user id or password");
	if (this->UserExist(NewID)) throw AuthException(400, "This user exists");
	const std::string str = this->GenerateAuthKey(NewID, NewPass);
	// 初期ユーザー登録が完了後はデフォルトアカウントは削除
	if (auto it = std::find(this->AuthInformation.begin(), this->AuthInformation.end(), "winserveradmin"); it != this->AuthInformation.end()) this->AuthInformation.erase(it);
	this->AuthInformation.emplace_back(str);
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

