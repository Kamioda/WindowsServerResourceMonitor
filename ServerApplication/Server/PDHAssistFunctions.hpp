#pragma once
constexpr double ByteToKiloByte(const long long& ByteVal) { return static_cast<double>(ByteVal) / 1024.0; }
constexpr double ByteToMegaByte(const long long& ByteVal) { return ByteToKiloByte(ByteVal) / 1024.0; }
constexpr double ByteToGigaByte(const long long& ByteVal) { return ByteToMegaByte(ByteVal) / 1024.0; }
constexpr double ByteToTeraByte(const long long& ByteVal) { return ByteToGigaByte(ByteVal) / 1024.0; }
constexpr double ToPercentBase(const long long& AvailOrUsage, const long long& Total) { return static_cast<double>(AvailOrUsage) * 100.0 / static_cast<double>(Total); }
constexpr double ToPercentCheckLower(const long long& AvailOrUsage, const long long& Total) { return AvailOrUsage > 0 && static_cast<int>(ToPercentBase(AvailOrUsage, Total) * 100) == 0; }
constexpr double ToPercentCheckUpper(const long long& AvailOrUsage, const long long& Total) { return AvailOrUsage != Total && static_cast<int>(ToPercentBase(AvailOrUsage, Total) * 100) == 10000; }
constexpr double ToPercent(const long long& AvailOrUsage, const long long& Total) { return ToPercentCheckLower(AvailOrUsage, Total) ? 0.01 : (ToPercentCheckUpper(AvailOrUsage, Total) ? 99.99 : ToPercentBase(AvailOrUsage, Total)); }
constexpr double digit(const double d) { return static_cast<double>(static_cast<long long>(d * 100.0 + 0.5)) / 100.0; }
