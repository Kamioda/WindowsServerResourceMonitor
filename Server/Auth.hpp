#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <random>

class AuthManager {
private:
	std::wstring AuthInfoFilePath;
	std::wstring Root;
	std::vector<std::string> AuthInformation;
	mutable std::mt19937 mt;
	size_t StartAuthDataSize;
	std::string GenerateAuthKey(const std::string& ID, const std::string& Pass) const;
	std::string GenerateRandomString(const size_t length) const;
	bool UserExist(const std::string& ID) const;
public:
	AuthManager(const std::wstring& AuthInfoFilePath, const std::wstring& Root = L"authinformation/allowuser");
	AuthManager(const std::string& AuthInfoFilePath, const std::string& Root = "authinformation/allowuser");
	~AuthManager();
	AuthManager(const AuthManager&) = delete;
	AuthManager(AuthManager&& a) noexcept;
	AuthManager& operator = (const AuthManager&) = delete;
	AuthManager& operator = (AuthManager&& a) noexcept;
	bool Auth(const std::string& id, const std::string& pass) const noexcept;
	void AddUser(const std::string& NewID, const std::string & NewPas, const std::string& AuthUserID, const std::string& AuthUserPass);
};

class AuthException {
private:
	int Code;
	std::string Message;
public:
	AuthException(const int ErrorCode, const std::string& ErrorMessage)
		: Code(ErrorCode), Message(ErrorMessage) {}
	int GetErrorCode() const noexcept { return this->Code; }
	std::string GetErrorMessage() const noexcept { return this->Message; }
};
