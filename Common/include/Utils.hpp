#pragma once
#include <atomic>
#include <iomanip>
#include <openssl/sha.h>

template <typename T>
inline uint32_t GenerateAutoIncrementId()
{
	static std::atomic<uint32_t> counter{ 1 };
	return counter.fetch_add(1);
}

inline std::string Sha256(const std::string& str)
{
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256(reinterpret_cast<const unsigned char*>(str.c_str()), str.size(), hash);

	std::stringstream ss;
	for (unsigned char c : hash)
		ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
	return ss.str();
}

template<typename T, typename U>
inline U GetOrDefault(const std::unordered_map<T, U>& row,
	const T& key,
	const U& default_val)
{
	auto it = row.find(key);
	return it != row.end() ? it->second : default_val;
}

inline std::string GetOrDefault(const std::unordered_map<std::string, std::string>& row,
	const std::string& key,
	const std::string& default_val = "")
{
	auto it = row.find(key);
	return it != row.end() ? it->second : default_val;
}