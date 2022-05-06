#include "Auth.hpp"
#include "Split.hpp"
#include "Base64Converter.hpp"
#include "Password.hpp"
#include <nlohmann/json.hpp>
#include <fstream>

AuthManager::AuthManager(const std::filesystem::path& AuthInfoFilePath) : AuthInformations() {
	if (std::filesystem::status(AuthInfoFilePath).type() != std::filesystem::file_type::regular)
		throw std::runtime_error("Failed to load auth data file");
	std::ifstream ifs(AuthInfoFilePath);
	nlohmann::json json = nlohmann::json::parse(std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>()));
	const auto& AuthInfo = json["authinfo"];
	if (!AuthInfo.is_array()) throw std::runtime_error("auth info file is invalid");
	for (const auto& i : AuthInfo) this->AuthInformations.emplace_back(AuthInformation{ i["id"].get<std::string>(), i["password"].get<std::string>() });
}

bool AuthManager::Auth(const std::string& User, const std::string& Pass) const noexcept {
	return std::find_if(
		this->AuthInformations.begin(), 
		this->AuthInformations.end(), 
		[&User, &Pass](const AuthInformation& a) { return a.ID == User && a.Password == HashPassword(Pass); }
	) != this->AuthInformations.end();
}

bool AuthManager::Auth(const std::string& AuthorizationHeaderValue) noexcept {
	const std::array<std::string, 2> Arr = SplitString<2>(AuthorizationHeaderValue, ' ');
	if (Arr[0] != "Basic") throw std::runtime_error("Auth type is invalid");
	const std::array<std::string, 2> IDPasswordPair = SplitString<2>(base64::decode(Arr[1]), ':');
	return this->Auth(IDPasswordPair[0], IDPasswordPair[1]);
}
