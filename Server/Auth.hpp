#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <random>
#include <utility>
#include <chrono>

class AuthManager {
private:
	struct XmlCreateInformation {
		std::wstring FilePath;
		std::wstring Root;
		std::wstring AllowUser;
		std::wstring DefaultUser;
	};
	using AccessTokenType = std::pair<std::string, std::chrono::milliseconds>;
	XmlCreateInformation xmlCreate;
	std::string DefaultUser;
	std::vector<std::string> AuthInformation;
	std::vector<AccessTokenType> AccessToken;
	mutable std::mt19937 mt;
	size_t StartAuthDataSize;
	long long MaxTokenExpirationTime;
	std::string GenerateAuthKey(const std::string& ID, const std::string& Pass) const;
	std::string GenerateRandomString(const size_t length) const;
	auto GetUserPos(const std::string& ID) const;
	auto GetReservedAccessTokenPos(const std::string& Token) const noexcept;
	bool UserExist(const std::string& ID) const;
public:
	AuthManager(const std::wstring& AuthInfoFilePath, const std::wstring& Root, const std::wstring& DefaultUserInfoPath, const std::wstring& AllowUserPath, const long long MaxExpirationTimeOfToken);
	~AuthManager();
	AuthManager(const AuthManager&) = delete;
	AuthManager(AuthManager&& a) noexcept;
	AuthManager& operator = (const AuthManager&) = delete;
	AuthManager& operator = (AuthManager&& a) noexcept;
	bool Auth(const std::string& id, const std::string& pass) const noexcept;
	bool Auth(const std::string& TargetAccessToken) noexcept;
	std::string CreateAccessToken(const std::string& InternalAccessKey);
	void DeleteAccessToken(const std::string& ReceivedAccessToken);
	void AddUser(const std::string& NewID, const std::string & NewPas, const std::string& AuthUserAccessToken);
	bool IsDefaultUser(const std::string& ID) const noexcept;
	void DeleteExpiredAccessToken();
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
