#pragma once
#include <nlohmann/json.hpp>
#include <string>

class AuthManager {
private:
	struct AuthInformation {
		std::string ID;
		std::string Password;
	};
	std::vector<AuthInformation> AuthInformations;
public:
	AuthManager(const nlohmann::json& AuthInfoJson);
	AuthManager(const AuthManager&) = delete;
	AuthManager& operator = (const AuthManager&) = delete;
	bool Auth(const std::string& id, const std::string& pass) const noexcept;
	bool Auth(const std::string& TargetAccessToken) noexcept;
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
