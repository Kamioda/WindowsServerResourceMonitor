#pragma once
#include <string>
#include "json.hpp"

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
private:
	bool Auth(const std::string& id, const std::string& pass) const noexcept;
public:
	bool Auth(const std::string& TargetAccessToken) const noexcept;
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
