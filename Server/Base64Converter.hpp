﻿#pragma once
#include <string>

namespace base64 {
	std::string encode(const std::string& str);
	std::string decode(const std::string& str);
}
