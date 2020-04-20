#include <unicode/unistr.h>
#include <vector>
#pragma comment(lib, "icuuc.lib")

std::string ShiftJIS_To_UTF8(const std::string& val) {
    constexpr const char* ShiftJIS = "shift-jis";
    constexpr const char* UTF8 = "utf8";
    icu::UnicodeString src(val.c_str(), ShiftJIS);
    const size_t length = static_cast<size_t>(src.extract(0, src.length(), nullptr, UTF8));
    std::vector<char> result(length + 1);
    src.extract(0, src.length(), &result[0], UTF8);
    return std::move(std::string(result.begin(), result.end() - 1));
}
